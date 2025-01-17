/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include "Document.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/ControlProperties/RootProperty.h"
#include "Model/YamlPackageSerializer.h"

#include "Ui/QtModelPackageCommandExecutor.h"
#include "EditorCore.h"

using namespace DAVA;
using namespace std;
using namespace placeholders;

Document::Document(const RefPtr<PackageNode>& package_, QObject* parent)
    : QObject(parent)
    , package(package_)
    , commandExecutor(new QtModelPackageCommandExecutor(this))
    , undoStack(new QUndoStack(this))
    , fileSystemWatcher(new QFileSystemWatcher(this))
{
    QString path = GetPackageAbsolutePath();
    DVASSERT(QFile::exists(path));
    if (!fileSystemWatcher->addPath(path))
    {
        DAVA::Logger::Error("can not add path to the file watcher: %s", path.toUtf8().data());
    }
    connect(GetEditorFontSystem(), &EditorFontSystem::UpdateFontPreset, this, &Document::RefreshAllControlProperties);
    connect(fileSystemWatcher, &QFileSystemWatcher::fileChanged, this, &Document::OnFileChanged, Qt::DirectConnection);
    connect(undoStack.get(), &QUndoStack::cleanChanged, this, &Document::OnCleanChanged);
}

Document::~Document()
{
    disconnect(undoStack.get(), &QUndoStack::cleanChanged, this, &Document::OnCleanChanged); //destructor of UndoStack send signal here
    for (auto& context : contexts)
    {
        delete context.second;
    }
}

const FilePath& Document::GetPackageFilePath() const
{
    return package->GetPath();
}

QString Document::GetPackageAbsolutePath() const
{
    return QString::fromStdString(GetPackageFilePath().GetAbsolutePathname());
}

QUndoStack* Document::GetUndoStack() const
{
    return undoStack.get();
}

PackageNode* Document::GetPackage() const
{
    return package.Get();
}

QtModelPackageCommandExecutor* Document::GetCommandExecutor() const
{
    return commandExecutor.get();
}

WidgetContext* Document::GetContext(void* requester) const
{
    auto iter = contexts.find(requester);
    if (iter != contexts.end())
    {
        return iter->second;
    }
    return nullptr;
}

void Document::Save()
{
    QString path = GetPackageAbsolutePath();
    fileSystemWatcher->removePath(path);
    YamlPackageSerializer serializer;
    serializer.SerializePackage(package.Get());
    serializer.WriteToFile(package->GetPath());
    undoStack->setClean();
    if (!fileSystemWatcher->addPath(path))
    {
        DAVA::Logger::Error("can not add path to the file watcher: %s", path.toUtf8().data());
    }
}

void Document::SetContext(void* requester, WidgetContext* widgetContext)
{
    auto iter = contexts.find(requester);
    if (iter != contexts.end())
    {
        DVASSERT_MSG(false, "document already have this context");
        delete iter->second;
        contexts.erase(iter);
    }
    contexts.emplace(requester, widgetContext);
}

void Document::RefreshLayout()
{
    package->RefreshPackageStylesAndLayout(true);
}

bool Document::CanSave() const
{
    return canSave;
}

bool Document::IsDocumentExists() const
{
    return fileExists;
}

void Document::RefreshAllControlProperties()
{
    package->GetPackageControlsNode()->RefreshControlProperties();
}

void Document::SetCanSave(bool arg)
{
    if (arg != canSave)
    {
        canSave = arg;
        CanSaveChanged(arg);
    }
}

void Document::OnFileChanged(const QString& path)
{
    DVASSERT(path == GetPackageAbsolutePath());
    fileExists = QFile::exists(GetPackageAbsolutePath());
    SetCanSave(!fileExists || !undoStack->isClean());
    emit FileChanged(this);
}

void Document::OnCleanChanged(bool clean)
{
    SetCanSave(fileExists && !clean);
}

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


#include "QtUtils.h"
#include "Deprecated/SceneValidator.h"

#include <QMessageBox>
#include <QToolButton>
#include <QFileInfo>

#include "mainwindow.h"

#include "CommandLine/CommandLineParser.h"
#include "Classes/CommandLine/TextureDescriptor/TextureDescriptorUtils.h"

#include "QtTools/FileDialog/FileDialog.h"
#include "QtTools/ConsoleWidget/PointerSerializer.h"

#include "DAVAEngine.h"
#include <QProcess>

using namespace DAVA;

FilePath PathnameToDAVAStyle(const QString& convertedPathname)
{
    return FilePath(convertedPathname.toStdString());
}

FilePath GetOpenFileName(const String& title, const FilePath& pathname, const String& filter)
{
    QString filePath = FileDialog::getOpenFileName(nullptr, QString(title.c_str()), QString(pathname.GetAbsolutePathname().c_str()),
                                                   QString(filter.c_str()));

    FilePath openedPathname = PathnameToDAVAStyle(filePath);
    if (!openedPathname.IsEmpty() && !SceneValidator::Instance()->IsPathCorrectForProject(openedPathname))
    {
        //Need to Show Error
        ShowErrorDialog(String(Format("File(%s) was selected from incorect project.", openedPathname.GetAbsolutePathname().c_str())));
        openedPathname = FilePath();
    }

    return openedPathname;
}

String SizeInBytesToString(float32 size)
{
    String retString = "";

    if (1000000 < size)
    {
        retString = Format("%0.2f MB", size / (1024 * 1024));
    }
    else if (1000 < size)
    {
        retString = Format("%0.2f KB", size / 1024);
    }
    else
    {
        retString = Format("%d B", (int32)size);
    }

    return retString;
}

WideString SizeInBytesToWideString(float32 size)
{
    return StringToWString(SizeInBytesToString(size));
}

Image* CreateTopLevelImage(const FilePath& imagePathname)
{
    Image* image = NULL;
    Vector<Image*> imageSet;
    ImageSystem::Instance()->Load(imagePathname, imageSet);
    if (0 != imageSet.size())
    {
        image = SafeRetain(imageSet[0]);
        for_each(imageSet.begin(), imageSet.end(), SafeRelease<Image>);
    }

    return image;
}

void ShowErrorDialog(const Set<String>& errors, const String& title /* = "" */)
{
    if (errors.empty())
        return;

    const uint32 maxErrorsPerDialog = 6;
    uint32 totalErrors = errors.size();

    const String dialogTitle = title + Format(" %u error(s) occured.", totalErrors);
    const String errorDivideLine("\n--------------------\n");

    String errorMessage;
    uint32 errorCounter = 0;
    for (const auto& message : errors)
    {
        errorMessage += PointerSerializer::CleanUpString(message) + errorDivideLine;
        errorCounter++;

        if (errorCounter == maxErrorsPerDialog)
        {
            errorMessage += "\n\nSee console log for details.";
            ShowErrorDialog(errorMessage, dialogTitle);
            errorMessage.clear();
            break;
        }
    }

    if (!errorMessage.empty())
        ShowErrorDialog(errorMessage, dialogTitle);
}

void ShowErrorDialog(const String& errorMessage, const String& title)
{
    bool forceClose = CommandLineParser::CommandIsFound(String("-force")) ||
    CommandLineParser::CommandIsFound(String("-forceclose"));

    if (!forceClose && !Core::Instance()->IsConsoleMode())
    {
        QMessageBox::critical(QApplication::activeWindow(), title.c_str(), errorMessage.c_str());
    }
}

bool IsKeyModificatorPressed(Key key)
{
    return InputSystem::Instance()->GetKeyboard().IsKeyPressed(key);
}

bool IsKeyModificatorsPressed()
{
    return (IsKeyModificatorPressed(Key::LSHIFT) || IsKeyModificatorPressed(Key::LCTRL) || IsKeyModificatorPressed(Key::LALT));
}

QColor ColorToQColor(const Color& color)
{
    float32 maxC = 1.0;

    if (maxC < color.r)
        maxC = color.r;
    if (maxC < color.g)
        maxC = color.g;
    if (maxC < color.b)
        maxC = color.b;

    return QColor::fromRgbF(color.r / maxC, color.g / maxC, color.b / maxC, Clamp(color.a, 0.0f, 1.0f));
}

Color QColorToColor(const QColor& qcolor)
{
    return Color(qcolor.redF(), qcolor.greenF(), qcolor.blueF(), qcolor.alphaF());
}

int ShowQuestion(const String& header, const String& question, int buttons, int defaultButton)
{
    int answer = QMessageBox::question(NULL, QString::fromStdString(header), QString::fromStdString(question),
                                       (QMessageBox::StandardButton)buttons, (QMessageBox::StandardButton)defaultButton);

    return answer;
}

void ShowActionWithText(QToolBar* toolbar, QAction* action, bool showText)
{
    if (NULL != toolbar && NULL != action)
    {
        QToolButton* toolBnt = dynamic_cast<QToolButton*>(toolbar->widgetForAction(action));
        if (NULL != toolBnt)
        {
            toolBnt->setToolButtonStyle(showText ? Qt::ToolButtonTextBesideIcon : Qt::ToolButtonIconOnly);
        }
    }
}

String ReplaceInString(const String& sourceString, const String& what, const String& on)
{
    String::size_type pos = sourceString.find(what);
    if (pos != String::npos)
    {
        String newString = sourceString;
        newString = newString.replace(pos, what.length(), on);
        return newString;
    }

    return sourceString;
}

void ShowFileInExplorer(const QString& path)
{
    const QFileInfo fileInfo(path);

#if defined(Q_OS_MAC)
    QStringList args;
    args << "-e";
    args << "tell application \"Finder\"";
    args << "-e";
    args << "activate";
    args << "-e";
    args << "select POSIX file \"" + fileInfo.absoluteFilePath() + "\"";
    args << "-e";
    args << "end tell";
    QProcess::startDetached("osascript", args);
#elif defined(Q_OS_WIN)
    QStringList args;
    args << "/select," << QDir::toNativeSeparators(fileInfo.absoluteFilePath());
    QProcess::startDetached("explorer", args);
#endif //
}

void SaveSpriteToFile(Sprite* sprite, const FilePath& path)
{
    if (sprite)
    {
        SaveTextureToFile(sprite->GetTexture(), path);
    }
}

void SaveTextureToFile(Texture* texture, const FilePath& path)
{
    if (texture)
    {
        Image* img = texture->CreateImageFromMemory();
        SaveImageToFile(img, path);
        img->Release();
    }
}

void SaveImageToFile(Image* image, const FilePath& path)
{
    ImageSystem::Instance()->Save(path, image);
}

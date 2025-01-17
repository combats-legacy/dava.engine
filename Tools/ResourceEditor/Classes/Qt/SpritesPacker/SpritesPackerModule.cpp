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

#include "SpritesPacker/SpritesPackerModule.h"

#include "Functional/Function.h"

#include "AssetCache/AssetCacheClient.h"
#include "QtTools/ReloadSprites/DialogReloadSprites.h"
#include "QtTools/ReloadSprites/SpritesPacker.h"

#include "Main/mainwindow.h"
#include "Project/ProjectManager.h"

#include <QAction>
#include <QDir>

SpritesPackerModule::SpritesPackerModule()
    : QObject(nullptr)
    , spritesPacker(new SpritesPacker())
{
    qRegisterMetaType<DAVA::eGPUFamily>("DAVA::eGPUFamily");
    qRegisterMetaType<DAVA::TextureConverter::eConvertQuality>("DAVA::TextureConverter::eConvertQuality");
}

SpritesPackerModule::~SpritesPackerModule()
{
    if (cacheClient != nullptr)
    {
        DisconnectCacheClient();
    }
}

QAction* SpritesPackerModule::GetReloadAction() const
{
    return reloadSpritesAction;
}

void SpritesPackerModule::SetAction(QAction* reloadSpritesAction_)
{
    if (reloadSpritesAction != nullptr)
    {
        disconnect(reloadSpritesAction, &QAction::triggered, this, &SpritesPackerModule::RepackWithDialog);
    }

    reloadSpritesAction = reloadSpritesAction_;

    if (reloadSpritesAction != nullptr)
    {
        connect(reloadSpritesAction, &QAction::triggered, this, &SpritesPackerModule::RepackWithDialog);
    }
}

void SpritesPackerModule::RepackWithDialog()
{
    SetupSpritesPacker(ProjectManager::Instance()->GetProjectPath());

    DAVA::JobManager::Instance()->CreateWorkerJob(DAVA::MakeFunction(this, &SpritesPackerModule::ConnectCacheClient));

    ShowPackerDialog();

    DisconnectCacheClient();

    ReloadObjects();
}

void SpritesPackerModule::RepackImmediately(const DAVA::FilePath& projectPath, DAVA::eGPUFamily gpu)
{
    SetupSpritesPacker(projectPath);

    DAVA::Function<void()> fn = DAVA::Bind(&SpritesPackerModule::ProcessSilentPacking, this, true, false, gpu, DAVA::TextureConverter::ECQ_DEFAULT);
    DAVA::JobManager::Instance()->CreateWorkerJob(fn);

    CreateWaitDialog(projectPath);
}

void SpritesPackerModule::SetupSpritesPacker(const DAVA::FilePath& projectPath)
{
    DAVA::FilePath inputDir = projectPath + "/DataSource/Gfx/Particles";
    DAVA::FilePath outputDir = projectPath + "/Data/Gfx/Particles";

    spritesPacker->ClearTasks();
    spritesPacker->AddTask(QString::fromStdString(inputDir.GetAbsolutePathname()), QString::fromStdString(outputDir.GetAbsolutePathname()));
}

void SpritesPackerModule::ProcessSilentPacking(bool clearDirs, bool forceRepack, const DAVA::eGPUFamily gpu, const DAVA::TextureConverter::eConvertQuality quality)
{
    ConnectCacheClient();
    spritesPacker->ReloadSprites(clearDirs, forceRepack, gpu, quality);
    DisconnectCacheClient();

    DAVA::JobManager::Instance()->CreateMainJob(DAVA::MakeFunction(this, &SpritesPackerModule::CloseWaitDialog));
    DAVA::JobManager::Instance()->CreateMainJob(DAVA::MakeFunction(this, &SpritesPackerModule::ReloadObjects));
}

void SpritesPackerModule::ShowPackerDialog()
{
    DialogReloadSprites dialogReloadSprites(spritesPacker.get(), QtMainWindow::Instance());
    dialogReloadSprites.exec();
}

void SpritesPackerModule::CreateWaitDialog(const DAVA::FilePath& projectPath)
{
    DVASSERT(waitDialog == nullptr);

    waitDialog = new QDialog(QtMainWindow::Instance(), Qt::Dialog | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
    QLabel* label = new QLabel("Reloading Particles for " + QString::fromStdString(projectPath.GetAbsolutePathname()), waitDialog);
    label->setAlignment(Qt::AlignCenter);
    label->setWordWrap(true);

    QVBoxLayout* layout = new QVBoxLayout(waitDialog);
    layout->addWidget(label);
    waitDialog->setLayout(layout);

    waitDialog->setFixedSize(300, 100);
    waitDialog->show();
    waitDialog->raise();
    waitDialog->activateWindow();
}

void SpritesPackerModule::CloseWaitDialog()
{
    if (waitDialog != nullptr)
    {
        waitDialog->close();
        delete waitDialog;
        waitDialog = nullptr;
    }
}

void SpritesPackerModule::ReloadObjects()
{
    DAVA::Sprite::ReloadSprites();

    DAVA::uint32 gpu = spritesPacker->GetResourcePacker().requestedGPUFamily;
    SettingsManager::SetValue(Settings::Internal_SpriteViewGPU, DAVA::VariantType(gpu));

    emit SpritesReloaded();
}

void SpritesPackerModule::ConnectCacheClient()
{
    DVASSERT(cacheClient == nullptr);
    if (SettingsManager::GetValue(Settings::General_AssetCache_UseCache).AsBool())
    {
        DAVA::String ipStr = SettingsManager::GetValue(Settings::General_AssetCache_Ip).AsString();
        DAVA::uint16 port = static_cast<DAVA::uint16>(SettingsManager::GetValue(Settings::General_AssetCache_Port).AsUInt32());
        DAVA::uint64 timeoutSec = SettingsManager::GetValue(Settings::General_AssetCache_Timeout).AsUInt32();

        DAVA::AssetCacheClient::ConnectionParams params;
        params.ip = (ipStr.empty() ? DAVA::AssetCache::LOCALHOST : ipStr);
        params.port = port;
        params.timeoutms = timeoutSec * 1000; //in ms

        cacheClient = new DAVA::AssetCacheClient(false);
        DAVA::AssetCache::Error connected = cacheClient->ConnectSynchronously(params);
        if (connected != DAVA::AssetCache::Error::NO_ERRORS)
        {
            DisconnectCacheClient();
        }
    }

    spritesPacker->SetCacheClient(cacheClient, "ResourceEditor.ReloadParticles");
}

void SpritesPackerModule::DisconnectCacheClient()
{
    if (cacheClient != nullptr)
    {
        DAVA::AssetCacheClient* disconnectingClient = cacheClient;
        cacheClient = nullptr;

        //we should destroy cache client on main thread
        DAVA::JobManager::Instance()->CreateMainJob(DAVA::Bind(&SpritesPackerModule::DisconnectCacheClientInternal, this, disconnectingClient));
    }
}

void SpritesPackerModule::DisconnectCacheClientInternal(DAVA::AssetCacheClient* cacheClientForDisconnect)
{
    DVASSERT(cacheClientForDisconnect != nullptr);

    cacheClientForDisconnect->Disconnect();
    SafeDelete(cacheClientForDisconnect);
}

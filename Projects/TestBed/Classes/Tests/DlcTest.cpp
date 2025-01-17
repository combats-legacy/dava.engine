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


#include "DlcTest.h"
#include "Base/GlobalEnum.h"
#include "Database/MongodbClient.h"
#include "Database/MongodbObject.h"
#include "Notification/LocalNotificationText.h"
#include "Notification/LocalNotificationProgress.h"
#include "Utils/StringUtils.h"

#if defined(__DAVAENGINE_ANDROID__)
#include "Platform/DeviceInfo.h"
#include "Platform/TemplateAndroid/CorePlatformAndroid.h"
#endif

//#define DLC_TEST
#define DLC_TEST_MAX_RETRY_COUNT 10
#define DLC_MONGO_HOST String("by2-badava-mac-11.corp.wargaming.local")
#define DLC_MONGO_PORT 27017
#define DLC_MONGO_TEST_DB "dlc"
#define DLC_MONGO_TEST_COLLECTION "test.exit"

#if defined(__DAVAENGINE_IPHONE__)
const DAVA::String localServerUrl = "http://by1-builddlc-01/DLC_Blitz";
#else
const DAVA::String localServerUrl = "http://by1-builddlc-01.corp.wargaming.local/DLC_Blitz";
#endif

const DAVA::String cdnServerUrl = "http://dl-wotblitz.wargaming.net/dlc/";

DlcTest::DlcTest()
    : BaseScreen("DlcTest")
    , currentDownloadUrl(localServerUrl)
    , dlc(nullptr)
{
}

void DlcTest::LoadResources()
{
    BaseScreen::LoadResources();

    Font* font = FTFont::Create("~res:/Fonts/korinna.ttf");
    Font* fontSmall = FTFont::Create("~res:/Fonts/korinna.ttf");
    DVASSERT(font);

    font->SetSize(24.0f);
    fontSmall->SetSize(14.0f);

    angle = 0;
    lastUpdateTime = 0;

#ifdef __DAVAENGINE_ANDROID__
    List<DeviceInfo::StorageInfo> storageList = DeviceInfo::GetStoragesList();
    DeviceInfo::StorageInfo external;
    bool externalStorage(false);
    for (const auto& it : storageList)
    {
        if (DeviceInfo::STORAGE_TYPE_PRIMARY_EXTERNAL == it.type)
        {
            external = it;
            DVASSERT(false == externalStorage);
            externalStorage = true;
        }
    }
    workingDir = external.path + "DLC/";
    sourceDir = external.path + "DLC_Resources/";
    destinationDir = external.path + "DLC_Resources/";
#else
    workingDir = "~doc:/DLC/";
    sourceDir = "~doc:/Resources/";
    destinationDir = "~doc:/Resources/";
#endif

    infoText = new UIStaticText(Rect(10.0f, 10.0f, 500.f, 190.f));
    infoText->SetTextColor(Color::White);
    infoText->SetFont(fontSmall);
    infoText->SetMultiline(true);
    infoText->SetTextAlign(ALIGN_LEFT | ALIGN_TOP);
    AddControl(infoText);

    UIStaticText* ver = new UIStaticText(Rect(10.0f, 100.0f, 235.f, 40.f));
    ver->SetTextColor(Color::White);
    ver->SetFont(font);
    ver->SetMultiline(false);
    ver->SetTextAlign(ALIGN_LEFT);
    ver->SetText(L"Game Veision :");
    AddControl(ver);
    SafeRelease(ver);

    gameVersionIn = new UITextField(Rect(255.0f, 100.f, 235.f, 40.f));
    gameVersionIn->SetDebugDraw(true);
    gameVersionIn->SetText(L"dlcdevtest");
    AddControl(gameVersionIn);

    UIButton* setDlInternalServerButton = new UIButton(Rect(10.0f, 150.f, 235.f, 40.f));
    setDlInternalServerButton->SetStateFont(0xFF, font);
    setDlInternalServerButton->SetStateFontColor(0xFF, Color::White);
    setDlInternalServerButton->SetStateText(0xFF, L"Set internal server");
    setDlInternalServerButton->SetDebugDraw(true);
    setDlInternalServerButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &DlcTest::SetInternalDlServer));
    AddControl(setDlInternalServerButton);
    SafeRelease(setDlInternalServerButton);

    UIButton* setDlexternalServerButton = new UIButton(Rect(255.0f, 150.f, 235.f, 40.f));
    setDlexternalServerButton->SetStateFont(0xFF, font);
    setDlexternalServerButton->SetStateFontColor(0xFF, Color::White);
    setDlexternalServerButton->SetStateText(0xFF, L"Set external server");
    setDlexternalServerButton->SetDebugDraw(true);
    setDlexternalServerButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &DlcTest::SetExternalDlServer));
    AddControl(setDlexternalServerButton);
    SafeRelease(setDlexternalServerButton);

    UIButton* incDlThreadsButton = new UIButton(Rect(10.0f, 200.f, 235.f, 40.f));
    incDlThreadsButton->SetStateFont(0xFF, font);
    incDlThreadsButton->SetStateFontColor(0xFF, Color::White);
    incDlThreadsButton->SetStateText(0xFF, L"+1 dl thread");
    incDlThreadsButton->SetDebugDraw(true);
    incDlThreadsButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &DlcTest::IncDlThreads));
    AddControl(incDlThreadsButton);
    SafeRelease(incDlThreadsButton);

    UIButton* decDlThreadsButton = new UIButton(Rect(255.0f, 200.f, 235.f, 40.f));
    decDlThreadsButton->SetStateFont(0xFF, font);
    decDlThreadsButton->SetStateFontColor(0xFF, Color::White);
    decDlThreadsButton->SetStateText(0xFF, L"-1 dl thread");
    decDlThreadsButton->SetDebugDraw(true);
    decDlThreadsButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &DlcTest::DecDlThreads));
    AddControl(decDlThreadsButton);
    SafeRelease(decDlThreadsButton);

    staticText = new UIStaticText(Rect(10.0f, 250.f, 400.f, 50.f));
    staticText->SetFont(font);
    staticText->SetTextColor(Color::White);
    staticText->SetDebugDraw(true);
    staticText->SetText(L"Press Start ...");
    AddControl(staticText);

    progressControl = new UIControl(Rect(10.0f, 310.0f, 400.0f, 5.0f));
    progressControl->SetDebugDraw(true);
    AddControl(progressControl);

    animControl = new UIControl(Rect(470.0f, 285.0f, 50.f, 50.f));
    animControl->SetDebugDraw(true);
    animControl->SetPivotPoint(Vector2(25.0f, 25.0f));
    AddControl(animControl);

    UIButton* startButton = new UIButton(Rect(10.0f, 350.f, 235.f, 50.f));
    startButton->SetStateFont(0xFF, font);
    startButton->SetStateFontColor(0xFF, Color::White);
    startButton->SetStateText(0xFF, L"Start download");
    startButton->SetDebugDraw(true);
    startButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &DlcTest::Start));
    AddControl(startButton);
    SafeRelease(startButton);

    UIButton* cancelButton = new UIButton(Rect(255.0f, 350.f, 235.f, 50.f));
    cancelButton->SetStateFont(0xFF, font);
    cancelButton->SetStateFontColor(0xFF, Color::White);
    cancelButton->SetStateText(0xFF, L"Cancel download");
    cancelButton->SetDebugDraw(true);
    cancelButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &DlcTest::Cancel));
    AddControl(cancelButton);
    SafeRelease(cancelButton);

    UIButton* restartButton = new UIButton(Rect(10.0f, 410.f, 480.f, 50.f));
    restartButton->SetStateFont(0xFF, font);
    restartButton->SetStateFontColor(0xFF, Color::White);
    restartButton->SetStateText(0xFF, L"Restart DLC");
    restartButton->SetDebugDraw(true);
    restartButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &DlcTest::Restart));
    AddControl(restartButton);
    SafeRelease(restartButton);

    DAVA::FilePath::AddResourcesFolder(destinationDir);

#ifdef DLC_TEST
    crashTest.Init(workingDir, destinationDir);
#endif

    DAVA::FileSystem::Instance()->CreateDirectory(workingDir);
    DAVA::FileSystem::Instance()->CreateDirectory(destinationDir);
    dlc = new DLC(currentDownloadUrl, sourceDir, destinationDir, workingDir, gameVersion, destinationDir + "/version/resources.txt");

    lastDLCState = dlc->GetState();

    SafeRelease(font);
    SafeRelease(fontSmall);
}

void DlcTest::UpdateInfoStr()
{
    infoStr = L"DLCWorkingDir: ";
    infoStr += StringToWString(workingDir.GetAbsolutePathname());
    infoStr += L"\nResourcesDir: ";
    infoStr += StringToWString(destinationDir.GetAbsolutePathname());
    infoStr += L"\nURL: ";
    infoStr += StringToWString(currentDownloadUrl);
    infoStr += L"\nDownloading threads count: ";
    infoStr += StringToWString(Format("%d", downloadTreadsCount));
    if (nullptr != infoText)
    {
        infoText->SetText(infoStr);
    }
}

void DlcTest::UnloadResources()
{
    BaseScreen::UnloadResources();

    SafeRelease(gameVersionIn);
    SafeRelease(infoText);
    SafeRelease(staticText);
    SafeRelease(animControl);
    SafeDelete(dlc);
}

void DlcTest::OnActive()
{
}

void DlcTest::Update(float32 timeElapsed)
{
    BaseScreen::Update(timeElapsed);

    lastUpdateTime += timeElapsed;
    if (lastUpdateTime > 0.05f)
    {
        UpdateInfoStr();
        if (nullptr != gameVersionIn)
        {
            gameVersion = WStringToString(gameVersionIn->GetText());
        }

        // update animation
        angle += 0.10f;
        lastUpdateTime = 0;

        animControl->SetAngle(angle);

        // update progress control
        Rect r = staticText->GetRect();

        if (nullptr == dlc)
        {
            return;
        }

        if (r.dx > 0)
        {
            float32 w = 0;
            switch (lastDLCState)
            {
            case DLC::DS_READY:
            case DLC::DS_DOWNLOADING:
            case DLC::DS_PATCHING:
            {
                uint64 cur = 0;
                uint64 total = 0;

                dlc->GetProgress(cur, total);
                if (total > 0)
                {
                    w = cur * r.dx / total;
                    if (0 == w && r.dx > 0)
                    {
                        w = 1;
                    }
                }
                else
                {
                    w = r.dx;
                }
            }
            break;
            default:
                w = r.dx;
                break;
            }

            Rect pr = progressControl->GetRect();
            progressControl->SetRect(Rect(pr.x, pr.y, w, pr.dy));
        }
    }

    uint32 dlcState = dlc->GetState();
    if (lastDLCState != dlcState)
    {
        lastDLCState = dlcState;

        switch (lastDLCState)
        {
        case DLC::DS_CHECKING_INFO:
            staticText->SetText(L"Checking version...");
            break;
        case DLC::DS_CHECKING_PATCH:
            staticText->SetText(L"Checking patch...");
            break;
        case DLC::DS_CHECKING_META:
            staticText->SetText(L"Checking meta information...");
            break;
        case DLC::DS_READY:
            staticText->SetText(L"Ready for download.");
            break;
        case DLC::DS_DOWNLOADING:
            staticText->SetText(L"Downloading...");
            break;
        case DLC::DS_PATCHING:
            staticText->SetText(L"Patching...");
            break;
        case DLC::DS_CANCELLING:
            staticText->SetText(L"Canceling...");
            break;
        case DLC::DS_DONE:
            if (dlc->GetError() == DLC::DE_NO_ERROR)
            {
                staticText->SetText(L"Done!");
            }
            else
            {
                DAVA::String errorText = DAVA::Format("Error %s!", GlobalEnumMap<DAVA::DLC::DLCError>::Instance()->ToString(dlc->GetError()));
                DAVA::WideString wErrorText;
                DAVA::UTF8Utils::EncodeToWideString((DAVA::uint8*)errorText.c_str(), errorText.size(), wErrorText);
                staticText->SetText(wErrorText);
            }
            break;
        default:
            break;
        }
    }

#ifdef DLC_TEST
    crashTest.Update(timeElapsed, dlc);
#endif
}

void DlcTest::Draw(const UIGeometricData& geometricData)
{
}

void DlcTest::SetExternalDlServer(BaseObject* obj, void* data, void* callerData)
{
    currentDownloadUrl = cdnServerUrl;
}

void DlcTest::SetInternalDlServer(BaseObject* obj, void* data, void* callerData)
{
    currentDownloadUrl = localServerUrl;
}

void DlcTest::IncDlThreads(BaseObject* obj, void* data, void* callerData)
{
    DownloadManager::Instance()->SetPreferredDownloadThreadsCount(++downloadTreadsCount);
}

void DlcTest::DecDlThreads(BaseObject* obj, void* data, void* callerData)
{
    --downloadTreadsCount;
    if (0 == downloadTreadsCount)
        downloadTreadsCount = 1;

    DownloadManager::Instance()->SetPreferredDownloadThreadsCount(downloadTreadsCount);
}

void DlcTest::Start(BaseObject* obj, void* data, void* callerData)
{
    staticText->SetText(L"Starting DLC...");

    dlc->Start();
}

void DlcTest::Cancel(BaseObject* obj, void* data, void* callerData)
{
    staticText->SetText(L"Cancelling DLC...");

    dlc->Cancel();
}

void DlcTest::Restart(BaseObject* obj, void* data, void* callerData)
{
    volatile static bool isRestarting = false;
    if (!isRestarting)
    {
        staticText->SetText(L"Restarting DLC...");

        isRestarting = true;
        dlc->Cancel();

        FileSystem::Instance()->DeleteDirectory(workingDir);
        FileSystem::Instance()->DeleteDirectory(sourceDir);
        FileSystem::Instance()->DeleteDirectory(destinationDir);

        SafeDelete(dlc);

        DAVA::FileSystem::Instance()->CreateDirectory(workingDir);
        DAVA::FileSystem::Instance()->CreateDirectory(destinationDir);
        dlc = new DLC(currentDownloadUrl, sourceDir, destinationDir, workingDir, gameVersion, destinationDir + "/version/resources.txt");

        lastDLCState = dlc->GetState();

        dlc->Start();
        isRestarting = false;
    }
}

void DLCCrashTest::Init(const DAVA::FilePath& workingDir, const DAVA::FilePath& destinationDir)
{
    forceExit = false;
    inExitMode = true;

    DAVA::Random::Instance()->Seed();
    cancelTimeout = DAVA::Random::Instance()->Rand(25);
    exitTimeout = DAVA::Random::Instance()->Rand(25);
    retryCount = DLC_TEST_MAX_RETRY_COUNT;

    testingFileFlag = workingDir + "run.test";
    if (!testingFileFlag.Exists())
    {
        DAVA::File* f = DAVA::File::Create(testingFileFlag, DAVA::File::CREATE | DAVA::File::WRITE);
        f->Write(&retryCount);

        DAVA::FileSystem::Instance()->DeleteDirectoryFiles(destinationDir, true);
        exitThread = Thread::Create(Message(this, &DLCCrashTest::ExitThread));
        exitThread->Start();

        DAVA::MongodbClient* dbClient = DAVA::MongodbClient::Create(DLC_MONGO_HOST, DLC_MONGO_PORT);
        if (dbClient->IsConnected())
        {
            dbClient->SetDatabaseName(DLC_MONGO_TEST_DB);
            dbClient->SetCollectionName(DLC_MONGO_TEST_COLLECTION);

            DAVA::MongodbObject* dbObject = new DAVA::MongodbObject();
            dbObject->SetUniqueObjectName();
            dbObject->AddString("state", "started");
            dbObject->AddInt32("retries", retryCount - DLC_TEST_MAX_RETRY_COUNT);
            dbObject->AddInt32("error", 0);
            dbObject->Finish();

            dbObjectId = dbObject->GetObjectName();
            f->WriteString(dbObjectId);

            dbClient->SaveDBObject(dbObject);
            SafeRelease(dbObject);
        }

        f->Release();
    }
    else
    {
        DAVA::File* f = DAVA::File::Create(testingFileFlag, DAVA::File::OPEN | DAVA::File::READ);
        if (nullptr != f && f->Read(&retryCount) > 0)
        {
            f->ReadString(dbObjectId);
        }
        else
        {
            retryCount = 0;
        }

        f->Release();
        inExitMode = false;
    }
}

void DLCCrashTest::Update(float32 timeElapsed, DLC* dlc)
{
    static float32 runtimeElapsed = 0;
    runtimeElapsed += timeElapsed;

    if (inExitMode)
    {
        if (runtimeElapsed > cancelTimeout)
        {
            dlc->Cancel();
        }

        if (forceExit)
        {
            exit(1);
        }
    }

    if (dlc->GetState() == DLC::DS_DONE)
    {
        int ret = 0;
        if (dlc->GetError() != DLC::DE_NO_ERROR)
        {
            ret = 1;
        }

        if (!inExitMode)
        {
            // finished without errors or there is no retry counts
            if (0 == ret || 0 == retryCount)
            {
                DAVA::FileSystem::Instance()->DeleteFile(testingFileFlag);
            }

            // dlc finished ok, write info into db
            if (0 == ret)
            {
                DAVA::MongodbClient* dbClient = DAVA::MongodbClient::Create(DLC_MONGO_HOST, DLC_MONGO_PORT);
                if (dbClient->IsConnected())
                {
                    dbClient->SetDatabaseName(DLC_MONGO_TEST_DB);
                    dbClient->SetCollectionName(DLC_MONGO_TEST_COLLECTION);

                    DAVA::MongodbObject* dbObject = dbClient->FindObjectByKey(dbObjectId);
                    if (nullptr != dbObject)
                    {
                        DAVA::MongodbObject* newDbObject = new DAVA::MongodbObject();
                        newDbObject->SetObjectName(dbObject->GetObjectName());
                        newDbObject->AddString("state", "finished");
                        newDbObject->AddInt32("retries", retryCount - DLC_TEST_MAX_RETRY_COUNT);
                        newDbObject->AddInt32("error", dlc->GetError());
                        newDbObject->Finish();

                        dbClient->SaveDBObject(newDbObject, dbObject);
                        SafeRelease(dbObject);
                        SafeRelease(newDbObject);
                    }
                }
            }
        }

        exit(ret);
    }
}

void DLCCrashTest::ExitThread(BaseObject* caller, void* callerData, void* userData)
{
    DAVA::uint64 start = DAVA::SystemTimer::Instance()->AbsoluteMS();
    DAVA::uint64 elapsed = 0;

    while (elapsed < exitTimeout)
    {
        elapsed = (DAVA::SystemTimer::Instance()->AbsoluteMS() - start) / 1000;
        Thread::Sleep(100);
    }

    forceExit = true;
}

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


#include "GameCore.h"
#include "AppScreens.h"
#include "TestScreen.h"

using namespace DAVA;

GameCore::GameCore()
{
}

GameCore::~GameCore()
{
}

void GameCore::OnAppStarted()
{
    cursor = 0;
    RenderManager::Instance()->SetFPS(60);

    testScreen = new TestScreen();

    UIScreenManager::Instance()->RegisterScreen(SCREEN_TEST, testScreen);

    UIScreenManager::Instance()->SetFirst(SCREEN_TEST);
}

void GameCore::OnAppFinished()
{
    SafeRelease(cursor);

    SafeRelease(testScreen);
}

void GameCore::OnSuspend()
{
//    Logger::Debug("GameCore::OnSuspend");
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
    ApplicationCore::OnSuspend();
#endif
}

void GameCore::OnResume()
{
    //    Logger::Debug("GameCore::OnResume");
    ApplicationCore::OnResume();
}

void GameCore::OnBackground()
{
    //    Logger::Debug("GameCore::OnBackground");
}

#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
void GameCore::OnDeviceLocked()
{
    //    Logger::Debug("GameCore::OnDeviceLocked");
    Core::Instance()->Quit();
}
#endif //#if defined (__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)

void GameCore::BeginFrame()
{
    ApplicationCore::BeginFrame();
    RenderManager::Instance()->ClearWithColor(0, 0, 0, 0);
}

void GameCore::Update(float32 timeElapsed)
{
    //	if (!cursor)
    //	{
    //		cursor = Cursor::Create("~res:/Cursor/cursor1.png", Vector2(6, 0));
    //		RenderManager::Instance()->SetCursor(cursor);
    //	}
    ApplicationCore::Update(timeElapsed);
}

void GameCore::Draw()
{
    ApplicationCore::Draw();
}
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


#ifndef __BEAST_ACTION_H__
#define __BEAST_ACTION_H__

#include "Commands2/Base/CommandAction.h"
#include "Beast/BeastProxy.h"

#if defined(__DAVAENGINE_BEAST__)

class SceneEditor2;
class BeastManager;
class QtWaitDialog;

class BeastAction : public CommandAction
{
public:
    BeastAction(SceneEditor2* scene, const DAVA::FilePath& outputPath, BeastProxy::eBeastMode mode, QtWaitDialog* _waitDialog);
    ~BeastAction();

    virtual void Redo();

private:
    void Start();
    bool Process();
    void Finish(bool canceled);

    void PackLightmaps();
    DAVA::FilePath GetLightmapDirectoryPath();

private:
    BeastManager* beastManager = nullptr;
    QtWaitDialog* waitDialog = nullptr;
    SceneEditor2* workingScene = nullptr;
    DAVA::FilePath outputPath;
    DAVA::uint64 startTime = 0;
    BeastProxy::eBeastMode beastMode = BeastProxy::eBeastMode::MODE_LIGHTMAPS;
};

#endif //#if defined (__DAVAENGINE_BEAST__)

#endif // #ifndef __BEAST_ACTION_H__
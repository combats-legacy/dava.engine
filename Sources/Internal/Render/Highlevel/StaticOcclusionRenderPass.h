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


#ifndef __DAVAENGINE_STATIC_OCCLUSION_RENDER_PASS__
#define __DAVAENGINE_STATIC_OCCLUSION_RENDER_PASS__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Base/BaseMath.h"
#include "Render/Highlevel/RenderPass.h"
#include "Render/RenderBase.h"
#include "Render/Texture.h"

namespace DAVA
{
// use only for debug purposes
// enabling this will save each rendered frame to documents folder
#define SAVE_OCCLUSION_IMAGES 0

struct StaticOcclusionFrameResult;
class StaticOcclusionData;

class StaticOcclusionRenderPass : public RenderPass
{
public:
    StaticOcclusionRenderPass(const FastName& name);
    ~StaticOcclusionRenderPass();

    void DrawOcclusionFrame(RenderSystem* renderSystem, Camera* occlusionCamera,
                            StaticOcclusionFrameResult& target, const StaticOcclusionData&, uint32 blockIndex);

private:
    bool ShouldEnableDepthWriteForRenderObject(RenderObject*);

private:
    enum RenderBatchDepthOption : uint32
    {
        Option_DepthWriteDisabled = 0,
        Option_DepthWriteEnabled = 1
    };
    using RenderBatchWithDepthOption = std::pair<RenderBatch*, RenderBatchDepthOption>;

    rhi::HTexture colorBuffer;
    rhi::HTexture depthBuffer;
    rhi::HDepthStencilState depthWriteStateState[2];
    Vector<RenderBatch*> terrainBatches;
    UnorderedMap<RenderObject*, bool> switchRenderObjects;
    Vector<RenderBatchWithDepthOption> meshBatchesWithDepthWriteOption;
};
};

#endif //__DAVAENGINE_STATIC_OCCLUSION_RENDER_PASS__

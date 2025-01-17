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
#ifndef __DAVAENGINE_NMATERIALSTATEDYNAMICPROPERTYSINSP_NAMES_H__
#define __DAVAENGINE_NMATERIALSTATEDYNAMICPROPERTYSINSP_NAMES_H__

#include "Base/BaseTypes.h"
#include "Base/HashMap.h"
#include "Base/FastNameMap.h"
#include "Base/Introspection.h"
#include "Render/RHI/rhi_ShaderSource.h"

namespace DAVA
{
class NMaterial;

class NMaterialStateDynamicPropertiesInsp : public InspInfoDynamic
{
public:
    DynamicData Prepare(void* object, int filter) const override;
    Vector<FastName> MembersList(const DynamicData& ddata) const override;
    InspDesc MemberDesc(const DynamicData& ddata, const FastName& key) const override;
    int MemberFlags(const DynamicData& ddata, const FastName& key) const override;
    VariantType MemberValueGet(const DynamicData& ddata, const FastName& key) const override;
    void MemberValueSet(const DynamicData& ddata, const FastName& key, const VariantType& value) override;

private:
    struct PropData
    {
        uint32 size;
        rhi::ShaderProp::Type type;
        const float32* defaultValue;
    };
    using PropDataMap = FastNameMap<PropData>;
    bool IsColor(const FastName& key) const;
    void FindMaterialPropertiesRecursive(NMaterial* state, PropDataMap& props) const;
    void FillGlobalMaterialMemebers(NMaterial* state, PropDataMap& props) const;
};
};

#endif /* defined(__DAVAENGINE_NMATERIALSTATEDYNAMICPROPERTYSINSP_NAMES_H__) */

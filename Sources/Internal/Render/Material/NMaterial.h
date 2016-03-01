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


#ifndef __DAVAENGINE_NMATERIAL_H__
#define __DAVAENGINE_NMATERIAL_H__

#include <memory>
#include "Base/FastNameMap.h"
#include "NMaterialNames.h"
#include "NMaterialStateDynamicFlagsInsp.h"
#include "NMaterialStateDynamicPropertiesInsp.h"
#include "NMaterialStateDynamicTexturesInsp.h"
#include "Render/Shader.h"
#include "Scene3D/DataNode.h"

#include "MemoryManager/MemoryProfiler.h"

namespace DAVA
{
struct MaterialBufferBinding;

struct NMaterialProperty
{
    FastName name;
    rhi::ShaderProp::Type type;
    uint32 arraySize;
    uint32 updateSemantic;
    std::unique_ptr<float32[]> data;

    inline void SetPropertyValue(const float32* newValue);

    inline static uint32 GetCurrentUpdateSemantic()
    {
        return globalPropertyUpdateSemanticCounter;
    }

private:
    static uint32 globalPropertyUpdateSemanticCounter;
};

struct MaterialTextureInfo
{
    Texture* texture = nullptr;
    FilePath path;
};

struct MaterialConfig
{
    MaterialConfig();
    ~MaterialConfig();
    MaterialConfig& operator=(const MaterialConfig& config);
    MaterialConfig(const MaterialConfig& config);

    void Clear();

    FastName name;
    FastName fxName;
    HashMap<FastName, NMaterialProperty*> localProperties;
    HashMap<FastName, MaterialTextureInfo*> localTextures;
    HashMap<FastName, int32> localFlags; // integer flags are just more generic than boolean (eg. #if SHADING == HIGH), it has nothing in common with eFlagValue
};

class RenderVariantInstance
{
    friend class NMaterial;
    ShaderDescriptor* shader = nullptr;

    rhi::HDepthStencilState depthState;
    rhi::HSamplerState samplerState;
    rhi::HTextureSet textureSet;
    rhi::CullMode cullMode;
    bool wireFrame = 0;

    Vector<rhi::HConstBuffer> vertexConstBuffers;
    Vector<rhi::HConstBuffer> fragmentConstBuffers;

    Vector<MaterialBufferBinding*> materialBufferBindings;

    uint32 renderLayer = 0;

    RenderVariantInstance();
    RenderVariantInstance(const RenderVariantInstance&) = delete;
    ~RenderVariantInstance();
};

class NMaterial : public DataNode
{
    // this classed need access to be able to generate
    // dynamic introspection for NMaterial class
    friend class NMaterialStateDynamicFlagsInsp;
    friend class NMaterialStateDynamicPropertiesInsp;
    friend class NMaterialStateDynamicTexturesInsp;

    DAVA_ENABLE_CLASS_ALLOCATION_TRACKING(ALLOC_POOL_NMATERIAL)

public:
    NMaterial();
    ~NMaterial();

    void Load(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Save(KeyedArchive* archive, SerializationContext* serializationContext) override;

    NMaterial* Clone();

    void SetFXName(const FastName& fxName);
    bool HasLocalFXName() const;
    const FastName& GetLocalFXName() const;
    const FastName& GetEffectiveFXName() const;

    const FastName& GetQualityGroup();
    void SetQualityGroup(const FastName& quality);

    inline void SetMaterialName(const FastName& name);
    inline const FastName& GetMaterialName() const;

    uint32 GetRequiredVertexFormat();

    void InvalidateBufferBindings();
    void InvalidateTextureBindings();
    void InvalidateRenderVariants();

    // properties
    void AddProperty(const FastName& propName, const float32* propData, rhi::ShaderProp::Type type, uint32 arraySize = 1);
    void RemoveProperty(const FastName& propName);
    void SetPropertyValue(const FastName& propName, const float32* propData);
    bool HasLocalProperty(const FastName& propName);
    rhi::ShaderProp::Type GetLocalPropType(const FastName& propName);
    uint32 GetLocalPropArraySize(const FastName& propName);
    const float32* GetLocalPropValue(const FastName& propName);
    const float32* GetEffectivePropValue(const FastName& propName);

    // textures
    void AddTexture(const FastName& slotName, Texture* texture);
    void RemoveTexture(const FastName& slotName);
    void SetTexture(const FastName& slotName, Texture* texture);
    bool HasLocalTexture(const FastName& slotName);
    Texture* GetLocalTexture(const FastName& slotName);
    Texture* GetEffectiveTexture(const FastName& slotName);

    void CollectLocalTextures(Set<MaterialTextureInfo*>& collection) const;
    bool ContainsTexture(Texture* texture) const;
    const HashMap<FastName, MaterialTextureInfo*>& GetLocalTextures() const;

    // flags
    void AddFlag(const FastName& flagName, int32 value);
    void RemoveFlag(const FastName& flagName);
    void SetFlag(const FastName& flagName, int32 value);
    bool HasLocalFlag(const FastName& flagName);
    const HashMap<FastName, int32>& GetLocalFlags() const;

    int32 GetLocalFlagValue(const FastName& flagName);
    int32 GetEffectiveFlagValue(const FastName& flagName);

    void SetParent(NMaterial* parent);
    NMaterial* GetParent();
    const Vector<NMaterial*>& GetChildren() const;

    inline uint32 GetRenderLayerID() const;
    inline uint32 GetSortingKey() const;

    //Configs managment
    uint32 GetConfigCount() const;
    const MaterialConfig& GetConfig(uint32 index) const;
    void InsertConfig(uint32 index, const MaterialConfig& config);
    void RemoveConfig(uint32 index);

    uint32 GetCurrentConfigIndex() const;
    void SetCurrentConfigIndex(uint32 index);

    const FastName& GetConfigName(uint32 index) const;
    void SetConfigName(uint32 index, const FastName& name);
    uint32 FindConfigByName(const FastName& name) const; //return size if config not found!
    const DAVA::FastName& GetCurrentConfigName() const;
    void SetCurrentConfigName(const DAVA::FastName& newName);

    void ReleaseConfigTextures(uint32 index);

    void BindParams(rhi::Packet& target);

    // returns true if has variant for this pass, false otherwise
    // if material doesn't support pass active variant will be not changed
    // later add engine flags here
    bool PreBuildMaterial(const FastName& passName);

    // RHI_COMPLETE - it's temporary solution to avoid FX loading and shaders compilation after loading
    void PreCacheFX();
    void PreCacheFXWithFlags(const HashMap<FastName, int32>& extraFlags, const FastName& extraFxName = FastName());

    static const float32 DEFAULT_LIGHTMAP_SIZE;

private:
    void LoadOldNMaterial(KeyedArchive* archive, SerializationContext* serializationContext);
    void SaveConfigToArchive(uint32 configId, KeyedArchive* archive, SerializationContext* serializationContext);
    void LoadConfigFromArchive(uint32 configId, KeyedArchive* archive, SerializationContext* serializationContext);

    void RebuildBindings();
    void RebuildTextureBindings();
    void RebuildRenderVariants();

    bool NeedLocalOverride(UniquePropertyLayout propertyLayout);
    void ClearLocalBuffers();
    void InjectChildBuffer(UniquePropertyLayout propLayoutId, MaterialBufferBinding* buffer);

    // the following functions will collect data recursively
    MaterialBufferBinding* GetConstBufferBinding(UniquePropertyLayout propertyLayout);
    NMaterialProperty* GetMaterialProperty(const FastName& propName);
    void CollectMaterialFlags(HashMap<FastName, int32>& target);

    void AddChildMaterial(NMaterial* material);
    void RemoveChildMaterial(NMaterial* material);

    const MaterialConfig& GetCurrentConfig() const;
    MaterialConfig& GetMutableCurrentConfig();
    MaterialConfig& GetMutableConfig(uint32 index);

private:
    // config time
    FastName materialName;
    FastName qualityGroup;

    Vector<MaterialConfig> materialConfigs;

    // runtime
    NMaterial* parent = nullptr;
    Vector<NMaterial*> children;

    uint32 currentConfig = 0;

    FastName activeVariantName;
    RenderVariantInstance* activeVariantInstance = nullptr;

    HashMap<UniquePropertyLayout, MaterialBufferBinding*> localConstBuffers;

    // this is for render passes - not used right now - only active variant instance
    HashMap<FastName, RenderVariantInstance*> renderVariants;

    uint32 sortingKey = 0;
    bool needRebuildBindings = true;
    bool needRebuildTextures = true;
    bool needRebuildVariants = true;

    static const FastName DEFAULT_CONFIG_NAME;

public:
    INTROSPECTION_EXTEND(NMaterial, DataNode,
                         PROPERTY("materialName", "Material name", GetMaterialName, SetMaterialName, I_VIEW | I_EDIT)
                         PROPERTY("configName", "Config name", GetCurrentConfigName, SetCurrentConfigName, I_VIEW | I_EDIT)
                         PROPERTY("configId", "Current config", GetCurrentConfigIndex, SetCurrentConfigIndex, I_VIEW | I_EDIT)
                         PROPERTY("fxName", "FX Name", GetLocalFXName, SetFXName, I_VIEW | I_EDIT)
                         PROPERTY("qualityGroup", "Quality group", GetQualityGroup, SetQualityGroup, I_VIEW | I_EDIT)
                         DYNAMIC(localFlags, "Material flags", new NMaterialStateDynamicFlagsInsp(), I_EDIT | I_VIEW)
                         DYNAMIC(localProperties, "Material properties", new NMaterialStateDynamicPropertiesInsp(), I_EDIT | I_VIEW)
                         DYNAMIC(localTextures, "Material textures", new NMaterialStateDynamicTexturesInsp(), I_EDIT | I_VIEW))
};

void NMaterialProperty::SetPropertyValue(const float32* newValue)
{
    //4 is because register size is float4
    Memcpy(data.get(), newValue, sizeof(float32) * ShaderDescriptor::CalculateDataSize(type, arraySize));
    updateSemantic = ++globalPropertyUpdateSemanticCounter;
}

void NMaterial::SetMaterialName(const FastName& name)
{
    materialName = name;
}
const FastName& NMaterial::GetMaterialName() const
{
    return materialName;
}
uint32 NMaterial::GetRenderLayerID() const
{
    if (activeVariantInstance)
        return activeVariantInstance->renderLayer;
    else
        return static_cast<uint32>(-1);
}
uint32 NMaterial::GetSortingKey() const
{
    return sortingKey;
}

inline uint32 NMaterial::GetCurrentConfigIndex() const
{
    return currentConfig;
}

inline const MaterialConfig& NMaterial::GetCurrentConfig() const
{
    return GetConfig(GetCurrentConfigIndex());
}

inline MaterialConfig& NMaterial::GetMutableCurrentConfig()
{
    return GetMutableConfig(GetCurrentConfigIndex());
}

inline const MaterialConfig& NMaterial::GetConfig(uint32 index) const
{
    DVASSERT(index < materialConfigs.size());
    return materialConfigs[index];
}

inline MaterialConfig& NMaterial::GetMutableConfig(uint32 index)
{
    DVASSERT(index < materialConfigs.size());
    return materialConfigs[index];
}
};

#endif // __DAVAENGINE_MATERIAL_H__

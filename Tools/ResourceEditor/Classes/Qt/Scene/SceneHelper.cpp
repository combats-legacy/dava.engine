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


#include "SceneHelper.h"
#include "Deprecated/SceneValidator.h"

SceneHelper::TextureCollector::TextureCollector(DAVA::uint32 options)
{
    includeNullTextures = (options & IncludeNullTextures) != 0;
    onlyActiveTextures = (options & OnlyActiveTextures) != 0;
}

void SceneHelper::TextureCollector::Apply(DAVA::NMaterial* material)
{
    DAVA::Set<DAVA::MaterialTextureInfo*> materialTextures;
    if (onlyActiveTextures)
        material->CollectActiveLocalTextures(materialTextures);
    else
        material->CollectLocalTextures(materialTextures);

    for (auto const& matTex : materialTextures)
    {
        const DAVA::FilePath& texturePath = matTex->path;
        DAVA::Texture* texture = matTex->texture;

        if (texturePath.IsEmpty() || !SceneValidator::Instance()->IsPathCorrectForProject(texturePath))
        {
            continue;
        }

        if ((includeNullTextures == false) && (nullptr == texture || texture->isRenderTarget))
        {
            continue;
        }

        textureMap[FILEPATH_MAP_KEY(texturePath)] = texture;
    }
}

DAVA::TexturesMap& SceneHelper::TextureCollector::GetTextures()
{
    return textureMap;
}

void SceneHelper::EnumerateSceneTextures(DAVA::Scene* forScene, TextureCollector& collector)
{
    EnumerateEntityTextures(forScene, forScene, collector);
}

void SceneHelper::BuildMaterialList(DAVA::Entity* forNode, DAVA::Set<DAVA::NMaterial*>& materialList, bool includeRuntime)
{
    if (nullptr == forNode)
        return;

    DAVA::List<DAVA::NMaterial*> materials;
    forNode->GetDataNodes(materials);

    for (auto& mat : materials)
    {
        if (!includeRuntime && mat->IsRuntime())
        {
            continue;
        }

        materialList.insert(mat);
    }
}

void SceneHelper::EnumerateEntityTextures(DAVA::Scene* forScene, DAVA::Entity* forNode, TextureCollector& collector)
{
    if (nullptr == forNode || nullptr == forScene)
    {
        return;
    }

    DAVA::Set<DAVA::NMaterial*> materials;
    BuildMaterialList(forNode, materials);

    DAVA::Set<DAVA::MaterialTextureInfo*> materialTextures;
    for (auto& mat : materials)
    {
        DAVA::String materialName = mat->GetMaterialName().c_str();
        DAVA::String parentName = mat->GetParent() ? mat->GetParent()->GetMaterialName().c_str() : DAVA::String();

        if ((parentName.find("Particle") != DAVA::String::npos) || (materialName.find("Particle") != DAVA::String::npos))
        { //because particle materials has textures only after first start, so we have different result during scene life.
            continue;
        }

        collector.Apply(mat);
    }
}

DAVA::int32 SceneHelper::EnumerateModifiedTextures(DAVA::Scene* forScene, DAVA::Map<DAVA::Texture*, DAVA::Vector<DAVA::eGPUFamily>>& textures)
{
    DAVA::int32 retValue = 0;
    textures.clear();
    TextureCollector collector;
    EnumerateSceneTextures(forScene, collector);

    for (auto& it : collector.GetTextures())
    {
        DAVA::Texture* texture = it.second;
        if (nullptr == texture)
        {
            continue;
        }

        DAVA::TextureDescriptor* descriptor = texture->GetDescriptor();
        DVASSERT(descriptor);
        DVASSERT(descriptor->compression);

        DAVA::Vector<DAVA::eGPUFamily> markedGPUs;
        for (int i = 0; i < DAVA::GPU_DEVICE_COUNT; ++i)
        {
            DAVA::eGPUFamily gpu = static_cast<DAVA::eGPUFamily>(i);
            if (DAVA::GPUFamilyDescriptor::IsFormatSupported(gpu, static_cast<DAVA::PixelFormat>(descriptor->compression[gpu].format)))
            {
                DAVA::FilePath texPath = descriptor->GetSourceTexturePathname();
                if (DAVA::FileSystem::Instance()->Exists(texPath) && !descriptor->IsCompressedTextureActual(gpu))
                {
                    markedGPUs.push_back(gpu);
                    retValue++;
                }
            }
        }
        if (markedGPUs.size() > 0)
        {
            textures[texture] = markedGPUs;
        }
    }
    return retValue;
}

void SceneHelper::EnumerateMaterials(DAVA::Entity* forNode, DAVA::Set<DAVA::NMaterial*>& materials)
{
    EnumerateMaterialInstances(forNode, materials);

    // collect parent materials
    for (auto material : materials)
    {
        while (material->GetParent() != nullptr)
        {
            material = material->GetParent();
            materials.insert(material);
        }
    }
}

void SceneHelper::EnumerateMaterialInstances(DAVA::Entity* forNode, DAVA::Set<DAVA::NMaterial*>& materials)
{
    DAVA::uint32 childrenCount = forNode->GetChildrenCount();
    for (DAVA::uint32 i = 0; i < childrenCount; ++i)
    {
        EnumerateMaterialInstances(forNode->GetChild(i), materials);
    }

    DAVA::RenderObject* ro = GetRenderObject(forNode);
    if (ro != nullptr)
    {
        DAVA::uint32 batchCount = ro->GetRenderBatchCount();
        for (DAVA::uint32 i = 0; i < batchCount; ++i)
        {
            auto material = ro->GetRenderBatch(i)->GetMaterial();
            if (material != nullptr)
            {
                materials.insert(material);
            }
        }
    }
}

DAVA::Entity* SceneHelper::CloneEntityWithMaterials(DAVA::Entity* fromNode)
{
    DAVA::Scene* scene = fromNode->GetScene();
    DAVA::NMaterial* globalMaterial = (scene) ? scene->GetGlobalMaterial() : nullptr;

    DAVA::Entity* newEntity = fromNode->Clone();

    DAVA::Set<DAVA::NMaterial*> materialInstances;
    EnumerateMaterialInstances(newEntity, materialInstances);

    DAVA::Set<DAVA::NMaterial*> materialParentsSet;
    for (auto material : materialInstances)
    {
        materialParentsSet.insert(material->GetParent());
    }
    materialParentsSet.erase(globalMaterial);

    DAVA::Map<DAVA::NMaterial*, DAVA::NMaterial*> clonedParents;
    for (auto& mp : materialParentsSet)
    {
        DAVA::NMaterial* mat = mp ? mp->Clone() : nullptr;
        if (mat && mat->GetParent() == globalMaterial)
        {
            mat->SetParent(nullptr); // exclude material from scene
        }
        clonedParents[mp] = mat;
    }

    for (auto material : materialInstances)
    {
        DAVA::NMaterial* parent = material->GetParent();
        material->SetParent(clonedParents[parent]);
    }

    for (auto& cp : clonedParents)
    {
        SafeRelease(cp.second);
    }

    return newEntity;
}

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


#include "stdafx.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"
#include "Scene3D/AnimationData.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/LodComponent.h"
#include "Scene3D/Components/AnimationComponent.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Render/Material/NMaterial.h"
#include "Render/Material/NMaterialNames.h"
#include "Render/Highlevel/RenderObject.h"
#include "Render/Highlevel/RenderBatch.h"
#include "Render/Highlevel/Mesh.h"
#include "Render/3D/MeshUtils.h"
#include "Render/3D/PolygonGroup.h"
#include "Collada/ColladaPolygonGroup.h"
#include "Collada/ColladaMeshInstance.h"
#include "Collada/ColladaSceneNode.h"
#include "Collada/ColladaScene.h"

#include "Collada/ColladaToSc2Importer/ColladaToSc2Importer.h"

namespace DAVA
{
namespace
{
void FlipTexCoords(Vector2 & v)
{
    v.y = 1.0f - v.y;
}

bool IsShadowNode(const String & nodeName)
{
    size_t fp = nodeName.find("_shadow");
    return fp != String::npos;
}
}

ColladaToSc2Importer::ImportLibrary::~ImportLibrary()
{
    for (auto & pair : polygons)
    {
        uint32 refCount = pair.second->Release();
        DVASSERT(0 == refCount);
    }
    polygons.clear();
 
    for (auto & pair : materials)
    {
        uint32 refCount = pair.second->Release();
        DVASSERT(0 == refCount);
    }
    materials.clear();

    for (auto & pair : materialParents)
    {
        uint32 refCount = pair.second->Release();
        DVASSERT(0 == refCount);
    }
    materialParents.clear();
}

void InitPolygon(PolygonGroup * davaPolygon, uint32 vertexFormat, Vector<ColladaVertex> &vertices)
{
    size_t vertexCount = vertices.size();
    for (uint32 vertexNo = 0; vertexNo < vertexCount; ++vertexNo)
    {
        auto & vertex = vertices[vertexNo];
        
        if (vertexFormat & EVF_VERTEX)
        {
            davaPolygon->SetCoord(vertexNo, vertex.position);
        }
        if (vertexFormat & EVF_NORMAL)
        {
            davaPolygon->SetNormal(vertexNo, vertex.normal);
        }
        if (vertexFormat & EVF_TANGENT)
        {
            davaPolygon->SetTangent(vertexNo, vertex.tangent);
        }
        if (vertexFormat & EVF_BINORMAL)
        {
            davaPolygon->SetBinormal(vertexNo, vertex.binormal);
        }
        if (vertexFormat & EVF_TEXCOORD0)
        {
            FlipTexCoords(vertex.texCoords[0]);
            davaPolygon->SetTexcoord(0, vertexNo, vertex.texCoords[0]);
        }
        if (vertexFormat & EVF_TEXCOORD1)
        {
            FlipTexCoords(vertex.texCoords[1]);
            davaPolygon->SetTexcoord(1, vertexNo, vertex.texCoords[1]);
        }
        if (vertexFormat & EVF_TEXCOORD2)
        {
            FlipTexCoords(vertex.texCoords[2]);
            davaPolygon->SetTexcoord(2, vertexNo, vertex.texCoords[2]);
        }
        if (vertexFormat & EVF_TEXCOORD3)
        {
            FlipTexCoords(vertex.texCoords[3]);
            davaPolygon->SetTexcoord(3, vertexNo, vertex.texCoords[3]);
        }
    }
}
    
PolygonGroup * ColladaToSc2Importer::ImportLibrary::GetOrCreatePolygon(ColladaPolygonGroupInstance * colladaPGI)
{
    // Try to take polygon from library
    PolygonGroup * davaPolygon = polygons[colladaPGI];
    
    // there is no polygon, so create new one
    if (nullptr == davaPolygon)
    {
        davaPolygon = new PolygonGroup();
        
        ColladaPolygonGroup *colladaPolygon = colladaPGI->polyGroup;
        DVASSERT(nullptr != colladaPolygon && "Empty collada polyton group instance.");

        auto vertices = colladaPolygon->GetVertices();
        size_t vertexCount = vertices.size();
        auto vertexFormat = colladaPolygon->GetVertexFormat();
        auto indecies = colladaPolygon->GetIndices();
        size_t indexCount = indecies.size();

        // Allocate data buffers before fill them
        davaPolygon->AllocateData(vertexFormat, vertexCount, indexCount);
        davaPolygon->triangleCount = colladaPolygon->GetTriangleCount();
        
        // Fill index array
        for(uint32 indexNo = 0; indexNo < indexCount; ++indexNo)
        {
            davaPolygon->indexArray[indexNo] = indecies[indexNo];
        }
        
        // Take collada vertices and set to polygon group
        InitPolygon(davaPolygon, vertexFormat, vertices);
        
        davaPolygon->BuildBuffers();
        
        // Put polygon to the library
        polygons[colladaPGI] = davaPolygon;
    }
        
    return davaPolygon;
}

namespace
{
void GetTextureTypeAndPathFromCollada(ColladaMaterial * material, FastName & type, FilePath & path)
{
    ColladaTexture * diffuse = material->diffuseTexture;
    bool useDiffuseTexture = nullptr != diffuse && material->hasDiffuseTexture;
    if (useDiffuseTexture)
    {
        type = NMaterial::TEXTURE_ALBEDO;
        path = diffuse->texturePathName.c_str();
        
        return;
    }
    
    ColladaTexture * lightmap = material->lightmapTexture;
    bool useLightmapTexture = nullptr != lightmap && material->hasLightmapTexture;
    if (useLightmapTexture)
    {
        type = NMaterial::TEXTURE_LIGHTMAP;
        path = lightmap->texturePathName.c_str();
        
        return;
    }
    
    ColladaTexture * reflective = material->reflectiveTexture;
    bool useReflectiveTexture = nullptr != reflective && material->hasReflectiveTexture;
    if (useReflectiveTexture)
    {
        type = NMaterial::TEXTURE_DYNAMIC_REFLECTION;
        path = reflective->texturePathName.c_str();
        
        return;
    }
    
    ColladaTexture * transparent = material->transparentTexture;
    bool useTransparentTexture = nullptr != transparent && material->hasTransparentTexture;
    if (useTransparentTexture)
    {
        // TO DO: are we support it?
        type = NMaterial::TEXTURE_ALBEDO;
        path = transparent->texturePathName.c_str();
        
        return;
    }
}
}

NMaterial * ColladaToSc2Importer::ImportLibrary::GetOrCreateMaterialParent(ColladaMaterial * colladaMaterial, const bool isShadow)
{
    FastName parentMaterialTemplate;
    FastName parentMaterialName;

    if (isShadow)
    {
        parentMaterialName = FastName("Shadow_Material");
        parentMaterialTemplate = NMaterialName::SHADOW_VOLUME;
    }
    else
    {
        parentMaterialName = FastName(colladaMaterial->material->GetDaeId().c_str());
        parentMaterialTemplate = NMaterialName::TEXTURED_OPAQUE;
    }
    
    NMaterial * davaMaterialParent = materialParents[parentMaterialName];
    if (nullptr == davaMaterialParent)
    {
        davaMaterialParent = (NMaterial::CreateMaterial(parentMaterialName, parentMaterialTemplate, NMaterial::DEFAULT_QUALITY_NAME));
        materialParents[parentMaterialName] = davaMaterialParent;
    }
    
    FastName textureType;
    FilePath texturePath;
    GetTextureTypeAndPathFromCollada(colladaMaterial, textureType, texturePath);
    davaMaterialParent->SetTexture(textureType, texturePath);
    
    return davaMaterialParent;
}
    
NMaterial * ColladaToSc2Importer::ImportLibrary::GetOrCreateMaterial(ColladaPolygonGroupInstance * colladaPolyGroupInst, const bool isShadow)
{
    ColladaMaterial * colladaMaterial = colladaPolyGroupInst->material;
    DVASSERT(nullptr != colladaMaterial && "Empty material");

    NMaterial * davaMaterialParent = GetOrCreateMaterialParent(colladaMaterial, isShadow);

    // Use daeId + parentMaterialName to make unique key for material in the library
    String daeId = colladaMaterial->material->GetDaeId().c_str();
    String parentMaterialName(davaMaterialParent->GetMaterialName().c_str());
    FastName materialKey = FastName(daeId + parentMaterialName.c_str());

    // Try to get material from library
    NMaterial * material = materials[materialKey];
    
    // There is no material in the library, so create new one
    if (nullptr == material)
    {
        material = NMaterial::CreateMaterialInstance();
        material->SetMaterialName(FastName(daeId));
        material->SetParent(davaMaterialParent);

        materials[materialKey] = material;
    }

    return material;
}

namespace
{
void CollapseRenderBatchesRecursive(Entity * node, uint32 lod, RenderObject * ro)
{
    for (auto child : node->children)
    {
        CollapseRenderBatchesRecursive(child, lod, ro);
    }
    
    RenderObject * lodRenderObject = GetRenderObject(node);
    if (nullptr != lodRenderObject)
    {
        uint32 batchNo = 0;
        while (lodRenderObject->GetRenderBatchCount() > 0)
        {
            ScopedPtr<RenderBatch> batch(lodRenderObject->GetRenderBatch(batchNo));
            batch->Retain();
            lodRenderObject->RemoveRenderBatch(batch);
            ro->AddRenderBatch(batch, lod, -1);
            
            ++batchNo;
        }
    }
}

void CollapseAnimations(Entity * node, Entity * parent)
{
    for (auto child : parent->children)
    {
        CollapseAnimations(child, parent);
    }
    
    AnimationComponent * ac = GetAnimationComponent(node);
    if (ac)
    {
        node->DetachComponent(ac);
        parent->AddComponent(ac);
    }

}
    
void CollapseLodsIntoOneEntity(Entity *forRootNode)
{
    const uint32 maxLodCount = 10;
    List<Entity *> lodNodes;
    forRootNode->FindNodesByNamePart("_lod0", lodNodes);
    
    for (Entity * oneLodNode : lodNodes)
    {
        // node name which contains lods
        String nodeWithLodsName(String(oneLodNode->GetName().c_str()), 0, oneLodNode->GetName().find("_lod0"));
        
        Entity * oldParent = oneLodNode->GetParent();

        ScopedPtr<Entity> newNodeWithLods(new Entity());
        newNodeWithLods->SetName(nodeWithLodsName.c_str());
        
        ScopedPtr<Mesh> newMesh(new Mesh());
        
        for (int i = 0; i < maxLodCount; ++i)
        {
            // try to find node with same name but with other lod
            Entity *ln = oldParent->FindByName(Format("%s_lod%d", nodeWithLodsName.c_str(), i).c_str());
            if (nullptr == ln)
            {//if layer is dummy
                ln = oldParent->FindByName(Format("%s_lod%ddummy", nodeWithLodsName.c_str(), i).c_str());
                if (nullptr != ln)
                {
                    ln->SetVisible(false);
                    ln->RemoveAllChildren();
                }
            }

            if (nullptr != ln)
            {
                CollapseRenderBatchesRecursive(ln, i, newMesh);
                CollapseAnimations(ln, newNodeWithLods);
                
                oldParent->RemoveNode(ln);
            }
        }
        
        LodComponent *lc = new LodComponent();
        newNodeWithLods->AddComponent(lc);
        
        RenderComponent * rc = new RenderComponent();
        rc->SetRenderObject(newMesh);

        newNodeWithLods->AddComponent(rc);
        oldParent->AddNode(newNodeWithLods);
        
        DVASSERT(oldParent->GetScene());
        DVASSERT(newNodeWithLods->GetScene());

    }
}
    
void CombineLods(Entity * root)
{
    for (auto child : root->children)
    {
        CollapseLodsIntoOneEntity(child);
    }
}

void BakeTransformsUpToParent(Entity * parent, Entity * currentNode)
{
    for (auto child : currentNode->children)
    {
        BakeTransformsUpToParent(parent, child);
    }
    
    // Get actual transformation for current entity
    Matrix4 totalTransform = currentNode->AccamulateTransformUptoFarParent(parent);
    
    // Bake transforms to geometry
    RenderObject * ro = GetRenderObject(currentNode);
    if (ro)
    {
        ro->BakeGeometry(totalTransform);
    }
    
    // Set local transform as Ident because transform is already baked up into geometry
    Matrix4 identMatrix;
    auto transformComponent = GetTransformComponent(currentNode);
    transformComponent->SetLocalTransform(&identMatrix);
    
    // do same thing with animation keys
    auto animationComponent = GetAnimationComponent(currentNode);
    if (animationComponent)
    {
        auto * animation = animationComponent->GetAnimation();
        for (auto & key : animation->keys)
        {
            Matrix4 animationMatrix;
            key.GetMatrix(animationMatrix);
            
            Matrix4 inverseTotal;
            totalTransform.GetInverse(inverseTotal);
            
            Matrix4 totalAnimation = inverseTotal * animationMatrix;
            totalAnimation.Decomposition(key.translation, key.scale, key.rotation);
        }
    }
}
}

// Creates Dava::Mesh from ColladaMeshInstance and puts it
Mesh * ColladaToSc2Importer::GetMeshFromCollada(ColladaMeshInstance * mesh, const bool isShadow)
{
    Mesh * davaMesh = nullptr;
    for (auto polygonGroupInstance : mesh->polyGroupInstances)
    {
        PolygonGroup * davaPolygon = colladaToDavaLibrary.GetOrCreatePolygon(polygonGroupInstance);

        if (isShadow)
        {
            davaPolygon = DAVA::MeshUtils::CreateShadowPolygonGroup(davaPolygon);
        }
        
        davaMesh = new Mesh();
        NMaterial * davaMaterial = colladaToDavaLibrary.GetOrCreateMaterial(polygonGroupInstance, isShadow);
        davaMesh->AddPolygonGroup(davaPolygon, davaMaterial);
    }
    DVASSERT(nullptr != davaMesh && "Can't create mesh from collada MeshInstance");
    return davaMesh;
}

void ColladaToSc2Importer::FillMeshes(const Vector<ColladaMeshInstance *> & meshInstances, Entity * node)
{
    DVASSERT(1 >= meshInstances.size() && "Should be only one meshInstance in one collada node");
    for (auto meshInstance : meshInstances)
    {
        bool isShadowNode = IsShadowNode(node->GetName().c_str());
        
        ScopedPtr<RenderObject> davaMesh(GetMeshFromCollada(meshInstance, isShadowNode));
        RenderComponent * davaRenderComponent = GetRenderComponent(node);
        if (nullptr == davaRenderComponent)
        {
            davaRenderComponent = new RenderComponent();
            node->AddComponent(davaRenderComponent);
        }
        davaRenderComponent->SetRenderObject(davaMesh);
    }
}
    
void ColladaToSc2Importer::BuildSceneAsCollada(Entity * root, ColladaSceneNode * colladaNode)
{
    String name(colladaNode->originalNode->GetName());
    if (name.empty())
    {
        static uint32 num = 0;
        name = Format("Node-%d", num++);
    }
    
    ScopedPtr<Entity> nodeEntity(new Entity());
    nodeEntity->SetName(FastName(name));
    
    // take mesh from node and put it into entity's render component
    FillMeshes(colladaNode->meshInstances, nodeEntity);
    
    if (nullptr != colladaNode->animation)
    {
        auto * animationComponent = new AnimationComponent();
        animationComponent->SetEntity(nodeEntity);
        
        ScopedPtr<AnimationData> animation(new AnimationData());
        animationComponent->SetAnimation(animation);
        
        SceneNodeAnimation * colladaAnimation = colladaNode->animation;
        
        animation->SetInvPose(colladaAnimation->invPose);
        animation->SetDuration(colladaAnimation->duration);
        if (nullptr != colladaAnimation->keys)
        {
            for (uint32 keyNo = 0; keyNo < colladaAnimation->keyCount; ++keyNo)
            {
                SceneNodeAnimationKey key = colladaAnimation->keys[keyNo];
                animation->AddKey(key);
            }
        }
        
        nodeEntity->AddComponent(animationComponent);
    }
    
    auto * transformComponent = GetTransformComponent(nodeEntity);
    transformComponent->SetLocalTransform(&colladaNode->localTransform);
    
    root->AddNode(nodeEntity);
    
    for (auto childNode : colladaNode->childs)
    {
        BuildSceneAsCollada(nodeEntity, childNode);
    }
}

void ColladaToSc2Importer::LoadMaterialParents(ColladaScene * colladaScene)
{
    for (auto cmaterial : colladaScene->colladaMaterials)
    {
        NMaterial * globalMaterial = colladaToDavaLibrary.GetOrCreateMaterialParent(cmaterial, false);
        DVASSERT(nullptr != globalMaterial);
    }
}
    
void ColladaToSc2Importer::SaveSC2(ColladaScene * colladaScene, const FilePath & scenePath, const String & sceneName)
{
    ScopedPtr<Scene> scene(new Scene());

    // Load scene global materials.
    LoadMaterialParents(colladaScene);
    
    // iterate recursive over collada scene and build Dava Scene with same ierarchy
    BuildSceneAsCollada(scene, colladaScene->rootNode);
    
    // Apply transforms to render batches and use identity local transforms
    BakeTransformsUpToParent(scene, scene);
    
    // post process Entities and create Lod nodes.
    CombineLods(scene);
    
    scene->SaveScene(scenePath + sceneName);
}

};
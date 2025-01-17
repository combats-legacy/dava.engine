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


#include "ConvertToShadowCommand.h"

#include "Qt/Scene/SceneEditor2.h"

ConvertToShadowCommand::ConvertToShadowCommand(DAVA::Entity* entity_, DAVA::RenderBatch* batch)
    : Command2(CMDID_CONVERT_TO_SHADOW, "Convert To Shadow")
    , entity(SafeRetain(entity_))
    , oldBatch(batch)
    , newBatch(NULL)
{
    DVASSERT(entity);
    DVASSERT(oldBatch);

    renderObject = DAVA::SafeRetain(oldBatch->GetRenderObject());
    DVASSERT(renderObject);

    oldBatch->Retain();

    newBatch = new DAVA::RenderBatch();
    DAVA::PolygonGroup* shadowPg = DAVA::MeshUtils::CreateShadowPolygonGroup(oldBatch->GetPolygonGroup());
    shadowPg->BuildBuffers();
    newBatch->SetPolygonGroup(shadowPg);
    shadowPg->Release();

    DAVA::ScopedPtr<DAVA::NMaterial> shadowMaterialInst(new DAVA::NMaterial());
    shadowMaterialInst->SetMaterialName(DAVA::FastName("Shadow_Material_Instance"));

    newBatch->SetMaterial(shadowMaterialInst);

    SceneEditor2* scene = static_cast<SceneEditor2*>(entity->GetScene());
    DVASSERT(scene != nullptr);
    const DAVA::Set<DAVA::NMaterial*> topLevelMaterials = scene->materialSystem->GetTopParents();
    DAVA::Set<DAVA::NMaterial*>::iterator iter = std::find_if(topLevelMaterials.begin(), topLevelMaterials.end(), [](DAVA::NMaterial* material)
                                                              {
                                                                  DVASSERT(material->HasLocalFXName());
                                                                  return material->GetLocalFXName() == DAVA::NMaterialName::SHADOW_VOLUME;
                                                              });

    if (iter != topLevelMaterials.end())
    {
        shadowMaterialInst->SetParent(*iter);
    }
    else
    {
        DAVA::ScopedPtr<DAVA::NMaterial> shadowMaterial(new DAVA::NMaterial());
        shadowMaterial->SetMaterialName(DAVA::FastName("Shadow_Material"));
        shadowMaterial->SetFXName(DAVA::NMaterialName::SHADOW_VOLUME);

        shadowMaterialInst->SetParent(shadowMaterial.get());
    }
}

ConvertToShadowCommand::~ConvertToShadowCommand()
{
    DAVA::SafeRelease(oldBatch);
    DAVA::SafeRelease(newBatch);
    DAVA::SafeRelease(renderObject);
    DAVA::SafeRelease(entity);
}

void ConvertToShadowCommand::Redo()
{
    renderObject->ReplaceRenderBatch(oldBatch, newBatch);
}

void ConvertToShadowCommand::Undo()
{
    renderObject->ReplaceRenderBatch(newBatch, oldBatch);
}

DAVA::Entity* ConvertToShadowCommand::GetEntity() const
{
    return entity;
}

bool ConvertToShadowCommand::CanConvertBatchToShadow(DAVA::RenderBatch* renderBatch)
{
    if (renderBatch && renderBatch->GetMaterial() && renderBatch->GetPolygonGroup())
    {
        return renderBatch->GetMaterial()->GetEffectiveFXName() != DAVA::NMaterialName::SHADOW_VOLUME;
    }

    return false;
}

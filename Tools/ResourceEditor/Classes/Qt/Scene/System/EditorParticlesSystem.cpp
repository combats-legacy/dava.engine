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


#include "Scene/SceneEditor2.h"
#include "Scene/System/EditorParticlesSystem.h"
#include "Scene/System/SelectionSystem.h"
#include "Scene/System/CollisionSystem.h"
#include "Scene/System/HoodSystem.h"
#include "Scene/SceneSignals.h"
#include "Scene/SceneTabWidget.h"
#include "Main/mainwindow.h"

// framework
#include "Base/BaseTypes.h"
#include "Entity/Component.h"
#include "Particles/ParticleEmitter.h"
#include "Scene3D/Components/RenderComponent.h"

// particles-related commands
#include "Commands2/ParticleEditorCommands.h"
#include "Commands2/ParticleLayerCommands.h"

EditorParticlesSystem::EditorParticlesSystem(DAVA::Scene* scene)
    : DAVA::SceneSystem(scene)
{
}

void EditorParticlesSystem::DrawDebugInfoForEffect(DAVA::Entity* effectEntity)
{
    DVASSERT(effectEntity != nullptr)

    SceneCollisionSystem* collisionSystem = static_cast<SceneEditor2*>(GetScene())->collisionSystem;

    DAVA::AABBox3 worldBox;
    DAVA::AABBox3 collBox = collisionSystem->GetBoundingBox(effectEntity);
    collBox.GetTransformedBox(effectEntity->GetWorldTransform(), worldBox);
    DAVA::float32 radius = (collBox.max - collBox.min).Length() / 3;
    GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawIcosahedron(worldBox.GetCenter(), radius, DAVA::Color(0.9f, 0.9f, 0.9f, 0.35f), DAVA::RenderHelper::DRAW_SOLID_DEPTH);
}

void EditorParticlesSystem::DrawEmitter(DAVA::ParticleEmitterInstance* emitter, DAVA::Entity* owner, bool selected)
{
    DVASSERT((emitter != nullptr) && (owner != nullptr));

    SceneCollisionSystem* collisionSystem = ((SceneEditor2*)GetScene())->collisionSystem;

    DAVA::Vector3 center = emitter->GetSpawnPosition();
    TransformPerserveLength(center, DAVA::Matrix3(owner->GetWorldTransform()));
    center += owner->GetWorldTransform().GetTranslationVector();

    DAVA::AABBox3 boundingBox = collisionSystem->GetBoundingBox(owner);
    DAVA::float32 radius = (boundingBox.max - boundingBox.min).Length() / 3;
    GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawIcosahedron(center, radius, DAVA::Color(1.0f, 1.0f, 1.0f, 0.5f), DAVA::RenderHelper::DRAW_SOLID_DEPTH);

    if (selected)
    {
        DrawVectorArrow(emitter, center);

        switch (emitter->GetEmitter()->emitterType)
        {
        case DAVA::ParticleEmitter::EMITTER_ONCIRCLE_VOLUME:
        case DAVA::ParticleEmitter::EMITTER_ONCIRCLE_EDGES:
        {
            DrawSizeCircle(owner, emitter);
            break;
        }
        case DAVA::ParticleEmitter::EMITTER_SHOCKWAVE:
        {
            DrawSizeCircleShockWave(owner, emitter);
            break;
        }

        case DAVA::ParticleEmitter::EMITTER_RECT:
        {
            DrawSizeBox(owner, emitter);
            break;
        }

        default:
            break;
        }
    }
}

void EditorParticlesSystem::Draw()
{
    const auto& selection = static_cast<SceneEditor2*>(GetScene())->selectionSystem->GetSelection();
    DAVA::Set<DAVA::ParticleEmitterInstance*> selectedEmitterInstances;
    for (auto instance : selection.ObjectsOfType<DAVA::ParticleEmitterInstance>())
    {
        selectedEmitterInstances.insert(instance);
    }

    for (auto entity : entities)
    {
        auto effect = static_cast<DAVA::ParticleEffectComponent*>(entity->GetComponent(DAVA::Component::PARTICLE_EFFECT_COMPONENT));
        if (effect != nullptr)
        {
            for (DAVA::uint32 i = 0, e = effect->GetEmittersCount(); i < e; ++i)
            {
                auto instance = effect->GetEmitterInstance(i);
                DrawEmitter(instance, entity, selectedEmitterInstances.count(instance) > 0);
            }
            DrawDebugInfoForEffect(entity);
        }
    }
}

void EditorParticlesSystem::DrawSizeCircleShockWave(DAVA::Entity* effectEntity, DAVA::ParticleEmitterInstance* emitter)
{
    DAVA::float32 time = GetEffectComponent(effectEntity)->GetCurrTime();
    DAVA::float32 emitterRadius = (emitter->GetEmitter()->radius) ? emitter->GetEmitter()->radius->GetValue(time) : 0.0f;
    DAVA::Vector3 emissionVector(0.0f, 0.0f, 1.0f);

    if (emitter->GetEmitter()->emissionVector)
    {
        DAVA::Matrix4 wMat = effectEntity->GetWorldTransform();
        wMat.SetTranslationVector(DAVA::Vector3(0.0f, 0.0f, 0.0f));
        emissionVector = emitter->GetEmitter()->emissionVector->GetValue(time) * wMat;
    }

    auto center = Selectable(emitter).GetWorldTransform().GetTranslationVector();

    auto drawer = GetScene()->GetRenderSystem()->GetDebugDrawer();
    drawer->DrawCircle(center, emissionVector, emitterRadius, 12, DAVA::Color(0.7f, 0.0f, 0.0f, 0.25f), DAVA::RenderHelper::DRAW_SOLID_DEPTH);
}

void EditorParticlesSystem::DrawSizeCircle(DAVA::Entity* effectEntity, DAVA::ParticleEmitterInstance* emitter)
{
    DAVA::float32 emitterRadius = 0.0f;
    DAVA::Vector3 emitterVector;
    DAVA::float32 time = GetEffectComponent(effectEntity)->GetCurrTime();

    if (emitter->GetEmitter()->radius)
    {
        emitterRadius = emitter->GetEmitter()->radius->GetValue(time);
    }

    if (emitter->GetEmitter()->emissionVector)
    {
        DAVA::Matrix4 wMat = effectEntity->GetWorldTransform();
        wMat.SetTranslationVector(DAVA::Vector3(0.0f, 0.0f, 0.0f));
        emitterVector = emitter->GetEmitter()->emissionVector->GetValue(time) * wMat;
    }

    auto center = Selectable(emitter).GetWorldTransform().GetTranslationVector();

    auto drawer = GetScene()->GetRenderSystem()->GetDebugDrawer();
    drawer->DrawCircle(center, emitterVector, emitterRadius, 12,
                       DAVA::Color(0.7f, 0.0f, 0.0f, 0.25f), DAVA::RenderHelper::DRAW_SOLID_DEPTH);
}

void EditorParticlesSystem::DrawSizeBox(DAVA::Entity* effectEntity, DAVA::ParticleEmitterInstance* emitter)
{
    // Default value of emitter size
    DAVA::Vector3 emitterSize;

    DAVA::float32 time = GetEffectComponent(effectEntity)->GetCurrTime();

    if (emitter->GetEmitter()->size)
    {
        emitterSize = emitter->GetEmitter()->size->GetValue(time);
    }

    DAVA::Matrix4 wMat = effectEntity->GetWorldTransform();
    wMat.SetTranslationVector(Selectable(emitter).GetWorldTransform().GetTranslationVector());

    DAVA::RenderHelper* drawer = GetScene()->GetRenderSystem()->GetDebugDrawer();
    drawer->DrawAABoxTransformed(DAVA::AABBox3(-0.5f * emitterSize, 0.5f * emitterSize), wMat,
                                 DAVA::Color(0.7f, 0.0f, 0.0f, 0.25f), DAVA::RenderHelper::DRAW_SOLID_DEPTH);
}

void EditorParticlesSystem::DrawVectorArrow(DAVA::ParticleEmitterInstance* emitter, DAVA::Vector3 center)
{
    auto effect = emitter->GetOwner();
    if (effect == nullptr)
        return;

    DAVA::Vector3 emitterVector(0.0f, 0.0f, 1.0f);
    if (emitter->GetEmitter()->emissionVector)
    {
        emitterVector = emitter->GetEmitter()->emissionVector->GetValue(effect->GetCurrTime());
        emitterVector.Normalize();
    }

    DAVA::float32 scale = 1.0f;
    HoodSystem* hoodSystem = ((SceneEditor2*)GetScene())->hoodSystem;
    if (hoodSystem != nullptr)
    {
        scale = hoodSystem->GetScale();
    }

    DAVA::float32 arrowSize = scale;
    DAVA::float32 arrowBaseSize = 5.0f;
    emitterVector = (emitterVector * arrowBaseSize * scale);

    DAVA::Matrix4 wMat = effect->GetEntity()->GetWorldTransform();
    wMat.SetTranslationVector(DAVA::Vector3(0, 0, 0));
    TransformPerserveLength(emitterVector, wMat);

    GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawArrow(center, center + emitterVector, arrowSize,
                                                               DAVA::Color(0.7f, 0.0f, 0.0f, 0.25f), DAVA::RenderHelper::DRAW_SOLID_DEPTH);
}

void EditorParticlesSystem::AddEntity(DAVA::Entity* entity)
{
    entities.push_back(entity);
}

void EditorParticlesSystem::RemoveEntity(DAVA::Entity* entity)
{
    int size = entities.size();
    for (int i = 0; i < size; ++i)
    {
        if (entities[i] == entity)
        {
            entities[i] = entities[size - 1];
            entities.pop_back();
            return;
        }
    }
}

void EditorParticlesSystem::RestartParticleEffects()
{
    for (DAVA::Entity* entity : entities)
    {
        DAVA::ParticleEffectComponent* effectComponent = GetEffectComponent(entity);
        DVASSERT(effectComponent);
        if (!effectComponent->IsStopped())
        {
            effectComponent->Restart();
        }
    }
}

void EditorParticlesSystem::ProcessCommand(const Command2* command, bool redo)
{
    if (command == nullptr)
        return;

    // Notify that the Particles-related value is changed.
    SceneEditor2* activeScene = (SceneEditor2*)GetScene();
    switch (command->GetId())
    {
    case CMDID_BATCH:
    {
        const CommandBatch* batch = static_cast<const CommandBatch*>(command);
        if (batch->MatchCommandIDs({ CMDID_PARTICLE_EMITTER_UPDATE, CMDID_PARTICLE_LAYER_UPDATE, CMDID_PARTICLE_LAYER_CHANGED_MATERIAL_VALUES,
                                     CMDID_PARTILCE_LAYER_UPDATE_TIME, CMDID_PARTICLE_LAYER_UPDATE_ENABLED, CMDID_PARTICLE_FORCE_UPDATE,
                                     CMDID_PARTICLE_EFFECT_START_STOP, CMDID_PARTICLE_EFFECT_RESTART, CMDID_PARTICLE_EMITTER_LOAD_FROM_YAML,
                                     CMDID_PARTICLE_EMITTER_SAVE_TO_YAML,
                                     CMDID_PARTICLE_INNER_EMITTER_LOAD_FROM_YAML, CMDID_PARTICLE_INNER_EMITTER_SAVE_TO_YAML,
                                     //            CMDID_REMOVE_PARTICLE_EMITTER_LAYER,
                                     CMDID_PARTICLE_EMITTER_LAYER_ADD }))
        {
            DAVA::uint32 count = batch->Size();
            for (DAVA::uint32 i = 0; i < count; ++i)
            {
                ProcessCommand(batch->GetCommand(i), redo);
            }
        }
        break;
    }

    case CMDID_PARTICLE_EMITTER_UPDATE:
    {
        const CommandUpdateEmitter* castedCmd = static_cast<const CommandUpdateEmitter*>(command);
        SceneSignals::Instance()->EmitParticleEmitterValueChanged(activeScene, castedCmd->GetEmitterInstance());
        break;
    }

    case CMDID_PARTICLE_LAYER_UPDATE:
    {
        const CommandUpdateParticleLayerBase* castedCmd = static_cast<const CommandUpdateParticleLayerBase*>(command);
        SceneSignals::Instance()->EmitParticleLayerValueChanged(activeScene, castedCmd->GetLayer());
        break;
    }
    case CMDID_PARTICLE_LAYER_CHANGED_MATERIAL_VALUES:
    {
        QtMainWindow::Instance()->RestartParticleEffects();

        const CommandChangeLayerMaterialProperties* cmd = static_cast<const CommandChangeLayerMaterialProperties*>(command);
        SceneSignals::Instance()->EmitParticleLayerValueChanged(activeScene, cmd->GetLayer());
        break;
    }

    case CMDID_PARTILCE_LAYER_UPDATE_TIME:
    case CMDID_PARTICLE_LAYER_UPDATE_ENABLED:
    {
        const CommandUpdateParticleLayerBase* castedCmd = static_cast<const CommandUpdateParticleLayerBase*>(command);
        SceneSignals::Instance()->EmitParticleLayerValueChanged(activeScene, castedCmd->GetLayer());
        break;
    }

    case CMDID_PARTICLE_FORCE_UPDATE:
    {
        const CommandUpdateParticleForce* castedCmd = static_cast<const CommandUpdateParticleForce*>(command);
        SceneSignals::Instance()->EmitParticleForceValueChanged(activeScene, castedCmd->GetLayer(), castedCmd->GetForceIndex());
        break;
    }

    case CMDID_PARTICLE_EFFECT_START_STOP:
    {
        const CommandStartStopParticleEffect* castedCmd = static_cast<const CommandStartStopParticleEffect*>(command);
        SceneSignals::Instance()->EmitParticleEffectStateChanged(activeScene, castedCmd->GetEntity(), castedCmd->GetStarted());
        break;
    }

    case CMDID_PARTICLE_EFFECT_RESTART:
    {
        const CommandRestartParticleEffect* castedCmd = static_cast<const CommandRestartParticleEffect*>(command);

        // An effect was stopped and then started.
        SceneSignals::Instance()->EmitParticleEffectStateChanged(activeScene, castedCmd->GetEntity(), false);
        SceneSignals::Instance()->EmitParticleEffectStateChanged(activeScene, castedCmd->GetEntity(), true);
        break;
    }

    case CMDID_PARTICLE_EMITTER_LOAD_FROM_YAML:
    {
        const CommandLoadParticleEmitterFromYaml* castedCmd = static_cast<const CommandLoadParticleEmitterFromYaml*>(command);
        SceneSignals::Instance()->EmitParticleEmitterLoaded(activeScene, castedCmd->GetEmitterInstance());
        break;
    }

    case CMDID_PARTICLE_EMITTER_SAVE_TO_YAML:
    {
        const CommandSaveParticleEmitterToYaml* castedCmd = static_cast<const CommandSaveParticleEmitterToYaml*>(command);
        SceneSignals::Instance()->EmitParticleEmitterSaved(activeScene, castedCmd->GetEmitterInstance());
        break;
    }

    case CMDID_PARTICLE_INNER_EMITTER_LOAD_FROM_YAML:
    {
        const CommandLoadInnerParticleEmitterFromYaml* castedCmd = static_cast<const CommandLoadInnerParticleEmitterFromYaml*>(command);
        SceneSignals::Instance()->EmitParticleEmitterLoaded(activeScene, castedCmd->GetEmitterInstance());
        break;
    }

    case CMDID_PARTICLE_INNER_EMITTER_SAVE_TO_YAML:
    {
        const CommandSaveInnerParticleEmitterToYaml* castedCmd = static_cast<const CommandSaveInnerParticleEmitterToYaml*>(command);
        SceneSignals::Instance()->EmitParticleEmitterSaved(activeScene, castedCmd->GetEmitterInstance());
        break;
    }

    case CMDID_PARTICLE_EMITTER_LAYER_ADD:
    {
        const CommandAddParticleEmitterLayer* castedCmd = static_cast<const CommandAddParticleEmitterLayer*>(command);
        SceneSignals::Instance()->EmitParticleLayerAdded(activeScene, castedCmd->GetParentEmitter(), castedCmd->GetCreatedLayer());
        break;
    }
    // Return to this code when implementing Layer popup menus.
    /*
		case CMDID_REMOVE_PARTICLE_EMITTER_LAYER:
		{
		const CommandRemoveParticleEmitterLayer* castedCmd = static_cast<const CommandRemoveParticleEmitterLayer*>(command);
		SceneSignals::Instance()->EmitParticleLayerRemoved(activeScene, castedCmd->GetEmitter());
		break;
		}
		*/
    default:
        break;
    }
}

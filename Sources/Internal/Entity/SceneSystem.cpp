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


#include "Entity/SceneSystem.h"
#include "Scene3D/Entity.h"

namespace DAVA
{
SceneSystem::SceneSystem(Scene* _scene)
    : requiredComponents(0)
    ,
    scene(_scene)
    , locked(false)
{
}

SceneSystem::~SceneSystem()
{
}

void SceneSystem::RegisterEntity(Entity* entity)
{
    uint64 requiredComponents = this->GetRequiredComponents();
    bool needAdd = ((requiredComponents & entity->GetAvailableComponentFlags()) == requiredComponents);

    if (needAdd)
        this->AddEntity(entity);
}

void SceneSystem::UnregisterEntity(Entity* entity)
{
    uint64 requiredComponents = this->GetRequiredComponents();
    bool needRemove = ((requiredComponents & entity->GetAvailableComponentFlags()) == requiredComponents);

    if (needRemove)
        this->RemoveEntity(entity);
}

bool SceneSystem::IsEntityComponentFitsToSystem(Entity* entity, Component* component)
{
    uint64 entityComponentFlags = entity->GetAvailableComponentFlags();
    uint64 componentToCheckType = MAKE_COMPONENT_MASK(component->GetType());
    uint64 requiredBySystemComponents = this->GetRequiredComponents();

    bool isAllRequiredComponentsAvailable = ((entityComponentFlags & requiredBySystemComponents) == requiredBySystemComponents);
    bool isComponentMarkedForCheckAvailable = ((requiredBySystemComponents & componentToCheckType) == componentToCheckType);

    return (isAllRequiredComponentsAvailable && isComponentMarkedForCheckAvailable);
}

void SceneSystem::RegisterComponent(Entity* entity, Component* component)
{
    if (IsEntityComponentFitsToSystem(entity, component))
    {
        if (entity->GetComponentCount(component->GetType()) == 1)
        {
            AddEntity(entity);
        }
        else
        {
            AddComponent(entity, component);
        }
    }
}

void SceneSystem::UnregisterComponent(Entity* entity, Component* component)
{
    if (IsEntityComponentFitsToSystem(entity, component))
    {
        if (entity->GetComponentCount(component->GetType()) == 1)
        {
            RemoveEntity(entity);
        }
        else
        {
            RemoveComponent(entity, component);
        }
    }
}

void SceneSystem::AddEntity(Entity* entity)
{
}

void SceneSystem::RemoveEntity(Entity* entity)
{
}

void SceneSystem::AddComponent(Entity* entity, Component* component)
{
}

void SceneSystem::RemoveComponent(Entity* entity, Component* component)
{
}

void SceneSystem::SceneDidLoaded()
{
}

void SceneSystem::ImmediateEvent(Component* component, uint32 event)
{
}

void SceneSystem::Process(float32 timeElapsed)
{
}

void SceneSystem::SetLocked(bool locked_)
{
    locked = locked_;
}

bool SceneSystem::IsLocked() const
{
    return locked;
}
};

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


#ifndef __DAVAENGINE_EDGE_COMPONENT_H__
#define __DAVAENGINE_EDGE_COMPONENT_H__

#include "Entity/Component.h"
#include "Base/Introspection.h"

namespace DAVA
{
class SerializationContext;
class KeyedArchive;
class Entity;

class EdgeComponent : public Component
{
protected:
    ~EdgeComponent();

public:
    IMPLEMENT_COMPONENT_TYPE(EDGE_COMPONENT);

    EdgeComponent();
    EdgeComponent(const EdgeComponent&);

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    void SetNextEntity(Entity* entity);
    Entity* GetNextEntity() const;

    void SetProperties(KeyedArchive* archieve);
    KeyedArchive* GetProperties() const;

private:
    //For property panel
    void SetNextEntityName(const FastName& name);
    const FastName GetNextEntityName() const;

    void SetNextEntityTag(int32 tag);
    int32 GetNextEntityTag() const;

private:
    Entity* nextEntity;
    KeyedArchive* properties;

public:
    INTROSPECTION_EXTEND(EdgeComponent, Component,
                         MEMBER(properties, "Edge properties", I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("nextEntityName", "Next Entity Name", GetNextEntityName, SetNextEntityName, I_VIEW)
                         PROPERTY("nextEntityTag", "Next Entity Tag", GetNextEntityTag, SetNextEntityTag, I_VIEW)
                         );
};

inline Entity* EdgeComponent::GetNextEntity() const
{
    return nextEntity;
}

inline KeyedArchive* EdgeComponent::GetProperties() const
{
    return properties;
}
}
#endif //__DAVAENGINE_EDGE_COMPONENT_H__

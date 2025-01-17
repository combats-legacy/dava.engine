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
    DIRECT, INDIRECT, INCIDENTAL, SpECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#ifndef __SELECTABLE_OBJECT_H__
#define __SELECTABLE_OBJECT_H__

#include "Scene3D/Entity.h"
#include <type_traits>

class Selectable
{
public:
    using Object = DAVA::InspBase;

    enum class TransformType : DAVA::uint32
    {
        Disabled,
        Translation,
        Rotation,
        Scale
    };

    class TransformProxy
    {
    public:
        virtual ~TransformProxy() = default;
        virtual const DAVA::Matrix4& GetWorldTransform(Object* object) = 0;
        virtual const DAVA::Matrix4& GetLocalTransform(Object* object) = 0;
        virtual void SetLocalTransform(Object* object, const DAVA::Matrix4& matrix) = 0;
        virtual bool SupportsTransformType(Object* object, TransformType transformType) const = 0;
        virtual bool TransformDependsFromObject(Object* dependant, Object* dependsOn) const = 0;
    };

    template <typename CLASS, typename PROXY>
    static void AddTransformProxyForClass();
    static void RemoveAllTransformProxies();

public:
    Selectable() = default;
    explicit Selectable(Object* baseObject);
    Selectable(const Selectable& other);
    Selectable(Selectable&& other);

    Selectable& operator=(const Selectable& other);
    Selectable& operator=(Selectable&& other);

    bool operator==(const Selectable& other) const;
    bool operator!=(const Selectable& other) const;

    // comparing only pointers here, and not using bounding box
    // added for compatibility with sorted containers
    bool operator<(const Selectable& other) const;

    template <typename T>
    bool CanBeCastedTo() const;

    template <typename T>
    T* Cast() const;

    Object* GetContainedObject() const;
    DAVA::Entity* AsEntity() const;

    const DAVA::AABBox3& GetBoundingBox() const;
    void SetBoundingBox(const DAVA::AABBox3& box);

    bool SupportsTransformType(TransformType) const;
    const DAVA::Matrix4& GetLocalTransform() const;
    const DAVA::Matrix4& GetWorldTransform() const;
    void SetLocalTransform(const DAVA::Matrix4& transform);

    bool TransformDependsOn(const Selectable&) const;

    bool ContainsObject() const;

private:
    static void AddConcreteProxy(DAVA::MetaInfo* classInfo, TransformProxy* proxy);
    static TransformProxy* GetTransformProxyForClass(const DAVA::MetaInfo* classInfo);

private:
    Object* object = nullptr;
    DAVA::AABBox3 boundingBox;
};

template <typename T>
bool Selectable::CanBeCastedTo() const
{
    DVASSERT(object != nullptr);
    return object->GetTypeInfo()->Type() == DAVA::MetaInfo::Instance<T>();
}

template <typename T>
inline T* Selectable::Cast() const
{
    DVASSERT(object != nullptr);
    if (CanBeCastedTo<T>())
    {
        return static_cast<T*>(object);
    }
    return nullptr;
}

inline Selectable::Object* Selectable::GetContainedObject() const
{
    return object;
}

inline const DAVA::AABBox3& Selectable::GetBoundingBox() const
{
    return boundingBox;
}

inline DAVA::Entity* Selectable::AsEntity() const
{
    return Cast<DAVA::Entity>();
}

template <typename CLASS, typename PROXY>
inline void Selectable::AddTransformProxyForClass()
{
    static_assert(std::is_base_of<Selectable::TransformProxy, PROXY>::value,
                  "Transform proxy should be derived from Selectable::TransformProxy");
    AddConcreteProxy(DAVA::MetaInfo::Instance<CLASS>(), new PROXY());
}

inline bool Selectable::ContainsObject() const
{
    return object != nullptr;
}

#endif // __SELECTABLE_OBJECT_H__

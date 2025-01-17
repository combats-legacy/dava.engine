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


#ifndef __DAVAENGINE_PARTICLE_EMITTER_INSTANCE_H__
#define __DAVAENGINE_PARTICLE_EMITTER_INSTANCE_H__

#include "Particles/ParticleEmitter.h"

namespace DAVA
{
class ParticleEffectComponent;
class ParticleEmitterInstance : public BaseObject
{
public:
    ParticleEmitterInstance(ParticleEffectComponent* owner, bool isInner = false);
    ParticleEmitterInstance(ParticleEffectComponent* owner, ParticleEmitter* emitter, bool isInner = false);

    ParticleEmitter* GetEmitter() const;
    const FilePath& GetFilePath() const;
    const Vector3& GetSpawnPosition() const;

    void SetEmitter(ParticleEmitter* emitter);
    void SetFilePath(const FilePath& filePath);
    void SetSpawnPosition(const Vector3& position);
    void SetOwner(ParticleEffectComponent* owner);

    ParticleEmitterInstance* Clone() const;
    ParticleEffectComponent* GetOwner() const;

    bool IsInnerEmitter() const;

private:
    ParticleEffectComponent* owner = nullptr;
    RefPtr<ParticleEmitter> emitter;
    FilePath filePath;
    Vector3 spawnPosition;
    bool isInnerEmitter = false;

public:
    INTROSPECTION_EXTEND(ParticleEmitterInstance, BaseObject, nullptr)
};

inline ParticleEmitter* ParticleEmitterInstance::GetEmitter() const
{
    return emitter.Get();
}

inline const FilePath& ParticleEmitterInstance::GetFilePath() const
{
    return filePath;
}

inline const Vector3& ParticleEmitterInstance::GetSpawnPosition() const
{
    return spawnPosition;
}

inline ParticleEffectComponent* ParticleEmitterInstance::GetOwner() const
{
    return owner;
}

inline bool ParticleEmitterInstance::IsInnerEmitter() const
{
    return isInnerEmitter;
}

inline void ParticleEmitterInstance::SetEmitter(ParticleEmitter* _emitter)
{
    emitter.Set(_emitter);
}

inline void ParticleEmitterInstance::SetFilePath(const FilePath& _filePath)
{
    filePath = _filePath;
}

inline void ParticleEmitterInstance::SetSpawnPosition(const Vector3& _position)
{
    spawnPosition = _position;
}

inline void ParticleEmitterInstance::SetOwner(ParticleEffectComponent* owner_)
{
    owner = owner_;
}
}
#endif // __DAVAENGINE_PARTICLE_EMITTER_INSTANCE_H__

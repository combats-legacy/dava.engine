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


#include "Scene3D/SceneNodeAnimation.h"

namespace DAVA
{
SceneNodeAnimation::SceneNodeAnimation(uint32 _keyCount)
    : keyCount(_keyCount)
    , keys(new SceneNodeAnimationKey[_keyCount])
{
}

SceneNodeAnimation::~SceneNodeAnimation()
{
    SafeDeleteArray(keys);
}

void SceneNodeAnimation::SetKey(int32 index, const SceneNodeAnimationKey& key)
{
    keys[index] = key;
}

SceneNodeAnimationKey& SceneNodeAnimation::Intepolate(float32 t)
{
    if (keyCount == 1)
    {
        currentValue = keys[0];
        return currentValue;
    }

    if (t < keys[startIdx].time)
    {
        startIdx = 0;
    }

    uint32 endIdx = 0;
    for (endIdx = startIdx; endIdx < keyCount; ++endIdx)
    {
        if (keys[endIdx].time > t)
        {
            break;
        }
        startIdx = endIdx;
    }

    if (endIdx == keyCount)
    {
        currentValue = keys[keyCount - 1];
        return currentValue;
    }

    SceneNodeAnimationKey& key1 = keys[startIdx];
    SceneNodeAnimationKey& key2 = keys[endIdx];

    float32 tInter = (t - key1.time) / (key2.time - key1.time);

    currentValue.translation.Lerp(key1.translation, key2.translation, tInter);
    currentValue.rotation.Slerp(key1.rotation, key2.rotation, tInter);
    currentValue.scale.Lerp(key1.scale, key2.scale, tInter);
    //currentValue.matrix = key1.matrix;
    return currentValue;
}

void SceneNodeAnimation::SetDuration(float32 _duration)
{
    duration = _duration;
}

void SceneNodeAnimation::SetBindNode(Entity* _bindNode)
{
    bindNode = _bindNode;
}

void SceneNodeAnimation::SetBindName(const FastName& _bindName)
{
    bindName = _bindName;
}

void SceneNodeAnimation::SetInvPose(const Matrix4& mat)
{
    invPose = mat;
}
const Matrix4& SceneNodeAnimation::GetInvPose() const
{
    return invPose;
}

void SceneNodeAnimation::Update(float32 timeElapsed)
{
    delayTime -= timeElapsed;
    if (delayTime <= 0.0f)
    {
        delayTime = 0.0f;
        currentTime += timeElapsed + delayTime;
        if (currentTime > duration)
        {
            currentTime = duration;
            //bindNode->DetachAnimation(this);
        }
    }
}

void SceneNodeAnimation::Execute()
{
    DVASSERT(0);
    // 	startIdx = 0;
    // 	currentTime = 0;
    // 	bindNode->ExecuteAnimation(this);
}

Vector3 SceneNodeAnimation::SetStartPosition(const Vector3& position)
{
    Vector3 sPos = keys[0].translation;
    for (uint32 idx = 0; idx < keyCount; ++idx)
    {
        keys[idx].translation = position + keys[idx].translation - sPos;
    }
    return position - sPos;
}

void SceneNodeAnimation::ShiftStartPosition(const Vector3& shift)
{
    for (uint32 idx = 0; idx < keyCount; ++idx)
    {
        keys[idx].translation += shift;
    }
}
}

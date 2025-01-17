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


#ifndef __SCENE_COLLISION_LANDSCAPE_H__
#define __SCENE_COLLISION_LANDSCAPE_H__

#include "bullet/BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h"

#include "Scene/System/CollisionSystem/CollisionBaseObject.h"
#include "Render/Highlevel/Landscape.h"

class CollisionLandscape : public CollisionBaseObject
{
public:
    CollisionLandscape(DAVA::Entity* entity, btCollisionWorld* word, DAVA::Landscape* landscape);
    virtual ~CollisionLandscape();

    CollisionBaseObject::ClassifyPlaneResult ClassifyToPlane(const DAVA::Plane& plane) override;
    CollisionBaseObject::ClassifyPlanesResult ClassifyToPlanes(DAVA::Plane* plane, size_t numPlanes) override;

protected:
    btHeightfieldTerrainShape* btTerrain;
    DAVA::Vector<DAVA::float32> btHMap;
};

#endif // __SCENE_COLLISION_LANDSCAPE_H__

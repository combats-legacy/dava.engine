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
#include "ColladaDocument.h"
#include "ColladaPolygonGroup.h"
#include "CommandLine/CommandLineParser.h"
#include "Collada/ColladaToSc2Importer/ColladaToSc2Importer.h"

///*
// INCLUDE DevIL
// */
//#ifdef _UNICODE
//#undef _UNICODE
//#undef UNICODE
//#define UNICODE_DISABLED
//#endif
//
//#include <IL/il.h>
//#include <IL/ilu.h>
//
//#ifdef UNICODE_DISABLED
//#define _UNICODE
//#endif

namespace DAVA
{
eColladaErrorCodes ColladaDocument::Open(const char* filename)
{
    document = FCollada::NewTopDocument();
    bool val = FCollada::LoadDocumentFromFile(document, FUStringConversion::ToFString(filename));
    if (!val)
        return COLLADA_ERROR;

    FCDSceneNode* loadedRootNode = document->GetVisualSceneInstance();
    if (!loadedRootNode)
    {
        return COLLADA_ERROR_OF_ROOT_NODE;
    }

    colladaScene = new ColladaScene(loadedRootNode);

    FCDGeometryLibrary* geometryLibrary = document->GetGeometryLibrary();

    DAVA::Logger::FrameworkDebug("* Export geometry: %d\n", (int)geometryLibrary->GetEntityCount());
    for (int entityIndex = 0; entityIndex < (int)geometryLibrary->GetEntityCount(); ++entityIndex)
    {
        FCDGeometry* geometry = geometryLibrary->GetEntity(entityIndex);
        if (geometry->IsMesh())
        {
            FCDGeometryMesh* geometryMesh = geometry->GetMesh();
            colladaScene->colladaMeshes.push_back(new ColladaMesh(geometryMesh, 0));
        }
    }

    //	ilInit();
    //	iluInit();
    //
    //	ilEnable(IL_ORIGIN_SET);
    //	ilOriginFunc(IL_ORIGIN_LOWER_LEFT);

    FCDImageLibrary* imageLibrary = document->GetImageLibrary();
    for (int entityIndex = 0; entityIndex < (int)imageLibrary->GetEntityCount(); ++entityIndex)
    {
        FCDImage* texture = imageLibrary->GetEntity(entityIndex);
        colladaScene->colladaTextures.push_back(new ColladaTexture(texture));
    }
    //	ilShutDown();

    FCDLightLibrary* lightLibrary = document->GetLightLibrary();

    for (int entityIndex = 0; entityIndex < (int)lightLibrary->GetEntityCount(); ++entityIndex)
    {
        FCDLight* light = lightLibrary->GetEntity(entityIndex);
        colladaScene->colladaLights.push_back(new ColladaLight(light));
    }

    FCDMaterialLibrary* materialLibrary = document->GetMaterialLibrary();

    for (int entityIndex = 0; entityIndex < (int)materialLibrary->GetEntityCount(); ++entityIndex)
    {
        FCDMaterial* material = materialLibrary->GetEntity(entityIndex);
        colladaScene->colladaMaterials.push_back(new ColladaMaterial(colladaScene, material));
    }

    FCDCameraLibrary* cameraLibrary = document->GetCameraLibrary();
    DAVA::Logger::FrameworkDebug("Cameras:%d\n", cameraLibrary->GetEntityCount());

    for (int entityIndex = 0; entityIndex < (int)cameraLibrary->GetEntityCount(); ++entityIndex)
    {
        FCDCamera* cam = cameraLibrary->GetEntity(entityIndex);
        colladaScene->colladaCameras.push_back(new ColladaCamera(cam));
    }

    FCDAnimationLibrary* animationLibrary = document->GetAnimationLibrary();
    FCDAnimationClipLibrary* animationClipLibrary = document->GetAnimationClipLibrary();
    DAVA::Logger::FrameworkDebug("[A] Animations:%d Clips:%d\n", animationLibrary->GetEntityCount(), animationClipLibrary->GetEntityCount());

    FCDControllerLibrary* controllerLibrary = document->GetControllerLibrary();

    DAVA::Logger::FrameworkDebug("* Export animation controllers: %d\n", controllerLibrary->GetEntityCount());
    for (int entityIndex = 0; entityIndex < (int)controllerLibrary->GetEntityCount(); ++entityIndex)
    {
        FCDController* controller = controllerLibrary->GetEntity(entityIndex);
        colladaScene->colladaAnimatedMeshes.push_back(new ColladaAnimatedMesh(controller));
    }

    colladaScene->ExportScene();

    for (int k = 0; k < (int)colladaScene->colladaAnimatedMeshes.size(); ++k)
    {
        colladaScene->colladaAnimatedMeshes[k]->MarkJoints(colladaScene->rootNode);
    }

    ExportNodeAnimations(document, document->GetVisualSceneInstance());
    colladaScene->SetExclusiveAnimation(0);
    return COLLADA_OK;
}

bool ColladaDocument::ExportNodeAnimations(FCDocument* exportDoc, FCDSceneNode* exportNode)
{
    FCDAnimationLibrary* animationLibrary = exportDoc->GetAnimationLibrary();
    if (animationLibrary->GetEntityCount() == 0)
    {
        return false;
    }

    float32 timeStart, timeEnd;
    GetAnimationTimeInfo(exportDoc, timeStart, timeEnd);
    DAVA::Logger::FrameworkDebug("== Additional animation: %s start: %0.3f end: %0.3f\n ", exportDoc->GetFileUrl().c_str(), timeStart, timeEnd);

    FilePath fullPathName(exportDoc->GetFileUrl().c_str());
    String name = fullPathName.GetBasename();

    ColladaAnimation* anim = new ColladaAnimation();
    anim->name = name; //Format("animation:%d", colladaScene->colladaAnimations.size());
    anim->duration = timeEnd;
    colladaScene->ExportAnimations(anim, exportNode, timeStart, timeEnd);
    colladaScene->colladaAnimations.push_back(anim);
    return true;
}

bool ColladaDocument::ExportAnimations(const char* filename)
{
    FCDocument* exportDoc = FCollada::NewTopDocument();
    bool val = FCollada::LoadDocumentFromFile(exportDoc, FUStringConversion::ToFString(filename));
    if (!val)
    {
        DAVA::Logger::Error("*** Can't find file: %s\n", filename);
        return false;
    }
    FCDSceneNode* exportNode = exportDoc->GetVisualSceneInstance();
    return ExportNodeAnimations(exportDoc, exportNode);
    ;
}

void ColladaDocument::GetAnimationTimeInfo(FCDocument* document, float32& retTimeStart, float32& retTimeEnd)
{
    FCDAnimationLibrary* animationLibrary = document->GetAnimationLibrary();

    colladaScene->animationStartTime = 0.0f;
    colladaScene->animationEndTime = 0.0f;

    float32 timeMin, timeMax;
    for (int entityIndex = 0; entityIndex < (int)animationLibrary->GetEntityCount(); ++entityIndex)
    {
        FCDAnimation* anim = animationLibrary->GetEntity(entityIndex);
        // DAVA::Logger::FrameworkDebug("* Export animation: %d channelCount:%d\n", entityIndex, anim->GetChannelCount());

        timeMin = 10000000.0f;
        timeMax = 0.0f;

        for (int channelIndex = 0; channelIndex < (int)anim->GetChannelCount(); ++channelIndex)
        {
            FCDAnimationChannel* channel = anim->GetChannel(channelIndex);
            // DAVA::Logger::FrameworkDebug("- channel: %d curveCount: %d\n", channelIndex, channel->GetCurveCount());
            for (int curveIndex = 0; curveIndex < (int)channel->GetCurveCount(); ++curveIndex)
            {
                FCDAnimationCurve* curve = channel->GetCurve(curveIndex);
                // DAVA::Logger::FrameworkDebug("-- curve: %d target:%s\n", curveIndex, curve->GetTargetQualifier().c_str());
                for (int keyIndex = 0; keyIndex < (int)curve->GetKeyCount(); ++keyIndex)
                {
                    FCDAnimationKey* key = curve->GetKey(keyIndex);
                    if (key->input < timeMin)
                        timeMin = key->input;
                    if (key->input > timeMax)
                        timeMax = key->input;
                }
            }
        }
        colladaScene->animationStartTime = Min(timeMin, colladaScene->animationStartTime);
        colladaScene->animationEndTime = Max(timeMax, colladaScene->animationEndTime);
        //DAVA::Logger::FrameworkDebug("- timeMin: %f timeMax: %f\n", timeMin, timeMax);
    }

    retTimeStart = 0.0f;
    retTimeEnd = timeMax;
}

void ColladaDocument::Close()
{
    SAFE_RELEASE(document);
}

void ColladaDocument::Render()
{
    colladaScene->Render();
}

static const char* DEFAULT_COLLADA_EXTENSION = ".png";

String ColladaDocument::GetTextureName(const FilePath& scenePath, ColladaTexture* texture)
{
    FilePath texPathname(texture->texturePathName.c_str());
    DAVA::Logger::FrameworkDebug("+ get texture name: %s", texPathname.GetAbsolutePathname().c_str());

    auto extension = texPathname.GetExtension();
    if (!extension.empty())
    {
        if (!TextureDescriptor::IsSourceTextureExtension(extension) && !TextureDescriptor::IsCompressedTextureExtension(extension))
        {
            texPathname.ReplaceExtension(DEFAULT_COLLADA_EXTENSION);
        }
    }
    return texPathname.GetRelativePathname(scenePath);
}

eColladaErrorCodes ColladaDocument::SaveSC2(const FilePath& scenePath) const
{
    ColladaToSc2Importer importer;
    return importer.SaveSC2(colladaScene, scenePath);
}
};
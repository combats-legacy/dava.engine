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

#include "CommandLine/SceneSaver/SceneSaver.h"
#include "Deprecated/SceneValidator.h"

#include "Scene/SceneHelper.h"
#include "Project/ProjectManager.h"

#include "StringConstants.h"
#include "Main/QtUtils.h"

#include "FileSystem/FileList.h"
#include "Scene3D/Components/CustomPropertiesComponent.h"

using namespace DAVA;

SceneSaver::SceneSaver()
{
}

SceneSaver::~SceneSaver()
{
    ReleaseTextures();
}

void SceneSaver::SetInFolder(const FilePath& folderPathname)
{
    sceneUtils.SetInFolder(folderPathname);
}

void SceneSaver::SetOutFolder(const FilePath& folderPathname)
{
    sceneUtils.SetOutFolder(folderPathname);
}

void SceneSaver::EnableCopyConverted(bool enabled)
{
    copyConverted = enabled;
}

void SceneSaver::SaveFile(const String& fileName)
{
    Logger::FrameworkDebug("[SceneSaver::SaveFile] %s", fileName.c_str());

    FilePath filePath = sceneUtils.dataSourceFolder + fileName;

    //Load scene with *.sc2
    Scene* scene = new Scene();
    if (SceneFileV2::ERROR_NO_ERROR == scene->LoadScene(filePath))
    {
        SaveScene(scene, filePath);
    }
    else
    {
        Logger::Error("[SceneSaver::SaveFile] Can't open file %s", fileName.c_str());
    }

    SafeRelease(scene);
    RenderObjectsFlusher::Flush();
}

void SceneSaver::ResaveFile(const String& fileName)
{
    Logger::FrameworkDebug("[SceneSaver::ResaveFile] %s", fileName.c_str());

    FilePath sc2Filename = sceneUtils.dataSourceFolder + fileName;

    //Load scene with *.sc2
    Scene* scene = new Scene();
    if (SceneFileV2::ERROR_NO_ERROR == scene->LoadScene(sc2Filename))
    {
        scene->SaveScene(sc2Filename, false);
    }
    else
    {
        Logger::Error("[SceneSaver::ResaveFile] Can't open file %s", fileName.c_str());
    }

    SafeRelease(scene);
    RenderObjectsFlusher::Flush();
}

void SceneSaver::SaveScene(Scene* scene, const FilePath& fileName)
{
    uint64 startTime = SystemTimer::Instance()->AbsoluteMS();

    DVASSERT(0 == texturesForSave.size())

    String relativeFilename = fileName.GetRelativePathname(sceneUtils.dataSourceFolder);
    sceneUtils.workingFolder = fileName.GetDirectory().GetRelativePathname(sceneUtils.dataSourceFolder);

    FileSystem::Instance()->CreateDirectory(sceneUtils.dataFolder + sceneUtils.workingFolder, true);

    //scene->Update(0.1f);

    FilePath oldPath = SceneValidator::Instance()->SetPathForChecking(sceneUtils.dataSourceFolder);
    Set<String> dummy_needBeRefactored;
    SceneValidator::Instance()->ValidateScene(scene, fileName, dummy_needBeRefactored);

    {
        SceneHelper::TextureCollector collector(SceneHelper::TextureCollector::IncludeNullTextures);
        SceneHelper::EnumerateSceneTextures(scene, collector);
        texturesForSave = std::move(collector.GetTextures());
    }

    CopyTextures(scene);
    ReleaseTextures();

    Landscape* landscape = FindLandscape(scene);
    if (landscape)
    {
        sceneUtils.AddFile(landscape->GetHeightmapPathname());
    }

    VegetationRenderObject* vegetation = FindVegetation(scene);
    if (vegetation)
    {
        const FilePath vegetationCustomGeometry = vegetation->GetCustomGeometryPath();
        if (!vegetationCustomGeometry.IsEmpty())
        {
            sceneUtils.AddFile(vegetationCustomGeometry);
        }
    }

    CopyReferencedObject(scene);
    CopyEffects(scene);
    CopyCustomColorTexture(scene, fileName.GetDirectory());

    //save scene to new place
    FilePath tempSceneName = sceneUtils.dataSourceFolder + relativeFilename;
    tempSceneName.ReplaceExtension(".saved.sc2");

    sceneUtils.CopyFiles();
    scene->SaveScene(tempSceneName, false);

    bool moved = FileSystem::Instance()->MoveFile(tempSceneName, sceneUtils.dataFolder + relativeFilename, true);
    if (!moved)
    {
        Logger::Error("Can't move file %s", fileName.GetAbsolutePathname().c_str());
    }

    SceneValidator::Instance()->SetPathForChecking(oldPath);

    uint64 saveTime = SystemTimer::Instance()->AbsoluteMS() - startTime;
    Logger::FrameworkDebug("Save of %s to folder was done for %ldms", fileName.GetStringValue().c_str(), saveTime);
}

void SceneSaver::CopyTextures(DAVA::Scene* scene)
{
    TexturesMap::const_iterator endIt = texturesForSave.end();
    TexturesMap::iterator it = texturesForSave.begin();
    for (; it != endIt; ++it)
    {
        CopyTexture(it->first);
    }
}

void SceneSaver::ReleaseTextures()
{
    texturesForSave.clear();
}

void SceneSaver::CopyTexture(const FilePath& texturePathname)
{
    FilePath descriptorPathname = TextureDescriptor::GetDescriptorPathname(texturePathname);

    TextureDescriptor* desc = TextureDescriptor::CreateFromFile(descriptorPathname);
    if (!desc)
    {
        Logger::Error("Can't open file %s", descriptorPathname.GetAbsolutePathname().c_str());
        return;
    }

    //copy descriptor
    sceneUtils.AddFile(descriptorPathname);

    //copy source textures
    if (desc->IsCubeMap())
    {
        Vector<FilePath> faceNames;

        desc->GetFacePathnames(faceNames);
        for (auto& faceName : faceNames)
        {
            if (!faceName.IsEmpty())
                sceneUtils.AddFile(faceName);
        }
    }
    else
    {
        sceneUtils.AddFile(desc->GetSourceTexturePathname());
    }

    //copy converted textures (*.pvr and *.dds)
    if (copyConverted)
    {
        for (int32 i = 0; i < GPU_DEVICE_COUNT; ++i)
        {
            eGPUFamily gpu = (eGPUFamily)i;

            PixelFormat format = desc->GetPixelFormatForGPU(gpu);
            if (format == FORMAT_INVALID)
            {
                continue;
            }

            FilePath imagePathname = desc->CreatePathnameForGPU(gpu);
            sceneUtils.AddFile(imagePathname);
        }
    }

    delete desc;
}

void SceneSaver::CopyReferencedObject(Entity* node)
{
    KeyedArchive* customProperties = GetCustomPropertiesArchieve(node);
    if (customProperties && customProperties->IsKeyExists(ResourceEditor::EDITOR_REFERENCE_TO_OWNER))
    {
        String path = customProperties->GetString(ResourceEditor::EDITOR_REFERENCE_TO_OWNER);
        sceneUtils.AddFile(path);
    }
    for (int i = 0; i < node->GetChildrenCount(); i++)
    {
        CopyReferencedObject(node->GetChild(i));
    }
}

void SceneSaver::CopyEffects(Entity* node)
{
    ParticleEffectComponent* effect = GetEffectComponent(node);
    if (effect)
    {
        for (int32 i = 0, sz = effect->GetEmittersCount(); i < sz; ++i)
        {
            CopyAllParticlesEmitters(effect->GetEmitterInstance(i));
        }
    }

    for (int i = 0; i < node->GetChildrenCount(); ++i)
    {
        CopyEffects(node->GetChild(i));
    }

    for (auto it = effectFolders.begin(), endIt = effectFolders.end(); it != endIt; ++it)
    {
        FilePath flagsTXT = *it + "flags.txt";

        if (FileSystem::Instance()->Exists(flagsTXT))
        {
            sceneUtils.AddFile(flagsTXT);
        }
    }

    effectFolders.clear();
}

void SceneSaver::CopyAllParticlesEmitters(ParticleEmitterInstance* instance)
{
    const Set<FilePath>& paths = EnumAlternativeEmittersFilepaths(instance->GetFilePath());
    for (const FilePath& alternativeFilepath : paths)
    {
        auto emitter = instance->GetEmitter();
        if (alternativeFilepath == emitter->configPath)
        {
            CopyEmitter(emitter);
        }
        else
        {
            CopyEmitterByPath(alternativeFilepath);
        }
    }
}

void SceneSaver::CopyEmitterByPath(const FilePath& emitterConfigPath)
{
    RefPtr<ParticleEmitter> emitter(ParticleEmitter::LoadEmitter(emitterConfigPath));

    CopyEmitter(emitter.Get());
}

void SceneSaver::CopyEmitter(ParticleEmitter* emitter)
{
    if (emitter->configPath.IsEmpty() == false)
    {
        sceneUtils.AddFile(emitter->configPath);
    }
    else
    {
        Logger::Warning("[SceneSaver::CopyEmitter] empty config path for emitter %s", emitter->name.c_str());
    }

    const Vector<ParticleLayer*>& layers = emitter->layers;

    uint32 count = (uint32)layers.size();
    for (uint32 i = 0; i < count; ++i)
    {
        if (layers[i]->type == ParticleLayer::TYPE_SUPEREMITTER_PARTICLES)
        {
            CopyEmitter(layers[i]->innerEmitter);
        }
        else
        {
            Sprite* sprite = layers[i]->sprite;
            if (!sprite)
                continue;

            FilePath psdPath = ReplaceInString(sprite->GetRelativePathname().GetAbsolutePathname(), "/Data/", "/DataSource/");
            psdPath.ReplaceExtension(".psd");
            sceneUtils.AddFile(psdPath);

            effectFolders.insert(psdPath.GetDirectory());
        }
    }
}

Set<FilePath> SceneSaver::EnumAlternativeEmittersFilepaths(const FilePath& originalFilepath) const
{
    Set<FilePath> qualityFilepaths;
    const ParticlesQualitySettings& particlesSettings = QualitySettingsSystem::Instance()->GetParticlesQualitySettings();

    for (const ParticlesQualitySettings::QualitySheet& qualitySheet : particlesSettings.GetQualitySheets())
    {
        FilePath alternativeFilepath;
        if (qualitySheet.Apply(originalFilepath, alternativeFilepath))
        {
            if (FileSystem::Instance()->Exists(alternativeFilepath))
            {
                qualityFilepaths.insert(alternativeFilepath);
            }
        }
    }

    qualityFilepaths.insert(originalFilepath);

    return qualityFilepaths;
}

void SceneSaver::CopyCustomColorTexture(Scene* scene, const FilePath& sceneFolder)
{
    Entity* land = FindLandscapeEntity(scene);
    if (!land)
        return;

    KeyedArchive* customProps = GetCustomPropertiesArchieve(land);
    if (!customProps)
        return;

    String pathname = customProps->GetString(ResourceEditor::CUSTOM_COLOR_TEXTURE_PROP);
    if (pathname.empty())
        return;

    FilePath projectPath = ProjectManager::CreateProjectPathFromPath(sceneFolder);
    if (projectPath.IsEmpty())
    {
        Logger::Error("Can't copy custom colors texture (%s)", pathname.c_str());
        return;
    }

    FilePath texPathname = projectPath + pathname;
    sceneUtils.AddFile(texPathname);

    FilePath newTexPathname = sceneUtils.GetNewFilePath(texPathname);
    FilePath newProjectPathname = ProjectManager::CreateProjectPathFromPath(sceneUtils.dataFolder);
    if (newProjectPathname.IsEmpty())
    {
        Logger::Error("Can't save custom colors texture (%s)", pathname.c_str());
        return;
    }

    //save new path to custom colors texture
    customProps->SetString(ResourceEditor::CUSTOM_COLOR_TEXTURE_PROP, newTexPathname.GetRelativePathname(newProjectPathname));
}

void SceneSaver::ResaveYamlFilesRecursive(const FilePath& folder) const
{
    ScopedPtr<FileList> fileList(new FileList(folder));
    for (int32 i = 0; i < fileList->GetCount(); ++i)
    {
        const FilePath& pathname = fileList->GetPathname(i);
        if (fileList->IsDirectory(i))
        {
            if (!fileList->IsNavigationDirectory(i))
            {
                ResaveYamlFilesRecursive(pathname);
            }
        }
        else if (pathname.IsEqualToExtension(".yaml"))
        {
            ScopedPtr<ParticleEmitter> emitter(new ParticleEmitter());
            emitter->LoadFromYaml(pathname);
            emitter->SaveToYaml(pathname);
        }
    }
}

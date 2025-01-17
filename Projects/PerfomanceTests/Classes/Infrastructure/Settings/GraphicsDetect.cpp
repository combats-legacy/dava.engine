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

#include "GraphicsDetect.h"
#include "Platform/DeviceInfo.h"

using namespace DAVA;

GraphicsDetect::GraphicsDetect()
    : activeGroup(NULL)
{
    Init();
}

GraphicsDetect::~GraphicsDetect()
{
    activeGroup = NULL;
    for (auto i = groupsMap.begin(); i != groupsMap.end(); ++i)
    {
        SafeDelete(i->second.level);
    }
}

void GraphicsDetect::Init()
{
    const RefPtr<YamlParser> parser(YamlParser::Create("~res:/GraphicSettings/Detect.yaml"));

    if (!parser.Get())
        return;

    const YamlNode* rootNode = parser->GetRootNode();
    if (!rootNode)
        return;

    const auto& mm = rootNode->AsMap();
    for (auto i = mm.begin(); i != mm.end(); ++i)
    {
        Group group;
        if (i->second->GetType() == YamlNode::TYPE_MAP)
        {
            const auto& groupMap = i->second->AsMap();
            auto fit = groupMap.find("base");
            if (fit != groupMap.end())
                group.base = fit->second->AsString();
            fit = groupMap.find("devices");
            if (fit != groupMap.end() && fit->second->GetType() == YamlNode::TYPE_ARRAY)
            {
                const auto& array = fit->second->AsVector();
                for (auto arrIt = array.begin(); arrIt != array.end(); ++arrIt)
                    group.devices.insert((*arrIt)->AsString());
            }
            group.level = new GraphicsLevel(group.base, i->second);
            groupsMap[i->first] = group;
        }
    }
}

DAVA::String GraphicsDetect::GetLevelString()
{
    return activeGroup->base;
}

GraphicsDetect::Group* GraphicsDetect::GetAutoDetected()
{
    const String& platform = DeviceInfo::GetPlatformString();
    const String& model = DeviceInfo::GetModel();
    Logger::Info("AutoDetect platform '%s' model '%s'", platform.c_str(), model.c_str());
    Group* result = 0;
    for (auto i = groupsMap.begin(); i != groupsMap.end(); ++i)
    {
        if (i->second.devices.find(model) != i->second.devices.end())
        {
            result = &(i->second);
            Logger::Info("Selected group '%s' base settings '%s'", i->first.c_str(), result->base.c_str());
            break;
        }
    }
    if (result == 0)
    {
        auto it = groupsMap.find(platform);
        if (it != groupsMap.end())
        {
            result = &(groupsMap[platform]);
            Logger::Info("Selected group '%s' base settings '%s'", it->first.c_str(), result->base.c_str());
        }
    }
    if (result == 0)
    {
        result = &(groupsMap["default"]);
        Logger::Info("needed group not found base settings '%s'", result->base.c_str());
    }
    return result;
}

void GraphicsDetect::ReloadSettings()
{
    activeGroup = GetAutoDetected();
    activeGroup->level->Activate();
}

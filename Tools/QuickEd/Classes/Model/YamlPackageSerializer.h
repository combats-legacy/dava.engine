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


#ifndef __QUICKED_YAML_PACKAGE_SERIALIZER_H__
#define __QUICKED_YAML_PACKAGE_SERIALIZER_H__

#include "PackageSerializer.h"

namespace DAVA
{
class YamlNode;
}

class YamlPackageSerializer : public PackageSerializer
{
public:
    YamlPackageSerializer();
    virtual ~YamlPackageSerializer();

    virtual void PutValue(const DAVA::String& name, const DAVA::VariantType& value) override;
    virtual void PutValue(const DAVA::String& name, const DAVA::String& value) override;
    virtual void PutValue(const DAVA::String& name, const DAVA::Vector<DAVA::String>& value) override;
    virtual void PutValue(const DAVA::VariantType& value) override;
    virtual void PutValue(const DAVA::String& value) override;

    virtual void BeginMap() override;
    virtual void BeginMap(const DAVA::String& name, bool quotes = false) override;
    virtual void EndMap() override;

    virtual void BeginArray() override;
    virtual void BeginArray(const DAVA::String& name, bool flow = false) override;
    virtual void EndArray() override;

    DAVA::YamlNode* GetYamlNode() const;
    bool WriteToFile(const DAVA::FilePath& path);
    DAVA::String WriteToString() const;

private:
    DAVA::Vector<DAVA::YamlNode*> nodesStack;
};


#endif // __QUICKED_YAML_PACKAGE_SERIALIZER_H__

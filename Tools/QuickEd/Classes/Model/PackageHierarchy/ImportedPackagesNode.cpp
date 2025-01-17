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


#include "ImportedPackagesNode.h"

#include "PackageControlsNode.h"
#include "PackageNode.h"
#include "PackageVisitor.h"

using namespace DAVA;

ImportedPackagesNode::ImportedPackagesNode(PackageBaseNode* parent)
    : PackageBaseNode(parent)
{
}

ImportedPackagesNode::~ImportedPackagesNode()
{
    for (PackageNode* package : packages)
        package->Release();
    packages.clear();
}

void ImportedPackagesNode::Add(PackageNode* node)
{
    DVASSERT(node->GetParent() == NULL);
    node->SetParent(this);
    packages.push_back(SafeRetain(node));
}

void ImportedPackagesNode::InsertAtIndex(DAVA::int32 index, PackageNode* node)
{
    DVASSERT(node->GetParent() == NULL);
    node->SetParent(this);

    packages.insert(packages.begin() + index, SafeRetain(node));
}

void ImportedPackagesNode::Remove(PackageNode* node)
{
    auto it = find(packages.begin(), packages.end(), node);
    if (it != packages.end())
    {
        DVASSERT(node->GetParent() == this);
        node->SetParent(NULL);

        packages.erase(it);
        SafeRelease(node);
    }
    else
    {
        DVASSERT(false);
    }
}

PackageNode* ImportedPackagesNode::GetImportedPackage(DAVA::int32 index) const
{
    return packages[index];
}

int ImportedPackagesNode::GetCount() const
{
    return static_cast<int>(packages.size());
}

PackageBaseNode* ImportedPackagesNode::Get(int index) const
{
    return packages[index];
}

void ImportedPackagesNode::Accept(PackageVisitor* visitor)
{
    visitor->VisitImportedPackages(this);
}

String ImportedPackagesNode::GetName() const
{
    return "Imported Packages";
}

bool ImportedPackagesNode::IsInsertingPackagesSupported() const
{
    return !IsReadOnly();
}

bool ImportedPackagesNode::CanInsertImportedPackage(PackageNode* package) const
{
    if (package->FindPackageInImportedPackagesRecursively(GetPackage()))
        return false;
    return true;
}

PackageNode* ImportedPackagesNode::FindPackageByName(const DAVA::String& name) const
{
    for (PackageNode* node : packages)
    {
        if (node->GetName() == name)
            return node;
    }
    return nullptr;
}

bool ImportedPackagesNode::IsReadOnly() const
{
    return GetParent() != nullptr ? GetParent()->IsReadOnly() : true;
}

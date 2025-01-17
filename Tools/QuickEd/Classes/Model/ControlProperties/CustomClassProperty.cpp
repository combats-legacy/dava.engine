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


#include "CustomClassProperty.h"

#include "PropertyVisitor.h"
#include "../PackageHierarchy/ControlNode.h"

#include "UI/UIControl.h"

using namespace DAVA;

CustomClassProperty::CustomClassProperty(ControlNode* aControl, const CustomClassProperty* sourceProperty, eCloneType cloneType)
    : ValueProperty("Custom Class", VariantType::TYPE_STRING)
    , control(aControl) // weak
{
    if (sourceProperty)
    {
        if (cloneType == CT_COPY)
        {
            SetOverridden(sourceProperty->IsOverriddenLocally());
            SetDefaultValue(sourceProperty->GetDefaultValue());
        }
        else
        {
            AttachPrototypeProperty(sourceProperty);
            SetDefaultValue(sourceProperty->GetValue());
        }
        customClass = sourceProperty->customClass;
    }
    else
    {
        SetDefaultValue(VariantType(String("")));
    }
}

CustomClassProperty::~CustomClassProperty()
{
    control = nullptr; //weak
}

void CustomClassProperty::Accept(PropertyVisitor* visitor)
{
    visitor->VisitCustomClassProperty(this);
}

bool CustomClassProperty::IsReadOnly() const
{
    return control->GetCreationType() == ControlNode::CREATED_FROM_PROTOTYPE_CHILD || ValueProperty::IsReadOnly();
}

CustomClassProperty::ePropertyType CustomClassProperty::GetType() const
{
    return TYPE_VARIANT;
}

VariantType CustomClassProperty::GetValue() const
{
    return VariantType(customClass);
}

const String& CustomClassProperty::GetCustomClassName() const
{
    return customClass;
}

void CustomClassProperty::ApplyValue(const DAVA::VariantType& value)
{
    customClass = value.AsString();
}

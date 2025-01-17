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


#ifndef __QUICKED_CHANGE_PROPERTY_VALUE_COMMAND_H__
#define __QUICKED_CHANGE_PROPERTY_VALUE_COMMAND_H__

#include <QUndoCommand>
#include "FileSystem/VariantType.h"
#include "EditorSystems/EditorSystemsManager.h"

class PackageNode;
class ControlNode;
class AbstractProperty;

class ChangePropertyValueCommand : public QUndoCommand
{
public:
    ChangePropertyValueCommand(PackageNode* _root, const DAVA::Vector<ChangePropertyAction>& propertyActions, size_t hash = 0, QUndoCommand* parent = nullptr);

    ChangePropertyValueCommand(PackageNode* _root, ControlNode* _node, AbstractProperty* _property, const DAVA::VariantType& newValue, size_t hash = 0, QUndoCommand* parent = nullptr);
    ChangePropertyValueCommand(PackageNode* _root, ControlNode* _node, AbstractProperty* _property, QUndoCommand* parent = nullptr);
    virtual ~ChangePropertyValueCommand();

    void redo() override;
    void undo() override;

    int id() const override;

    bool mergeWith(const QUndoCommand* other) override;

private:
    struct PropertyActionWithOldValue
    {
        PropertyActionWithOldValue(ControlNode* node_, AbstractProperty* property_, const DAVA::VariantType& oldValue_, const DAVA::VariantType& newValue_)
            : node(node_)
            , property(property_)
            , oldValue(oldValue_)
            , newValue(newValue_)
        {
        }
        ControlNode* node = nullptr;
        AbstractProperty* property = nullptr;
        DAVA::VariantType oldValue;
        DAVA::VariantType newValue;
    };
    void Init();
    DAVA::VariantType GetValueFromProperty(AbstractProperty* property);
    PackageNode* root = nullptr;

    DAVA::Vector<PropertyActionWithOldValue> changedProperties;

    size_t hash = 0;
};

#endif // __QUICKED_CHANGE_PROPERTY_VALUE_COMMAND_H__

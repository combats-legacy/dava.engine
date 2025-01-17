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


#include "DAVAEngine.h"
#include "Debug/DVAssert.h"
#include "Main/QtUtils.h"
#include "QtPropertyDataDavaKeyedArchive.h"
#include "QtPropertyDataKeyedArchiveMember.h"
#include "Deprecated/EditorConfig.h"

#include <QSet>
#include <QMenu>
#include <QGridLayout>
#include <QAction>
#include <QLabel>
#include <QMessageBox>
#include <QKeyEvent>

#include "QtTools/WidgetHelpers/SharedIcon.h"

QtPropertyDataDavaKeyedArcive::QtPropertyDataDavaKeyedArcive(const DAVA::FastName& name, DAVA::KeyedArchive* _archive)
    : QtPropertyData(name)
    , archive(_archive)
    , lastCommand(NULL)
    , lastAddedType(DAVA::VariantType::TYPE_STRING)
{
    if (NULL != archive)
    {
        archive->Retain();
    }

    SetEnabled(false);
}

QtPropertyDataDavaKeyedArcive::~QtPropertyDataDavaKeyedArcive()
{
    if (NULL != archive)
    {
        archive->Release();
    }

    if (NULL != lastCommand)
    {
        delete lastCommand;
    }
}

const DAVA::MetaInfo* QtPropertyDataDavaKeyedArcive::MetaInfo() const
{
    return DAVA::MetaInfo::Instance<DAVA::KeyedArchive*>();
}

QVariant QtPropertyDataDavaKeyedArcive::GetValueInternal() const
{
    QVariant v;

    if (NULL != archive)
    {
        v = QString("KeyedArchive");
    }
    else
    {
        v = QString("KeyedArchive [NULL]");
    }

    return v;
}

bool QtPropertyDataDavaKeyedArcive::UpdateValueInternal()
{
    // update children
    {
        QSet<QtPropertyData*> dataToRemove;

        // at first step of sync we mark (placing to vector) items to remove
        for (int i = 0; i < ChildCount(); ++i)
        {
            QtPropertyData* child = ChildGet(i);
            DVASSERT(child != nullptr);
            dataToRemove.insert(child);
        }

        // as second step we go through keyed archive and add new data items,
        // and remove deleting mark from items that are still in archive
        if (NULL != archive)
        {
            DAVA::KeyedArchive::UnderlyingMap data = archive->GetArchieveData();
            DAVA::KeyedArchive::UnderlyingMap::iterator i = data.begin();

            for (; i != data.end(); ++i)
            {
                DAVA::FastName fieldName = DAVA::FastName(i->first);
                QtPropertyData* childData = ChildGet(fieldName);

                // this key already in items list
                if (childData != nullptr)
                {
                    // remove deleting mark
                    dataToRemove.remove(childData);
                }
                // create new child data
                else
                {
                    ChildCreate(fieldName, i->second);
                }
            }
        }

        // delete all marked items
        QSetIterator<QtPropertyData*> it(dataToRemove);
        while (it.hasNext())
        {
            ChildRemove(it.next());
        }
    }

    return false;
}

void QtPropertyDataDavaKeyedArcive::ChildCreate(const DAVA::FastName& name, DAVA::VariantType* value)
{
    QtPropertyData* childData = NULL;

    if (value->type == DAVA::VariantType::TYPE_KEYED_ARCHIVE)
    {
        childData = new QtPropertyDataDavaKeyedArcive(name, value->AsKeyedArchive());
    }
    else
    {
        childData = new QtPropertyKeyedArchiveMember(name, archive, name.c_str());
    }

    ChildAdd(std::unique_ptr<QtPropertyData>(childData));

    // add optional widget (button) to remove this key
    QToolButton* remButton = childData->AddButton();
    remButton->setIcon(SharedIcon(":/QtIcons/keyminus.png"));
    remButton->setToolTip("Remove keyed archive member");
    remButton->setIconSize(QSize(12, 12));

    connections.AddConnection(remButton, &QToolButton::clicked, [this, remButton]() {
        RemKeyedArchiveField(remButton);
    });
}

void QtPropertyDataDavaKeyedArcive::AddKeyedArchiveField(QToolButton* button)
{
    DVASSERT(button != nullptr);
    if (archive != nullptr)
    {
        KeyedArchiveItemWidget* w = new KeyedArchiveItemWidget(archive, lastAddedType, GetOWViewport());
        connections.AddConnection(w, &KeyedArchiveItemWidget::ValueReady, [this](const DAVA::String& key, const DAVA::VariantType& value) {
            ForeachMergedItem([&key, &value](QtPropertyData* item) {
                DVASSERT(dynamic_cast<QtPropertyDataDavaKeyedArcive*>(item) != nullptr);
                QtPropertyDataDavaKeyedArcive* arciveItem = static_cast<QtPropertyDataDavaKeyedArcive*>(item);
                arciveItem->NewKeyedArchiveFieldReady(key, value);
                return true;
            });
            NewKeyedArchiveFieldReady(key, value);
        });

        w->show();

        QRect bRect = button->geometry();
        QPoint bPos = button->mapToGlobal(button->mapFromParent(bRect.topLeft()));

        QRect wRect = w->geometry();
        QPoint wPos = QPoint(bPos.x() - wRect.width() + bRect.width(), bPos.y() + bRect.height());

        w->move(wPos);
    }
}

void QtPropertyDataDavaKeyedArcive::RemKeyedArchiveField(QToolButton* button)
{
    DVASSERT(button != nullptr);
    if (archive != nullptr)
    {
        // search for child data with such button
        for (int i = 0; i < ChildCount(); ++i)
        {
            QtPropertyData* childData = ChildGet(i);
            DVASSERT(childData != nullptr);
            // search btn thought this child optional widgets
            for (int j = 0; j < childData->GetButtonsCount(); j++)
            {
                if (button == childData->GetButton(j))
                {
                    DAVA::FastName key = childData->GetName();
                    ForeachMergedItem([&key](QtPropertyData* item) {
                        DVASSERT(dynamic_cast<QtPropertyDataDavaKeyedArcive*>(item) != nullptr);
                        QtPropertyDataDavaKeyedArcive* arciveItem = static_cast<QtPropertyDataDavaKeyedArcive*>(item);
                        arciveItem->RemKeyedArchiveField(key);
                        return true;
                    });
                    RemKeyedArchiveField(key);
                    break;
                }
            }
        }
    }
}

void QtPropertyDataDavaKeyedArcive::RemKeyedArchiveField(const DAVA::FastName& key)
{
    if (archive != nullptr)
    {
        if (lastCommand)
        {
            delete lastCommand;
        }

        QtPropertyData* child = ChildGet(key);

        if (child)
        {
            lastCommand = new KeyeadArchiveRemValueCommand(archive, key.c_str());
            ChildRemove(child);
            EmitDataChanged(QtPropertyData::VALUE_EDITED);
        }
    }
}

void QtPropertyDataDavaKeyedArcive::NewKeyedArchiveFieldReady(const DAVA::String& key, const DAVA::VariantType& value)
{
    DVASSERT(value.type != DAVA::VariantType::TYPE_NONE && value.type < DAVA::VariantType::TYPES_COUNT);
    if (NULL != archive)
    {
        archive->SetVariant(key, value);
        lastAddedType = value.type;

        if (NULL != lastCommand)
        {
            delete lastCommand;
        }

        lastCommand = new KeyedArchiveAddValueCommand(archive, key, value);
        ChildCreate(DAVA::FastName(key.c_str()), archive->GetVariant(key));
        EmitDataChanged(QtPropertyData::VALUE_EDITED);
    }
}

Command2::Pointer QtPropertyDataDavaKeyedArcive::CreateLastCommand() const
{
    if (nullptr != lastCommand)
    {
        if (CMDID_KEYEDARCHIVE_REM_KEY == lastCommand->GetId())
        {
            return Command2::Create<KeyeadArchiveRemValueCommand>(*((KeyeadArchiveRemValueCommand*)lastCommand));
        }
        else if (CMDID_KEYEDARCHIVE_ADD_KEY == lastCommand->GetId())
        {
            return Command2::Create<KeyedArchiveAddValueCommand>(*((KeyedArchiveAddValueCommand*)lastCommand));
        }
    }

    return Command2::CreateEmptyCommand();
}

void QtPropertyDataDavaKeyedArcive::FinishTreeCreation()
{
    // add optional widget (button) to add new key
    QToolButton* addButton = AddButton();
    addButton->setIcon(SharedIcon(":/QtIcons/keyplus.png"));
    addButton->setToolTip("Add keyed archive member");
    addButton->setIconSize(QSize(12, 12));
    connections.AddConnection(addButton, &QToolButton::clicked, [this, addButton]() {
        AddKeyedArchiveField(addButton);
    });

    UpdateValue();

    QtPropertyData::FinishTreeCreation();
}

KeyedArchiveItemWidget::KeyedArchiveItemWidget(DAVA::KeyedArchive* _arch, int defaultType, QWidget* parent /* = NULL */)
    : QWidget(parent)
    , arch(_arch)
    , presetWidget(NULL)
{
    QGridLayout* grLayout = new QGridLayout();
    int delautTypeIndex = 0;

    if (NULL != arch)
    {
        arch->Retain();
    }

    defaultBtn = new QPushButton("Ok", this);
    keyWidget = new QLineEdit(this);
    valueWidget = new QComboBox(this);

    int j = 0;
    for (int type = (DAVA::VariantType::TYPE_NONE + 1); type < DAVA::VariantType::TYPES_COUNT; type++)
    {
        // don't allow byte array
        if (type != DAVA::VariantType::TYPE_BYTE_ARRAY)
        {
            valueWidget->addItem(DAVA::VariantType::variantNamesMap[type].variantName.c_str(), type);

            if (type == defaultType)
            {
                delautTypeIndex = j;
            }

            j++;
        }
    }
    valueWidget->setCurrentIndex(delautTypeIndex);

    int row = 0;
    grLayout->addWidget(new QLabel("Key:", this), row, 0, 1, 1);
    grLayout->addWidget(keyWidget, row, 1, 1, 2);
    grLayout->addWidget(new QLabel("Value type:", this), ++row, 0, 1, 1);
    grLayout->addWidget(valueWidget, row, 1, 1, 2);

    const DAVA::Vector<DAVA::String>& presetValues = EditorConfig::Instance()->GetProjectPropertyNames();
    if (presetValues.size() > 0)
    {
        presetWidget = new QComboBox(this);

        presetWidget->addItem("None", DAVA::VariantType::TYPE_NONE);
        for (size_t i = 0; i < presetValues.size(); ++i)
        {
            presetWidget->addItem(presetValues[i].c_str(), EditorConfig::Instance()->GetPropertyValueType(presetValues[i]));
        }

        grLayout->addWidget(new QLabel("Preset:", this), ++row, 0, 1, 1);
        grLayout->addWidget(presetWidget, row, 1, 1, 2);

        QObject::connect(presetWidget, SIGNAL(activated(int)), this, SLOT(PreSetSelected(int)));
    }
    presetWidget->setMaxVisibleItems(presetWidget->count());

    grLayout->addWidget(defaultBtn, ++row, 2, 1, 1);

    grLayout->setMargin(5);
    grLayout->setSpacing(3);
    setLayout(grLayout);

    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);
    setWindowOpacity(0.95);

    QObject::connect(defaultBtn, SIGNAL(pressed()), this, SLOT(OkKeyPressed()));
}

KeyedArchiveItemWidget::~KeyedArchiveItemWidget()
{
    if (NULL != arch)
    {
        arch->Release();
    }
}

void KeyedArchiveItemWidget::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    keyWidget->setFocus();
}

void KeyedArchiveItemWidget::keyPressEvent(QKeyEvent* e)
{
    if (!e->modifiers() || (e->modifiers() & Qt::KeypadModifier && e->key() == Qt::Key_Enter))
    {
        switch (e->key())
        {
        case Qt::Key_Enter:
        case Qt::Key_Return:
            defaultBtn->click();
            break;
        case Qt::Key_Escape:
            this->deleteLater();
            break;
        default:
            e->ignore();
            return;
        }
    }
    else
    {
        e->ignore();
    }
}

void KeyedArchiveItemWidget::OkKeyPressed()
{
    if (NULL != arch)
    {
        DAVA::String key = keyWidget->text().toStdString();

        if (key.empty())
        {
            // TODO:
            // other way to report error without losing focus
            // ...
            //

            QMessageBox::warning(NULL, "Wrong key value", "Key value can't be empty");
        }
        else if (arch->IsKeyExists(key))
        {
            // TODO:
            // other way to report error without losing focus
            // ...
            //

            QMessageBox::warning(NULL, "Wrong key value", "That key already exists");
        }
        else
        {
            // preset?
            int presetType = DAVA::VariantType::TYPE_NONE;
            if (NULL != presetWidget)
            {
                presetType = presetWidget->itemData(presetWidget->currentIndex()).toInt();
            }

            if (DAVA::VariantType::TYPE_NONE != presetType)
            {
                DAVA::VariantType presetValue = *(EditorConfig::Instance()->GetPropertyDefaultValue(key));
                emit ValueReady(key, presetValue);
            }
            else
            {
                emit ValueReady(key, DAVA::VariantType::FromType(valueWidget->itemData(valueWidget->currentIndex()).toInt()));
            }

            this->deleteLater();
        }
    }
    else
    {
        this->deleteLater();
    }
}

void KeyedArchiveItemWidget::PreSetSelected(int index)
{
    if (presetWidget->itemData(index).toInt() != DAVA::VariantType::TYPE_NONE)
    {
        keyWidget->setText(presetWidget->itemText(index));
        keyWidget->setEnabled(false);
        valueWidget->setEnabled(false);
    }
    else
    {
        keyWidget->setText("");
        keyWidget->setEnabled(true);
        valueWidget->setEnabled(true);
    }
}
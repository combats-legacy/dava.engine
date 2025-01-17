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


#include "RunActionEventWidget.h"

#include <QStringList>
#include <QSet>
#include <QCompleter>

#include "ui_RunActionEventWidget.h"

#include "DAVAEngine.h"
#include "Scene3D/Components/ActionComponent.h"

#include "Classes/Qt/Main/mainwindow.h"
#include "Scene/SceneEditor2.h"
#include "Settings/SettingsManager.h"

namespace
{
const DAVA::String settingsType("Internal/RunActionEventWidget/CurrentType");
}

RunActionEventWidget::RunActionEventWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::RunActionEventWidget())
    , scene(NULL)
{
    ui->setupUi(this);
    setObjectName("RunActionEventWidget");

    ui->eventType->addItem("Switch", DAVA::ActionComponent::Action::EVENT_SWITCH_CHANGED);
    ui->eventType->addItem("Added", DAVA::ActionComponent::Action::EVENT_ADDED_TO_SCENE);
    ui->eventType->addItem("User", DAVA::ActionComponent::Action::EVENT_CUSTOM);

    editorIdMap[DAVA::ActionComponent::Action::EVENT_SWITCH_CHANGED] = 0;
    editorIdMap[DAVA::ActionComponent::Action::EVENT_ADDED_TO_SCENE] = 1;
    editorIdMap[DAVA::ActionComponent::Action::EVENT_CUSTOM] = 2;

    autocompleteModel = new QStringListModel(this);
    ui->name->setModel(autocompleteModel);
    if (ui->name->completer())
    {
        ui->name->completer()->setCompletionMode(QCompleter::PopupCompletion);
    }

    connect(ui->eventType, SIGNAL(currentIndexChanged(int)), SLOT(OnTypeChanged()));
    connect(ui->run, SIGNAL(clicked()), SLOT(OnInvoke()));
    connect(SceneSignals::Instance(), SIGNAL(SelectionChanged(SceneEditor2*, const SelectableGroup*, const SelectableGroup*)), this, SLOT(sceneSelectionChanged(SceneEditor2*, const SelectableGroup*, const SelectableGroup*)));
    connect(SceneSignals::Instance(), SIGNAL(Activated(SceneEditor2*)), this, SLOT(sceneActivated(SceneEditor2*)));

    const DAVA::ActionComponent::Action::eEvent eventType = static_cast<DAVA::ActionComponent::Action::eEvent>(SettingsManager::Instance()->GetValue(settingsType).AsUInt32());
    ui->eventType->setCurrentIndex(editorIdMap[eventType]);
}

RunActionEventWidget::~RunActionEventWidget()
{
}

void RunActionEventWidget::OnTypeChanged()
{
    const DAVA::uint32 eventTypeId = ui->eventType->itemData(ui->eventType->currentIndex()).toUInt();
    const int editorindex = editorIdMap[eventTypeId];
    DVASSERT(editorindex < ui->stackedWidget->count());

    ui->stackedWidget->setCurrentIndex(editorindex);
    SettingsManager::Instance()->SetValue(settingsType, DAVA::VariantType(eventTypeId));
}

void RunActionEventWidget::OnInvoke()
{
    const uint eventTypeId = ui->eventType->itemData(ui->eventType->currentIndex()).toUInt();
    SceneEditor2* editor = QtMainWindow::Instance()->GetCurrentScene();
    if (editor == NULL)
        return;

    const DAVA::uint32 switchIndex = ui->switchIndex->value();
    const DAVA::FastName name(ui->name->currentText().toStdString().c_str());

    const SelectableGroup& selection = editor->selectionSystem->GetSelection();
    for (auto entity : selection.ObjectsOfType<DAVA::Entity>())
    {
        DAVA::ActionComponent* component = static_cast<DAVA::ActionComponent*>(entity->GetComponent(DAVA::Component::ACTION_COMPONENT));
        if (component == nullptr)
            continue;

        const DAVA::uint32 nEvents = component->GetCount();
        for (DAVA::uint32 componentIdx = 0; componentIdx < nEvents; componentIdx++)
        {
            DAVA::ActionComponent::Action& act = component->Get(componentIdx);
            if (act.eventType == eventTypeId)
            {
                switch (eventTypeId)
                {
                case DAVA::ActionComponent::Action::EVENT_SWITCH_CHANGED:
                    if (act.switchIndex == switchIndex)
                    {
                        component->StartSwitch(switchIndex);
                    }
                    break;
                case DAVA::ActionComponent::Action::EVENT_ADDED_TO_SCENE:
                    component->StartAdd();
                    break;
                case DAVA::ActionComponent::Action::EVENT_CUSTOM:
                    if (act.userEventId == name)
                    {
                        component->StartUser(name);
                    }
                    break;
                default:
                    break;
                }
            }
        }
    }
}

void RunActionEventWidget::sceneActivated(SceneEditor2* scene_)
{
    scene = scene_;
    sceneSelectionChanged(scene, NULL, NULL);
}

void RunActionEventWidget::sceneSelectionChanged(SceneEditor2* scene_, const SelectableGroup* selected, const SelectableGroup* deselected)
{
    Q_UNUSED(selected);
    Q_UNUSED(deselected);

    if (scene == nullptr)
    {
        autocompleteModel->setStringList(QStringList());
        return;
    }

    if (scene != scene_)
    {
        return;
    }

    QSet<QString> nameSet;

    const SelectableGroup& selection = scene->selectionSystem->GetSelection();
    for (auto entity : selection.ObjectsOfType<DAVA::Entity>())
    {
        DAVA::ActionComponent* component = static_cast<DAVA::ActionComponent*>(entity->GetComponent(DAVA::Component::ACTION_COMPONENT));
        if (component == nullptr)
            continue;

        const DAVA::uint32 nEvents = component->GetCount();
        for (DAVA::uint32 componentIdx = 0; componentIdx < nEvents; componentIdx++)
        {
            DAVA::ActionComponent::Action& act = component->Get(componentIdx);
            if (act.eventType == DAVA::ActionComponent::Action::EVENT_CUSTOM)
            {
                nameSet.insert(QString(act.userEventId.c_str()));
            }
        }
    }

    autocompleteModel->setStringList(nameSet.toList());
    setEnabled(!selection.IsEmpty());
}

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


#include "Input/InputSystem.h"
#include "Input/KeyboardDevice.h"
#include "EditorSystems/SelectionSystem.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "UI/UIEvent.h"
#include "UI/UIControl.h"
#include "EditorSystems/EditorSystemsManager.h"
#include "EditorSystems/KeyboardProxy.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"
#include "Model/ControlProperties/RootProperty.h"
#include "Model/ControlProperties/VisibleValueProperty.h"

using namespace DAVA;

SelectionSystem::SelectionSystem(EditorSystemsManager* parent)
    : BaseEditorSystem(parent)
{
    systemManager->SelectionChanged.Connect(this, &SelectionSystem::OnSelectionChanged);
    systemManager->PackageNodeChanged.Connect(this, &SelectionSystem::OnPackageNodeChanged);
    systemManager->SelectionRectChanged.Connect(this, &SelectionSystem::OnSelectByRect);
}

SelectionSystem::~SelectionSystem() = default;

bool SelectionSystem::OnInput(UIEvent* currentInput)
{
    switch (currentInput->phase)
    {
    case UIEvent::Phase::BEGAN:
        mousePressed = true;
        return ProcessMousePress(currentInput->point, currentInput->mouseButton);
    case UIEvent::Phase::ENDED:
        if (!mousePressed)
        {
            return ProcessMousePress(currentInput->point, currentInput->mouseButton);
        }
        mousePressed = false;
        return false;
    default:
        return false;
    }
    return false;
}

void SelectionSystem::OnPackageNodeChanged(PackageNode* packageNode_)
{
    if (nullptr != packageNode)
    {
        packageNode->RemoveListener(this);
    }
    packageNode = packageNode_;
    if (nullptr != packageNode)
    {
        packageNode->AddListener(this);
    }
}

void SelectionSystem::ControlWasRemoved(ControlNode* node, ControlsContainerNode*)
{
    SelectedNodes deselected;
    deselected.insert(node);
    SetSelection(SelectedNodes(), deselected);
}

void SelectionSystem::OnSelectByRect(const Rect& rect)
{
    SelectedNodes deselected;
    SelectedNodes selected;
    Set<ControlNode*> areaNodes;
    auto predicate = [rect](const ControlNode* node) -> bool {
        const auto control = node->GetControl();
        DVASSERT(nullptr != control);
        return control->GetVisibilityFlag() && rect.RectContains(control->GetGeometricData().GetAABBox());
    };
    auto stopPredicate = [](const ControlNode* node) -> bool {
        const auto control = node->GetControl();
        DVASSERT(nullptr != control);
        return !control->GetVisibilityFlag();
    };
    systemManager->CollectControlNodes(std::inserter(areaNodes, areaNodes.end()), predicate, stopPredicate);
    if (!areaNodes.empty())
    {
        for (auto node : areaNodes)
        {
            selected.insert(node);
        }
    }
    if (!IsKeyPressed(KeyboardProxy::KEY_SHIFT))
    {
        //deselect all not selected by rect
        std::set_difference(selectionContainer.selectedNodes.begin(), selectionContainer.selectedNodes.end(), areaNodes.begin(), areaNodes.end(), std::inserter(deselected, deselected.end()));
    }
    SetSelection(selected, deselected);
}

void SelectionSystem::ClearSelection()
{
    SetSelection(SelectedNodes(), selectionContainer.selectedNodes);
}

void SelectionSystem::SelectAllControls()
{
    SelectedNodes selected;
    systemManager->CollectControlNodes(std::inserter(selected, selected.end()), [](const ControlNode*) { return true; });
    SetSelection(selected, SelectedNodes());
}

void SelectionSystem::FocusNextChild()
{
    FocusToChild(true);
}

void SelectionSystem::FocusPreviousChild()
{
    FocusToChild(false);
}

void SelectionSystem::FocusToChild(bool next)
{
    PackageBaseNode* startNode = nullptr;
    if (!selectionContainer.selectedNodes.empty())
    {
        startNode = *selectionContainer.selectedNodes.rbegin();
    }
    PackageBaseNode* nextNode = nullptr;
    Vector<PackageBaseNode*> allNodes;
    systemManager->CollectControlNodes(std::back_inserter(allNodes), [](const ControlNode*) { return true; });
    if (allNodes.empty())
    {
        return;
    }
    auto findIt = std::find(allNodes.begin(), allNodes.end(), startNode);
    if (findIt == allNodes.end())
    {
        nextNode = next ? allNodes.front() : allNodes.back();
    }
    else if (next)
    {
        ++findIt;
        nextNode = findIt == allNodes.end() ? allNodes.front() : *findIt;
    }
    else
    {
        nextNode = findIt == allNodes.begin() ? allNodes.back() : *(--findIt);
    }

    SelectedNodes newSelectedNodes;
    newSelectedNodes.insert(nextNode);
    SetSelection(newSelectedNodes, selectionContainer.selectedNodes);
}

bool SelectionSystem::ProcessMousePress(const DAVA::Vector2& point, UIEvent::MouseButton buttonID)
{
    SelectedNodes selected;
    SelectedNodes deselected;
    if (!IsKeyPressed(KeyboardProxy::KEY_SHIFT) && !IsKeyPressed(KeyboardProxy::KEY_CTRL))
    {
        deselected = selectionContainer.selectedNodes;
    }

    ControlNode* selectedNode = nullptr;
    if (buttonID == UIEvent::MouseButton::LEFT)
    {
        Vector<ControlNode*> nodesUnderPoint;
        auto predicate = [point](const ControlNode* node) -> bool {
            const auto control = node->GetControl();
            DVASSERT(nullptr != control);
            return control->GetVisibilityFlag() && control->IsPointInside(point);
        };
        auto stopPredicate = [](const ControlNode* node) -> bool {
            const auto control = node->GetControl();
            DVASSERT(nullptr != control);
            return !control->GetVisibilityFlag();
        };
        systemManager->CollectControlNodes(std::back_inserter(nodesUnderPoint), predicate, stopPredicate);
        if (!nodesUnderPoint.empty())
        {
            selectedNode = nodesUnderPoint.back();
        }
    }
    else if (buttonID == UIEvent::MouseButton::RIGHT)
    {
        Vector<ControlNode*> nodesUnderPointForMenu;
        auto predicateForMenu = [point](const ControlNode* node) -> bool
        {
            const auto control = node->GetControl();
            DVASSERT(nullptr != control);
            const auto visibleProp = node->GetRootProperty()->GetVisibleProperty();
            return visibleProp->GetVisibleInEditor() && control->IsPointInside(point);
        };
        auto stopPredicate = [](const ControlNode* node) -> bool {
            const auto visibleProp = node->GetRootProperty()->GetVisibleProperty();
            return !visibleProp->GetVisibleInEditor();
        };
        systemManager->CollectControlNodes(std::back_inserter(nodesUnderPointForMenu), predicateForMenu, stopPredicate);
        selectedNode = systemManager->GetControlByMenu(nodesUnderPointForMenu, point);
        if (nullptr == selectedNode)
        {
            return true; //selection was required but cancelled
        }
    }

    if (selectedNode != nullptr)
    {
        if (IsKeyPressed(KeyboardProxy::KEY_CTRL) && selectionContainer.IsSelected(selectedNode))
        {
            deselected.insert(selectedNode);
        }
        else
        {
            selected.insert(selectedNode);
        }
    }
    for (auto controlNode : selected)
    {
        deselected.erase(controlNode);
    }
    SetSelection(selected, deselected);
    return !selected.empty() || !deselected.empty();
}

void SelectionSystem::OnSelectionChanged(const SelectedNodes& selected, const SelectedNodes& deselected)
{
    selectionContainer.MergeSelection(selected, deselected);
}

void SelectionSystem::SetSelection(const SelectedNodes& selected, const SelectedNodes& deselected)
{
    SelectedNodes reallySelected;
    SelectedNodes reallyDeselected;
    selectionContainer.GetOnlyExistedItems(deselected, reallyDeselected);
    selectionContainer.GetNotExistedItems(selected, reallySelected);
    selectionContainer.MergeSelection(reallySelected, reallyDeselected);

    if (!reallySelected.empty() || !reallyDeselected.empty())
    {
        systemManager->SelectionChanged.Emit(reallySelected, reallyDeselected);
    }
}

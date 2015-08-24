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

#include "Document.h"
#include "HUDSystem.h"

#include "UI/UIControl.h"
#include "UI/UIEvent.h"
#include "Base/BaseTypes.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Render/RenderState.h"
#include "Render/RenderManager.h"
#include "Render/RenderHelper.h"

using namespace DAVA;

class ControlContainer : public UIControl
{
public:
    explicit ControlContainer(UIControl *container, const ControlAreaInterface::eArea area_)
        : UIControl()
        , control(container)
        , area(area_)
    {}
    ControlAreaInterface::eArea GetArea() const
    {
        return area;
    }
protected:
    const UIControl *control = nullptr;
    const ControlAreaInterface::eArea area = ControlAreaInterface::NO_AREA;
};

class FrameControl : public ControlContainer
{
public:
    explicit FrameControl(UIControl *container)
        : ControlContainer(container, ControlAreaInterface::FRAME)
    { }
private:
    void Draw(const UIGeometricData &geometricData) override
    {
        Rect rect(control->GetGeometricData().GetAABBox());
        SetAbsoluteRect(rect);
        Color oldColor = RenderManager::Instance()->GetColor();
        RenderManager::Instance()->SetColor(Color(1.0f, 0.0f, 0.0f, 1.f));
        RenderHelper::Instance()->DrawRect(GetAbsoluteRect(), RenderState::RENDERSTATE_2D_BLEND);
        RenderManager::Instance()->SetColor(oldColor);
    }
};

class FrameRectControl : public ControlContainer
{
public:
    explicit FrameRectControl(UIControl *container, const ControlAreaInterface::eArea area_)
        : ControlContainer(container, area_)
    { }
private:
    void Draw(const UIGeometricData &geometricData) override
    {
        Rect rect(0, 0, 8, 8);
        rect.SetCenter(GetPos());
        SetAbsoluteRect(rect);

        Color oldColor = RenderManager::Instance()->GetColor();
        RenderManager::Instance()->SetColor(Color(0.0f, 1.0f, 0.0f, 1.f));
        RenderHelper::Instance()->FillRect(GetAbsoluteRect(), RenderState::RENDERSTATE_2D_BLEND);
        RenderManager::Instance()->SetColor(oldColor);
    }

    Vector2 GetPos() const
    {
        Rect rect = control->GetGeometricData().GetAABBox();
        Vector2 retVal = rect.GetPosition();
        switch (area)
        {
        case ControlAreaInterface::TOP_LEFT: return retVal;
        case ControlAreaInterface::TOP_CENTER: return retVal + Vector2(rect.dx / 2.0f, 0);
        case ControlAreaInterface::TOP_RIGHT: return retVal + Vector2(rect.dx, 0);
        case ControlAreaInterface::CENTER_LEFT: return retVal + Vector2(0, rect.dy / 2.0f);
        case ControlAreaInterface::CENTER_RIGHT: return retVal + Vector2(rect.dx, rect.dy / 2.0f);
        case ControlAreaInterface::BOTTOM_LEFT: return retVal + Vector2(0, rect.dy);
        case ControlAreaInterface::BOTTOM_CENTER: return retVal + Vector2(rect.dx / 2.0f, rect.dy);
        case ControlAreaInterface::BOTTOM_RIGHT: return retVal + Vector2(rect.dx, rect.dy);
        default: DVASSERT_MSG(false, "what are you doing here?!"); return Vector2(0, 0);
        }
    }
};

class PivotPointControl : public ControlContainer
{
public:
    explicit PivotPointControl(UIControl *container)
        : ControlContainer(container, ControlAreaInterface::PIVOT_POINT)
    { }
private:
    void Draw(const UIGeometricData &geometricData) override
    {
        Color oldColor = RenderManager::Instance()->GetColor();
        RenderManager::Instance()->SetColor(Color(0.0f, 0.0f, 1.0f, 1.f));
        Rect rect(0, 0, 5, 5);
        rect.SetCenter(control->GetGeometricData().GetAABBox().GetPosition() + control->GetPivotPoint());
        SetAbsoluteRect(rect);
        RenderHelper::Instance()->FillRect(GetAbsoluteRect(), RenderState::RENDERSTATE_2D_BLEND);
        RenderManager::Instance()->SetColor(oldColor);
    }
};

class RotateControl : public ControlContainer
{
public:
    explicit RotateControl(UIControl *container)
        : ControlContainer(container, ControlAreaInterface::ROTATE)
    { }
private:
    void Draw(const UIGeometricData &geometricData) override
    {
        Color oldColor = RenderManager::Instance()->GetColor();
        RenderManager::Instance()->SetColor(Color(1.0f, 1.0f, 0.0f, 1.0f));
        Rect rect(0, 0, 20, 20);
        Rect controlRect = control->GetGeometricData().GetAABBox();
        rect.SetCenter(Vector2(controlRect.GetPosition().x + controlRect.dx / 2.0f, controlRect.GetPosition().y - 20));
        SetAbsoluteRect(rect);
        RenderHelper::Instance()->FillRect(GetAbsoluteRect(), RenderState::RENDERSTATE_2D_BLEND);
        RenderManager::Instance()->SetColor(oldColor);
    }
};

HUDSystem::HUDSystem(Document *document_)
    : document(document_)
    , hudControl(new UIControl())
    , selectionRect(new UIControl())
{
    selectionRect->SetDebugDraw(true);
    selectionRect->SetDebugDrawColor(Color(1.0f, 1.0f, 0.0f, 1.0f));
    hudControl->AddControl(selectionRect);
    hudControl->SetName("hud");
    hudControl->SetDebugDraw(true);
    hudControl->SetDebugDrawColor(Color(0.0f, 1.0f, 0.0f, 1.0f));
    hudControl->SetSize(Vector2(10, 10));
}

void HUDSystem::Attach(UIControl* root)
{
    root->AddControl(hudControl);
}

void HUDSystem::Detach()
{
    hudControl->RemoveFromParent();
}

void HUDSystem::SelectionWasChanged(const SelectedControls& selected, const SelectedControls& deselected)
{
    for (auto control : deselected)
    {
        hudMap.erase(control);
    }
    for (auto control : selected)
    {
        hudMap.emplace(std::piecewise_construct, 
            std::forward_as_tuple(control),
            std::forward_as_tuple(control, hudControl));
    }
}

bool HUDSystem::OnInput(UIEvent *currentInput)
{
    switch (currentInput->phase)
    {
    case UIEvent::PHASE_MOVE:
        ProcessCursor(currentInput->point);
        return false;
    case UIEvent::PHASE_BEGAN:
        canDrawRect = document->GetControlNodeByPos(currentInput->point) == nullptr;
        pressedPoint = currentInput->point;
        return false;
    case UIEvent::PHASE_DRAG:
        if(canDrawRect)
        {
            Vector2 point(currentInput->point);
            Vector2 size(point - pressedPoint);
            selectionRect->SetAbsoluteRect(Rect(pressedPoint, size));
        }
        return canDrawRect;
    case UIEvent::PHASE_ENDED:
        if(canDrawRect)
        {
            //auto rect = selectionRect->GetAbsoluteRect();
            selectionRect->SetSize(Vector2(0, 0));
        }
        bool retVal = canDrawRect;
        canDrawRect = false;
        return retVal;
    }
    return false;
}

void HUDSystem::AddListener(ControlAreaInterface *listener)
{
    auto it = std::find(listeners.begin(), listeners.end(), listener);
    if (it == listeners.end())
    {
        listeners.push_back(listener);
    }
    else
    {
        DVASSERT_MSG(false, "listener has already attached");
    }
}

void HUDSystem::RemoveListener(ControlAreaInterface *listener)
{
    auto it = std::find(listeners.begin(), listeners.end(), listener);
    if (it != listeners.end())
    {
        listeners.erase(it);
    }
    else
    {
        DVASSERT_MSG(false, "listener was not attached");
    }
}

void HUDSystem::ProcessCursor(const Vector2& pos)
{
    ControlNode *node = nullptr;
    ControlAreaInterface::eArea area = ControlAreaInterface::NO_AREA;
    GetControlArea(node, area, pos);
    SetNewArea(node, area);
}

void HUDSystem::GetControlArea(ControlNode*& node, ControlAreaInterface::eArea& area, const Vector2 &pos)
{
    for (const auto &iter : hudMap)
    {
        auto &hud = iter.second;
        for (const auto &hudControl : hud.hudControls)
        {
            if (hudControl->IsPointInside(pos))
            {
                auto container = static_cast<ControlContainer*>(hudControl.get());
                node = hud.node;
                area = container->GetArea();
                DVASSERT_MSG(area != ControlAreaInterface::NO_AREA && node != nullptr
                             , "no control node for area");
                return;
            }
        }
    }
    node = nullptr;
    area = ControlAreaInterface::NO_AREA;
    return;
}

void HUDSystem::SetNewArea(ControlNode* node, const ControlAreaInterface::eArea area)
{
    if (activeControl != node || activeArea != area)
    {
        activeControl = node;
        activeArea = area;
        if (area != ControlAreaInterface::NO_AREA)
        {
            for (auto listener : listeners)
            {
                listener->MouseEnterArea(node, area);
            }
        }
        else
        {
            for (auto listener : listeners)
            {
                listener->MouseLeaveArea();
            }
        }
    }
}

HUDSystem::HUD::HUD(ControlNode *node_, UIControl* hudControl)
    : node(node_)
    , control(node_->GetControl())
{
    hudControls.emplace_back(new PivotPointControl(control));
    hudControls.emplace_back(new RotateControl(control));
    for (int i = ControlAreaInterface::TOP_LEFT; i < ControlAreaInterface::CORNER_COUNT; ++i)
    {
        ControlAreaInterface::eArea area = static_cast<ControlAreaInterface::eArea>(i);
        hudControls.emplace_back(new FrameRectControl(control, area));
    }
    hudControls.emplace_back(new FrameControl(control));
    for (auto iter = hudControls.rbegin(); iter != hudControls.rend(); ++iter)
    {
        hudControl->AddControl(*iter);
    }
}

HUDSystem::HUD::~HUD()
{
    for (auto control : hudControls)
    {
        control->RemoveFromParent();
    }
}
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

#include "TabTraversalAlgorithm.h"

#include "UI/UIControl.h"
#include "UI/Focus/UIFocusComponent.h"

namespace DAVA
{
TabTraversalAlgorithm::TabTraversalAlgorithm(UIControl* root_)
{
    root = root_;
}

TabTraversalAlgorithm::~TabTraversalAlgorithm()
{
}

UIControl* TabTraversalAlgorithm::GetNextControl(UIControl* focusedControl, FocusHelpers::TabDirection dir)
{
    if (focusedControl != nullptr && root != focusedControl)
    {
        UIControl* parent = focusedControl->GetParent();

        if (parent != nullptr)
        {
            Vector<UIControl*> children;
            PrepareChildren(parent, children);

            UIControl* res = nullptr;
            if (dir == FocusHelpers::FORWARD)
            {
                res = FindNextControl(focusedControl, children.begin(), children.end(), dir);
            }
            else
            {
                res = FindNextControl(focusedControl, children.rbegin(), children.rend(), dir);
            }

            if (res != nullptr)
            {
                return res;
            }
            else
            {
                return GetNextControl(parent, dir);
            }
        }
    }
    return nullptr;
}

template <typename It>
UIControl* TabTraversalAlgorithm::FindNextControl(UIControl* focusedControl, It begin, It end, FocusHelpers::TabDirection dir)
{
    auto it = begin;
    while (it != end && *it != focusedControl)
    {
        ++it;
    }

    if (it == end)
    {
        return nullptr;
    }

    ++it;

    for (; it != end; ++it)
    {
        UIControl* res = FindFirstControl(*it, dir);
        if (res != nullptr)
        {
            return res;
        }
    }

    return nullptr;
}

UIControl* TabTraversalAlgorithm::FindFirstControl(UIControl* control, FocusHelpers::TabDirection dir)
{
    if (FocusHelpers::CanFocusControl(control))
    {
        return control;
    }

    Vector<UIControl*> children;
    PrepareChildren(control, children);

    if (dir == FocusHelpers::FORWARD)
    {
        return FindFirstControlRecursive(children.begin(), children.end(), dir);
    }
    else
    {
        return FindFirstControlRecursive(children.rbegin(), children.rend(), dir);
    }
}

template <typename It>
UIControl* TabTraversalAlgorithm::FindFirstControlRecursive(It begin, It end, FocusHelpers::TabDirection dir)
{
    for (auto it = begin; it != end; ++it)
    {
        UIControl* res = FindFirstControl(*it, dir);
        if (res)
        {
            return res;
        }
    }
    return nullptr;
}

void TabTraversalAlgorithm::PrepareChildren(UIControl* control, Vector<UIControl*>& children)
{
    DVASSERT(children.empty());

    children.reserve(control->GetChildren().size());
    children.insert(children.end(), control->GetChildren().begin(), control->GetChildren().end());

    std::stable_sort(children.begin(), children.end(), [](UIControl* c1, UIControl* c2) {
        UIFocusComponent* f1 = c1->GetComponent<UIFocusComponent>();
        if (f1 == nullptr)
        {
            return false;
        }

        UIFocusComponent* f2 = c2->GetComponent<UIFocusComponent>();
        return f2 == nullptr || f1->GetTabOrder() < f2->GetTabOrder(); // important: f1 != nullptr
    });
}
}

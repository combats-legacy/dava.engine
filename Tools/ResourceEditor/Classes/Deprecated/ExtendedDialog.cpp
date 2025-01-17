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


#include "ExtendedDialog.h"
#include "ControlsFactory.h"

ExtendedDialog::ExtendedDialog()
    : UIControl(DAVA::UIScreenManager::Instance()->GetScreen()->GetRect())
{
    ControlsFactory::CustomizeDialogFreeSpace(this);

    draggableDialog = new DraggableDialog(GetDialogRect());
    ControlsFactory::CustomizeDialog(draggableDialog);
    AddControl(draggableDialog);
}

ExtendedDialog::~ExtendedDialog()
{
    SafeRelease(draggableDialog);
}

void ExtendedDialog::Show()
{
    isShown = true;
}

bool ExtendedDialog::IsShown() const
{
    return isShown;
}

const DAVA::Rect ExtendedDialog::GetScreenRect() const
{
    DAVA::UIScreen* activeScreen = DAVA::UIScreenManager::Instance()->GetScreen();
    if (activeScreen)
    {
        return activeScreen->GetRect();
    }

    return DAVA::Rect();
}

const DAVA::Rect ExtendedDialog::GetDialogRect() const
{
    const DAVA::Rect screenRect = GetScreenRect();

    return DAVA::Rect(screenRect.dx / 4, screenRect.dy / 4, screenRect.dx / 2, screenRect.dy / 2);
}

void ExtendedDialog::Close()
{
    isShown = false;
    if (GetParent())
    {
        GetParent()->RemoveControl(this);
    }
}

void ExtendedDialog::OnActive()
{
    UIControl::OnActive();

    UpdateSize();
}

void ExtendedDialog::UpdateSize()
{
    SetRect(GetScreenRect());

    draggableDialog->SetRect(GetDialogRect());
}
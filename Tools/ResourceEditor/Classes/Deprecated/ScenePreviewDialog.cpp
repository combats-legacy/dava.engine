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


#include "ScenePreviewDialog.h"

#include "Deprecated/ControlsFactory.h"
#include "Qt/Settings/SettingsManager.h"
#include "ScenePreviewControl.h"

ScenePreviewDialog::ScenePreviewDialog()
    : ExtendedDialog()
    , preview(nullptr)
    , errorMessage(nullptr)
    , clickableBackgound(nullptr)
{
    UpdateSize();
    GetBackground()->color = DAVA::Color(2.0f / 3.0f, 2.0f / 3.0f, 2.0f / 3.0f, 1.0f);

    clickableBackgound.reset(new DAVA::UIControl());
    clickableBackgound->SetInputEnabled(true, true);
    clickableBackgound->AddEvent(DAVA::UIControl::EVENT_TOUCH_UP_INSIDE, DAVA::Message(this, &ScenePreviewDialog::OnClose));

    preview.reset(new ScenePreviewControl(DAVA::Rect(0, 0, ControlsFactory::PREVIEW_PANEL_HEIGHT, ControlsFactory::PREVIEW_PANEL_HEIGHT)));

    errorMessage.reset(new DAVA::UIStaticText(preview->GetRect()));
    errorMessage->SetMultiline(true);
    errorMessage->SetSpriteAlign(DAVA::ALIGN_HCENTER | DAVA::ALIGN_VCENTER);
    errorMessage->SetTextColor(ControlsFactory::GetColorError());
    errorMessage->SetFont(ControlsFactory::GetFont20());

    DAVA::ScopedPtr<DAVA::UIButton> button(ControlsFactory::CreateButton(DAVA::Rect(0, ControlsFactory::PREVIEW_PANEL_HEIGHT,
                                                                                    ControlsFactory::PREVIEW_PANEL_HEIGHT, ControlsFactory::BUTTON_HEIGHT),
                                                                         DAVA::LocalizedString(L"dialog.close")));
    button->AddEvent(DAVA::UIControl::EVENT_TOUCH_UP_INSIDE, DAVA::Message(this, &ScenePreviewDialog::OnClose));
    draggableDialog->AddControl(button);
}

ScenePreviewDialog::~ScenePreviewDialog()
{
    if (IsShown())
    {
        Close();
    }
}

void ScenePreviewDialog::Show(const DAVA::FilePath& scenePathname)
{
    bool enabled = SettingsManager::GetValue(Settings::General_PreviewEnabled).AsBool();
    if (!enabled)
        return;

    if (!GetParent())
    {
        DAVA::UIScreen* screen = DAVA::UIScreenManager::Instance()->GetScreen();
        clickableBackgound->SetSize(screen->GetSize());
        clickableBackgound->SetPosition(DAVA::Vector2(0, 0));
        screen->AddControl(clickableBackgound);
        screen->AddControl(this);

        screen->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, DAVA::Message(this, &ScenePreviewDialog::OnClose));
    }

    //show preview
    if (errorMessage->GetParent())
    {
        draggableDialog->RemoveControl(errorMessage);
    }

    DAVA::int32 error = preview->OpenScene(scenePathname);
    if (DAVA::SceneFileV2::ERROR_NO_ERROR == error)
    {
        if (!preview->GetParent())
        {
            draggableDialog->AddControl(preview);
        }
    }
    else
    {
        switch (error)
        {
        case DAVA::SceneFileV2::ERROR_FAILED_TO_CREATE_FILE:
        {
            errorMessage->SetText(DAVA::LocalizedString(L"library.errormessage.failedtocreeatefile"));
            break;
        }

        case DAVA::SceneFileV2::ERROR_FILE_WRITE_ERROR:
        {
            errorMessage->SetText(DAVA::LocalizedString(L"library.errormessage.filewriteerror"));
            break;
        }

        case DAVA::SceneFileV2::ERROR_VERSION_IS_TOO_OLD:
        {
            errorMessage->SetText(DAVA::LocalizedString(L"library.errormessage.versionistooold"));
            break;
        }

        case ScenePreviewControl::ERROR_CANNOT_OPEN_FILE:
        {
            errorMessage->SetText(DAVA::LocalizedString(L"library.errormessage.cannotopenfile"));
            break;
        }

        case ScenePreviewControl::ERROR_WRONG_EXTENSION:
        {
            errorMessage->SetText(DAVA::LocalizedString(L"library.errormessage.wrongextension"));
            break;
        }

        default:
            errorMessage->SetText(DAVA::LocalizedString(L"library.errormessage.unknownerror"));
            break;
        }

        draggableDialog->AddControl(errorMessage);
    }
    ExtendedDialog::Show();
}

void ScenePreviewDialog::OnClose(BaseObject*, void*, void*)
{
    Close();
}

void ScenePreviewDialog::Close()
{
    UIControl* backgourndParent = clickableBackgound->GetParent();
    if (backgourndParent != nullptr)
    {
        backgourndParent->RemoveControl(clickableBackgound);
    }

    preview->ReleaseScene();
    preview->RecreateScene();
    draggableDialog->RemoveControl(preview);
    ExtendedDialog::Close();
}

const DAVA::Rect ScenePreviewDialog::GetDialogRect() const
{
    DAVA::Rect screenRect = GetScreenRect();

    DAVA::float32 x = (screenRect.dx - ControlsFactory::PREVIEW_PANEL_HEIGHT);
    DAVA::float32 h = ControlsFactory::PREVIEW_PANEL_HEIGHT + ControlsFactory::BUTTON_HEIGHT;
    DAVA::float32 y = (screenRect.dy - h) / 2;

    return DAVA::Rect(x, y, ControlsFactory::PREVIEW_PANEL_HEIGHT, h);
}

void ScenePreviewDialog::UpdateSize()
{
    DAVA::Rect dialogRect = GetDialogRect();
    SetRect(dialogRect);

    dialogRect.x = dialogRect.y = 0;
    draggableDialog->SetRect(dialogRect);
}

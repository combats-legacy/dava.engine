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


#include "UI/UITextField.h"
#include "Input/KeyboardDevice.h"
#include "Input/InputSystem.h"
#include "UI/UIControlSystem.h"
#include "Render/2D/FontManager.h"

#if defined(__DAVAENGINE_ANDROID__)
#include "UITextFieldAndroid.h"
#elif defined(__DAVAENGINE_IPHONE__)
#include "UI/UITextFieldiPhone.h"
#elif defined(__DAVAENGINE_WIN_UAP__)
#include "UI/UITextFieldWinUAP.h"
#elif defined(__DAVAENGINE_MACOS__) && !defined(DISABLE_NATIVE_TEXTFIELD)
#include "UI/UITextFieldMacOS.h"
#else
#define DAVA_TEXTFIELD_USE_STB
#include "UI/UITextFieldStb.h"
namespace DAVA
{
class TextFieldPlatformImpl : public TextFieldStbImpl
{
public:
    TextFieldPlatformImpl(UITextField* control)
        : TextFieldStbImpl(control)
    {
    }
};
}
#endif

namespace DAVA
{
UITextField::UITextField(const Rect& rect)
    : UIControl(rect)
{
    textFieldImpl = new TextFieldPlatformImpl(this);
    textFieldImpl->SetVisible(false);

    SetupDefaults();
}

void UITextField::SetupDefaults()
{
    SetInputEnabled(true, false);

    SetAutoCapitalizationType(AUTO_CAPITALIZATION_TYPE_SENTENCES);
    SetAutoCorrectionType(AUTO_CORRECTION_TYPE_DEFAULT);
    SetSpellCheckingType(SPELL_CHECKING_TYPE_DEFAULT);
    SetKeyboardAppearanceType(KEYBOARD_APPEARANCE_DEFAULT);
    SetKeyboardType(KEYBOARD_TYPE_DEFAULT);
    SetReturnKeyType(RETURN_KEY_DEFAULT);
    SetEnableReturnKeyAutomatically(false);
    SetTextUseRtlAlign(TextBlock::RTL_DONT_USE);

    SetMaxLength(-1);

    SetIsPassword(false);
    SetTextColor(GetTextColor());
    SetTextAlign(ALIGN_LEFT | ALIGN_VCENTER);

    SetFontSize(26); //12 is default size for IOS

    SetText(L"");
    SetRenderToTexture(true);
}

UITextField::~UITextField()
{
    SafeDelete(textFieldImpl);
    UIControl::RemoveAllControls();
}

bool UITextField::IsEditing() const
{
    return isEditing;
}

void UITextField::StartEdit()
{
    if (!isEditing)
    {
        isEditing = true;
        OnStartEditing();
        SetRenderToTexture(false);
        textFieldImpl->OpenKeyboard();
    }
}

void UITextField::StopEdit()
{
    if (isEditing)
    {
        isEditing = false;
        SetRenderToTexture(true);
        textFieldImpl->CloseKeyboard();
        OnStopEditing();
    }
}

void UITextField::Update(float32 timeElapsed)
{
    textFieldImpl->UpdateRect(GetGeometricData().GetUnrotatedRect());
}

void UITextField::OnActive()
{
#ifdef __DAVAENGINE_IPHONE__
    textFieldImpl->ShowField();
    textFieldImpl->SetVisible(IsVisible());
#endif
}

void UITextField::OnInactive()
{
#ifdef __DAVAENGINE_IPHONE__
    textFieldImpl->HideField();
#endif
}

void UITextField::OnFocused()
{
    if (startEditPolicy == START_EDIT_WHEN_FOCUSED)
    {
        StartEdit();
    }
}

void UITextField::SetFocused()
{
    UIControlSystem::Instance()->SetFocusedControl(this);
}

void UITextField::OnFocusLost()
{
    StopEdit();

    if (delegate != nullptr)
    {
        delegate->TextFieldLostFocus(this);
    }
}

void UITextField::OnTouchOutsideFocus()
{
    if (stopEditPolicy == STOP_EDIT_BY_USER_REQUEST)
    {
        StopEdit();
    }
}

void UITextField::SetSelectionColor(const Color& selectionColor)
{
#if defined(DAVA_TEXTFIELD_USE_STB)
    textFieldImpl->SetSelectionColor(selectionColor);
#endif
}

const Color& UITextField::GetSelectionColor() const
{
#if defined(DAVA_TEXTFIELD_USE_STB)
    return textFieldImpl->GetSelectionColor();
#else
    return Color::Transparent;
#endif
}

void UITextField::ReleaseFocus()
{
    StopEdit();
}

void UITextField::SetFont(Font* font)
{
#if defined(DAVA_TEXTFIELD_USE_STB)
    textFieldImpl->SetFont(font);
#endif // !defined(DAVA_TEXTFIELD_USE_STB)
}

void UITextField::SetTextColor(const Color& fontColor)
{
    textFieldImpl->SetTextColor(fontColor);
}

void UITextField::SetShadowOffset(const DAVA::Vector2& offset)
{
#if defined(DAVA_TEXTFIELD_USE_STB)
    textFieldImpl->SetShadowOffset(offset);
#endif
}

void UITextField::SetShadowColor(const Color& color)
{
#if defined(DAVA_TEXTFIELD_USE_STB)
    textFieldImpl->SetShadowColor(color);
#endif
}

void UITextField::SetTextAlign(int32 align)
{
    textFieldImpl->SetTextAlign(align);
}

TextBlock::eUseRtlAlign UITextField::GetTextUseRtlAlign() const
{
#ifdef DAVA_TEXTFIELD_USE_STB
    return textFieldImpl->GetTextUseRtlAlign();
#else
    return textFieldImpl->GetTextUseRtlAlign() ? TextBlock::RTL_USE_BY_CONTENT : TextBlock::RTL_DONT_USE;
#endif
}

void UITextField::SetTextUseRtlAlign(TextBlock::eUseRtlAlign useRtlAlign)
{
#ifdef DAVA_TEXTFIELD_USE_STB
    textFieldImpl->SetTextUseRtlAlign(useRtlAlign);
#else
    textFieldImpl->SetTextUseRtlAlign(useRtlAlign == TextBlock::RTL_USE_BY_CONTENT);
#endif
}

void UITextField::SetTextUseRtlAlignFromInt(int32 value)
{
    SetTextUseRtlAlign(static_cast<TextBlock::eUseRtlAlign>(value));
}

int32 UITextField::GetTextUseRtlAlignAsInt() const
{
    return static_cast<TextBlock::eUseRtlAlign>(GetTextUseRtlAlign());
}

void UITextField::SetFontSize(float32 size)
{
    textFieldImpl->SetFontSize(size);
}

void UITextField::SetDelegate(UITextFieldDelegate* _delegate)
{
    delegate = _delegate;
#if defined(__DAVAENGINE_WIN_UAP__)
    textFieldImpl->SetDelegate(_delegate);
#endif
}

UITextFieldDelegate* UITextField::GetDelegate()
{
    return delegate;
}

void UITextField::SetSpriteAlign(int32 align)
{
    UIControl::SetSpriteAlign(align);
}

void UITextField::SetSize(const DAVA::Vector2& newSize)
{
    UIControl::SetSize(newSize);
#if defined(DAVA_TEXTFIELD_USE_STB)
    textFieldImpl->SetSize(newSize);
#endif
}

void UITextField::SetPosition(const DAVA::Vector2& position)
{
    UIControl::SetPosition(position);
}

void UITextField::SetMultiline(bool value)
{
    if (value != isMultiline)
    {
        isMultiline = value;
        textFieldImpl->SetMultiline(isMultiline);
    }
}

bool UITextField::IsMultiline() const
{
    return isMultiline;
}

void UITextField::SetText(const WideString& text_)
{
    textFieldImpl->SetText(text_);
    text = text_;
}

const WideString& UITextField::GetText()
{
    textFieldImpl->GetText(text);
    return text;
}

Font* UITextField::GetFont() const
{
#if defined(DAVA_TEXTFIELD_USE_STB)
    return textFieldImpl->GetFont();
#else
    return nullptr;
#endif
}

Color UITextField::GetTextColor() const
{
#if defined(DAVA_TEXTFIELD_USE_STB)
    return textFieldImpl->GetTextColor();
#else
    return Color::White;
#endif
}

Vector2 UITextField::GetShadowOffset() const
{
#if defined(DAVA_TEXTFIELD_USE_STB)
    return textFieldImpl->GetShadowOffset();
#else
    return Vector2::Zero;
#endif
}

Color UITextField::GetShadowColor() const
{
#if defined(DAVA_TEXTFIELD_USE_STB)
    return textFieldImpl->GetShadowColor();
#else
    return Color::White;
#endif
}

int32 UITextField::GetTextAlign() const
{
    return textFieldImpl->GetTextAlign();
}

void UITextField::Input(UIEvent* currentInput)
{
#if defined(DAVA_TEXTFIELD_USE_STB)
    textFieldImpl->Input(currentInput);

#else
    if (this != UIControlSystem::Instance()->GetFocusedControl())
        return;

    if (currentInput->phase == UIEvent::Phase::ENDED)
    {
        if (startEditPolicy == START_EDIT_BY_USER_REQUEST)
        {
            StartEdit();
        }
    }
#endif
}

WideString UITextField::GetAppliedChanges(int32 replacementLocation, int32 replacementLength, const WideString& replacementString)
{ //TODO: fix this for copy/paste
    WideString txt = GetText();

    if (replacementLocation >= 0)
    {
        if (replacementLocation <= static_cast<int32>(txt.length()))
        {
            txt.replace(replacementLocation, replacementLength, replacementString);
            if (GetMaxLength() > 0)
            {
                int32 outOfBounds = static_cast<int32>(txt.size()) - GetMaxLength();
                if (outOfBounds > 0)
                {
                    txt.erase(GetMaxLength(), outOfBounds);
                }
            }
        }
        else
        {
            Logger::Error("[UITextField::GetAppliedChanges] - index out of bounds.");
        }
    }

    return txt;
}

UITextField* UITextField::Clone()
{
    UITextField* t = new UITextField();
    t->CopyDataFrom(this);
    return t;
}

void UITextField::CopyDataFrom(UIControl* srcControl)
{
    UIControl::CopyDataFrom(srcControl);
    UITextField* t = static_cast<UITextField*>(srcControl);

#if defined(DAVA_TEXTFIELD_USE_STB)
    textFieldImpl->CopyDataFrom(t->textFieldImpl);
#endif
    isPassword = t->isPassword;
    cursorBlinkingTime = t->cursorBlinkingTime;
    SetText(t->GetText());
    SetRect(t->GetRect());

    SetAutoCapitalizationType(t->GetAutoCapitalizationType());
    SetAutoCorrectionType(t->GetAutoCorrectionType());
    SetSpellCheckingType(t->GetSpellCheckingType());
    SetKeyboardAppearanceType(t->GetKeyboardAppearanceType());
    SetKeyboardType(t->GetKeyboardType());
    SetReturnKeyType(t->GetReturnKeyType());
    SetEnableReturnKeyAutomatically(t->IsEnableReturnKeyAutomatically());
    SetTextUseRtlAlign(t->GetTextUseRtlAlign());
    SetMaxLength(t->GetMaxLength());
}

void UITextField::SetIsPassword(bool isPassword_)
{
    isPassword = isPassword_;
    textFieldImpl->SetIsPassword(isPassword_);
}

bool UITextField::IsPassword() const
{
    return isPassword;
}

WideString UITextField::GetVisibleText()
{
    if (!isPassword)
    {
        return GetText();
    }
    return WideString(GetText().length(), L'*');
}

int32 UITextField::GetAutoCapitalizationType() const
{
    return autoCapitalizationType;
}

void UITextField::SetAutoCapitalizationType(int32 value)
{
    autoCapitalizationType = static_cast<eAutoCapitalizationType>(value);
    textFieldImpl->SetAutoCapitalizationType(value);
}

int32 UITextField::GetAutoCorrectionType() const
{
    return autoCorrectionType;
}

void UITextField::SetAutoCorrectionType(int32 value)
{
    autoCorrectionType = static_cast<eAutoCorrectionType>(value);
    textFieldImpl->SetAutoCorrectionType(value);
}

int32 UITextField::GetSpellCheckingType() const
{
    return spellCheckingType;
}

void UITextField::SetSpellCheckingType(int32 value)
{
    spellCheckingType = static_cast<eSpellCheckingType>(value);
    textFieldImpl->SetSpellCheckingType(value);
}

int32 UITextField::GetKeyboardAppearanceType() const
{
    return keyboardAppearanceType;
}

void UITextField::SetKeyboardAppearanceType(int32 value)
{
    keyboardAppearanceType = static_cast<eKeyboardAppearanceType>(value);
    textFieldImpl->SetKeyboardAppearanceType(value);
}

int32 UITextField::GetKeyboardType() const
{
    return keyboardType;
}

void UITextField::SetKeyboardType(int32 value)
{
    keyboardType = static_cast<eKeyboardType>(value);
    textFieldImpl->SetKeyboardType(value);
}

int32 UITextField::GetReturnKeyType() const
{
    return returnKeyType;
}

void UITextField::SetReturnKeyType(int32 value)
{
    returnKeyType = static_cast<eReturnKeyType>(value);
    textFieldImpl->SetReturnKeyType(value);
}

UITextField::eStartEditPolicy UITextField::GetStartEditPolicy() const
{
    return startEditPolicy;
}

void UITextField::SetStartEditPolicy(eStartEditPolicy policy)
{
    startEditPolicy = policy;
}

UITextField::eStopEditPolicy UITextField::GetStopEditPolicy() const
{
    return stopEditPolicy;
}

void UITextField::SetStopEditPolicy(eStopEditPolicy policy)
{
    stopEditPolicy = policy;
}

bool UITextField::IsEnableReturnKeyAutomatically() const
{
    return enableReturnKeyAutomatically;
}

void UITextField::SetEnableReturnKeyAutomatically(bool value)
{
    enableReturnKeyAutomatically = value;
    textFieldImpl->SetEnableReturnKeyAutomatically(value);
}

void UITextField::SetInputEnabled(bool isEnabled, bool hierarchic)
{
    UIControl::SetInputEnabled(isEnabled, hierarchic);
    textFieldImpl->SetInputEnabled(isEnabled);
}

void UITextField::SetRenderToTexture(bool value)
{
    // Workaround! Users need scrolling of large texts in
    // multiline mode so we have to disable render into texture
    if (isMultiline)
    {
        value = false;
    }

    textFieldImpl->SetRenderToTexture(value);
}

bool UITextField::IsRenderToTexture() const
{
    return textFieldImpl->IsRenderToTexture();
}

uint32 UITextField::GetCursorPos()
{
    return textFieldImpl->GetCursorPos();
}

void UITextField::SetCursorPos(uint32 pos)
{
    textFieldImpl->SetCursorPos(pos);
}

void UITextField::SetMaxLength(int32 newMaxLength)
{
    maxLength = Max(-1, newMaxLength); //-1 valid value
    textFieldImpl->SetMaxLength(maxLength);
}

int32 UITextField::GetMaxLength() const
{
    return maxLength;
}

void UITextField::OnStartEditing()
{
    if (delegate != nullptr)
    {
        delegate->OnStartEditing();
    }
}

void UITextField::OnStopEditing()
{
    if (delegate != nullptr)
    {
        delegate->OnStopEditing();
    }
}

void UITextField::OnKeyboardShown(const Rect& keyboardRect)
{
    if (delegate != nullptr)
    {
        delegate->OnKeyboardShown(keyboardRect);
    }
}

void UITextField::OnKeyboardHidden()
{
    if (delegate != nullptr)
    {
        delegate->OnKeyboardHidden();
    }
}

void UITextField::OnVisible()
{
    UIControl::OnVisible();
    textFieldImpl->SetVisible(visible);
}

void UITextField::OnInvisible()
{
    UIControl::OnInvisible();
    textFieldImpl->SetVisible(false);
}

String UITextField::GetFontPresetName() const
{
    String name;
    Font* font = GetFont();
    if (font != nullptr)
    {
        name = FontManager::Instance()->GetFontName(font);
    }
    return name;
}

void UITextField::SetFontByPresetName(const String& presetName)
{
    Font* font = nullptr;
    if (!presetName.empty())
    {
        font = FontManager::Instance()->GetFont(presetName);
    }

    SetFont(font);
    if (font)
    {
        SetFontSize(static_cast<float32>(font->GetFontHeight()));
    }
}

void UITextField::SystemDraw(const UIGeometricData& geometricData)
{
    UIControl::SystemDraw(geometricData);

    UIGeometricData localData = GetLocalGeometricData();
    localData.AddGeometricData(geometricData);
    textFieldImpl->SystemDraw(localData);
}

int32 UITextField::GetStartEditPolicyAsInt() const
{
    return GetStartEditPolicy();
}

void UITextField::SetStartEditPolicyFromInt(int32 policy)
{
    SetStartEditPolicy(static_cast<eStartEditPolicy>(policy));
}

int32 UITextField::GetStopEditPolicyAsInt() const
{
    return GetStopEditPolicy();
}

void UITextField::SetStopEditPolicyFromInt(int32 policy)
{
    SetStopEditPolicy(static_cast<eStopEditPolicy>(policy));
}

} // namespace DAVA

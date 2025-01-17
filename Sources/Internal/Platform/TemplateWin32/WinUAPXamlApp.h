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

// clang-format off

#ifndef __DAVAENGINE_WINUAPFRAME_H__
#define __DAVAENGINE_WINUAPFRAME_H__

#include "Base/Platform.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include <agile.h>
#include <concrt.h>

#include "Core/Core.h"
#include "Core/DisplayMode.h"

#include "UI/UIEvent.h"
#include "Input/InputSystem.h"
#include "Functional/Signal.h"

#include "Platform/DeviceInfo.h"

namespace DAVA
{

class Thread;
class CorePlatformWinUAP;
class DispatcherWinUAP;

/************************************************************************
 Class WinUAPXamlApp represents WinRT XAML application with embedded framework's render loop
 On startup application creates minimal neccesary infrastructure to allow coexistance of
 XAML native controls and DirectX.
 Application makes explicit use of two threads:
    - UI thread, created by system, where all interaction with UI and XAML controls must be done
    - main thread, created by WinUAPXamlApp instance, where framework lives
 To run code on UI thread you should use CorePlatformWinUAP::RunOnUIThread or CorePlatformWinUAP::RunOnUIThreadBlocked
 To run code on main thread you should use CorePlatformWinUAP::RunOnMainThread
************************************************************************/

ref class WinUAPXamlApp sealed : public ::Windows::UI::Xaml::Application
{
public:
    // Deleted and defaulted functions are not supported in WinRT classes
    WinUAPXamlApp();
    virtual ~WinUAPXamlApp();

    Windows::Graphics::Display::DisplayOrientations GetDisplayOrientation();
    Windows::UI::ViewManagement::ApplicationViewWindowingMode GetScreenMode();
    void SetScreenMode(Windows::UI::ViewManagement::ApplicationViewWindowingMode screenMode);

    bool IsPhoneApiDetected();

    Windows::UI::Core::CoreDispatcher^ UIThreadDispatcher();

internal:   // Only internal methods of ref class can return pointers to non-ref objects
    DispatcherWinUAP* MainThreadDispatcher();
    Signal<::Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^> pushNotificationSignal; //TODO: add implementation for all platform, before remove this

    void SetWindowMinimumSize(float32 width, float32 height);
    Vector2 GetWindowMinimumSize() const;

public:
    void SetQuitFlag();

    void AddUIElement(Windows::UI::Xaml::UIElement^ uiElement);
    void RemoveUIElement(Windows::UI::Xaml::UIElement^ uiElement);
    void PositionUIElement(Windows::UI::Xaml::UIElement^ uiElement, float32 x, float32 y);
    void SetTextBoxCustomStyle(Windows::UI::Xaml::Controls::TextBox^ textBox);
    void SetPasswordBoxCustomStyle(Windows::UI::Xaml::Controls::PasswordBox^ passwordBox);
    void UnfocusUIElement();
    void CaptureTextBox(Windows::UI::Xaml::Controls::Control^ text);

protected:
    void OnLaunched(::Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^ args) override;
    void OnActivated(::Windows::ApplicationModel::Activation::IActivatedEventArgs^ args) override;

private:
    void Run(::Windows::ApplicationModel::Activation::LaunchActivatedEventArgs ^ args);

    // App state handlers
    void OnSuspending(::Platform::Object^ sender, Windows::ApplicationModel::SuspendingEventArgs^ args);
    void OnResuming(::Platform::Object^ sender, ::Platform::Object^ args);

    // Windows state change handlers
    void OnWindowActivationChanged(::Windows::UI::Core::CoreWindow^ sender, ::Windows::UI::Core::WindowActivatedEventArgs^ args);
    void OnWindowVisibilityChanged(::Windows::UI::Core::CoreWindow^ sender, ::Windows::UI::Core::VisibilityChangedEventArgs^ args);

    // Size and scale changing handlers
    void OnCoreWindowSizeChanged(::Windows::UI::Core::CoreWindow^ coreWindow, ::Windows::UI::Core::WindowSizeChangedEventArgs^ arg);
    void OnSwapChainPanelSizeChanged(Platform::Object^ sender, ::Windows::UI::Xaml::SizeChangedEventArgs^ arg);
    void OnSwapChainPanelCompositionScaleChanged(::Windows::UI::Xaml::Controls::SwapChainPanel^ panel, Platform::Object^ obj);

    // Mouse and touch handlers
    void OnSwapChainPanelPointerPressed(Platform::Object ^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs ^ args);
    void OnSwapChainPanelPointerReleased(Platform::Object ^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs ^ args);
    void OnSwapChainPanelPointerMoved(Platform::Object ^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs ^ args);
    void OnSwapChainPanelPointerWheel(Platform::Object ^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs ^ args);

    void OnMouseMoved(Windows::Devices::Input::MouseDevice ^ mouseDevice, Windows::Devices::Input::MouseEventArgs ^ args);

    void OnHardwareBackButtonPressed(Platform::Object^ sender, Windows::Phone::UI::Input::BackPressedEventArgs ^args);
    void OnBackRequested(Platform::Object ^ sender, Windows::UI::Core::BackRequestedEventArgs ^ args);

    // Keyboard handlers
    void OnAcceleratorKeyActivated(Windows::UI::Core::CoreDispatcher ^ sender, Windows::UI::Core::AcceleratorKeyEventArgs ^ keyEventArgs);
    void OnChar(Windows::UI::Core::CoreWindow ^ sender, Windows::UI::Core::CharacterReceivedEventArgs ^ args);

    void DAVATouchEvent(UIEvent::Phase phase, float32 x, float32 y, int32 id, UIEvent::Device deviceIndex);

    void StartMainLoopThread(::Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^ args);
    void PreStartAppSettings();

    DeviceInfo::ScreenInfo ObtainScreenInfo();
    void SetupEventHandlers();
    void CreateBaseXamlUI();

    void SetTitleName();
    void SetDisplayOrientations();
    void LoadWindowMinimumSizeSettings();

    void PrepareScreenSize();
    void SetFullScreen(bool isFullScreenFlag);
    // in units of effective (view) pixels
    void EmitPushNotification(::Windows::ApplicationModel::Activation::LaunchActivatedEventArgs ^ args);
    void AllowDisplaySleep(bool sleep);
    void SendPressedMouseButtons(float32 x, float32 y, UIEvent::Device type);
    void SendBackKeyEvents();

private:
    Windows::UI::Xaml::Input::PointerRoutedEventArgs ^pressedEventArgs = nullptr;

    CorePlatformWinUAP* core = nullptr;
    Windows::UI::Core::CoreDispatcher^ uiThreadDispatcher = nullptr;
    std::unique_ptr<DispatcherWinUAP> dispatcher = nullptr;

    Windows::UI::Xaml::Controls::SwapChainPanel^ swapChainPanel = nullptr;
    Windows::UI::Xaml::Controls::Canvas^ canvas = nullptr;
    Windows::UI::Xaml::Controls::Button^ controlThatTakesFocus = nullptr;
    Windows::UI::Xaml::Style^ customTextBoxStyle = nullptr;
    Windows::UI::Xaml::Style^ customPasswordBoxStyle = nullptr;

    float32 minWindowWidth = 0.0f;
    float32 minWindowHeight = 0.0f;

    Windows::UI::Input::PointerPoint ^ mousePointer = nullptr;

    Thread* mainLoopThread = nullptr;

    volatile bool quitFlag = false;

    Vector<UIEvent> events;

    bool isMouseDetected = false;
    bool isTouchDetected = false;
    bool isPhoneApiDetected = false;

    bool isWindowVisible = true;
    bool isWindowClosed = false;
    bool isFullscreen = false;
    bool isRenderCreated = false;
    DisplayMode windowedMode = DisplayMode(DisplayMode::DEFAULT_WIDTH,
                                           DisplayMode::DEFAULT_HEIGHT,
                                           DisplayMode::DEFAULT_BITS_PER_PIXEL,
                                           DisplayMode::DEFAULT_DISPLAYFREQUENCY);
    DisplayMode currentMode = windowedMode;
    DisplayMode fullscreenMode = windowedMode;

    bool isMouseCursorShown = true;

    Bitset<static_cast<size_t>(UIEvent::MouseButton::NUM_BUTTONS)> mouseButtonsState;

    struct MouseButtonChange
    {
        UIEvent::Phase beginOrEnd;
        UIEvent::MouseButton button;
    };

    void WinUAPXamlApp::UpdateMouseButtonsState(Windows::UI::Input::PointerPointProperties ^ pointProperties, Vector<MouseButtonChange>& out);

    Vector<MouseButtonChange> mouseButtonChanges;

    bool GetMouseButtonState(UIEvent::MouseButton button);

    void SetMouseButtonState(UIEvent::MouseButton button, bool value);

    Windows::Graphics::Display::DisplayOrientations displayOrientation = ::Windows::Graphics::Display::DisplayOrientations::None;

    // Hardcoded styles for TextBox and PasswordBox to apply features:
    //  - transparent background in focus state
    //  - removed 'X' button
    static const wchar_t* xamlTextBoxStyles;
    static const wchar_t* xamlWebView;
    static const wchar_t* xamlTextBox;
    Windows::System::Display::DisplayRequest^ displayRequest = nullptr;
    Windows::Foundation::EventRegistrationToken token;
};

//////////////////////////////////////////////////////////////////////////

inline bool WinUAPXamlApp::GetMouseButtonState(UIEvent::MouseButton button)
{
    unsigned index = static_cast<unsigned>(button) - 1;
    return mouseButtonsState[index];
}

inline void WinUAPXamlApp::SetMouseButtonState(UIEvent::MouseButton button, bool value)
{
    unsigned index = static_cast<unsigned>(button) - 1;
    mouseButtonsState[index] = value;
}

inline Windows::UI::Core::CoreDispatcher^ WinUAPXamlApp::UIThreadDispatcher()
{
    return uiThreadDispatcher;
}

inline DispatcherWinUAP* WinUAPXamlApp::MainThreadDispatcher()
{
    return dispatcher.get();
}

inline void WinUAPXamlApp::SetQuitFlag()
{
    quitFlag = true;
}

inline bool WinUAPXamlApp::IsPhoneApiDetected()
{
    return isPhoneApiDetected;
}

}   // namespace DAVA

#endif  // __DAVAENGINE_WIN_UAP__
#endif  // __DAVAENGINE_WINUAPFRAME_H__

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


#ifndef __DAVAENGINE_UI_EVENT_H__
#define __DAVAENGINE_UI_EVENT_H__

#include "Math/Vector.h"
#include "Input/KeyboardDevice.h"
#include "Input/GamepadDevice.h"

namespace DAVA
{
class UIControl;
/**
\ingroup controlsystem
\brief User input representation.
	Represent user input event used in the control system. Contains all input data.
*/
class UIEvent
{
public:
    enum class Phase : int32
    {
        ERROR = 0,
        BEGAN, //!<Screen touch or mouse button press is began.
        DRAG, //!<User moves mouse with presset button or finger over the screen.
        ENDED, //!<Screen touch or mouse button press is ended.
        MOVE, //!<Mouse move event. Mouse moves without pressing any buttons. Works only with mouse controller.
        WHEEL, //!<Mouse wheel event. MacOS & Win32 only
        CANCELLED, //!<(ios only)Event was cancelled by the platform or by the control system for the some reason.
        CHAR, //!<Event some symbol was intered.
        CHAR_REPEAT, //!< Usefull if User hold key in text editor and wait
        KEY_DOWN,
        KEY_DOWN_REPEAT, //!< Usefull if user hold key in text editor and wait cursor to move
        KEY_UP,
        JOYSTICK,
        GESTURE, // mac os touch pad gestures only for now
    };

    /**
	 \enum Input state accordingly to control.
	 */
    enum eControlInputState
    {
        CONTROL_STATE_RELEASED = 0, //!<Input is released
        CONTROL_STATE_INSIDE, //!<Input processed inside control rerct for now
        CONTROL_STATE_OUTSIDE //!<Input processed outside of the control rerct for now
    };

    /**
	 \ Input can be handled in the different ways.
	 */
    enum eInputHandledType
    {
        INPUT_NOT_HANDLED = 0, //!<Input is not handled at all.
        INPUT_HANDLED_SOFT = 1, //!<Input is handled, but input control can be changed by UIControlSystem::Instance()->SwitchInputToControl() method.
        INPUT_HANDLED_HARD = 2, //!<Input is handled completely, input control can't be changed.
    };

    friend class UIControlSystem;

    enum class MouseButton : uint32
    {
        NONE = 0,
        LEFT = 1,
        RIGHT = 2,
        MIDDLE = 3,
        EXTENDED1 = 4,
        EXTENDED2 = 5,

        NUM_BUTTONS = EXTENDED2
    };

    enum class Device : uint32
    {
        UNKNOWN = 0,
        TOUCH_SURFACE,
        MOUSE,
        KEYBOARD,
        GAMEPAD,
        PEN,
        TOUCH_PAD,
    };

    UIEvent() = default;

    void SetInputHandledType(eInputHandledType value)
    {
        // Input Handled Type can be only increased.
        if (inputHandledType < value)
        {
            inputHandledType = value;
        }
    }

    eInputHandledType GetInputHandledType()
    {
        return inputHandledType;
    }

    void ResetInputHandledType()
    {
        inputHandledType = INPUT_NOT_HANDLED;
    }

    struct WheelDelta
    {
        float32 x;
        float32 y;
    };

    struct Gesture
    {
        float32 magnification; // delta -1..1
        float32 rotation; // delta angle in degrees -cw +ccw
        float32 dx; // -1..1 (-1 left)
        float32 dy; // -1..1 (-1 top)
    };

    union {
        uint32 touchId;
        Key key;
        char32_t keyChar; // unicode utf32 char
        MouseButton mouseButton;
        GamepadDevice::eDavaGamepadElement element;
        WheelDelta wheelDelta; // scroll delta in mouse wheel clicks (or lines)
        Gesture gesture; // pinch/rotate/swipe
    };
    Vector2 point; // point of pressure in virtual coordinates
    Vector2 physPoint; // point of pressure in physical coordinates
    float64 timestamp = 0.0; //(TODO not all platforms) time stemp of the event occurrence
    Phase phase = Phase::ERROR; // began, ended, moved. See Phase
    UIControl* touchLocker = nullptr; // control that handles this input
    int32 controlState = CONTROL_STATE_RELEASED; // input state relative to control (outside, inside). Used for point inputs only(mouse, touch)
    uint32 tapCount = 0; // (TODO not all platforms) count of the continuous inputs (clicks for mouse)
    Device device = Device::UNKNOWN;
    eInputHandledType inputHandledType = INPUT_NOT_HANDLED; //!< input handled type, INPUT_NOT_HANDLED by default.
};
};

#endif

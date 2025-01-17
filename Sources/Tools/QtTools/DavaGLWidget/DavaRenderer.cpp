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

#include "DavaRenderer.h"
#include "Render/RenderBase.h"

#include "Core/Core.h"
#include "Platform/Qt5/QtLayer.h"
#include "Input/InputSystem.h"
#include "Input/KeyboardDevice.h"

#include "Base/BaseTypes.h"
#include "Base/Singleton.h"

#include <QOpenGLContext>
#include <QApplication>

namespace DAVA
{
class DavaQtApplyModifier
{
public:
    void operator()(DAVA::KeyboardDevice& keyboard, Qt::KeyboardModifiers const& currentModifiers, Qt::KeyboardModifier qtModifier, DAVA::Key davaModifier)
    {
        if (true == (currentModifiers.testFlag(qtModifier)))
            keyboard.OnKeyPressed(davaModifier);
        else
            keyboard.OnKeyUnpressed(davaModifier);
    }
};
} // end namespace DAVA

namespace
{
class OGLContextBinder : public DAVA::Singleton<OGLContextBinder>
{
public:
    OGLContextBinder(QSurface* surface, QOpenGLContext* context)
        : renderSurface(surface)
        , renderContext(context)
    {
    }

    void AcquireContext()
    {
        prevContext = QOpenGLContext::currentContext();
        if (prevContext != nullptr)
            prevSurface = prevContext->surface();
        else
            prevSurface = nullptr;

        renderContext->makeCurrent(renderSurface);
    }

    void ReleaseContext()
    {
        renderContext->doneCurrent();

        if (prevContext != nullptr && prevSurface != nullptr)
            prevContext->makeCurrent(prevSurface);

        prevContext = nullptr;
        prevSurface = nullptr;
    }

private:
    QSurface* renderSurface = nullptr;
    QOpenGLContext* renderContext = nullptr;

    QSurface* prevSurface = nullptr;
    QOpenGLContext* prevContext = nullptr;
};

void AcqureContext()
{
    OGLContextBinder::Instance()->AcquireContext();
}

void ReleaseContext()
{
    OGLContextBinder::Instance()->ReleaseContext();
}
}

DavaRenderer::DavaRenderer(QSurface* surface, QOpenGLContext* context)
{
    DVASSERT(OGLContextBinder::Instance() == nullptr);
    new OGLContextBinder(surface, context);

    DAVA::Core::Instance()->rendererParams.acquireContextFunc = &AcqureContext;
    DAVA::Core::Instance()->rendererParams.releaseContextFunc = &ReleaseContext;

    DAVA::QtLayer::Instance()->AppStarted();
    DAVA::QtLayer::Instance()->OnResume();
}

DavaRenderer::~DavaRenderer()
{
    OGLContextBinder::Instance()->Release();
}

void DavaRenderer::paint()
{
    // HACK Qt send key event to widget with focus not globaly
    // if user hold ALT(CTRL, SHIFT) and then clicked DavaWidget(focused)
    // we miss key down event, so we have to check for SHIFT, ALT, CTRL
    // read about same problem http://stackoverflow.com/questions/23193038/how-to-detect-global-key-sequence-press-in-qt
    using namespace DAVA;
    Qt::KeyboardModifiers modifiers = qApp->keyboardModifiers();
    KeyboardDevice& keyboard = InputSystem::Instance()->GetKeyboard();
    DavaQtApplyModifier mod;
    mod(keyboard, modifiers, Qt::AltModifier, Key::LALT);
    mod(keyboard, modifiers, Qt::ShiftModifier, Key::LSHIFT);
    mod(keyboard, modifiers, Qt::ControlModifier, Key::LCTRL);

    if (DAVA::DVAssertMessage::IsHidden())
    {
        DAVA::QtLayer::Instance()->ProcessFrame();
    }
}

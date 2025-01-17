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


#import "Platform/TemplateMacOS/MainWindowController.h"
#include "Platform/TemplateMacOS/CorePlatformMacOS.h"
#include "Platform/DeviceInfo.h"
#include "Render/2D/Systems/RenderSystem2D.h"

#import <AppKit/NSApplication.h>
#import "Platform/TemplateMacOS/HelperAppDelegate.h"

using namespace DAVA;

@interface DavaApp : NSApplication
@end

@implementation DavaApp
- (void)sendEvent:(NSEvent*)theEvent
{
    // http://stackoverflow.com/questions/970707/cocoa-keyboard-shortcuts-in-dialog-without-an-edit-menu
    if ([theEvent type] == NSKeyDown)
    {
        int cmdOrCmdWithCaps = ([theEvent modifierFlags] & NSDeviceIndependentModifierFlagsMask);
        if ((cmdOrCmdWithCaps == NSCommandKeyMask) || (cmdOrCmdWithCaps == (NSCommandKeyMask | NSAlphaShiftKeyMask)))
        {
            if ([[[theEvent charactersIgnoringModifiers] lowercaseString] isEqualToString:@"x"])
            {
                if ([self sendAction:@selector(cut:) to:nil from:self])
                    return;
            }
            else if ([[[theEvent charactersIgnoringModifiers] lowercaseString] isEqualToString:@"c"])
            {
                if ([self sendAction:@selector(copy:) to:[[NSApp keyWindow] firstResponder] from:self])
                    return;
            }
            else if ([[[theEvent charactersIgnoringModifiers] lowercaseString] isEqualToString:@"v"])
            {
                // HACK if user trying to paste text into textfield
                // we have to check room for it
                // and if no more room skip paste operation here
                // because some time NSFormatter not called
                NSResponder* view = [[NSApp keyWindow] firstResponder];
                DAVA::UIControl* focused = DAVA::UIControlSystem::Instance()->GetFocusedControl();
                if (focused != nullptr)
                {
                    DAVA::UITextField* tf = dynamic_cast<DAVA::UITextField*>(focused);
                    if (tf)
                    {
                        DAVA::WideString text = tf->GetText();
                        int size = tf->GetMaxLength();
                        int textSize = static_cast<int>(text.length());
                        if (size > 0 && size > (textSize + 1))
                        {
                            if ([self sendAction:@selector(paste:) to:view from:self])
                                return;
                        }
                        else
                        {
                            // skip paste into no room textfield
                        }
                    }
                }
                else
                {
                    if ([self sendAction:@selector(paste:) to:view from:self])
                        return;
                }
            }
            else if ([[[theEvent charactersIgnoringModifiers] lowercaseString] isEqualToString:@"z"])
            {
                if ([self sendAction:@selector(undo:) to:nil from:self])
                    return;
            }
            else if ([[[theEvent charactersIgnoringModifiers] lowercaseString] isEqualToString:@"a"])
            {
                if ([self sendAction:@selector(selectAll:) to:nil from:self])
                    return;
            }
        }
    }

    // HACK first part if any textfield(native) is focused send keyUp and keyDown events to
    // openGLView manualy but only if current event not change focus control
    // need for client battle chat work and other textfield without modifications
    DAVA::UIControl* focusedCtrl = DAVA::UIControlSystem::Instance()->GetFocusedControl();

    // http://stackoverflow.com/questions/4001565/missing-keyup-events-on-meaningful-key-combinations-e-g-select-till-beginning?lq=1
    [super sendEvent:theEvent];
    if (theEvent.modifierFlags & NSCommandKeyMask)
    {
        if (theEvent.type == NSKeyUp)
        {
            [[NSNotificationCenter defaultCenter] postNotificationName:@"DavaKeyUp" object:theEvent];
        }
    }

    // HACK second part
    DAVA::UIControl* focusedAfterCtrl = DAVA::UIControlSystem::Instance()->GetFocusedControl();

    if (focusedCtrl != nullptr && focusedCtrl == focusedAfterCtrl)
    {
        DAVA::UITextField* tf = dynamic_cast<DAVA::UITextField*>(focusedCtrl);
        if (tf && tf->IsEditing())
        {
            if (theEvent.type == NSKeyDown || theEvent.type == NSKeyUp)
            {
                [[NSNotificationCenter defaultCenter] postNotificationName:@"DavaKey" object:theEvent];
            }
        }
    }
}

@end

extern void FrameworkDidLaunched();
extern void FrameworkWillTerminate();

namespace DAVA
{
int Core::Run(int argc, char* argv[], AppHandle handle)
{
    NSAutoreleasePool* globalPool = 0;
    globalPool = [[NSAutoreleasePool alloc] init];
    CoreMacOSPlatform* core = new CoreMacOSPlatform();
    core->SetCommandLine(argc, argv);
    core->CreateSingletons();

    // try to create delegate from client code
    Class delegateClass = NSClassFromString(@"MacOSHelperAppDelegate");
    if (nullptr == delegateClass)
    {
        // have no delegate in client code - create byself
        delegateClass = NSClassFromString(@"HelperAppDelegate");
    }

    DVASSERT_MSG(nullptr != delegateClass, "Cannot find NSApplicationDelegate class!");

    HelperAppDelegate* appDelegate = [[[delegateClass alloc] init] autorelease];

    MainWindowController* mainWindowController = [[[MainWindowController alloc] init] autorelease];

    // window controller used from app delegate
    [appDelegate setWindowController:mainWindowController];

    [[DavaApp sharedApplication] setDelegate:static_cast<id<NSApplicationDelegate>>(appDelegate)];

    // NSApplicationMain expects const char*[]
    const char* argvForNSApplicationMain[argc];
    for (int i = 0; i < argc; ++i)
    {
        argvForNSApplicationMain[i] = argv[i];
    }
    int retVal = NSApplicationMain(argc, argvForNSApplicationMain);

    // This method never returns, so release code transfered to termination message
    // - (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
    // core->ReleaseSingletons() is called from there

    [globalPool release];
    globalPool = 0;
    return retVal;
}

int Core::RunCmdTool(int argc, char* argv[], AppHandle handle)
{
    NSAutoreleasePool* globalPool = 0;
    globalPool = [[NSAutoreleasePool alloc] init];
    CoreMacOSPlatform* core = new CoreMacOSPlatform();
    core->SetCommandLine(argc, argv);
    core->EnableConsoleMode();
    core->CreateSingletons();

    Logger::Instance()->EnableConsoleMode();

    FrameworkDidLaunched();
    FrameworkWillTerminate();

    core->ReleaseSingletons();

    [globalPool release];
    globalPool = 0;
    return 0;
}
}

@interface MainWindowController ()
- (void)startAnimationTimer;
- (void)stopAnimationTimer;
- (void)animationTimerFired:(NSTimer*)timer;

- (void)windowWillMiniaturize:(NSNotification*)notification;
- (void)windowDidMiniaturize:(NSNotification*)notification;
- (void)windowDidDeminiaturize:(NSNotification*)notification;
- (void)OnKeyUpDuringCMDHold:(NSNotification*)notification;
- (void)OnKeyDuringTextFieldInFocus:(NSNotification*)notification;

- (void)setMinimumWindowSize:(DAVA::float32)width height:(DAVA::float32)height;
@end

@implementation MainWindowController

static MainWindowController* mainWindowController = nil;

/* This code disabled for now and left for the future
 */
namespace DAVA
{
void CoreMacOSPlatform::SetWindowMinimumSize(float32 width, float32 height)
{
    DVASSERT((width == 0.0f && height == 0.0f) || (width > 0.0f && height > 0.0f));
    minWindowWidth = width;
    minWindowHeight = height;

    [mainWindowController setMinimumWindowSize:minWindowWidth height:minWindowHeight];
}

Vector2 CoreMacOSPlatform::GetWindowMinimumSize() const
{
    return Vector2(minWindowWidth, minWindowHeight);
}
}

@synthesize willQuit;

- (id)init
{
    self = [super init];
    if (self)
    {
        mainWindowController = self;
        openGLView = nil;
        mainWindow = nil;
        animationTimer = nil;
        core = 0;
        assertionID = kIOPMNullAssertionID;
        willQuit = false;

        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(OnKeyUpDuringCMDHold:)
                                                     name:@"DavaKeyUp"
                                                   object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(OnKeyDuringTextFieldInFocus:)
                                                     name:@"DavaKey"
                                                   object:nil];

        [self allowDisplaySleep:false];
    }
    return self;
}

- (void)dealloc
{
    [self allowDisplaySleep:true];
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [super dealloc];
}

- (void)allowDisplaySleep:(bool)sleep
{
    bool displaySleepAllowed = assertionID == kIOPMNullAssertionID;
    if (sleep == displaySleepAllowed)
    {
        return;
    }

    IOReturn result;

    if (sleep)
    {
        result = IOPMAssertionRelease(assertionID);
        assertionID = kIOPMNullAssertionID;
    }
    else
    {
        result = IOPMAssertionCreateWithName(kIOPMAssertionTypeNoDisplaySleep,
                                             kIOPMAssertionLevelOn,
                                             CFSTR("DAVA display sleeping preventing"),
                                             &assertionID);
    }

    if (result != kIOReturnSuccess)
    {
        DVASSERT_MSG(false, "IOPM Assertion manipulation failed");
        return;
    }
}

- (void)createWindows
{
    FrameworkDidLaunched();

    core = Core::GetApplicationCore();

    String title;
    int32 width = 800;
    int32 height = 600;
    bool isFull = false;

    float32 minWidth = 0.0f;
    float32 minHeight = 0.0f;
    KeyedArchive* options = Core::Instance()->GetOptions();
    if (nullptr != options)
    {
        title = options->GetString("title", "[set application title using core options property 'title']");
        if (options->IsKeyExists("width") && options->IsKeyExists("height"))
        {
            width = options->GetInt32("width");
            height = options->GetInt32("height");
        }

        isFull = (0 != options->GetInt32("fullscreen", 0));
        minWidth = static_cast<float32>(options->GetInt32("min-width", 0));
        minHeight = static_cast<float32>(options->GetInt32("min-height", 0));
    }

    openGLView = [[OpenGLView alloc] initWithFrame:NSMakeRect(0, 0, width, height)];

    NSRect displayRect = [[NSScreen mainScreen] frame];
    NSUInteger wStyle = NSTitledWindowMask + NSMiniaturizableWindowMask + NSClosableWindowMask + NSResizableWindowMask;
    NSRect wRect = NSMakeRect((displayRect.size.width - width) / 2, (displayRect.size.height - height) / 2, width, height);
    mainWindow = [[NSWindow alloc] initWithContentRect:wRect styleMask:wStyle backing:NSBackingStoreBuffered defer:FALSE];
    [mainWindow setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
    [mainWindow setDelegate:self];
    [mainWindow setContentView:openGLView];
    mainWindow.contentMinSize = NSMakeSize(width, height);
    [mainWindowController setMinimumWindowSize:0.0f height:0.0f];
    if (minWidth > 0 && minHeight > 0)
    {
        // Call Core::SetWindowMinimumSize to save minimum width and height and limit window size
        // Such a strange way due to my little knowledge of Objective-C
        Core::Instance()->SetWindowMinimumSize(minWidth, minHeight);
    }

    // start animation
    currFPS = Renderer::GetDesiredFPS();
    [self startAnimationTimer];

    // make window main
    [mainWindow makeKeyAndOrderFront:nil];
    [mainWindow setTitle:[NSString stringWithFormat:@"%s", title.c_str()]];
    [mainWindow setAcceptsMouseMovedEvents:YES];

    if (isFull)
    {
        [self setFullScreen:true];
    }

    NSSize windowSize = [openGLView frame].size;
    float32 backingScale = Core::Instance()->GetScreenScaleFactor();

    GLint backingSize[2] = { GLint(windowSize.width * backingScale), GLint(windowSize.height * backingScale) };
    CGLSetParameter([[openGLView openGLContext] CGLContextObj], kCGLCPSurfaceBackingSize, backingSize);
    CGLEnable([[openGLView openGLContext] CGLContextObj], kCGLCESurfaceBackingSize);
    CGLUpdateContext([[openGLView openGLContext] CGLContextObj]);

    float32 scale = DeviceInfo::GetScreenInfo().scale;
    Core::Instance()->InitWindowSize(openGLView, windowSize.width, windowSize.height, scale, scale);
}

- (void)setMinimumWindowSize:(float32)width height:(float32)height
{
    const float32 MIN_WIDTH = 64.0f;
    const float32 MIN_HEIGHT = 64.0f;

    // Always limit minimum window size to 64x64, as application crashes
    // when resizing window to zero height (stack overflow occures)
    // It seems that NSOpenGLView is responsible for that
    if (width < MIN_WIDTH)
        width = MIN_WIDTH;
    if (height < MIN_HEIGHT)
        height = MIN_HEIGHT;
    mainWindow.contentMinSize = NSMakeSize(width, height);
}

- (void)windowWillMiniaturize:(NSNotification*)notification
{
}

- (void)windowDidMiniaturize:(NSNotification*)notification
{
    CoreMacOSPlatformBase* xcore = static_cast<CoreMacOSPlatformBase*>(Core::Instance());
    xcore->signalAppMinimizedRestored.Emit(true);

    [self OnSuspend];
}

- (void)windowDidDeminiaturize:(NSNotification*)notification
{
    CoreMacOSPlatformBase* xcore = static_cast<CoreMacOSPlatformBase*>(Core::Instance());
    xcore->signalAppMinimizedRestored.Emit(false);

    [self OnResume];
}

- (void)windowDidBecomeKey:(NSNotification*)notification
{
    Core::Instance()->FocusReceived();
}

- (void)windowDidResignKey:(NSNotification*)notification
{
    Core::Instance()->FocusLost();
    InputSystem::Instance()->GetKeyboard().ClearAllKeys();
}

- (void)windowDidEnterFullScreen:(NSNotification*)notification
{
    fullScreen = true;
    Core::Instance()->GetApplicationCore()->OnEnterFullscreen();
}

- (void)windowDidExitFullScreen:(NSNotification*)notification
{
    fullScreen = false;
    Core::Instance()->GetApplicationCore()->OnExitFullscreen();
}

- (void)OnKeyUpDuringCMDHold:(NSNotification*)notification
{
    [self keyUp:static_cast<NSEvent*>([notification object])];
}

- (void)OnKeyDuringTextFieldInFocus:(NSNotification*)notification
{
    NSEvent* theEvent = static_cast<NSEvent*>([notification object]);

    if (theEvent.type == NSKeyDown)
    {
        [self keyDown:theEvent];
    }
    else if (theEvent.type == NSKeyUp)
    {
        [self keyUp:theEvent];
    }
}

- (bool)isFullScreen
{
    return fullScreen;
}

- (bool)setFullScreen:(bool)_fullScreen
{
    if (fullScreen != _fullScreen)
    {
        double macOSVer = floor(NSAppKitVersionNumber);
        // fullscreen for new 10.7+ MacOS
        if (macOSVer >= NSAppKitVersionNumber10_7)
        {
            // just toggle current state
            // fullScreen variable will be set in windowDidEnterFullScreen/windowDidExitFullScreen callbacks
            [mainWindowController->mainWindow toggleFullScreen:nil];
            return YES;
        }
        else
        {
            // fullscreen for older macOS isn't supperted
            DVASSERT_MSG(false, "Fullscreen isn't supported for this MacOS version");
            return NO;
        }
    }
    return YES;
}

- (void)keyDown:(NSEvent*)event
{
    [openGLView keyDown:event];
}

- (void)keyUp:(NSEvent*)event
{
    [openGLView keyUp:event];
}

- (void)flagsChanged:(NSEvent*)event
{
    [openGLView flagsChanged:event];
}

- (void)mouseDown:(NSEvent*)theEvent
{
    [openGLView mouseDown:theEvent];
}

- (void)scrollWheel:(NSEvent*)theEvent
{
    [openGLView scrollWheel:theEvent];
}

- (void)mouseMoved:(NSEvent*)theEvent
{
    [openGLView mouseMoved:theEvent];
}

- (void)mouseUp:(NSEvent*)theEvent
{
    [openGLView mouseUp:theEvent];
}

- (void)mouseDragged:(NSEvent*)theEvent
{
    [openGLView mouseDragged:theEvent];
}

- (void)mouseEntered:(NSEvent*)theEvent
{
}

- (void)rightMouseDown:(NSEvent*)theEvent
{
    [openGLView rightMouseDown:theEvent];
}

- (void)rightMouseDragged:(NSEvent*)theEvent
{
    [openGLView rightMouseDragged:theEvent];
}

- (void)rightMouseUp:(NSEvent*)theEvent
{
    [openGLView rightMouseUp:theEvent];
}

- (void)otherMouseDown:(NSEvent*)theEvent
{
    [openGLView otherMouseDown:theEvent];
}

- (void)otherMouseDragged:(NSEvent*)theEvent
{
    [openGLView otherMouseDragged:theEvent];
}

- (void)otherMouseUp:(NSEvent*)theEvent
{
    [openGLView otherMouseUp:theEvent];
}

- (void)startAnimationTimer
{
    if (animationTimer == nil)
    {
        animationTimer = [[NSTimer scheduledTimerWithTimeInterval:1.0f / currFPS target:self selector:@selector(animationTimerFired:) userInfo:nil repeats:YES] retain];
    }
}

- (void)stopAnimationTimer
{
    if (animationTimer != nil)
    {
        [animationTimer invalidate];
        [animationTimer release];
        animationTimer = nil;
    }
}

- (void)animationTimerFired:(NSTimer*)timer
{
    if (willQuit)
        return;

    DAVA::Core::Instance()->SystemProcessFrame();

    if (currFPS != Renderer::GetDesiredFPS())
    {
        currFPS = Renderer::GetDesiredFPS();
        [self stopAnimationTimer];
        [self startAnimationTimer];
    }
}

- (void)OnSuspend
{
    [self allowDisplaySleep:true];

    if (core)
    {
        core->OnSuspend();
    }
    else
    {
        Core::Instance()->SetIsActive(false);
    }
}

- (void)OnResume
{
    [self allowDisplaySleep:false];

    if (core)
    {
        core->OnResume();
    }
    else
    {
        Core::Instance()->SetIsActive(true);
    }
}

@end

namespace DAVA
{
Core::eScreenMode CoreMacOSPlatform::GetScreenMode()
{
    return ([mainWindowController isFullScreen]) ? Core::eScreenMode::FULLSCREEN : Core::eScreenMode::WINDOWED;
}

bool CoreMacOSPlatform::SetScreenMode(eScreenMode screenMode)
{
    if (screenMode == Core::eScreenMode::FULLSCREEN)
    {
        return [mainWindowController setFullScreen:true] == YES;
    }
    else if (screenMode == Core::eScreenMode::WINDOWED)
    {
        return [mainWindowController setFullScreen:false] == YES;
    }
    else
    {
        Logger::Error("Unsupported screen mode");
        return false;
    }
}

void CoreMacOSPlatform::Quit()
{
    mainWindowController->willQuit = true;
    [[NSApplication sharedApplication] terminate:nil];
}

void CoreMacOSPlatform::SetScreenScaleMultiplier(float32 multiplier)
{
    if (!FLOAT_EQUAL(Core::GetScreenScaleMultiplier(), multiplier))
    {
        Core::SetScreenScaleMultiplier(multiplier);

        //This magick needed to correctly 'reshape' GLView and resize back-buffer.
        //Directly call [openGLView reshape] doesn't help, as an other similar 'tricks'
        [mainWindowController->mainWindow setContentView:nil];
        [mainWindowController->mainWindow setContentView:mainWindowController->openGLView];
        [mainWindowController->mainWindow makeFirstResponder:mainWindowController->openGLView];
    }
}
};

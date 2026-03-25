// GLWindow implementation for Linux using raw X11+GLX (no SDL2 dependency)
#ifndef _WIN32

#include "basetypes.h"
#include <mutex>
#include <GL/glew.h>
#include <GL/gl.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#include <cstdio>
#include <cstring>
#include "GLWindow.h"
#include "NuanceMain.h"
#include "NuanceUI.h"
#include "video.h"
#include "joystick.h"

extern vidTexInfo videoTexInfo;
extern std::mutex gfx_lock;

static Display* xDisplay = nullptr;
static Window xWindow = 0;
static GLXContext glxContext = nullptr;
static Atom wmDeleteMessage;

GLWindow::GLWindow()
{
  bFullScreen = false;
  bVisible = false;
  keyDownHandler = nullptr;
  keyUpHandler = nullptr;
  paintHandler = nullptr;
  resizeHandler = nullptr;
  applyControllerState = nullptr;
  inputManager = nullptr;
  clientWidth = VIDEO_WIDTH;
  clientHeight = VIDEO_HEIGHT;
  fullScreenWidth = 1920;
  fullScreenHeight = 1080;
  x = 100; y = 100;
  width = clientWidth; height = clientHeight;
  windowStyle = 0; windowExtendedStyle = 0;
  fullScreenWindowStyle = 0; fullScreenWindowExtendedStyle = 0;
  threadHandle = 0; threadID = 0;
  restoreWidth = clientWidth; restoreHeight = clientHeight;
  restoreX = x; restoreY = y;
  hInstance = nullptr; hWnd = nullptr; hDC = nullptr; hRC = nullptr;
}

GLWindow::~GLWindow() {}

void GLWindow::UpdateRestoreValues()
{
  restoreX = x; restoreY = y;
  restoreWidth = clientWidth; restoreHeight = clientHeight;
}

void GLWindow::OnResize(int _width, int _height)
{
  clientWidth = _width;
  clientHeight = _height;
  if (!bFullScreen) UpdateRestoreValues();
}

void GLWindow::ToggleFullscreen()
{
  // Simple fullscreen toggle via _NET_WM_STATE
  if (!xDisplay || !xWindow) return;
  bFullScreen = !bFullScreen;

  XEvent ev = {};
  ev.type = ClientMessage;
  ev.xclient.window = xWindow;
  ev.xclient.message_type = XInternAtom(xDisplay, "_NET_WM_STATE", False);
  ev.xclient.format = 32;
  ev.xclient.data.l[0] = bFullScreen ? 1 : 0; // _NET_WM_STATE_ADD / REMOVE
  ev.xclient.data.l[1] = XInternAtom(xDisplay, "_NET_WM_STATE_FULLSCREEN", False);
  ev.xclient.data.l[2] = 0;
  XSendEvent(xDisplay, DefaultRootWindow(xDisplay), False, SubstructureRedirectMask | SubstructureNotifyMask, &ev);
  XFlush(xDisplay);

  // Re-grab keyboard focus after fullscreen toggle
  if (bFullScreen) {
    XGrabKeyboard(xDisplay, xWindow, True, GrabModeAsync, GrabModeAsync, CurrentTime);
  } else {
    XUngrabKeyboard(xDisplay, CurrentTime);
  }
  XSetInputFocus(xDisplay, xWindow, RevertToParent, CurrentTime);
  XFlush(xDisplay);
}

bool GLWindow::ChangeScreenResolution(int, int) { return true; }

bool GLWindow::CreateWindowGL()
{
  xDisplay = XOpenDisplay(nullptr);
  if (!xDisplay) {
    fprintf(stderr, "Cannot open X display\n");
    return false;
  }

  int screen = DefaultScreen(xDisplay);

  int glxAttribs[] = {
    GLX_RGBA,
    GLX_DOUBLEBUFFER,
    GLX_RED_SIZE, 8,
    GLX_GREEN_SIZE, 8,
    GLX_BLUE_SIZE, 8,
    None
  };

  XVisualInfo* vi = glXChooseVisual(xDisplay, screen, glxAttribs);
  if (!vi) {
    fprintf(stderr, "glXChooseVisual failed\n");
    return false;
  }

  Colormap cmap = XCreateColormap(xDisplay, RootWindow(xDisplay, vi->screen), vi->visual, AllocNone);

  XSetWindowAttributes swa = {};
  swa.colormap = cmap;
  swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | StructureNotifyMask | FocusChangeMask
    | ButtonPressMask | ButtonReleaseMask | PointerMotionMask;

  xWindow = XCreateWindow(xDisplay, RootWindow(xDisplay, vi->screen),
    100, 100, clientWidth, clientHeight, 0,
    vi->depth, InputOutput, vi->visual,
    CWColormap | CWEventMask, &swa);

  XStoreName(xDisplay, xWindow, "Nuance (F1 to toggle fullscreen)");

  wmDeleteMessage = XInternAtom(xDisplay, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(xDisplay, xWindow, &wmDeleteMessage, 1);

  XMapWindow(xDisplay, xWindow);

  glxContext = glXCreateContext(xDisplay, vi, nullptr, GL_TRUE);
  XFree(vi);

  if (!glxContext) {
    fprintf(stderr, "glXCreateContext failed\n");
    return false;
  }

  glXMakeCurrent(xDisplay, xWindow, glxContext);

  fprintf(stderr, "GL Vendor: %s\nGL Renderer: %s\nGL Version: %s\n",
    glGetString(GL_VENDOR), glGetString(GL_RENDERER), glGetString(GL_VERSION));

  glewExperimental = GL_TRUE;
  GLenum err = glewInit();
  if (err != GLEW_OK) {
    fprintf(stderr, "glewInit failed (code %d): %s\n", (int)err, glewGetErrorString(err));
    fprintf(stderr, "Continuing without GLEW...\n");
  } else {
    fprintf(stderr, "GLEW initialized OK\n");
  }
  fflush(stderr);

  NuanceUI_Init(xDisplay, xWindow);

  inputManager = InputManager::Create();
  if (!inputManager->Init(nullptr)) {
    delete inputManager;
    inputManager = nullptr;
  }

  bVisible = true;
  return true;
}

void GLWindow::CleanUp()
{
  NuanceUI_Shutdown();
  delete inputManager;
  inputManager = nullptr;

  if (glxContext) {
    glXMakeCurrent(xDisplay, None, nullptr);
    glXDestroyContext(xDisplay, glxContext);
    glxContext = nullptr;
  }
  if (xWindow) { XDestroyWindow(xDisplay, xWindow); xWindow = 0; }
  if (xDisplay) { XCloseDisplay(xDisplay); xDisplay = nullptr; }
}

bool GLWindow::RegisterWindowClass() { return true; }

bool GLWindow::Create()
{
  GLWindowMain(this);
  return true;
}

static int XKeyToVKey(KeySym key)
{
  if (key >= XK_a && key <= XK_z) return 'A' + (key - XK_a);
  if (key >= XK_A && key <= XK_Z) return 'A' + (key - XK_A);
  if (key >= XK_0 && key <= XK_9) return '0' + (key - XK_0);
  switch (key) {
    case XK_Up: return VK_UP;
    case XK_Down: return VK_DOWN;
    case XK_Left: return VK_LEFT;
    case XK_Right: return VK_RIGHT;
    case XK_Return: return VK_RETURN;
    case XK_space: return VK_SPACE;
    case XK_Escape: return VK_ESCAPE;
    case XK_F1: return VK_F1;
    default: return key & 0xFF;
  }
}

void GLWindow::MessagePump()
{
  if (inputManager) inputManager->UpdateState(applyControllerState, nullptr, nullptr);

  if (!xDisplay) return;

  while (XPending(xDisplay)) {
    XEvent ev;
    XNextEvent(xDisplay, &ev);

    // Let ImGui process events first; if UI captures input, skip game input
    bool uiCaptured = NuanceUI_ProcessEvent(&ev);

    switch (ev.type) {
      case ConfigureNotify:
      {
        int newW = ev.xconfigure.width;
        int newH = ev.xconfigure.height;
        if (newW != clientWidth || newH != clientHeight) {
          videoTexInfo.bUpdateDisplayList = true;
          videoTexInfo.bUpdateTextureStates = true;
          OnResize(newW, newH);
          if (resizeHandler) resizeHandler((uint16)newW, (uint16)newH);
        }
        break;
      }
      case KeyPress:
      {
        if (uiCaptured) break;
        KeySym key = XLookupKeysym(&ev.xkey, 0);
        int vkey = XKeyToVKey(key);
        if (inputManager) inputManager->keyDown(applyControllerState, (int16)vkey);
        if (vkey == VK_F1 || (vkey == VK_ESCAPE && bFullScreen))
          ToggleFullscreen();
        break;
      }
      case KeyRelease:
      {
        if (uiCaptured) break;
        // Filter out X11 autorepeat
        if (XEventsQueued(xDisplay, QueuedAfterReading)) {
          XEvent next;
          XPeekEvent(xDisplay, &next);
          if (next.type == KeyPress && next.xkey.time == ev.xkey.time && next.xkey.keycode == ev.xkey.keycode) {
            XNextEvent(xDisplay, &next);
            break;
          }
        }
        KeySym key = XLookupKeysym(&ev.xkey, 0);
        int vkey = XKeyToVKey(key);
        if (inputManager) inputManager->keyUp(applyControllerState, (int16)vkey);
        break;
      }
      case ClientMessage:
        if ((Atom)ev.xclient.data.l[0] == wmDeleteMessage)
          bQuit = true;
        break;
    }
  }
}

unsigned WINAPI GLWindow::GLWindowMain(void *param)
{
  GLWindow* glWindow = (GLWindow*)param;
  if (glWindow->CreateWindowGL()) {
    glWindow->OnResize(glWindow->clientWidth, glWindow->clientHeight);
  }
  return 1;
}

// X11 swap buffers
void SDL2_SwapWindow()
{
  if (xDisplay && xWindow) glXSwapBuffers(xDisplay, xWindow);
}

#endif // !_WIN32

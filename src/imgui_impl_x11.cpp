// Minimal ImGui platform backend for raw X11 (no SDL/GLFW dependency).
// Translates XEvents into ImGuiIO state for mouse/keyboard/focus/scroll.
//
// What this implements:
//   - Mouse position / buttons / wheel
//   - Keyboard (mapped X11 KeySyms -> ImGuiKey)
//   - Text input (XLookupString)
//   - Focus tracking (FocusIn/FocusOut)
//   - DisplaySize from current window geometry (queried each NewFrame)
//   - DeltaTime from monotonic clock
//
// What it deliberately doesn't do:
//   - Multi-viewport / IME / clipboard. Phase 1 status panel doesn't need them.
//   - Cursor shape switching. The X cursor stays as the window's default.
#include "imgui_impl_x11.h"

#ifndef IMGUI_DISABLE

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <time.h>

namespace {
::Display* g_Display = nullptr;
::Window   g_Window  = 0;
double     g_LastTimeSec = 0.0;
}

static double NowSec()
{
    timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

static ImGuiKey KeySymToImGuiKey(KeySym k)
{
    switch (k) {
        case XK_Tab:        return ImGuiKey_Tab;
        case XK_Left:       return ImGuiKey_LeftArrow;
        case XK_Right:      return ImGuiKey_RightArrow;
        case XK_Up:         return ImGuiKey_UpArrow;
        case XK_Down:       return ImGuiKey_DownArrow;
        case XK_Prior:      return ImGuiKey_PageUp;
        case XK_Next:       return ImGuiKey_PageDown;
        case XK_Home:       return ImGuiKey_Home;
        case XK_End:        return ImGuiKey_End;
        case XK_Insert:     return ImGuiKey_Insert;
        case XK_Delete:     return ImGuiKey_Delete;
        case XK_BackSpace:  return ImGuiKey_Backspace;
        case XK_space:      return ImGuiKey_Space;
        case XK_Return:     return ImGuiKey_Enter;
        case XK_Escape:     return ImGuiKey_Escape;
        case XK_apostrophe: return ImGuiKey_Apostrophe;
        case XK_comma:      return ImGuiKey_Comma;
        case XK_minus:      return ImGuiKey_Minus;
        case XK_period:     return ImGuiKey_Period;
        case XK_slash:      return ImGuiKey_Slash;
        case XK_semicolon:  return ImGuiKey_Semicolon;
        case XK_equal:      return ImGuiKey_Equal;
        case XK_bracketleft:  return ImGuiKey_LeftBracket;
        case XK_backslash:    return ImGuiKey_Backslash;
        case XK_bracketright: return ImGuiKey_RightBracket;
        case XK_grave:        return ImGuiKey_GraveAccent;
        case XK_Caps_Lock:    return ImGuiKey_CapsLock;
        case XK_Scroll_Lock:  return ImGuiKey_ScrollLock;
        case XK_Num_Lock:     return ImGuiKey_NumLock;
        case XK_Print:        return ImGuiKey_PrintScreen;
        case XK_Pause:        return ImGuiKey_Pause;
        case XK_Control_L:    return ImGuiKey_LeftCtrl;
        case XK_Shift_L:      return ImGuiKey_LeftShift;
        case XK_Alt_L:        return ImGuiKey_LeftAlt;
        case XK_Super_L:      return ImGuiKey_LeftSuper;
        case XK_Control_R:    return ImGuiKey_RightCtrl;
        case XK_Shift_R:      return ImGuiKey_RightShift;
        case XK_Alt_R:        return ImGuiKey_RightAlt;
        case XK_Super_R:      return ImGuiKey_RightSuper;
        case XK_F1:  return ImGuiKey_F1;   case XK_F2:  return ImGuiKey_F2;
        case XK_F3:  return ImGuiKey_F3;   case XK_F4:  return ImGuiKey_F4;
        case XK_F5:  return ImGuiKey_F5;   case XK_F6:  return ImGuiKey_F6;
        case XK_F7:  return ImGuiKey_F7;   case XK_F8:  return ImGuiKey_F8;
        case XK_F9:  return ImGuiKey_F9;   case XK_F10: return ImGuiKey_F10;
        case XK_F11: return ImGuiKey_F11;  case XK_F12: return ImGuiKey_F12;
        default: break;
    }
    if (k >= XK_0 && k <= XK_9) return (ImGuiKey)(ImGuiKey_0 + (k - XK_0));
    if (k >= XK_a && k <= XK_z) return (ImGuiKey)(ImGuiKey_A + (k - XK_a));
    if (k >= XK_A && k <= XK_Z) return (ImGuiKey)(ImGuiKey_A + (k - XK_A));
    if (k >= XK_KP_0 && k <= XK_KP_9) return (ImGuiKey)(ImGuiKey_Keypad0 + (k - XK_KP_0));
    return ImGuiKey_None;
}

bool ImGui_ImplX11_Init(::Display* display, ::Window window)
{
    g_Display = display;
    g_Window = window;
    g_LastTimeSec = NowSec();

    ImGuiIO& io = ImGui::GetIO();
    io.BackendPlatformName = "imgui_impl_x11";
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
    return true;
}

void ImGui_ImplX11_Shutdown()
{
    ImGuiIO& io = ImGui::GetIO();
    io.BackendPlatformName = nullptr;
    g_Display = nullptr;
    g_Window = 0;
}

void ImGui_ImplX11_NewFrame()
{
    ImGuiIO& io = ImGui::GetIO();

    if (g_Display && g_Window) {
        ::Window root; int x, y; unsigned int w, h, border, depth;
        if (XGetGeometry(g_Display, g_Window, &root, &x, &y, &w, &h, &border, &depth))
            io.DisplaySize = ImVec2((float)w, (float)h);
    }

    const double now = NowSec();
    io.DeltaTime = (float)((now - g_LastTimeSec) > 0.0 ? (now - g_LastTimeSec) : 1.0/60.0);
    g_LastTimeSec = now;
}

bool ImGui_ImplX11_ProcessEvent(const ::XEvent* ev)
{
    if (!g_Display) return false;
    ImGuiIO& io = ImGui::GetIO();

    switch (ev->type) {
        case MotionNotify:
            io.AddMousePosEvent((float)ev->xmotion.x, (float)ev->xmotion.y);
            return io.WantCaptureMouse;
        case ButtonPress:
        case ButtonRelease: {
            const bool down = (ev->type == ButtonPress);
            const unsigned int b = ev->xbutton.button;
            // X11: 1=L, 2=M, 3=R, 4/5=wheel up/down, 6/7=wheel left/right
            if (b == 1) io.AddMouseButtonEvent(0, down);
            else if (b == 2) io.AddMouseButtonEvent(2, down);
            else if (b == 3) io.AddMouseButtonEvent(1, down);
            else if (down && (b == 4 || b == 5)) io.AddMouseWheelEvent(0.0f, b == 4 ? +1.0f : -1.0f);
            else if (down && (b == 6 || b == 7)) io.AddMouseWheelEvent(b == 6 ? -1.0f : +1.0f, 0.0f);
            return io.WantCaptureMouse;
        }
        case KeyPress:
        case KeyRelease: {
            const bool down = (ev->type == KeyPress);
            KeySym ks = XLookupKeysym(const_cast<XKeyEvent*>(&ev->xkey), 0);
            ImGuiKey key = KeySymToImGuiKey(ks);
            if (key != ImGuiKey_None)
                io.AddKeyEvent(key, down);

            io.AddKeyEvent(ImGuiMod_Ctrl,  (ev->xkey.state & ControlMask) != 0);
            io.AddKeyEvent(ImGuiMod_Shift, (ev->xkey.state & ShiftMask) != 0);
            io.AddKeyEvent(ImGuiMod_Alt,   (ev->xkey.state & Mod1Mask) != 0);
            io.AddKeyEvent(ImGuiMod_Super, (ev->xkey.state & Mod4Mask) != 0);

            if (down) {
                char buf[16] = {};
                KeySym ignored;
                int n = XLookupString(const_cast<XKeyEvent*>(&ev->xkey), buf, sizeof(buf) - 1, &ignored, nullptr);
                for (int i = 0; i < n; i++)
                    if ((unsigned char)buf[i] >= 32) io.AddInputCharacter((unsigned)buf[i]);
            }
            return io.WantCaptureKeyboard;
        }
        case FocusIn:  io.AddFocusEvent(true);  return false;
        case FocusOut: io.AddFocusEvent(false); return false;
        default: break;
    }
    return false;
}

#endif // IMGUI_DISABLE

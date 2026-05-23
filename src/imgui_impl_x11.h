// Minimal ImGui platform backend for raw X11 (no SDL/GLFW dependency).
// Pairs with imgui_impl_opengl3 for rendering. Linux/standalone build only.
#pragma once

#include "../external/imgui/imgui.h"

#ifndef IMGUI_DISABLE

struct _XDisplay;
typedef struct _XDisplay Display;
typedef unsigned long XID;
typedef XID Window;
typedef union _XEvent XEvent;

bool ImGui_ImplX11_Init(Display* display, Window window);
void ImGui_ImplX11_Shutdown();
void ImGui_ImplX11_NewFrame();
bool ImGui_ImplX11_ProcessEvent(const XEvent* event);

#endif // IMGUI_DISABLE

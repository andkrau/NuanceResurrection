#ifndef NUANCE_UI_H
#define NUANCE_UI_H

void NuanceUI_Init(void* display, unsigned long window);
void NuanceUI_Shutdown();
bool NuanceUI_ProcessEvent(void* event);
void NuanceUI_Render();
bool NuanceUI_IsVisible();
void NuanceUI_SetVisible(bool v);
void NuanceUI_TogglePause();
void NuanceUI_UpdateTitle(int kcs, int fps);

#endif

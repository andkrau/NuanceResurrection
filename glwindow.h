#ifndef GLWindowH
#define GLWindowH

#include "basetypes.h"
#include "InputManager.h"
#include <windows.h>

#define bUseSeparateThread false

//---------------------------------------------------------------------------
typedef bool (* GLWINDOW_CALLBACK)(WPARAM wparam, LPARAM lparam);
typedef bool (* GLWINDOW_RESIZECALLBACK)(unsigned __int16 width, unsigned __int16 height);
typedef bool (* GLWINDOW_KEYCALLBACK)(__int16 vkey, unsigned __int32 keydata);

#define WM_TOGGLEFULLSCREEN (WM_USER+1)

#ifndef CDS_FULLSCREEN
#define CDS_FULLSCREEN (4)
#endif

class GLWindow
{
public:
  HINSTANCE hInstance;
  HWND  hWnd;
  HDC   hDC;
  HGLRC hRC;

  int x, y;
  int width, height;
  int clientWidth, clientHeight;
  int fullScreenWidth, fullScreenHeight;
  volatile bool bVisible;
  bool bFullScreen;

  GLWindow();
  ~GLWindow();

  bool Create();
  void MessagePump();
  void ToggleFullscreen();
  void UpdateRestoreValues();
  InputManager* GetInputManager() { return inputManager; }

  GLWINDOW_KEYCALLBACK keyDownHandler;
  GLWINDOW_KEYCALLBACK keyUpHandler;
  GLWINDOW_RESIZECALLBACK resizeHandler;
  GLWINDOW_CALLBACK paintHandler;
  InputManager::CONTROLLER_CALLBACK applyControllerState;

private:
  void OnResize(int _width, int _height);
  bool CreateWindowGL();
  bool RegisterWindowClass();
  void CleanUp();
  bool ChangeScreenResolution(int width, int height);

  static unsigned WINAPI GLWindowMain(void *glWindow);
  static LRESULT CALLBACK GLWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

  uintptr_t threadHandle;
  unsigned int threadID;

  uint32 windowStyle, windowExtendedStyle;
  uint32 fullScreenWindowStyle, fullScreenWindowExtendedStyle;

  InputManager *inputManager;

  int restoreWidth;
  int restoreHeight;
  int restoreX;
  int restoreY;
};

#endif

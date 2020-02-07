//---------------------------------------------------------------------------
#ifndef GLWindowH
#define GLWindowH

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

  const char *className;

  int x, y;
  int width, height;
  int clientWidth, clientHeight;
  int fullScreenWidth, fullScreenHeight;
  volatile bool bVisible;
  bool bFullScreen;
  const char *title;

  GLWindow();
  ~GLWindow();
  bool Create();
  void MessagePump();
  void ToggleFullscreen();
  void UpdateRestoreValues();

  GLWINDOW_KEYCALLBACK keyDownHandler;
  GLWINDOW_KEYCALLBACK keyUpHandler;
  GLWINDOW_RESIZECALLBACK resizeHandler;
  GLWINDOW_CALLBACK createHandler;
  GLWINDOW_CALLBACK closeHandler;
  GLWINDOW_CALLBACK closeQueryHandler;
  GLWINDOW_CALLBACK destroyHandler;
  GLWINDOW_CALLBACK paintHandler;
protected:

  void OnResize(int width, int height);
  bool CreateWindowGL();
  bool RegisterWindowClass();
  void CleanUp();
  static DWORD WINAPI GLWindowMain(void *glWindow);
  static LRESULT CALLBACK GLWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
  bool ChangeScreenResolution(int width, int height);
  unsigned long threadID;
  uint32 windowStyle, windowExtendedStyle;
  uint32 fullScreenWindowStyle, fullScreenWindowExtendedStyle;
  HANDLE threadHandle;
  int restoreWidth;
  int restoreHeight;
  int restoreX;
  int restoreY;
};

#endif

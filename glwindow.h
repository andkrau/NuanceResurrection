//---------------------------------------------------------------------------
#ifndef GLWindowH
#define GLWindowH

#include <windows.h>
//---------------------------------------------------------------------------
typedef bool (* GLWINDOW_CALLBACK)(WPARAM wparam, LPARAM lparam);
typedef bool (* GLWINDOW_RESIZECALLBACK)(unsigned __int16 width, unsigned __int16 height);
typedef bool (* GLWINDOW_KEYCALLBACK)(__int16 vkey, unsigned __int32 keydata);
typedef void (* GLWINDOW_TIMERCALLBACK)(unsigned __int32 idEvent);

#define WM_TOGGLEFULLSCREEN (WM_USER+1)

#ifndef CDS_FULLSCREEN
#define CDS_FULLSCREEN (4)
#endif

class GLWindow
{
public:
	HINSTANCE	hInstance;
	HWND hWnd;
	HDC	hDC;
	HGLRC	hRC;

	const char *className;

  int x, y;
  int width, height;
  int clientWidth, clientHeight;
  int fullScreenWidth, fullScreenHeight;
  int bitsPerPixel;
  volatile bool bVisible;
  bool bFullScreen;
  bool bUseSeparateThread;
  const char *title;
  unsigned int timerInterval;
  unsigned int timerID;

  GLWindow();
  ~GLWindow();
  bool Create();
  void Close();
  bool MessagePump();
  void ToggleFullscreen();
  void UpdateRestoreValues();
  bool KillTimer();
  bool SetTimer();
  GLWINDOW_KEYCALLBACK keyDownHandler;
  GLWINDOW_KEYCALLBACK keyUpHandler;
  GLWINDOW_TIMERCALLBACK timerHandler;
  GLWINDOW_RESIZECALLBACK resizeHandler;
  GLWINDOW_CALLBACK idleHandler;
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
  static void CALLBACK MMTimerCallback(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);
  bool ChangeScreenResolution(int width, int height, int bitsPerPixel);
  bool bCreated;
  unsigned long threadID;
  unsigned __int32 windowStyle, windowExtendedStyle;
  unsigned __int32 fullScreenWindowStyle, fullScreenWindowExtendedStyle;
  HANDLE threadHandle;
  int restoreWidth;
  int restoreHeight;
  int restoreX;
  int restoreY;
};

#endif

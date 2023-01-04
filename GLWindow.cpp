#include "basetypes.h"
#include <windows.h>
#include <tchar.h>
#include <mutex>
#include "external\glew-2.2.0\include\GL\glew.h"
#include <GL/gl.h>
#include "GLWindow.h"
#include "video.h"

/****************************************************************************
OpenGL Window Code
****************************************************************************/

extern vidTexInfo videoTexInfo;

extern std::mutex gfx_lock;

GLWindow::GLWindow()
{
  bFullScreen = false; // starting fullscreen from the start is broken at the moment, so leave at false and rather toggle dynamically later on

  bVisible = false;

  keyDownHandler = 0;
  keyUpHandler = 0;
  paintHandler = 0;
  resizeHandler = 0;

  clientWidth = VIDEO_WIDTH; // native res
  clientHeight = VIDEO_HEIGHT;
  fullScreenWidth = 1920; // fullscreen res, only used if bFullScreen is set to true in ctor!
  fullScreenHeight = 1080;

  if(!bFullScreen)
  {
    x = 100;
    y = 100;
  }
  else
  {
    x = 0;
    y = 0;
  }

  windowStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME;
  windowExtendedStyle = WS_EX_APPWINDOW;

  fullScreenWindowStyle = WS_POPUP;
  fullScreenWindowExtendedStyle = WS_EX_APPWINDOW | WS_EX_TOPMOST;

  UpdateRestoreValues();
}

GLWindow::~GLWindow()
{
}

void GLWindow::UpdateRestoreValues()
{
  RECT windowRect = {0, 0, clientWidth, clientHeight};
  restoreX = x;
  restoreY = y;
  AdjustWindowRectEx(&windowRect, windowStyle, 0, windowExtendedStyle);
  restoreWidth = width = windowRect.right - windowRect.left;
  restoreHeight = height = windowRect.bottom - windowRect.top;
}

void GLWindow::OnResize(int _width, int _height)
{
  RECT windowRect;

  GetWindowRect(hWnd, &windowRect);
  x = windowRect.left;
  y = windowRect.top;
  GetClientRect(hWnd, &windowRect);
  clientWidth = windowRect.right - windowRect.left;
  clientHeight = windowRect.bottom - windowRect.top;
  if(!bFullScreen)
  {
    UpdateRestoreValues();
  }
}

void GLWindow::ToggleFullscreen()
{
  PostMessage(hWnd, WM_TOGGLEFULLSCREEN, 0, 0);
}

bool GLWindow::ChangeScreenResolution(int _width, int _height)
{
  DEVMODE dmScreenSettings;
  ZeroMemory(&dmScreenSettings, sizeof(DEVMODE));
  dmScreenSettings.dmSize	= sizeof(DEVMODE);
  dmScreenSettings.dmPelsWidth = _width;
  dmScreenSettings.dmPelsHeight = _height;
  dmScreenSettings.dmBitsPerPel = 32;
  dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
  if(ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
    return false;
  return true;
}

bool GLWindow::CreateWindowGL()
{
  uint32 wStyle = windowStyle;
  uint32 wStyleEx = windowExtendedStyle;

  PIXELFORMATDESCRIPTOR pfd;
  pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 32;
  pfd.cRedBits = pfd.cRedShift = pfd.cGreenBits = pfd.cGreenShift = pfd.cBlueBits = pfd.cBlueShift = 0;
  pfd.cAlphaBits = pfd.cAlphaShift = pfd.cAccumBits = pfd.cAccumRedBits = pfd.cAccumGreenBits = pfd.cAccumBlueBits
                 = pfd.cAccumAlphaBits = 0;
  pfd.cDepthBits = 0;
  pfd.cStencilBits = 0;
  pfd.cAuxBuffers = 0;
  pfd.iLayerType = PFD_MAIN_PLANE;
  pfd.bReserved = 0;
  pfd.dwLayerMask = 0;
  pfd.dwVisibleMask = 0;
  pfd.dwDamageMask = 0;

  RECT windowRect = {x, y, clientWidth + x, clientHeight + y};

  AdjustWindowRectEx(&windowRect, windowStyle, 0, windowExtendedStyle);
  restoreX = x;
  restoreY = y;
  restoreWidth = windowRect.right - windowRect.left;
  restoreHeight = windowRect.bottom - windowRect.top;

  if(bFullScreen)
  {
    if(!ChangeScreenResolution(fullScreenWidth, fullScreenHeight))
    {
      // Fullscreen Mode Failed.  Run In Windowed Mode Instead
      MessageBox(HWND_DESKTOP,"Mode Switch Failed.\nRunning In Windowed Mode.", "Error", MB_OK | MB_ICONEXCLAMATION);
      bFullScreen = false;
    }
    else
    {
      // Fullscreen mode switch succeeded
      wStyle = fullScreenWindowStyle;
      wStyleEx = fullScreenWindowExtendedStyle;
      clientWidth = fullScreenWidth;
      clientHeight = fullScreenHeight;
      windowRect.left = 0;
      windowRect.top = 0;
      windowRect.right = fullScreenWidth;
      windowRect.bottom = fullScreenHeight;
      AdjustWindowRectEx(&windowRect, fullScreenWindowStyle, 0, fullScreenWindowExtendedStyle);
      x = windowRect.left;
      y = windowRect.top;
      width = windowRect.right - windowRect.left;
      height = windowRect.bottom - windowRect.top;
    }
  }

  // Create The OpenGL Window
  hWnd = CreateWindowEx(
    wStyleEx,
    "NuanceGLWindow",
    "Nuance (F1 to toggle fullscreen)",
    wStyle,
    windowRect.left, windowRect.top,
    windowRect.right - windowRect.left,
    windowRect.bottom - windowRect.top,
    HWND_DESKTOP,
    0,
    hInstance,
    this);

  if(!hWnd)
  {
    LPVOID lpMsgBuf;
    if (!FormatMessage( 
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM | 
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        (LPTSTR) &lpMsgBuf,
        0,
        NULL ))
    {
      // Handle the error.
      return false;
    }

    // Process any inserts in lpMsgBuf.
    // ...

    // Display the string.
    MessageBox( NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION );

    // Free the buffer.
    LocalFree( lpMsgBuf );  

    return false;
  }

  hDC = GetDC(hWnd);

  if(!hDC)
  {
    LPVOID lpMsgBuf;
    if (!FormatMessage( 
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM | 
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        (LPTSTR) &lpMsgBuf,
        0,
        NULL ))
    {
      // Handle the error.
      DestroyWindow(hWnd);
      hWnd = 0;
      return false;
    }

    // Process any inserts in lpMsgBuf.
    // ...

    // Display the string.
    MessageBox( NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION );

    // Free the buffer.
    LocalFree( lpMsgBuf );  

    DestroyWindow(hWnd);
    hWnd = 0;
    return false;
  }

  const GLuint PixelFormat = ChoosePixelFormat(hDC, &pfd);
  if(!PixelFormat)
  {
    LPVOID lpMsgBuf;
    if (!FormatMessage( 
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM | 
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        (LPTSTR) &lpMsgBuf,
        0,
        NULL ))
    {
      // Handle the error.
		  ReleaseDC(hWnd, hDC);
		  hDC = 0;
		  DestroyWindow(hWnd);
		  hWnd = 0;
      return false;
    }

    // Process any inserts in lpMsgBuf.
    // ...

    // Display the string.
    MessageBox( NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION );

    // Free the buffer.
    LocalFree( lpMsgBuf );
    ReleaseDC(hWnd, hDC);
    hDC = 0;
    DestroyWindow(hWnd);
    hWnd = 0;
    return false;
  }

  if(!SetPixelFormat(hDC, PixelFormat, &pfd))
  {
    LPVOID lpMsgBuf;
    if (!FormatMessage( 
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM | 
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        (LPTSTR) &lpMsgBuf,
        0,
        NULL ))
    {
      // Handle the error.
      ReleaseDC(hWnd, hDC);
      hDC = 0;
      DestroyWindow(hWnd);
      hWnd = 0;
      return false;
    }

    // Process any inserts in lpMsgBuf.
    // ...

    // Display the string.
    MessageBox( NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION );

    // Free the buffer.
    LocalFree( lpMsgBuf );  

    ReleaseDC(hWnd, hDC);
    hDC = 0;
    DestroyWindow(hWnd);
    hWnd = 0;
    return false;
  }

  hRC = wglCreateContext(hDC);

  if(!hRC)
  {
    LPVOID lpMsgBuf;
    if (!FormatMessage( 
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM | 
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        (LPTSTR) &lpMsgBuf,
        0,
        NULL ))
    {
      // Handle the error.
      ReleaseDC(hWnd, hDC);
      hDC = 0;
      DestroyWindow(hWnd);
      hWnd = 0;
      return false;
    }

    // Display the string.
    MessageBox( NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION );

    // Free the buffer.
    LocalFree( lpMsgBuf );  
    ReleaseDC(hWnd, hDC);
    hDC = 0;
    DestroyWindow(hWnd);
    hWnd = 0;
    return false;
  }

  if(!wglMakeCurrent(hDC, hRC))
  {
    LPVOID lpMsgBuf;
    if (!FormatMessage( 
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM | 
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        (LPTSTR) &lpMsgBuf,
        0,
        NULL ))
    {
      // Handle the error.
      wglDeleteContext(hRC);
      hRC = 0;
      ReleaseDC(hWnd, hDC);
      hDC = 0;
      DestroyWindow(hWnd);
      hWnd = 0;
      return false;
    }

    // Process any inserts in lpMsgBuf.
    // ...

    // Display the string.
    MessageBox( NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION );

    // Free the buffer.
    LocalFree( lpMsgBuf );  

    wglDeleteContext(hRC);
    hRC = 0;
    ReleaseDC(hWnd, hDC);
    hDC = 0;
    DestroyWindow(hWnd);
    hWnd = 0;
    return false;
  }

  DescribePixelFormat(hDC,GetPixelFormat(hDC),sizeof(PIXELFORMATDESCRIPTOR),&pfd);
  //const bool bAccelerated = pfd.dwFlags & PFD_GENERIC_ACCELERATED;
  //const bool bGeneric = pfd.dwFlags & PFD_GENERIC_FORMAT;

  ShowWindow(hWnd, SW_NORMAL);
  bVisible = true;
  OnResize(clientWidth, clientHeight);

  return true;
}

void GLWindow::CleanUp()
{
  if(hWnd)
  {
    delete inputManager;
    inputManager = NULL;

    if(bUseSeparateThread) gfx_lock.lock();
		if(hDC)
		{
			wglMakeCurrent(hDC, 0);
			if(hRC)
			{
				wglDeleteContext(hRC);
				hRC = 0;
			}
			ReleaseDC(hWnd, hDC);
			hDC = 0;
		}
		DestroyWindow(hWnd);
		hWnd = 0;
    if(bUseSeparateThread) gfx_lock.unlock();
  }

  if(bFullScreen)
  {
		ChangeDisplaySettings(NULL,0);
		ShowCursor(TRUE);
  }

  UnregisterClass("NuanceGLWindow", hInstance);
}

LRESULT CALLBACK GLWindow::GLWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  GLWindow *window = (GLWindow *)(GetWindowLongPtr(hWnd, GWLP_USERDATA));
  RECT windowRect;
  PAINTSTRUCT ps;

  switch(uMsg)
  {
    case WM_ACTIVATE:
      if (window->inputManager) window->inputManager->Activate();
      break;

    case WM_SYSCOMMAND:
		{
			switch(wParam)
			{
				case SC_SCREENSAVE:
				case SC_MONITORPOWER:
				  return 0;
			}
			break;
		}
    case WM_CREATE:
		{
			const CREATESTRUCT *creation = (CREATESTRUCT *)(lParam);
			window = (GLWindow *)(creation->lpCreateParams);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)(window));
		}
		return 0;

    case WM_CLOSE:
      return 0;

    case WM_DESTROY:
      return 0;

    case WM_MOVE:
      if(!window->bFullScreen)
      {
        GetWindowRect(window->hWnd,&windowRect);
        window->x = windowRect.left;
        window->y = windowRect.top;
        window->UpdateRestoreValues();
      }
      break;

    case WM_SIZE:
      videoTexInfo.bUpdateDisplayList = true;   // as we need to update the display list (videoTexInfo.windowTexCoords) due to changed resolution
      videoTexInfo.bUpdateTextureStates = true; // as we need to update the uniform that holds the resolution

      switch(wParam)
      {
        case SIZE_MINIMIZED:
          window->bVisible = false;
          return 0;

        case SIZE_MAXIMIZED:
        case SIZE_RESTORED:
          window->bVisible = true;
          window->OnResize(LOWORD(lParam), HIWORD(lParam));
          if(window->resizeHandler)
          {
            window->resizeHandler(LOWORD(lParam),HIWORD(lParam));
          }
          return 0;
      }
      break;

    case WM_KEYDOWN:
      if (window->inputManager) window->inputManager->keyDown(window->applyControllerState, (int16)wParam);

      if((int)wParam == VK_F1 || ((int)wParam == VK_ESCAPE && window->bFullScreen))
      {
        window->ToggleFullscreen();
      }
      break;

    case WM_KEYUP:
      if (window->inputManager) window->inputManager->keyUp(window->applyControllerState, (int16)wParam);
      break;

    case WM_PAINT:
      BeginPaint(hWnd,&ps);
      if(window->paintHandler)
      {
        window->paintHandler(wParam,lParam);
      }
      else
      {
        if(bUseSeparateThread) gfx_lock.lock();
        glClear(GL_COLOR_BUFFER_BIT);
        //glFlush();
        SwapBuffers(window->hDC);
        if(bUseSeparateThread) gfx_lock.unlock();
      }
      EndPaint(hWnd,&ps);
      break;

    case WM_ERASEBKGND:
      return 0;

    case WM_TOGGLEFULLSCREEN:
      window->bFullScreen = !window->bFullScreen;
      if(window->bFullScreen)
      {
        window->restoreX = window->x;
        window->restoreY = window->y;
        window->restoreWidth = window->clientWidth;
        window->restoreHeight = window->clientHeight;

        ShowWindow(window->hWnd,SW_HIDE);
        SetWindowLongPtr(window->hWnd, GWL_STYLE, window->fullScreenWindowStyle);
        SetWindowLongPtr(window->hWnd, GWL_EXSTYLE, window->fullScreenWindowExtendedStyle);
        //if(!window->ChangeScreenResolution(window->fullScreenWidth, window->fullScreenHeight))
        //  MessageBox(HWND_DESKTOP, "Mode Switch Failed.\nRunning In Windowed Mode.", "Error", MB_OK | MB_ICONEXCLAMATION);
        SetWindowPos(window->hWnd, HWND_TOP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SWP_SHOWWINDOW); //!! aspect ratio of widescreens wrong for Nuon!
        SetCursorPos(GetSystemMetrics(SM_CXSCREEN)/2, GetSystemMetrics(SM_CYSCREEN));

        window->clientWidth = GetSystemMetrics(SM_CXSCREEN);
        window->clientHeight = GetSystemMetrics(SM_CYSCREEN);
        window->width = GetSystemMetrics(SM_CXSCREEN);
        window->height = GetSystemMetrics(SM_CYSCREEN);
        window->x = 0;
        window->y = 0;
      }
      else
      {
        //Switch back to the desktop resolution
        window->x = window->restoreX;
        window->y = window->restoreY;
        window->clientWidth = window->restoreWidth;
        window->clientHeight = window->restoreHeight;

        windowRect.left = 0;
        windowRect.top = 0;
        windowRect.right = window->restoreWidth;
        windowRect.bottom = window->restoreHeight;
        AdjustWindowRectEx(&windowRect, window->windowStyle, 0, window->windowExtendedStyle);
        window->width = windowRect.right - windowRect.left;
        window->height = windowRect.bottom - windowRect.top;

        ShowWindow(window->hWnd,SW_HIDE);
        SetWindowLongPtr(window->hWnd, GWL_STYLE, window->windowStyle);
        SetWindowLongPtr(window->hWnd, GWL_EXSTYLE, window->windowExtendedStyle);
        //window->ChangeScreenResolution(0,0);
        SetWindowPos(window->hWnd, HWND_TOP,window->x,window->y,window->width,window->height, SWP_SHOWWINDOW);
        SetCursorPos(window->x + window->width/2, window->y + window->height/2);
      }
      break;
  }

  return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

bool GLWindow::RegisterWindowClass()
{
  WNDCLASSEX windowClass = {};
  windowClass.cbSize = sizeof (WNDCLASSEX);
  windowClass.style	= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  windowClass.lpfnWndProc = (WNDPROC)(GLWindowProc);
  windowClass.hInstance	= hInstance;
  windowClass.hbrBackground	= (HBRUSH)NULL;
  windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
  windowClass.lpszClassName	= "NuanceGLWindow";

  if(!RegisterClassEx(&windowClass))
  {
    // NOTE: Failure, Should Never Happen
    MessageBox(HWND_DESKTOP, "RegisterClassEx Failed!", "Error", MB_OK | MB_ICONEXCLAMATION);
    return false;
  }

  return true;
}

bool GLWindow::Create()
{
  if(!bUseSeparateThread)
  {
    GLWindowMain(this);
    return true;
  }
  
  threadHandle = _beginthreadex(NULL,0,GLWindow::GLWindowMain,this,0,&threadID);
 
  if(!threadHandle)
  {
    LPVOID lpMsgBuf;
    if (!FormatMessage( 
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM | 
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        (LPTSTR) &lpMsgBuf,
        0,
        NULL ))
    {
      // Handle the error.
      return false;
    }

    // Process any inserts in lpMsgBuf.
    // ...

    // Display the string.
    MessageBox( NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION );

    // Free the buffer.
    LocalFree( lpMsgBuf );  
  }

  return threadHandle;
}

void GLWindow::MessagePump()
{
  if (inputManager) inputManager->UpdateState(applyControllerState, NULL, NULL);

  if(!bUseSeparateThread)
  {
    MSG msg;
    while(PeekMessage(&msg, hWnd, 0, 0, PM_REMOVE))
      DispatchMessage(&msg);
  }
}

unsigned WINAPI GLWindow::GLWindowMain(void *param)
{
  GLWindow * const glWindow = (GLWindow *)param;

  if(!glWindow->RegisterWindowClass())
    return 0;

  if(glWindow->CreateWindowGL())
  {
    glWindow->OnResize(glWindow->clientWidth,glWindow->clientHeight);
    
    if(bUseSeparateThread) gfx_lock.lock();
    const GLenum err = glewInit();
    if (err != GLEW_OK)
      MessageBox(NULL, (char*)glewGetErrorString(err), "Error", MB_ICONWARNING);
    if(bUseSeparateThread) gfx_lock.unlock();

    // Failure not currently fatal.
    glWindow->inputManager = InputManager::Create();
    if (!glWindow->inputManager->Init(glWindow->hWnd))
    {
        delete glWindow->inputManager;
        glWindow->inputManager = NULL;
    }

    if(bUseSeparateThread)
    {
      // Success Creating Window.  Check For Window Messages
      MSG msg;
      BOOL bRet;
      while((bRet = GetMessage(&msg, glWindow->hWnd, 0, 0)) != 0)
      {
        if(bRet == -1 || msg.message == WM_QUIT)
          break;

        DispatchMessage(&msg);
      }

      glWindow->CleanUp();
    }
  }
  else
    return 0;

  return 1;
}

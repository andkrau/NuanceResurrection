//---------------------------------------------------------------------------

#include <windows.h>
#include "external\glew-2.1.0\include\GL\glew.h"
#include "GLWindow.h"
#include <GL/gl.h>
//---------------------------------------------------------------------------
/****************************************************************************
OpenGL Window Code
****************************************************************************/
bool bTerminate;
bool bFullScreen;

char defaultTitle[] = "GLWindow";

GLWindow::GLWindow()
{
  bVisible = false;
  bUseSeparateThread = false;
  bitsPerPixel = 32;
  keyDownHandler = 0;
  keyUpHandler = 0;
  timerHandler = 0;
  idleHandler = 0;
  createHandler = 0;
  closeHandler = 0;
  closeQueryHandler = 0;
  destroyHandler = 0;
  paintHandler = 0;
  resizeHandler = 0;
  timerInterval = 0;
  timerID = 0;
  x = 160;
  y = 120;
  clientWidth = 320;
  clientHeight = 240;
	windowStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_THICKFRAME;
	windowExtendedStyle = WS_EX_APPWINDOW;
  fullScreenWindowStyle = WS_POPUP;
  fullScreenWindowStyle = WS_EX_APPWINDOW | WS_EX_TOPMOST;
  title = defaultTitle;

  UpdateRestoreValues();
}

GLWindow::~GLWindow()
{
}

void CALLBACK GLWindow::MMTimerCallback(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
  GLWindow *window = (GLWindow *)dwUser;
  SendMessage(window->hWnd,WM_TIMER,uID,(LPARAM)NULL);
}

bool GLWindow::SetTimer()
{
  timeBeginPeriod(1);
  timerID = timeSetEvent(timerInterval,0,this->MMTimerCallback,(UINT)this,TIME_PERIODIC);
  return (timerID != NULL);
}

bool GLWindow::KillTimer()
{
  if(timerID)
  {
    timeEndPeriod(1);
    timeKillEvent(timerID);
  }

  return true;
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

void GLWindow::OnResize(int width, int height)
{
  RECT windowRect;

  GetWindowRect(hWnd, &windowRect);
  x = windowRect.left;
  y = windowRect.top;
  width = windowRect.right - windowRect.left;
  height = windowRect.bottom - windowRect.top;
  GetClientRect(hWnd, &windowRect);
  clientWidth = windowRect.right - windowRect.left;
  clientHeight = windowRect.bottom - windowRect.top;
  if(!bFullScreen)
  {
    UpdateRestoreValues();
  }
}

void GLWindow::Close()
{
  if(bCreated)
  {
	  //PostMessage(hWnd, WM_QUIT, 0, 0);
	  //bTerminate = true;
  }
}

void GLWindow::ToggleFullscreen()
{
	PostMessage(hWnd, WM_TOGGLEFULLSCREEN, 0, 0);
}

bool GLWindow::ChangeScreenResolution(int width, int height, int bitsPerPixel)
{
	DEVMODE dmScreenSettings;
	ZeroMemory(&dmScreenSettings, sizeof(DEVMODE));
	dmScreenSettings.dmSize	= sizeof(DEVMODE);
	dmScreenSettings.dmPelsWidth = width;
	dmScreenSettings.dmPelsHeight	= height;
	dmScreenSettings.dmBitsPerPel	= bitsPerPixel;
	dmScreenSettings.dmFields	= DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
	if(ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
	{
		return false;
	}
	return true;
}

bool GLWindow::CreateWindowGL()
{
  GLuint PixelFormat;
  unsigned __int32 wStyle, wStyleEx;

  wStyle = windowStyle;
  wStyleEx = windowExtendedStyle;

  if(!bitsPerPixel)
  {
    bitsPerPixel = 32;
  }
  
  PIXELFORMATDESCRIPTOR pfd;
  pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
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

  if(bFullScreen == true)
  {
    if(!ChangeScreenResolution(fullScreenWidth, fullScreenHeight, bitsPerPixel))
    {
      // Fullscreen Mode Failed.  Run In Windowed Mode Instead
      MessageBox(HWND_DESKTOP,"Mode Switch Failed.\nRunning In Windowed Mode.", "Error", MB_OK | MB_ICONEXCLAMATION);
      bFullScreen = false;
    }
    else
    {
      // Fullscreen mode switch succeeded
      ShowCursor(FALSE);
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
    "GLWindow",
    title,
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

  PixelFormat = ChoosePixelFormat(hDC, &pfd);
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
  bool bAccelerated = pfd.dwFlags & PFD_GENERIC_ACCELERATED;
  bool bGeneric = pfd.dwFlags & PFD_GENERIC_FORMAT;

  ShowWindow(hWnd, SW_NORMAL);
  bVisible = true;
  OnResize(clientWidth, clientHeight);

  return true;
}

void GLWindow::CleanUp()
{
	if(hWnd)
	{
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
	}

	if(bFullScreen)
	{
		ChangeDisplaySettings(NULL,0);
		ShowCursor(TRUE);
	}

	UnregisterClass(className, hInstance);
}

LRESULT CALLBACK GLWindow::GLWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  GLWindow *window = (GLWindow *)(GetWindowLong(hWnd, GWL_USERDATA));
  RECT windowRect;
  PAINTSTRUCT ps;

  switch(uMsg)
  {
    case WM_TIMER:
      if(window->timerHandler)
      {
        window->timerHandler((unsigned __int32)wParam);
      }
      return 0;
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
			CREATESTRUCT *creation = (CREATESTRUCT *)(lParam);
			window = (GLWindow *)(creation->lpCreateParams);
			SetWindowLong(hWnd, GWL_USERDATA, (LONG)(window));

      if(window->createHandler)
      {
        window->createHandler(wParam, lParam);
      }

		}
		return 0;

		case WM_CLOSE:
      if(window->closeQueryHandler)
      {
        if(!window->closeQueryHandler(0,0))
        {
          //CloseQuery callback said not to close the window
          return -1;
        }
        else
        {
          if(window->closeHandler)
          {
            //Close callback exists, so call it
            window->closeHandler(0,0);
          }
        }
      }

      //Call the default close handler
			window->Close();
		  return 0;

    case WM_DESTROY:
      if(window->destroyHandler)
      {
        window->destroyHandler(0,0);
      }
      return 0;

    case WM_MOVE:
      GetWindowRect(window->hWnd,&windowRect);
      if(!window->bFullScreen)
      {
        window->x = windowRect.left;
        window->y = windowRect.top;
        window->UpdateRestoreValues();
      }
      break;

		case WM_SIZE:
			switch(wParam)
			{
				case SIZE_MINIMIZED:
					window->bVisible = false;
				  return 0;

				case SIZE_MAXIMIZED:
					window->bVisible = true;
	        window->OnResize(LOWORD(lParam), HIWORD(lParam));
          if(window->resizeHandler)
          {
            window->resizeHandler(LOWORD(lParam),HIWORD(lParam));
          }
				  return 0;

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
      if(window->keyDownHandler)
      {
        window->keyDownHandler((int)wParam,(unsigned __int32)lParam);
      }

	    if((int)wParam == VK_F1)
	    {
		    window->ToggleFullscreen();
	    }

		  break;

		case WM_KEYUP:
      if(window->keyUpHandler)
      {
        window->keyUpHandler((int)wParam,(unsigned __int32)lParam);
      }
		  break;

    case WM_PAINT:
      BeginPaint(hWnd,&ps);
      if(window->paintHandler)
      {
        window->paintHandler((int)wParam,(unsigned __int32)lParam);
      }
      else
      {
        glClear(GL_COLOR_BUFFER_BIT);
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
        window->restoreWidth = window->width;
        window->restoreHeight = window->height;

        windowRect.left = 0;
        windowRect.top = 0;
        windowRect.right = window->fullScreenWidth;
        windowRect.bottom = window->fullScreenHeight;
     		AdjustWindowRectEx(&windowRect, window->windowStyle, 0, window->windowExtendedStyle);

        window->clientWidth = window->fullScreenWidth;
        window->clientHeight = window->fullScreenHeight;
        window->width = windowRect.right - windowRect.left;
        window->height = windowRect.bottom - windowRect.top;
        window->x = windowRect.left;
        window->y = windowRect.top;

        ShowWindow(window->hWnd,SW_HIDE);
        SetWindowPos(window->hWnd, HWND_TOP, window->x, window->y, window->width, window->height, 0);
        ShowWindow(window->hWnd,SW_NORMAL);
        window->ChangeScreenResolution(window->clientWidth,window->clientHeight,window->bitsPerPixel);
      }
      else
      {
        //Switch back to the desktop resolution
        window->x = window->restoreX;
        window->y = window->restoreY;
        window->width = window->restoreWidth;
        window->height = window->restoreHeight;

        ShowWindow(window->hWnd,SW_HIDE);
        window->ChangeScreenResolution(0,0,0);
        SetWindowPos(window->hWnd, HWND_TOP,window->x,window->y,window->width,window->height,0);
        ShowWindow(window->hWnd,SW_NORMAL);

        GetClientRect(window->hWnd,&windowRect);
        window->clientWidth = windowRect.right;
        window->clientHeight = windowRect.bottom;
      }
		  break;
  }

  return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

bool GLWindow::RegisterWindowClass()
{
	WNDCLASSEX windowClass;
	memset(&windowClass, 0, sizeof (WNDCLASSEX));
	windowClass.cbSize = sizeof (WNDCLASSEX);
	windowClass.style	= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	windowClass.lpfnWndProc	= (WNDPROC)(GLWindowProc);
	windowClass.hInstance	= hInstance;
	windowClass.hbrBackground	= (HBRUSH)NULL;
	windowClass.hCursor	= LoadCursor(NULL, IDC_ARROW);
	windowClass.lpszClassName	= "GLWindow";

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
  
  threadHandle = CreateThread(NULL,0,GLWindow::GLWindowMain,this,0,&threadID);
 
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

char className[] = "GLWindow";

bool GLWindow::MessagePump(void)
{
  MSG	msg;

  if(PeekMessage(&msg, hWnd, 0, 0, PM_REMOVE))
  {
    DispatchMessage(&msg);
  }

  return true;
}

DWORD WINAPI GLWindow::GLWindowMain(void *param)
{
  GLWindow *glWindow = (GLWindow *)param;
  bool bMessagePumpActive;
  MSG msg;

  // Fill Out Application Data
  glWindow->className = ::className;

  // Fill Out Window

  // Register the window class
  if(!glWindow->RegisterWindowClass())
  {
		MessageBox(HWND_DESKTOP, "Error Registering Window Class!", "Error", MB_OK | MB_ICONEXCLAMATION);
		return false;
  }

  bTerminate = false;

  // Create A Window
  if(glWindow->CreateWindowGL())
  {
    glWindow->OnResize(glWindow->clientWidth,glWindow->clientHeight);
    
    bMessagePumpActive = true;
    glWindow->bCreated = true;
    if(glWindow->bUseSeparateThread)
    {
		  while(bMessagePumpActive)
		  {
			  // Success Creating Window.  Check For Window Messages
			  if(PeekMessage(&msg, glWindow->hWnd, 0, 0, PM_REMOVE))
			  {
				  // Check For WM_QUIT Message
				  if(msg.message != WM_QUIT)
				  {
					  DispatchMessage(&msg);
				  }
				  else
				  {
					  bMessagePumpActive = false;
				  }
        }
        else
			  {
  			  // Process Application Loop
				  if(glWindow->idleHandler)
          {
            glWindow->idleHandler(0,0);
          }

          WaitMessage();
			  }
      }

      glWindow->CleanUp();
      glWindow->bCreated = false;
    }
  }
  else
  {
    MessageBox(HWND_DESKTOP, "Error Creating OpenGL Window", "Error", MB_OK | MB_ICONEXCLAMATION);
    return false;
  }

  return true;
}

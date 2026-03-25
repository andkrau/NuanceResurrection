#ifndef LINUX_COMPAT_H
#define LINUX_COMPAT_H

#ifdef _WIN32
// On Windows, include the real headers
#include <windows.h>
#else
// Linux compatibility layer for Windows API types and functions

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <climits>
#include <cerrno>
#include <strings.h>
#include <atomic>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include <dlfcn.h>

// Basic Windows types
typedef int BOOL;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef long LONG;
typedef unsigned long ULONG;
typedef void* LPVOID;
typedef const char* LPCTSTR;
typedef const char* LPTSTR;
typedef char TCHAR;
typedef void* HANDLE;
typedef void* HWND;
typedef void* HDC;
typedef void* HGLRC;
typedef void* HINSTANCE;
typedef void* HMODULE;
typedef void* HICON;
typedef void* HBITMAP;
typedef void* HBRUSH;
typedef intptr_t WPARAM;
typedef intptr_t LPARAM;
typedef intptr_t LRESULT;
typedef intptr_t INT_PTR;
typedef uintptr_t UINT_PTR;
typedef char* LPSTR;
typedef unsigned int UINT;

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef MAX_PATH
#define MAX_PATH 260
#endif
#ifndef CALLBACK
#define CALLBACK
#endif
#ifndef WINAPI
#define WINAPI
#endif
#ifndef APIENTRY
#define APIENTRY
#endif
#ifndef IN
#define IN
#endif
#ifndef OUT
#define OUT
#endif

typedef unsigned long* PULONG;
typedef unsigned char BOOLEAN;

// GUID struct for joystick identification
struct GUID {
    uint32_t Data1;
    uint16_t Data2;
    uint16_t Data3;
    uint8_t Data4[8];
    bool operator==(const GUID& other) const {
        return memcmp(this, &other, sizeof(GUID)) == 0;
    }
};

// RECT struct
struct RECT {
    long left, top, right, bottom;
};

// Windows message constants (stubs)
#define WM_USER 0x0400
#define WM_CLOSE 0x0010
#define WM_DESTROY 0x0002
#define WM_COMMAND 0x0111
#define WM_SETTEXT 0x000C
#define WM_KEYDOWN 0x0100
#define WM_KEYUP 0x0101
#define WM_PAINT 0x000F
#define WM_SIZE 0x0005
#define WM_ACTIVATE 0x0006
#define WM_CREATE 0x0001
#define WM_MOVE 0x0003
#define WM_SYSCOMMAND 0x0112
#define WM_ERASEBKGND 0x0014
#define WM_TIMER 0x0113
#define WM_CTLCOLORDLG 0x0136
#define WM_CTLCOLORSTATIC 0x0138
#define WM_GETDLGCODE 0x0087
#define WM_QUIT 0x0012
#define WM_INITDIALOG 0x0110
#define WM_SETICON 0x0080
#define EM_SETSEL 0x00B1
#define EM_REPLACESEL 0x00C2
#define BN_CLICKED 0
#define STN_CLICKED 0
#define STN_DBLCLK 1
#define EN_SETFOCUS 0x0100
#define EN_KILLFOCUS 0x0200
#define CBN_SELENDOK 9
#define CB_ADDSTRING 0x0143
#define CB_SETCURSEL 0x014E
#define CB_GETCURSEL 0x0147
#define CB_ERR (-1)
#define STM_SETIMAGE 0x0172
#define IMAGE_BITMAP 0
#define ICON_SMALL 0
#define PM_REMOVE 0x0001

// Virtual key codes
#define VK_UP    0x26
#define VK_DOWN  0x28
#define VK_LEFT  0x25
#define VK_RIGHT 0x27
#define VK_F1    0x70
#define VK_ESCAPE 0x1B
#define VK_RETURN 0x0D
#define VK_SPACE 0x20

// Window styles (stubs)
#define WS_OVERLAPPED  0x00000000L
#define WS_CAPTION     0x00C00000L
#define WS_SYSMENU     0x00080000L
#define WS_MINIMIZEBOX 0x00020000L
#define WS_MAXIMIZEBOX 0x00010000L
#define WS_THICKFRAME  0x00040000L
#define WS_POPUP       0x80000000L
#define WS_EX_APPWINDOW  0x00040000L
#define WS_EX_TOPMOST    0x00000008L
#define GWL_STYLE (-16)
#define GWL_EXSTYLE (-20)
#define GWL_HINSTANCE (-6)
#define GWL_WNDPROC (-4)
#define GWLP_USERDATA (-21)
#define SWP_NOSIZE 0x0001
#define SWP_NOMOVE 0x0002
#define SWP_SHOWWINDOW 0x0040
#define SW_NORMAL 1
#define SW_HIDE 0
#define SIZE_MINIMIZED 1
#define SIZE_MAXIMIZED 2
#define SIZE_RESTORED 3
#define SC_SCREENSAVE 0xF140
#define SC_MONITORPOWER 0xF170
#define MB_OK 0
#define MB_ICONWARNING 0x30
#define MB_ICONERROR 0x10
#define MB_ICONEXCLAMATION 0x30
#define MB_ICONINFORMATION 0x40
#define MB_ICONQUESTION 0x20
#define MB_YESNO 0x04
#define IDYES 6
#define IDOK 1
#define ERROR "Error"
#define HWND_DESKTOP ((HWND)0)
#define HWND_TOPMOST ((HWND)-1)
#define HWND_TOP ((HWND)0)

// Interlocked operations → std::atomic
inline long _InterlockedExchange(volatile long* target, long value) {
    return __atomic_exchange_n(target, value, __ATOMIC_SEQ_CST);
}
inline long _InterlockedOr(volatile long* target, long value) {
    return __atomic_fetch_or(target, value, __ATOMIC_SEQ_CST);
}

// MSVC calling conventions
#define __fastcall __attribute__((fastcall))
#define __cdecl
#define __stdcall

// MSVC-specific functions
#define sprintf_s snprintf
#define _stricmp strcasecmp
#define _countof(arr) (sizeof(arr)/sizeof((arr)[0]))
#define __forceinline inline __attribute__((always_inline))
#define ZeroMemory(p,n) memset((p),0,(n))

// fopen_s replacement
inline int fopen_s(FILE** pFile, const char* filename, const char* mode) {
    *pFile = fopen(filename, mode);
    return (*pFile == nullptr) ? errno : 0;
}
typedef int errno_t;

// fscanf_s → fscanf
#define fscanf_s fscanf

// localtime_s replacement
inline int localtime_s(struct tm* result, const time_t* timer) {
    struct tm* r = localtime_r(timer, result);
    return r ? 0 : errno;
}

// _get_timezone replacement
inline int _get_timezone(long* offset) {
    *offset = timezone;
    return 0;
}

// memcpy_s replacement
inline int memcpy_s(void* dest, size_t destSize, const void* src, size_t count) {
    if (count > destSize) return -1;
    memcpy(dest, src, count);
    return 0;
}

// MessageBox stub - just print to stderr
inline int MessageBox(HWND, const char* text, const char* caption, unsigned int) {
    fprintf(stderr, "[%s] %s\n", caption ? caption : "Message", text ? text : "");
    return IDOK;
}

// Sleep
inline void Sleep(unsigned int ms) {
    usleep(ms * 1000);
}

// VirtualAlloc / VirtualFree replacements
#define MEM_RESERVE 0x2000
#define MEM_COMMIT 0x1000
#define MEM_RELEASE 0x8000
#define PAGE_EXECUTE_READWRITE 0x40
#define PAGE_READWRITE 0x04

inline void* VirtualAlloc(void* addr, size_t size, unsigned int, unsigned int prot) {
    int mprot = PROT_READ | PROT_WRITE;
    if (prot == PAGE_EXECUTE_READWRITE)
        mprot |= PROT_EXEC;
    int flags = MAP_PRIVATE | MAP_ANONYMOUS;
#ifdef MAP_32BIT
    flags |= MAP_32BIT; // allocate in lower 2GB for 32-bit pointer compatibility
#endif
    void* p = mmap(addr, size, mprot, flags, -1, 0);
    return (p == MAP_FAILED) ? nullptr : p;
}

inline int VirtualFree(void* addr, size_t size, unsigned int) {
    // For MEM_RELEASE, size must be 0 on Windows; we stored the size elsewhere
    // This is a simplification - the caller needs to track size
    // We'll use a large default if size==0
    if (size == 0) size = 64 * 1024 * 1024; // generous upper bound
    return munmap(addr, size) == 0;
}

// FlushInstructionCache
inline int FlushInstructionCache(void*, void* addr, size_t size) {
    __builtin___clear_cache((char*)addr, (char*)addr + size);
    return 1;
}
inline void* GetCurrentProcess() { return nullptr; }

// QueryPerformanceCounter / Frequency
struct _LARGE_INTEGER {
    union {
        struct { uint32_t LowPart; int32_t HighPart; };
        int64_t QuadPart;
    };
};

inline int QueryPerformanceFrequency(_LARGE_INTEGER* freq) {
    freq->QuadPart = 1000000000LL; // nanoseconds
    return 1;
}

inline int QueryPerformanceCounter(_LARGE_INTEGER* count) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    count->QuadPart = (int64_t)ts.tv_sec * 1000000000LL + ts.tv_nsec;
    return 1;
}

// EnableWindow / ShowWindow stubs
inline int EnableWindow(HWND, int) { return 0; }
inline int ShowWindow(HWND, int) { return 0; }
inline int SetWindowPos(HWND, HWND, int, int, int, int, unsigned int) { return 0; }
inline int SetWindowText(HWND, const char*) { return 0; }
inline int InvalidateRect(HWND, void*, int) { return 0; }
inline int GetWindowTextA(HWND, char*, int) { return 0; }
inline int SetFocus(HWND) { return 0; }
inline HWND GetNextDlgTabItem(HWND, HWND, int) { return nullptr; }
inline HWND GetParent(HWND) { return nullptr; }
inline HWND GetDlgItem(HWND, int) { return nullptr; }
inline int SendMessage(HWND, unsigned int, WPARAM, LPARAM) { return 0; }
inline int PostMessage(HWND, unsigned int, WPARAM, LPARAM) { return 0; }
inline int EndDialog(HWND, int) { return 0; }

// Objbase.h stubs
inline int CoInitializeEx(void*, unsigned int) { return 0; }
inline void CoUninitialize() {}
#define COINIT_MULTITHREADED 0

// MSVC pragma stubs
#define __pragma(x)

// GetModuleFileName stub
inline unsigned int GetModuleFileName(void*, char* buf, unsigned int size) {
    // Try /proc/self/exe
    ssize_t len = readlink("/proc/self/exe", buf, size - 1);
    if (len > 0) { buf[len] = '\0'; return (unsigned int)len; }
    buf[0] = '\0';
    return 0;
}

// __popcnt replacement
inline unsigned int __popcnt(unsigned int x) {
    return __builtin_popcount(x);
}

// _addcarry_u32 / _subborrow_u32 - use GCC builtins via immintrin.h
#include <immintrin.h>

// _lrotr / _lrotl replacements (32-bit rotate)
#ifndef _lrotr
inline unsigned long _lrotr(unsigned long val, int shift) {
    shift &= 31;
    return (val >> shift) | (val << (32 - shift));
}
#endif
#ifndef _lrotl
inline unsigned long _lrotl(unsigned long val, int shift) {
    shift &= 31;
    return (val << shift) | (val >> (32 - shift));
}
#endif

// Windows HRESULT
#define FAILED(hr) ((hr) < 0)
#define SUCCEEDED(hr) ((hr) >= 0)
typedef long HRESULT;
#define S_OK 0

#define LOWORD(l) ((WORD)((DWORD)(l) & 0xffff))
#define HIWORD(l) ((WORD)(((DWORD)(l) >> 16) & 0xffff))
#define MAKEINTRESOURCE(i) ((LPCTSTR)((UINT_PTR)((WORD)(i))))
#define MAKELANGID(p, s) ((((WORD)(s)) << 10) | (WORD)(p))

// stubs for dialog/resource functions
inline HWND CreateDialog(HINSTANCE, LPCTSTR, HWND, void*) { return nullptr; }
inline int DialogBox(HINSTANCE, LPCTSTR, HWND, void*) { return 0; }
inline HICON LoadIcon(HINSTANCE, LPCTSTR) { return nullptr; }
inline HBITMAP LoadBitmap(HINSTANCE, LPCTSTR) { return nullptr; }
inline int SetTimer(HWND, unsigned int, unsigned int, void*) { return 0; }
inline int KillTimer(HWND, unsigned int) { return 0; }
inline HMODULE LoadLibrary(const char*) { return nullptr; }
inline void FreeLibrary(HMODULE) {}
inline void* GetProcAddress(HMODULE, const char*) { return nullptr; }

// OPENFILENAME stub
struct OPENFILENAME {
    unsigned int lStructSize;
    HWND hwndOwner;
    HINSTANCE hInstance;
    unsigned int Flags;
    char* lpstrFile;
    unsigned int nMaxFile;
};
#define OFN_LONGNAMES 0
#define OFN_FILEMUSTEXIST 0
#define OFN_PATHMUSTEXIST 0
#define OFN_NOCHANGEDIR 0

inline int GetOpenFileName(OPENFILENAME*) { return 0; }

// Window procedure types
typedef LRESULT (CALLBACK *WNDPROC)(HWND, UINT, WPARAM, LPARAM);
inline long GetWindowLong(HWND, int) { return 0; }
inline long SetWindowLongPtr(HWND, int, long) { return 0; }
inline long GetWindowLongPtr(HWND, int) { return 0; }
inline LRESULT CallWindowProc(WNDPROC, HWND, UINT, WPARAM, LPARAM) { return 0; }
inline LRESULT DefWindowProc(HWND, UINT, WPARAM, LPARAM) { return 0; }
#define DLGC_WANTMESSAGE 0x0004

// Multimedia timer stubs
struct TIMECAPS { unsigned int wPeriodMin; unsigned int wPeriodMax; };
typedef unsigned int MMRESULT;
#define TIMERR_NOERROR 0
#define TIMERR_NOCANDO 1
inline MMRESULT timeGetDevCaps(TIMECAPS*, unsigned int) { return TIMERR_NOCANDO; }
inline MMRESULT timeBeginPeriod(unsigned int) { return TIMERR_NOCANDO; }
inline MMRESULT timeEndPeriod(unsigned int) { return TIMERR_NOCANDO; }

// Window class registration stubs
struct WNDCLASSEX { unsigned int cbSize; unsigned int style; void* lpfnWndProc; HINSTANCE hInstance; void* hbrBackground; void* hCursor; const char* lpszClassName; };
#define CS_HREDRAW 0x0002
#define CS_VREDRAW 0x0001
#define CS_OWNDC 0x0020
inline int RegisterClassEx(WNDCLASSEX*) { return 1; }
inline int UnregisterClass(const char*, HINSTANCE) { return 1; }
inline void* LoadCursor(HINSTANCE, const char*) { return nullptr; }
#define IDC_ARROW ((const char*)32512)

// CreateWindow stubs
inline HWND CreateWindowEx(DWORD, const char*, const char*, DWORD, int, int, int, int, HWND, void*, HINSTANCE, void*) { return nullptr; }
inline int AdjustWindowRectEx(RECT* r, DWORD, int, DWORD) {
    // No adjustment needed on Linux
    return 1;
}
inline HDC GetDC(HWND) { return nullptr; }
inline int ReleaseDC(HWND, HDC) { return 0; }
inline int DestroyWindow(HWND) { return 0; }
inline int GetWindowRect(HWND, RECT*) { return 0; }
inline int GetClientRect(HWND, RECT*) { return 0; }
inline int ShowCursor(int) { return 0; }
inline int SetCursorPos(int, int) { return 0; }
inline int GetSystemMetrics(int) { return 0; }
#define SM_CXSCREEN 0
#define SM_CYSCREEN 1
inline void* GetStockObject(int) { return nullptr; }
#define BLACK_BRUSH 4

// WGL stubs (not used on Linux - we use SDL2)
inline HGLRC wglCreateContext(HDC) { return nullptr; }
inline int wglMakeCurrent(HDC, HGLRC) { return 0; }
inline int wglDeleteContext(HGLRC) { return 0; }
inline int wglShareLists(HGLRC, HGLRC) { return 0; }
inline int SwapBuffers(HDC) { return 0; }

// Pixel format stubs
struct PIXELFORMATDESCRIPTOR {
    WORD nSize, nVersion; DWORD dwFlags; BYTE iPixelType;
    BYTE cColorBits, cRedBits, cRedShift, cGreenBits, cGreenShift, cBlueBits, cBlueShift;
    BYTE cAlphaBits, cAlphaShift, cAccumBits, cAccumRedBits, cAccumGreenBits, cAccumBlueBits, cAccumAlphaBits;
    BYTE cDepthBits, cStencilBits, cAuxBuffers, iLayerType, bReserved;
    DWORD dwLayerMask, dwVisibleMask, dwDamageMask;
};
#define PFD_DRAW_TO_WINDOW 0x00000004
#define PFD_SUPPORT_OPENGL 0x00000020
#define PFD_DOUBLEBUFFER   0x00000001
#define PFD_TYPE_RGBA 0
#define PFD_MAIN_PLANE 0
#define PFD_GENERIC_ACCELERATED 0x00001000
#define PFD_GENERIC_FORMAT 0x00000040
inline unsigned int ChoosePixelFormat(HDC, const PIXELFORMATDESCRIPTOR*) { return 1; }
inline int SetPixelFormat(HDC, unsigned int, const PIXELFORMATDESCRIPTOR*) { return 1; }
inline int DescribePixelFormat(HDC, int, unsigned int, PIXELFORMATDESCRIPTOR*) { return 0; }
inline int GetPixelFormat(HDC) { return 0; }

// Display settings stubs
struct DEVMODE { DWORD dmSize, dmPelsWidth, dmPelsHeight, dmBitsPerPel, dmFields; };
#define DM_BITSPERPEL 0x00040000
#define DM_PELSWIDTH 0x00080000
#define DM_PELSHEIGHT 0x00100000
#define CDS_FULLSCREEN 4
#define DISP_CHANGE_SUCCESSFUL 0
inline int ChangeDisplaySettings(DEVMODE*, DWORD) { return 0; }

// MSG / PeekMessage stubs
struct MSG { HWND hwnd; UINT message; WPARAM wParam; LPARAM lParam; };
inline int PeekMessage(MSG*, HWND, UINT, UINT, UINT) { return 0; }
inline int DispatchMessage(const MSG*) { return 0; }
inline int GetMessage(MSG*, HWND, UINT, UINT) { return 0; }
inline int IsDialogMessage(HWND, MSG*) { return 0; }
inline int TranslateMessage(const MSG*) { return 0; }

// BeginPaint / EndPaint stubs
struct PAINTSTRUCT { HDC hdc; int fErase; RECT rcPaint; };
inline HDC BeginPaint(HWND, PAINTSTRUCT*) { return nullptr; }
inline int EndPaint(HWND, PAINTSTRUCT*) { return 0; }

// FormatMessage stubs
#define FORMAT_MESSAGE_ALLOCATE_BUFFER 0x00000100
#define FORMAT_MESSAGE_FROM_SYSTEM 0x00001000
#define FORMAT_MESSAGE_IGNORE_INSERTS 0x00000200
#define LANG_NEUTRAL 0x00
#define SUBLANG_DEFAULT 0x01
inline DWORD FormatMessage(DWORD, LPVOID, DWORD, DWORD, LPTSTR, DWORD, void*) { return 0; }
inline void LocalFree(void*) {}
inline DWORD GetLastError() { return 0; }

// _beginthreadex stub
inline uintptr_t _beginthreadex(void*, unsigned, unsigned(*)(void*), void*, unsigned, unsigned*) { return 0; }

// CREATESTRUCT stub
struct CREATESTRUCT { void* lpCreateParams; };

// Color macros
inline DWORD RGB(BYTE r, BYTE g, BYTE b) { return ((DWORD)r) | (((DWORD)g)<<8) | (((DWORD)b)<<16); }
inline HBRUSH CreateSolidBrush(DWORD) { return nullptr; }
inline DWORD SetBkColor(HDC, DWORD) { return 0; }

// io.h replacement
#include <unistd.h>
#include <fcntl.h>
#define _O_RDONLY O_RDONLY
#define _O_WRONLY O_WRONLY
#define _O_APPEND O_APPEND
#define _O_BINARY 0
#define O_BINARY 0
#define O_TEXT 0
#define _FCREAT O_CREAT
#define _FTRUNC O_TRUNC
#define _FEXCL O_EXCL
#define _FBINARY 0
#define _open open
#define _close close
#define _read read
#define _write write
#define _lseek lseek
#define _tell(fd) lseek(fd, 0, SEEK_CUR)

// _sopen_s stub
#define _SH_DENYWR 0
#define _SH_DENYNO 0
#define _S_IREAD S_IRUSR
#define _S_IWRITE S_IWUSR
#define _S_IFIFO S_IFIFO
#define _S_IFREG S_IFREG
#define _S_IFEXEC 0
#define _S_IEXEC S_IXUSR

// strcpy_s / strcat_s
inline int strcpy_s(char* d, size_t n, const char* s) { strncpy(d,s,n); d[n-1]='\0'; return 0; }
inline int strcat_s(char* d, size_t n, const char* s) { strncat(d,s,n-strlen(d)-1); return 0; }
// 2-arg template overloads for MSVC array form: strcpy_s(arr, src)
template<size_t N> inline int strcpy_s(char (&d)[N], const char* s) { return strcpy_s(d, N, s); }
template<size_t N> inline int strcat_s(char (&d)[N], const char* s) { return strcat_s(d, N, s); }

// Other MSVC string functions
#define _strnicmp strncasecmp
#define memmove_s(d,dn,s,n) memmove(d,s,n)
#define strtok_s strtok_r
#define fprintf_s fprintf
#define wcstombs_s(ret,d,dn,s,n) do { size_t r = wcstombs(d,s,n); if(ret) *(ret) = r; } while(0)
#define mbstowcs_s(ret,d,dn,s,n) do { size_t r = mbstowcs(d,s,n); if(ret) *(ret) = r; } while(0)
inline int _sopen_s(int* fd, const char* path, int oflag, int, int pmode) {
    *fd = open(path, oflag, pmode);
    return (*fd == -1) ? errno : 0;
}

// _S_* file type macros
#define _S_IFCHR S_IFCHR
#define _S_IFDIR S_IFDIR

// _tcscpy_s / TCHAR macros
#define _T(x) x
#define _tcscpy_s(d,n,s) strncpy(d,s,n)
#define _tcscat_s(d,n,s) strncat(d,s,(n)-strlen(d)-1)
#define _sntprintf_s(b,n,...) snprintf(b,n,__VA_ARGS__)

// LONG_PTR
typedef long LONG_PTR;

// mmsystem.h typedef for timer.h
// Already provided above

#endif // _WIN32

#endif // LINUX_COMPAT_H

// dinput.h stub for Linux
// DirectInput types are not used on Linux (SDL2 replaces them)
#ifndef _COMPAT_DINPUT_H
#define _COMPAT_DINPUT_H

#define DIRECTINPUT_VERSION 0x0800

struct IDirectInput8;
struct IDirectInputDevice8;
typedef IDirectInput8* LPDIRECTINPUT8;
typedef IDirectInputDevice8* LPDIRECTINPUTDEVICE8;

#define IID_IDirectInput8 GUID{}

struct DIDEVICEINSTANCE { GUID guidInstance; char tszProductName[260]; char tszInstanceName[260]; };
struct DIDEVICEOBJECTINSTANCE { unsigned long dwType; char tszName[260]; };
struct DIJOYSTATE2 { long lX, lY, lZ, lRx, lRy, lRz, rglSlider[2]; unsigned long rgdwPOV[4]; unsigned char rgbButtons[128]; };
struct DIPROPHEADER { unsigned long dwSize, dwHeaderSize, dwHow, dwObj; };
struct DIPROPRANGE { DIPROPHEADER diph; long lMin, lMax; };
#define DIPROP_RANGE ((void*)0)
#define DIPH_BYID 0
#define DIDFT_ABSAXIS 0x01
#define DIDFT_PSHBUTTON 0x04
#define DIDFT_POV 0x10
#define DISCL_EXCLUSIVE 0x01
#define DISCL_BACKGROUND 0x04
#define DI8DEVCLASS_GAMECTRL 4
#define DIEDFL_ATTACHEDONLY 1
#define DIENUM_CONTINUE 1
#define DIENUM_STOP 0
#define DIERR_INPUTLOST 0x8007001EL
#define c_dfDIJoystick2 (*(void*)0)

typedef void* VOID;

inline long DirectInput8Create(void*, unsigned long, const GUID&, void**, void*) { return -1; }

#endif

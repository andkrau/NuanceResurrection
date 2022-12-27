#include "InputManager.h"
#include "joystick.h"
#include "nuonenvironment.h"

#include <stdio.h>

#pragma warning(push)
#pragma warning(disable:6000 28251)
#include <dinput.h>
#pragma warning(pop)

extern NuonEnvironment nuonEnv;

InputManager::~InputManager() {}

class InputManagerImpl : public InputManager
{
private:
  HINSTANCE hInstance;
  HWND hWnd;
  HWND hDisplayWnd;

  static BOOL CALLBACK EnumJoystickObjectsCallback(const DIDEVICEOBJECTINSTANCE* pDi8DevObj, VOID* pContext);
  static BOOL CALLBACK EnumJoysticksCallback(const DIDEVICEINSTANCE* pDi8Dev, VOID* pContext);

  LPDIRECTINPUT8 pDi8;
  LPDIRECTINPUTDEVICE8* pDi8Devs;

  Joystick* pJoysticks;
  size_t joyArrayLen;

  size_t numJoysticks;
  LPDIRECTINPUTDEVICE8 pJoystick1;
  LPDIRECTINPUTDEVICE8 pGrabbedJoy;

  unsigned int whichController;
  uint16 keyButtons;
  uint16 joyButtons;

public:
  InputManagerImpl() : InputManager(),
    hInstance(nullptr),
    hWnd(nullptr),
    hDisplayWnd(nullptr),
    pDi8(nullptr),
    pDi8Devs(nullptr),
    pJoysticks(nullptr),
    joyArrayLen(0),
    numJoysticks(0),
    pJoystick1(nullptr),
    pGrabbedJoy(nullptr),
    whichController(1),
    keyButtons(0),
    joyButtons(0)
  {
  };
  ~InputManagerImpl();

  bool Init(HWND hDisplayWnd);
  void Activate();
  bool SetJoystick(size_t i);
  void UpdateState(CONTROLLER_CALLBACK applyState, ANYPRESSED_CALLBACK anyPressed, void* anyCtx);
  virtual void keyDown(CONTROLLER_CALLBACK applyState, int16 vkey);
  virtual void keyUp(CONTROLLER_CALLBACK applyState, int16 vkey);
  bool GrabJoystick(HWND hWnd, size_t i);
  void UngrabJoystick();
  const Joystick* EnumJoysticks(size_t* pNumJoysticks);
};

struct ManagerAndDev
{
  InputManagerImpl* im;
  LPDIRECTINPUTDEVICE8 pDi8Dev;
};

BOOL CALLBACK InputManagerImpl::EnumJoystickObjectsCallback(const DIDEVICEOBJECTINSTANCE* pDi8DevObj, VOID* pContext)
{
  ManagerAndDev* md = (ManagerAndDev*)pContext;
  TCHAR objMsg[1024];
  (void)objMsg;

  if (pDi8DevObj->dwType & DIDFT_PSHBUTTON)
  {
    /*_tcscpy_s(objMsg, _countof(objMsg), _T("Found a joystick button: "));
    _tcscat_s(objMsg, _countof(objMsg), pDi8DevObj->tszName);
    MessageBox(nullptr, objMsg, "Button found", MB_ICONINFORMATION);*/
  }
  else if (pDi8DevObj->dwType & DIDFT_ABSAXIS)
  {
    /*_tcscpy_s(objMsg, _countof(objMsg), _T("Found a joystick axis: "));
    _tcscat_s(objMsg, _countof(objMsg), pDi8DevObj->tszName);
    MessageBox(nullptr, objMsg, "Button found", MB_ICONINFORMATION);*/

    // XXX Hack: Force axis to trinary values (-1, 0, 1)
    DIPROPRANGE propRange = { 0 };
    propRange.diph.dwSize = sizeof(propRange);
    propRange.diph.dwHeaderSize = sizeof(propRange.diph);
    propRange.diph.dwHow = DIPH_BYID;
    propRange.diph.dwObj = pDi8DevObj->dwType;
    propRange.lMin = -1;
    propRange.lMax = 1;

    HRESULT hr = md->pDi8Dev->SetProperty(DIPROP_RANGE, &propRange.diph);

    if (FAILED(hr))
    {
      MessageBox(nullptr, "Failed DIRECTINPUTDEVICE8::SetProperty()", "Error", MB_ICONWARNING);
      return DIENUM_STOP;
    }
  }

  return DIENUM_CONTINUE;
}

BOOL CALLBACK InputManagerImpl::EnumJoysticksCallback(const DIDEVICEINSTANCE* pDi8Dev, VOID* pContext)
{
  InputManagerImpl* im = (InputManagerImpl*)pContext;
  size_t i = im->numJoysticks;

  if (im->joyArrayLen < (i + 1))
  {
    size_t newJoyArrayLen = im->joyArrayLen ? im->joyArrayLen * 2 : 1;
    Joystick* newJoyArray = (Joystick*)realloc(im->pJoysticks, sizeof(im->pJoysticks[0]) * newJoyArrayLen);
    LPDIRECTINPUTDEVICE8* newDi8Devs = (LPDIRECTINPUTDEVICE8*)realloc(im->pDi8Devs, sizeof(im->pDi8Devs[0]) * newJoyArrayLen);

    if (!newJoyArray || !newDi8Devs) {
      im->numJoysticks--;
      return DIENUM_STOP;
    }

    im->pJoysticks = newJoyArray;
    im->pDi8Devs = newDi8Devs;
    im->joyArrayLen = newJoyArrayLen;
  }

  /*TCHAR joyMsg[1024];

  _sntprintf_s(joyMsg, _countof(joyMsg), _T("Found a joystick: %s - %s"), pDi8Dev->tszProductName, pDi8Dev->tszInstanceName);
  MessageBox(nullptr, joyMsg, "Joystick found", MB_ICONINFORMATION); */

  HRESULT hr = im->pDi8->CreateDevice(pDi8Dev->guidInstance, &im->pDi8Devs[i], nullptr);

  if (FAILED(hr))
  {
    MessageBox(nullptr, "Failed DirectInput::CreateDevice()", "Error", MB_ICONWARNING);
    return DIENUM_CONTINUE;
  }

  hr = im->pDi8Devs[i]->SetDataFormat(&c_dfDIJoystick2);

  if (FAILED(hr))
  {
    MessageBox(nullptr, "Failed DIRECTINPUTDEVICE8::SetDataFormat()", "Error", MB_ICONWARNING);
    im->pDi8Devs[i]->Release();
    im->pDi8Devs[i] = nullptr;
    return DIENUM_CONTINUE;
  }

  hr = im->pDi8Devs[i]->SetCooperativeLevel(im->hWnd, DISCL_EXCLUSIVE | DISCL_BACKGROUND);

  if (FAILED(hr))
  {
    MessageBox(nullptr, "Failed DIRECTINPUTDEVICE8::SetCooperativeLevel()", "Error", MB_ICONWARNING);
    im->pDi8Devs[i]->Release();
    im->pDi8Devs[i] = nullptr;
    return DIENUM_CONTINUE;
  }

  ManagerAndDev md;
  md.im = im;
  md.pDi8Dev = im->pDi8Devs[i];
  hr = im->pDi8Devs[i]->EnumObjects(EnumJoystickObjectsCallback, &md, DIDFT_ABSAXIS | DIDFT_PSHBUTTON | DIDFT_POV);

  if (FAILED(hr))
  {
    MessageBox(nullptr, "Failed DIRECTINPUTDEVICE8::EnumObjects()", "Error", MB_ICONWARNING);
    im->pDi8Devs[i]->Release();
    im->pDi8Devs[i] = nullptr;
    return DIENUM_CONTINUE;
  }

  im->pJoysticks[i].guid = pDi8Dev->guidInstance;
  _tcscpy_s(im->pJoysticks[i].tszName, _countof(im->pJoysticks[i].tszName), pDi8Dev->tszInstanceName);
  im->numJoysticks++;

  if (im->pJoysticks[i].guid == nuonEnv.GetController1Joystick())
  {
    im->pJoystick1 = im->pDi8Devs[i];
  }

  return DIENUM_CONTINUE;
}

bool InputManagerImpl::Init(HWND hDpyWnd)
{
  hDisplayWnd = hWnd = hDpyWnd;
  hInstance = (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE);

  HRESULT hr = DirectInput8Create(hInstance, DIRECTINPUT_VERSION,
    IID_IDirectInput8, (VOID**)&pDi8, nullptr);

  if (FAILED(hr))
  {
    MessageBox(nullptr, "Failed DirectInput8Create()", "Error", MB_ICONWARNING);
    return false;
  }

  hr = pDi8->EnumDevices(DI8DEVCLASS_GAMECTRL, EnumJoysticksCallback, this, DIEDFL_ATTACHEDONLY);

  if (FAILED(hr) || (numJoysticks == 0))
  {
    MessageBox(nullptr, "Failed to enumerate joysticks, or no joystick found", "Error", MB_ICONWARNING);
    return false;
  }

  if (!pJoystick1) pJoystick1 = pDi8Devs[0];

  return true;
}

InputManagerImpl::~InputManagerImpl()
{
  if (pJoystick1)
  {
    pJoystick1->Unacquire();
    pJoystick1 = nullptr;
  }

  for (size_t i = 0; i < numJoysticks; i++)
  {
    pDi8Devs[i]->Release();
  }

  free(pDi8Devs);
  free(pJoysticks);

  if (pDi8)
  {
    pDi8->Release();
    pDi8 = nullptr;
  }
}

void InputManagerImpl::Activate()
{
  if (pJoystick1) pJoystick1->Acquire();
}

bool InputManagerImpl::SetJoystick(size_t idx)
{
  if (idx >= numJoysticks) return false;
  if (pGrabbedJoy) return false;
  if (pJoystick1) pJoystick1->Unacquire();

  pJoystick1 = pDi8Devs[idx];

  pJoystick1->Acquire();

  return true;
}

bool InputManagerImpl::GrabJoystick(HWND hWndGrab, size_t i)
{
  // Can't grab twice without ungrabbing
  if (hWnd != hDisplayWnd) return false;

  if (i >= numJoysticks) return false;

  if (pDi8Devs[i] == pJoystick1) pJoystick1->Unacquire();

  HRESULT hr = pDi8Devs[i]->SetCooperativeLevel(hWndGrab, DISCL_EXCLUSIVE | DISCL_BACKGROUND);

  if (FAILED(hr))
  {
    MessageBox(nullptr, "Failed DIRECTINPUTDEVICE8::SetCooperativeLevel()", "Error", MB_ICONWARNING);
    return false;
  }

  hr = pDi8Devs[i]->Acquire();

  if (FAILED(hr))
  {
    pDi8Devs[i]->SetCooperativeLevel(hWnd, DISCL_EXCLUSIVE | DISCL_BACKGROUND);
    MessageBox(nullptr, "Failed DIRECTINPUTDEVICE8::Acquire()", "Error", MB_ICONWARNING);
    return false;
  }

  hWnd = hWndGrab;
  pGrabbedJoy = pDi8Devs[i];

  return true;
}

void InputManagerImpl::UngrabJoystick()
{
  if (hWnd == hDisplayWnd) return;

  hWnd = hDisplayWnd;

  pGrabbedJoy->Unacquire();

  pGrabbedJoy->SetCooperativeLevel(hWnd, DISCL_EXCLUSIVE | DISCL_BACKGROUND);

  if (pGrabbedJoy == pJoystick1)
  {
    pJoystick1->Acquire();
  }

  pGrabbedJoy = nullptr;
}

const Joystick* InputManagerImpl::EnumJoysticks(size_t* pNumJoysticks)
{
  *pNumJoysticks = numJoysticks;
  return pJoysticks;
}

void InputManagerImpl::UpdateState(CONTROLLER_CALLBACK applyState, ANYPRESSED_CALLBACK anyPressed, void* anyCtx)
{
  DIJOYSTATE2 js;
  HRESULT hr;
  LPDIRECTINPUTDEVICE8 pJoy = pGrabbedJoy ? pGrabbedJoy : pJoystick1;

  do {
    hr = pJoy->Poll();
    if (!FAILED(hr)) hr = pJoy->GetDeviceState(sizeof(DIJOYSTATE2), &js);
    if (FAILED(hr)) hr = pJoy->Acquire();
  } while (hr == DIERR_INPUTLOST);

  if (!FAILED(hr))
  {
    InputType type;
    int idx;
    int subIdx;

#define DO_CTRLR1()                                                                             \
  do {                                                                                          \
    CURSTATE() = true;                                                                          \
    int bitnum = nuonEnv.GetCTRLRBitnumFromMapping(ControllerButtonMapping(type, idx, subIdx)); \
    if (bitnum >= 0) joyButtons |= (1 << bitnum);                                               \
  } while (0)

#define DO_ANYPRESSED()                                   \
    do {                                                  \
        if (anyPressed && (CURSTATE() && !PREVSTATE())) { \
            anyPressed(anyCtx, type, idx, subIdx);        \
            return;                                       \
        }                                                 \
    } while (0)

    joyButtons = 0;

    static bool povLastState[_countof(js.rgdwPOV)][4] = {};
    bool povState[_countof(js.rgdwPOV)][4] = {};
#define CURSTATE() (povState[idx][subIdx])
#define PREVSTATE() (povLastState[idx][subIdx])
    type = JOYPOV;
    for (idx = 0; idx < _countof(js.rgdwPOV); idx++)
    {
      DWORD angle = js.rgdwPOV[idx];

      if (LOWORD(angle) == 0xFFFF) continue; // Centered

      if ((angle > 33750) || (angle <= 2250))
      {
        // North
        subIdx = 0;
        povState[idx][subIdx] = true;
        DO_CTRLR1();
        DO_ANYPRESSED();
      }
      else if ((angle > 2250) && (angle <= 6750))
      {
        // Northeast
        subIdx = 0;
        DO_CTRLR1();
        subIdx = 1;
        DO_CTRLR1();
        // DO_ANYPRESSED(); Unclear which button this is.
      }
      else if ((angle > 6750) && (angle <= 11250))
      {
        // East
        subIdx = 1;
        DO_CTRLR1();
        DO_ANYPRESSED();
      }
      else if ((angle > 11250) && (angle <= 15750))
      {
        // Southeast
        subIdx = 1;
        DO_CTRLR1();
        subIdx = 2;
        DO_CTRLR1();
        // DO_ANYPRESSED(); Unclear which button this is.
      }
      else if ((angle > 15750) && (angle <= 20250))
      {
        // Down
        subIdx = 2;
        DO_CTRLR1();
        DO_ANYPRESSED();
      }
      else if ((angle > 20250) && (angle <= 24750))
      {
        // Southwest
        subIdx = 2;
        DO_CTRLR1();
        subIdx = 3;
        DO_CTRLR1();
        // DO_ANYPRESSED(); Unclear which button this is.
      }
      else if ((angle > 24750) && (angle <= 29250))
      {
        // West
        subIdx = 3;
        DO_CTRLR1();
        DO_ANYPRESSED();
      }
      else if ((angle > 29250) && (angle <= 33750))
      {
        // Northwest
        subIdx = 3;
        DO_CTRLR1();
        subIdx = 0;
        DO_CTRLR1();
        // DO_ANYPRESSED(); Unclear which button this is.
      }
    }
    memcpy_s(&povLastState[0][0], sizeof(povLastState), &povState[0][0], sizeof(povState));
#undef CURSTATE
#undef PREVSTATE

    static bool axisLastState[&js.rglSlider[_countof(js.rglSlider)] - &js.lX][2] = {};
    bool axisState[&js.rglSlider[_countof(js.rglSlider)] - &js.lX][2] = {};
#define CURSTATE() (axisState[idx][subIdx])
#define PREVSTATE() (axisLastState[idx][subIdx])
    type = JOYAXIS;
    LONG* pAxis;
    for (idx = 0, pAxis = &js.lX; &pAxis[idx] < &js.rglSlider[_countof(js.rglSlider)]; idx++)
    {
      // type = JOYAXIS, index = i, subIndex = MIN->0, MAX->1
      switch (pAxis[idx]) {
      case -1:
        subIdx = 0;
        DO_CTRLR1();
        DO_ANYPRESSED();
        break;
      case 1:
        subIdx = 1;
        DO_CTRLR1();
        DO_ANYPRESSED();
        break;
      default:
        break;
      }
    }
    memcpy_s(&axisLastState[0][0], sizeof(axisLastState), &axisState[0][0], sizeof(axisState));
#undef CURSTATE
#undef PREVSTATE

    static bool butLastState[_countof(js.rgbButtons)] = {};
    bool butState[_countof(js.rgbButtons)] = {};
#define CURSTATE() (butState[idx])
#define PREVSTATE() (butLastState[idx])
    type = JOYBUT;
    subIdx = 0;
    for (idx = 0; idx < _countof(js.rgbButtons); idx++)
    {
      if (js.rgbButtons[idx] & 0x80)
      {
        /*
        TCHAR buttonMsg[1024];
        _sntprintf_s(buttonMsg, _countof(buttonMsg), _T("Button %d pressed"), i);
        MessageBox(nullptr, buttonMsg, "Button pressed", MB_ICONINFORMATION);
        */
        DO_CTRLR1();
        DO_ANYPRESSED();
      }
    }
    memcpy_s(&butLastState[0], sizeof(butLastState), &butState[0], sizeof(butState));
#undef CURSTATE
#undef PREVSTATE

    if (applyState) applyState(whichController, keyButtons | joyButtons);
  }
}

void InputManagerImpl::keyDown(CONTROLLER_CALLBACK applyState, int16 vkey)
{
  int bitNum = nuonEnv.GetCTRLRBitnumFromMapping(ControllerButtonMapping(KEY, vkey, 0));

  if (bitNum >= 0)
  {
    keyButtons |= 1 << bitNum;
  }

  if (applyState) applyState(whichController, keyButtons | joyButtons);
}

void InputManagerImpl::keyUp(CONTROLLER_CALLBACK applyState, int16 vkey)
{
  int bitNum = nuonEnv.GetCTRLRBitnumFromMapping(ControllerButtonMapping(KEY, vkey, 0));

  if (bitNum >= 0)
  {
    keyButtons &= ~(1 << bitNum);
  }

  if (vkey == 'Z')
  {
    whichController = 1 - whichController;
  }

  if (applyState) applyState(whichController, keyButtons | joyButtons);
}


InputManager* InputManager::Create()
{
  return new InputManagerImpl();
}

bool InputManager::StrToInputType(const char* str, InputType* type)
{
#define TYPE_CASE(t) \
do { \
  if (!_stricmp(#t, str)) \
  { \
    *type = t; \
    return true; \
  } \
} while (0)

  TYPE_CASE(KEY);
  TYPE_CASE(JOYPOV);
  TYPE_CASE(JOYAXIS);
  TYPE_CASE(JOYBUT);

#undef TYPE_CASE

  return false;
}

const char* InputManager::InputTypeToStr(InputType type)
{
#define TYPE_CASE(t) \
do { \
  case t: return #t; \
} while (0)

  switch (type)
  {
    TYPE_CASE(KEY);
    TYPE_CASE(JOYPOV);
    TYPE_CASE(JOYAXIS);
    TYPE_CASE(JOYBUT);
  default:
    // Only reached if someone adds a new type and forgets to update this function
    return "<Update_InputTypeToStr>";
  }

#undef TYPE_CASE
}


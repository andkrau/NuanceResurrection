#ifndef InputManagerH
#define InputManagerH

#include "basetypes.h"
#include <windows.h>
#include <tchar.h>

struct Joystick
{
  GUID guid;
  TCHAR tszName[MAX_PATH];
};

class InputManager
{
protected:
  InputManager() {}
public:
  enum InputType {
    // When modifying, update StrToInputType().
    KEY,
    JOYPOV,
    JOYAXIS,
    JOYBUT,
  };

  static InputManager* Create();
  static bool StrToInputType(const char* str, InputType* type);
  static const char* InputTypeToStr(InputType type);


  typedef void (*CONTROLLER_CALLBACK)(unsigned int controllerIdx, uint16 buttons);
  typedef void (*ANYPRESSED_CALLBACK)(void* ctx, InputType type, int idx, int subIdx);

  virtual ~InputManager() = 0;

  virtual bool Init(HWND hDisplayWnd) = 0;
  virtual void Activate() = 0;
  virtual bool SetJoystick(size_t idx) = 0;
  virtual void UpdateState(CONTROLLER_CALLBACK applyState, ANYPRESSED_CALLBACK anyPressed, void* anyCtx) = 0;
  virtual void keyDown(CONTROLLER_CALLBACK applyState, int16 vkey) = 0;
  virtual void keyUp(CONTROLLER_CALLBACK applyState, int16 vkey) = 0;
  virtual bool GrabJoystick(HWND hWnd, size_t i) = 0;
  virtual void UngrabJoystick() = 0;
  virtual const Joystick* EnumJoysticks(size_t* pNumJoysticks) = 0;
};

#endif

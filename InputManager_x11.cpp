// InputManager implementation for Linux - keyboard only (no SDL2 dependency)
#ifndef _WIN32

#include "InputManager.h"
#include "joystick.h"
#include "nuonenvironment.h"
#include <cstdio>
#include <cstring>

extern NuonEnvironment nuonEnv;

InputManager::~InputManager() {}

class InputManagerImpl : public InputManager
{
private:
  unsigned int whichController;
  uint16 keyButtons;

public:
  InputManagerImpl() : InputManager(), whichController(1), keyButtons(0) {}
  ~InputManagerImpl() {}

  bool Init(HWND) override { return true; }
  void Activate() override {}
  bool SetJoystick(size_t) override { return true; }

  void UpdateState(CONTROLLER_CALLBACK applyState, ANYPRESSED_CALLBACK, void*) override {
    if (applyState) applyState(whichController, keyButtons);
  }

  void keyDown(CONTROLLER_CALLBACK applyState, int16 vkey) override {
    const int bitNum = nuonEnv.GetCTRLRBitnumFromMapping(ControllerButtonMapping(KEY, vkey, 0));
    if (bitNum >= 0) keyButtons |= 1 << bitNum;
    if (applyState) applyState(whichController, keyButtons);
  }

  void keyUp(CONTROLLER_CALLBACK applyState, int16 vkey) override {
    const int bitNum = nuonEnv.GetCTRLRBitnumFromMapping(ControllerButtonMapping(KEY, vkey, 0));
    if (bitNum >= 0) keyButtons &= ~(1 << bitNum);
    if (vkey == 'Z') whichController = 1 - whichController;
    if (applyState) applyState(whichController, keyButtons);
  }

  bool GrabJoystick(HWND, size_t) override { return false; }
  void UngrabJoystick() override {}
  const Joystick* EnumJoysticks(size_t* pNumJoysticks) override { *pNumJoysticks = 0; return nullptr; }
};

InputManager* InputManager::Create() { return new InputManagerImpl(); }

bool InputManager::StrToInputType(const char* str, InputType* type)
{
  if (!strcasecmp("KEY", str)) { *type = KEY; return true; }
  if (!strcasecmp("JOYPOV", str)) { *type = JOYPOV; return true; }
  if (!strcasecmp("JOYAXIS", str)) { *type = JOYAXIS; return true; }
  if (!strcasecmp("JOYBUT", str)) { *type = JOYBUT; return true; }
  return false;
}

const char* InputManager::InputTypeToStr(InputType type)
{
  switch (type) {
    case KEY: return "KEY";
    case JOYPOV: return "JOYPOV";
    case JOYAXIS: return "JOYAXIS";
    case JOYBUT: return "JOYBUT";
    default: return "<unknown>";
  }
}

#endif

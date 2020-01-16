//---------------------------------------------------------------------------
#include "Bios.h"
#include "byteswap.h"
#include "mpe.h"
#include "joystick.h"
#include "NuonEnvironment.h"
#include "NuonMemoryMap.h"
//---------------------------------------------------------------------------
extern NuonEnvironment *nuonEnv;
ControllerData *controller;

#define NUM_CONTROLLER_ENTRIES (9)

void ControllerInitialize(MPE *mpe)
{
  uint32 properties;
  uint32 config;

  //return the last 144 bytes of the BIOS region
  mpe->regs[0] = CONTROLLER_ADDRESS;
  controller = (ControllerData *)(nuonEnv->GetPointerToMemory(mpe,CONTROLLER_ADDRESS));
  for(uint32 i = 0; i < NUM_CONTROLLER_ENTRIES; i++)
  {
    controller[i].config = 0;
    controller[i].buttons = 0;
    controller[i].remote_buttons = 0;
    controller[i].d1.xAxis = 0;
    controller[i].d2.yAxis = 0;
    controller[i].d3.xAxis2 = 0;
    controller[i].d4.yAxis2 = 0;
    controller[i].d5.quadjoyX = 0;
    controller[i].d6.quadjoyY = 0;
  }

  properties = KEYBOARD_JOYSTICK_PROPERTIES;
  config = CONTROLLER_CHANGED_BIT | properties;
  SwapScalarBytes(&config);
  controller[1].config = config;
}

void DeviceDetect(MPE *mpe)
{
  uint32 slot = mpe->regs[0];
  uint32 config;

  if(slot == 1)
  {
    config = controller[1].config;
    SwapScalarBytes(&config);

    if((config & CONTROLLER_CHANGED_BIT) == 0)
    {
      mpe->regs[0] = 0xFFFFFFFFUL; //unchanged: -1
      return;
    }

    if(controller)
    {
      config = CONTROLLER_STATUS_BIT | KEYBOARD_JOYSTICK_PROPERTIES;
      SwapScalarBytes(&config);

      controller[1].config = config;
    }

    mpe->regs[0] = 1;
  }
  else
  {
    controller[slot].config = 0;
    controller[slot].buttons = 0;
    controller[slot].d1.xAxis = 0;
    controller[slot].d2.yAxis = 0;
    controller[slot].d3.xAxis2 = 0;
    controller[slot].d4.yAxis2 = 0;
    controller[slot].d5.quadjoyX = 0;
    controller[slot].d6.quadjoyY = 0;
    mpe->regs[0] = 0; //No controller, for now
  }
}

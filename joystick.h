#ifndef joystickH
#define joystickH

#include "basetypes.h"

/////////////////////////////////
// Controller properties values
/////////////////////////////////

#define CTRLR_STDBUTTONS        (0x00000001)
#define CTRLR_DPAD              (0x00000002)
#define CTRLR_SHOULDER          (0x00000004)
#define CTRLR_EXTBUTTONS        (0x00000008)

#define CTRLR_ANALOG1           (0x00000010)
#define CTRLR_ANALOG2           (0x00000020)
#define CTRLR_WHEEL             (0x00000040)
#define CTRLR_PADDLE            (0x00000040)
#define CTRLR_THROTTLEBRAKE     (0x00000080)

#define CTRLR_THROTTLE          (0x00000100)
#define CTRLR_BRAKE             (0x00000200)
#define CTRLR_RUDDER            (0x00000400)
#define CTRLR_TWIST             (0x00000400)
#define CTRLR_MOUSE             (0x00000800)

#define CTRLR_TRACKBALL         (0x00000800)

#define CTRLR_QUADSPINNER1      (0x00001000)
#define CTRLR_QUADSPINNER2      (0x00002000)
#define CTRLR_THUMBWHEEL1       (0x00004000)
#define CTRLR_THUMBWHEEL2       (0x00008000)

#define CTRLR_FISHINGREEL       (0x00010000)
#define CTRLR_QUADJOY1          (0x00020000)
#define CTRLR_GENERIC           (0x00040000)
#define CTRLR_RESERVED          (0x00080000)

#define CTRLR_REMOTE            (0x00100000)
#define CTRLR_EXTENDED          (0x00200000)


/////////////////////////////////////////
// New-style POLYFACE button layout
/////////////////////////////////////////

#define CTRLR_BITNUM_DPAD_RIGHT			(8)
#define CTRLR_BITNUM_DPAD_UP			(9)
#define CTRLR_BITNUM_DPAD_LEFT			(10)
#define CTRLR_BITNUM_DPAD_DOWN			(11)
#define CTRLR_BITNUM_BUTTON_A			(14)
#define CTRLR_BITNUM_BUTTON_B			(3)
#define CTRLR_BITNUM_BUTTON_START		(13)
#define CTRLR_BITNUM_BUTTON_NUON		(12)
#define CTRLR_BITNUM_BUTTON_C_DOWN		(15)
#define CTRLR_BITNUM_BUTTON_C_RIGHT		(0)
#define CTRLR_BITNUM_BUTTON_C_LEFT		(2)
#define CTRLR_BITNUM_BUTTON_C_UP		(1)
#define CTRLR_BITNUM_BUTTON_R			(4)
#define CTRLR_BITNUM_BUTTON_L			(5)

#define CTRLR_BITNUM_UNUSED_1			(6)
#define CTRLR_BITNUM_UNUSED_2			(7)

//////////////
// Bit masks for buttons
//////////////

// CTRLR_DPAD group
constexpr uint16 CTRLR_DPAD_RIGHT = (1<<CTRLR_BITNUM_DPAD_RIGHT);
constexpr uint16 CTRLR_DPAD_UP    = (1<<CTRLR_BITNUM_DPAD_UP);
constexpr uint16 CTRLR_DPAD_LEFT  = (1<<CTRLR_BITNUM_DPAD_LEFT);
constexpr uint16 CTRLR_DPAD_DOWN  = (1<<CTRLR_BITNUM_DPAD_DOWN);


// CTRLR_STDBUTTONS group
constexpr uint16 CTRLR_BUTTON_A     = (1<<CTRLR_BITNUM_BUTTON_A);
constexpr uint16 CTRLR_BUTTON_B     = (1<<CTRLR_BITNUM_BUTTON_B);
constexpr uint16 CTRLR_BUTTON_START = (1<<CTRLR_BITNUM_BUTTON_START);
constexpr uint16 CTRLR_BUTTON_Z     = (1<<CTRLR_BITNUM_BUTTON_NUON);	// Don't use CTRLR_BUTTON_Z, use CTRLR_BUTTON_NUON instead
constexpr uint16 CTRLR_BUTTON_NUON  = (1<<CTRLR_BITNUM_BUTTON_NUON);

// CTRLR_EXTBUTTONS group
constexpr uint16 CTRLR_BUTTON_C_DOWN  = (1<<CTRLR_BITNUM_BUTTON_C_DOWN);
constexpr uint16 CTRLR_BUTTON_C_RIGHT = (1<<CTRLR_BITNUM_BUTTON_C_RIGHT);
constexpr uint16 CTRLR_BUTTON_C_LEFT  = (1<<CTRLR_BITNUM_BUTTON_C_LEFT);
constexpr uint16 CTRLR_BUTTON_C_UP    = (1<<CTRLR_BITNUM_BUTTON_C_UP);

// CTRLR_SHOULDER group
constexpr uint16 CTRLR_BUTTON_R = (1<<CTRLR_BITNUM_BUTTON_R);
constexpr uint16 CTRLR_BUTTON_L = (1<<CTRLR_BITNUM_BUTTON_L);

// UNUSED group
constexpr uint16 CTRLR_UNUSED_1 = (1<<CTRLR_BITNUM_UNUSED_1);
constexpr uint16 CTRLR_UNUSED_2 = (1<<CTRLR_BITNUM_UNUSED_2);

//Push current alignment and force byte aligment
#pragma pack(push, 1)

struct ControllerData
{
// scalar 0
/*
  unsigned int            changed : 1; //LSb
  unsigned int            status : 1;
  unsigned long           manufacture_id : 8;
  unsigned long           properties : 22; //MSb
*/
  uint32 config;

// scalar 1
  uint16 buttons;

  union
  {
    int8 xAxis;
    int8 wheel;
    int8 paddle;
    int8 rodX;
  } d1;

  union
  {
    int8 yAxis;
    int8 rodY;
  } d2;

// scalar 2
  union
  {
    int8 xAxis2;
    uint8 throttle;
    int8 throttle_brake;
  } d3;

  union
  {
    uint8 yAxis2;
    uint8 brake;
    int8 rudder;
    int8 twist;
  } d4;

  union
  {
    int8 quadjoyX;
    int8 mouseX;
    int8 thumbwheel1;
    int8 spinner1;
    int8 reelY;
  } d5;

  union
  {
    int8 quadjoyY;
    int8 mouseY;
    int8 thumbwheel2;
    int8 spinner2;
  } d6;

// scalar 3
  uint32 remote_buttons;
};

//restore pushed alignment
#pragma pack(pop)

class MPE;

void ControllerInitialize(MPE &mpe);
void DeviceDetect(MPE &mpe);

#endif

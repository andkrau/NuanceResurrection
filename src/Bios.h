#ifndef BIOS_H
#define BIOS_H

#include "basetypes.h"

class MPE;

typedef void (*NuonBiosHandler)(MPE &);

void InitBios(MPE &mpe);
void BiosPoll(MPE &mpe);
void BiosPauseMsg(MPE &mpe);
uint32 PatchJumptable(const uint32 vectorAddress, uint32 newEntry);

struct BiosInfo {
    unsigned short major_version;
    unsigned short minor_version;
    unsigned short vm_revision;
    unsigned short oem_revision;
    char *info_string;
    char *date_string;
    unsigned int HAL_version;
};

//BIOS Memory Map

#define MAX_RECV_HANDLERS (88U)

#define MINIBIOS_QUEUE2_TAIL_ADDRESS (0x20101C18U)
#define MINIBIOS_INTVEC1_HANDLER_ADDRESS (0x20301FF4U)
#define MINIBIOS_INTVEC2_HANDLER_ADDRESS (0x20301C1AU)
#define MINIBIOS_ENTRY_POINT (0x20301A80U)
#define MINIBIOS_SPINWAIT_ADDRESS (0x20301ACEU)

#define MINIBIOSX_QUEUE2_TAIL_ADDRESS (0x20100C18U)
#define MINIBIOSX_INTVEC1_HANDLER_ADDRESS (0x20300FF4U)
#define MINIBIOSX_INTVEC2_HANDLER_ADDRESS (0x20300C12U)
#define MINIBIOSX_SPINWAIT_ADDRESS (0x20300ACAU)

#define MEDIAWAITING_ADDRESS (0x80000804U)

#define MPERUNMEDIAMPE_ADDRESS (0x80760308U)

#define INTVEC1_HANDLER_ADDRESS (0x8076037AU)
#define INTVEC2_HANDLER_ADDRESS (0x80760480U)

#define ISR_EXIT_HOOK_ADDRESS (0x807FFC90U)

#define COMMRECV_PACKET_AVAILABLE_ADDRESS (0x807FFCE8U)
#define ENVIRONMENT_ADDRESS (0x807FFCF0U)
#define SCRATCH_MEM_ADDRESS (0x807FFD00U)
#define CYCLE_COUNTER_ADDRESS (0x807FFDBCU)
#define VIDEO_FIELD_COUNTER_ADDRESS (0x807FFDC0U)
#define LAST_VIDEO_CONFIG_FIELD_COUNTER_ADDRESS (0x807FFDC4U)
#define NUM_INSTALLED_COMMRECV_HANDLERS_ADDRESS (0x807FFDC8U)
#define BIOS_VIDEO_ISR_ADDRESS (0x807FFDCCU)
#define INTERRUPT_VECTOR_ADDRESS (0x807FFDD0U)
#define COMMRECV_HANDLER_LIST_ADDRESS (0x807FFE50U)
#define MPE_THREAD_RETURN_ADDRESS (0x807FFF60U)
#define CONTROLLER_ADDRESS (0x807FFF70U)
#define CONTROLLER_CHANGED_BIT (0x80000000U)
#define CONTROLLER_STATUS_BIT (0x40000000U)
#define CONTROLLER_MANUFACTURER_ID_MASK (0x3FC00000U)
#define CONTROLLER_PROPERTIES_MASK (0x003FFFFFU)
#define KEYBOARD_JOYSTICK_PROPERTIES (CTRLR_STDBUTTONS | CTRLR_DPAD | CTRLR_SHOULDER | CTRLR_EXTBUTTONS)

/* defines for interrupt vector numbers */
#define kIntrVideo      31
#define kIntrSystimer1  30
#define kIntrSystimer0  29
#define kIntrGPIO       28
#define kIntrAudio      27
#define kIntrHost       26
#define kIntrDebug      25
#define kIntrMBDone     24
#define kIntrDCTDone    23
#define kIntrIIC        20
#define kIntrSystimer2  16
#define kIntrCommXmit    5
#define kIntrCommRecv    4
#define kIntrSoftware    1
#define kIntrException   0

/* flags for _MemAlloc and _MemAdd */
#define kMemSDRAM  1
#define kMemSysRam 2
#define kMemKernel 4

#define kPollContinue    0
#define kPollSaveExit    1
#define kPollDisplayMsg  2
#define kPollPauseMsg    3

#endif

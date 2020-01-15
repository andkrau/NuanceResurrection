#include <assert.h>
#include "basetypes.h"
#include "byteswap.h"
#include "Bios.h"
#include "mpe.h"
#include "NuonEnvironment.h"

extern NuonEnvironment *nuonEnv;
extern void NullBiosHandler(MPE *mpe);

void API_EnterPE(MPE *mpe)
{
}

void API_NDK(MPE *mpe)
{
}

void API_DVD(MPE *mpe)
{
}

void API_FindDiskType(MPE *mpe)
{
  //Ballistic routine wont exit loop until FindDiskType returns non-zero
  mpe->regs[0] = -1;
}

void API_IsPresenting(MPE *mpe)
{
  //Return "not presenting" for now
  mpe->regs[0] = 0;
}

void API_GetFieldCounter(MPE *mpe)
{
  uint32 fieldCounter = *((uint32 *)&nuonEnv->systemBusDRAM[VIDEO_FIELD_COUNTER_ADDRESS & SYSTEM_BUS_VALID_MEMORY_MASK]);
  SwapScalarBytes(&fieldCounter);

  mpe->regs[0] = fieldCounter;
}

void API_IsNDK(MPE *mpe)
{
  mpe->regs[0] = 0;
}

void API_IsDVD(MPE *mpe)
{
  mpe->regs[0] = 1;
}

void API_StepOneFrame(MPE *mpe)
{
  //mpe->regs[0] = 1;
}

void API_AUDIOStreamSelect(MPE *mpe)
{
  //mpe->regs[0] = 1;
}

void API_ForwardSpeed(MPE *mpe)
{
  //mpe->regs[0] = 1;
}

void API_PresentVOB(MPE *mpe)
{
  //mpe->regs[0] = 1;
}

void API_StartCellStill(MPE *mpe)
{
  //mpe->regs[0] = 1;
}

void API_CellStillDone(MPE *mpe)
{
  //mpe->regs[0] = 1;
}

void API_PresentCell(MPE *mpe)
{
  //mpe->regs[0] = 1;
}

void API_CellReadDone(MPE *mpe)
{
  //mpe->regs[0] = 1;
}

void API_ProgramChange(MPE *mpe)
{
  mpe->regs[0] = 0;
}

void API_SetAngle(MPE *mpe)
{
  //mpe->regs[0] = 1;
}

NuonBiosHandler SVC_API_Table[] = {
API_EnterPE, //_API_EnterPE
API_NDK, //_API_NDK
API_DVD, //_API_DVD
NullBiosHandler, //_API_VCD
NullBiosHandler, //_API_CDA
NullBiosHandler, //_API_VLM
NullBiosHandler, //_API_SVR
NullBiosHandler, //_API_DVD_AUDIO
NullBiosHandler, //_API_GetCGMS
NullBiosHandler, //_API_SetCGMS
NullBiosHandler, //_API_Eject
NullBiosHandler, //_API_Retract
API_IsNDK, //_API_IsNDK
API_IsDVD, //_API_IsDVD
NullBiosHandler, //_API_IsDVD_AUDIO
NullBiosHandler, //_API_IsVCD
NullBiosHandler, //_API_IsCDA
NullBiosHandler, //_API_IsVLM
NullBiosHandler, //_API_IsSVR
NullBiosHandler, //_API_JoyReset
NullBiosHandler, //_API_JoyPoll
NullBiosHandler, //_API_IsSet
NullBiosHandler, //_API_WasSet
NullBiosHandler, //_API_WasReleased
NullBiosHandler, //_API_JoyData
NullBiosHandler, //_API_JoyXData
NullBiosHandler, //_API_JoyYData
NullBiosHandler, //_API_STC
NullBiosHandler, //_API_GetMsClk
NullBiosHandler, //_API_SetMsClk
NullBiosHandler, //_API_WaitMs
API_GetFieldCounter, //_API_GetFieldCounter
NullBiosHandler, //_API_HostGetChar
NullBiosHandler, //_API_HostPutChar
NullBiosHandler, //_API_HostPeekChar
NullBiosHandler, //_API_HostQuery
NullBiosHandler, //_API_HostPeek
NullBiosHandler, //_API_HostWrite
NullBiosHandler, //_API_TryHostSend
NullBiosHandler, //_API_EchoEnable
NullBiosHandler, //_API_EchoDisable
API_FindDiskType, //_API_FindDiskType
NullBiosHandler, //_API_DriveReset
NullBiosHandler, //_API_TitleKey
NullBiosHandler, //_API_SearchDisk
NullBiosHandler, //_API_DMA_Read32
NullBiosHandler, //_API_DVD_IFO_Read
NullBiosHandler, //_API_DVD_SetEndOfDat
NullBiosHandler, //_API_DVD_SetEndOfLay
NullBiosHandler, //_API_DVD_DiskIsOpposite
NullBiosHandler, //_API_DVD_DiskIsParallel
NullBiosHandler, //_API_CSS_ReadKeyA
NullBiosHandler, //_API_CSS_ReadKeyB
NullBiosHandler, //_API_StopDrive
NullBiosHandler, //_API_DVD_ReadPSNsToC
NullBiosHandler, //_API_DiskType
NullBiosHandler, //_API_DVD_IsDiskOpposite
NullBiosHandler, //_API_DVD_DriveSetDisabled
NullBiosHandler, //_API_ReadPSNs
NullBiosHandler, //_API_FastReadPSNs
NullBiosHandler, //_API_ReadCPR_MAI
NullBiosHandler, //_API_StartReadPSNs
NullBiosHandler, //_API_ReadPSNsDone
NullBiosHandler, //_API_ResetReadPSNs
NullBiosHandler, //_API_TrickType
NullBiosHandler, //_API_I2C_StartMaster
NullBiosHandler, //_API_I2C_StartMaster
NullBiosHandler, //_API_I2C_StartMaster
NullBiosHandler, //_API_I2C_MasterWrite
NullBiosHandler, //_API_I2C_MasterReadD
NullBiosHandler, //_API_I2C_MasterWrite
NullBiosHandler, //_API_I2C_MasterReset
NullBiosHandler, //_API_FindVMG_Key
NullBiosHandler, //_API_VMG
NullBiosHandler, //_API_FindTitleKey
NullBiosHandler, //_API_VTSN
NullBiosHandler, //_API_RLBN_To_PSN
NullBiosHandler, //_API_LSN_To_PSN
NullBiosHandler, //_API_FindUDFFile
NullBiosHandler, //_API_FirstLSN
NullBiosHandler, //_API_FirstDataSector
NullBiosHandler, //_API_DVD_Mount
NullBiosHandler, //_API_DVD_UnMount
NullBiosHandler, //_API_HasDVDVideo
NullBiosHandler, //_API_HasDVDAudio
NullBiosHandler, //_API_SelectDVDVideo
NullBiosHandler, //_API_SelectDVDAudio
NullBiosHandler, //_API_FindDiskKey
NullBiosHandler, //_API_SetDiskKey
NullBiosHandler, //_API_SetTitleKey
NullBiosHandler, //_API_SetEndOfDataSec
NullBiosHandler, //_API_SetEndOfLayer0S
NullBiosHandler, //_API_DVD_TempRead
NullBiosHandler, //_API_DVD_ReadBlock
NullBiosHandler, //_API_DVD_ReadBlocks
NullBiosHandler, //_API_FP_Ready
NullBiosHandler, //_API_FP_Send
NullBiosHandler, //_API_HaveIRInput
NullBiosHandler, //_API_IR_Receiver
NullBiosHandler, //_API_PollIRController
NullBiosHandler, //_API_FlashErase
NullBiosHandler, //_API_HaveFlash
NullBiosHandler, //_API_FlashBusy
NullBiosHandler, //_API_FlashRead
NullBiosHandler, //_API_FlashWrite
NullBiosHandler, //_API_FetchNVData
NullBiosHandler, //_API_StoreNVData
NullBiosHandler, //_API_Start
NullBiosHandler, //_API_GetControlEvent
NullBiosHandler, //_API_TranslateEvent
NullBiosHandler, //_API_FLDisplay
NullBiosHandler, //_API_MemScratch
NullBiosHandler, //_API_StackInfoAddres
NullBiosHandler, //_API_CommXmit
NullBiosHandler, //_API_DVD_IFO_ReadBlock
NullBiosHandler, //_API_IsTrayOut
NullBiosHandler, //_API_DVD_ReMount
NullBiosHandler, //_API_LoadPE
NullBiosHandler, //_API_SetPowerState
NullBiosHandler, //_API_DVD_DriveIsTray
NullBiosHandler, //_API_CancelAPS
NullBiosHandler, //_API_TVSystem
NullBiosHandler, //_API_IsTrayEmpty
NullBiosHandler, //_API_ReadPSNsOK
NullBiosHandler, //_API_SetDiskType
NullBiosHandler, //_API_GetTitleKey
NullBiosHandler, //_API_FlashSectorSize
NullBiosHandler, //_API_GetError
};

NuonBiosHandler DVD_API_Table[] = {
API_StepOneFrame, //_API_StepOneFrame
API_AUDIOStreamSelect, //_API_AUDIOStreamSelect
NullBiosHandler, //_API_SetSPPalette
NullBiosHandler, //_API_SPStreamSelection
NullBiosHandler, //_API_VideoAttributes
NullBiosHandler, //_API_ChangeAngle
NullBiosHandler, //_API_Speed
NullBiosHandler, //_API_Rate
API_ForwardSpeed, //_API_ForwardSpeed
NullBiosHandler, //_API_BackwardSpeed
NullBiosHandler, //_API_IsForward
NullBiosHandler, //_API_IsReverse
NullBiosHandler, //_API_PauseOn
NullBiosHandler, //_API_PauseOff
NullBiosHandler, //_API_Paused
NullBiosHandler, //_API_ReleaseVOBU_Still
NullBiosHandler, //_API_StillOff
NullBiosHandler, //_API_StopDVD
NullBiosHandler, //_API_SetButton
API_PresentCell, //_API_PresentCell
API_CellReadDone, //_API_CellReadDone
API_StartCellStill, //_API_StartCellStill
API_CellStillDone, //_API_CellStillDone
NullBiosHandler, //_API_VOBU_UOP_CTL
API_IsPresenting, //_API_IsPresenting
NullBiosHandler, //_API_InVOBU_Still
NullBiosHandler, //_API_GetVOBU_Count
NullBiosHandler, //_API_FetchNAVPACK
NullBiosHandler, //_API_PTM
NullBiosHandler, //_API_S_PTM
NullBiosHandler, //_API_E_PTM
NullBiosHandler, //_API_ELTM
NullBiosHandler, //_API_GetVOBU_EA
NullBiosHandler, //_API_SetVOBU_Count
NullBiosHandler, //_API_HideButton
API_ProgramChange, //_API_ProgramChange
API_SetAngle, //_API_SetAngle
NullBiosHandler, //_API_ButtonDisplayMode
NullBiosHandler, //_API_NAVPACKADR
NullBiosHandler, //_API_GetNavPackAdr
NullBiosHandler, //_API_GetButtonCMD
NullBiosHandler, //_API_Button
NullBiosHandler, //_API_ButtonActivate
NullBiosHandler, //_API_SelectAndActivate
NullBiosHandler, //_API_UpperButton
NullBiosHandler, //_API_LowerButton
NullBiosHandler, //_API_LeftButton
NullBiosHandler, //_API_RightButton
NullBiosHandler, //_API_NumberOfButtons
NullBiosHandler, //_API_AutoActionButton
NullBiosHandler, //_API_ButtonSelect
NullBiosHandler, //_API_ButtonGroup
NullBiosHandler, //_API_AllowFOSL
NullBiosHandler, //_API_FreezeDisplayMode
NullBiosHandler, //_API_SlowDisplayMode
NullBiosHandler, //_API_TrickDisplayMode
NullBiosHandler, //_API_VOBU_Jump
NullBiosHandler, //_API_GetAngle
NullBiosHandler, //_API_AllowFSTA
NullBiosHandler, //_API_GetAudioStreamNumber
NullBiosHandler, //_API_GetSPStreamNumber
API_PresentVOB, //_API_PresentVOB
NullBiosHandler, //_API_SetScreenFit
NullBiosHandler, //_API_VOBU_HasVideo
NullBiosHandler, //_API_SetAudioStreamNumber
NullBiosHandler, //_API_CSS
NullBiosHandler, //_API_PresentStream
NullBiosHandler, //_API_CancelProcessed
NullBiosHandler, //_API_DisplayProcessed
NullBiosHandler, //_API_DisplayingFrame
NullBiosHandler, //_API_PresentDualStreams
NullBiosHandler, //_API_GetNonVideoNavP
NullBiosHandler, //_API_SetPECompatLevel
NullBiosHandler //_API_ButtonOffsetNo
};

void InitDVDJumpTable()
{
  for(uint32 i = 0; i < 1024; i++)
  {
    PatchJumptable(DVD_JUMPTABLE_START + (i << 3UL), ROM_PE_BASE + (i << 3));
  }
}

void CallPEHandler(MPE *mpe, uint32 address)
{
  NuonBiosHandler *table;
  uint32 offset;

  if(((address & 0x8FFFFFFFUL) < SVC_API_JUMPTABLE_START) || ((address & 0x8FFFFFFFUL) >= PTR_API_JUMPTABLE_START))
  {
    return;
  }
  else
  {
    offset = (address >> 3) & 0x7F;
    switch((address >> 10) & 0x7)
    {
      case 0:
        table = SVC_API_Table;
        break;
      case 1:
        //table = Audio_API_Table;
        return;
        break;
      case 2:
        //table = CDA_API_Table;
        return;
        break;
      case 3:
        table = DVD_API_Table;
        break;
      case 4:
        //offset &= 0x3F;
        //table = DVDA_API_Table;
        //if((address & 0x200)
        //{
        //  table = NTC_API_Table;
        //}
        return;
        break;
      case 5:
        //table = SVR_API_Table;
        return;
        break;
      case 6:
        //table = VCD_API_Table;
        return;
        break;
      case 7:
        //table = VIDEO_API_Table;
        return;
        break;
      default:
        assert(false);
        break;
    }
  
    table[offset](mpe);
  }
}

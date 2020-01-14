//---------------------------------------------------------------------------
#include "basetypes.h"
#include "byteswap.h"
#include "mpe.h"
#include "comm.h"
#include "NuonEnvironment.h"
#include <stdio.h>

extern NuonEnvironment *nuonEnv;
char msgBuf[512];
FILE *commLogFile;

//#define LOG_COMM

extern uint32 vdgCLUT[];

void DoCommBusController(void)
{
  static uint32 currentTransmitID = 0;
  static uint32 target;
  uint32 cmdValue;
  uint32 *clutPtr;
  static uint32 i;
  static bool bLocked;
  static bool bPending;

  bLocked = false;
  bPending = false;

  currentTransmitID &= 0x03UL;

  if(!(nuonEnv->mpe[currentTransmitID]->commctl & COMM_LOCKED_BIT))
  {
    for(i = 0; i < 4; i++)
    {
      if(nuonEnv->mpe[(currentTransmitID + i) & 0x03UL]->commctl & COMM_XMIT_BUFFER_FULL_BIT)
      {
        currentTransmitID = (currentTransmitID + i) & 0x03UL;
        bPending = true;
        break;
      }
    }
  }
  else
  {
    bLocked = true;
    bPending = (nuonEnv->mpe[currentTransmitID]->commctl & COMM_XMIT_BUFFER_FULL_BIT);
  }

  if(!bPending)
  {
    return;
  }

  target = nuonEnv->mpe[currentTransmitID]->commctl & COMM_TARGET_ID_BITS;

  if(target < 4)
  {
    if(!(nuonEnv->mpe[target]->commctl & COMM_DISABLED_BITS))
    {
      nuonEnv->mpe[currentTransmitID]->commctl &= ~(COMM_XMIT_BUFFER_FULL_BIT);
      nuonEnv->mpe[target]->commrecv0 = nuonEnv->mpe[currentTransmitID]->commxmit0;
      nuonEnv->mpe[target]->commrecv1 = nuonEnv->mpe[currentTransmitID]->commxmit1;
      nuonEnv->mpe[target]->commrecv2 = nuonEnv->mpe[currentTransmitID]->commxmit2;
      nuonEnv->mpe[target]->commrecv3 = nuonEnv->mpe[currentTransmitID]->commxmit3;
      nuonEnv->mpe[target]->comminfo &= 0xFFUL;
      nuonEnv->mpe[target]->comminfo |= (nuonEnv->mpe[currentTransmitID]->comminfo << 16);
      nuonEnv->mpe[target]->commctl &= ~(COMM_SOURCE_ID_BITS);
      nuonEnv->mpe[target]->commctl |= (COMM_RECV_BUFFER_FULL_BIT | (currentTransmitID << 16));

      nuonEnv->mpe[currentTransmitID]->TriggerInterrupt(INT_COMMXMIT);
      nuonEnv->mpe[target]->TriggerInterrupt(INT_COMMRECV);
      nuonEnv->pendingCommRequests--;

#ifdef LOG_COMM
      fprintf(commLogFile,"Comm packet sent: MPE%ld->MPE%ld, packet contents = {$%lx,$%lx,$%lx,$%lx, comminfo = $%lx\n",
        currentTransmitID,
        target,
        nuonEnv->mpe[currentTransmitID]->commxmit0,
        nuonEnv->mpe[currentTransmitID]->commxmit1,
        nuonEnv->mpe[currentTransmitID]->commxmit2,
        nuonEnv->mpe[currentTransmitID]->commxmit3,
        nuonEnv->mpe[currentTransmitID]->comminfo);
      fflush(commLogFile);
#endif
    }
    else
    {
      //xmit failed
      if(nuonEnv->mpe[currentTransmitID]->commctl & COMM_XMIT_RETRY_BIT)
      {
#ifdef LOG_COMM
        fprintf(commLogFile,"Comm packet failed: MPE%ld->MPE%ld, will retry\n",currentTransmitID,target);
#endif
      }
      else
      {
        //No Retry

        //Set Transmit Failed bit
        nuonEnv->mpe[currentTransmitID]->commctl |= COMM_XMIT_FAILED_BIT;
        //Clear Transmit Buffer full bit
        nuonEnv->mpe[currentTransmitID]->commctl &= ~(COMM_XMIT_BUFFER_FULL_BIT);
        nuonEnv->pendingCommRequests--;

#ifdef LOG_COMM
        fprintf(commLogFile,"Comm packet failed: MPE%ld->MPE%ld, no retry\n",currentTransmitID,target);
#endif
      }
    }
  }
  else
  {
    //Reserved MPE ID or hardware target (eg audio)

    //Pretend the device received the packet
    nuonEnv->mpe[currentTransmitID]->commctl &= (~COMM_XMIT_BUFFER_FULL_BIT);
    nuonEnv->mpe[currentTransmitID]->TriggerInterrupt(INT_COMMXMIT);

    if(target == COMM_ID_AUDIO)
    {
      //audio target
      cmdValue = nuonEnv->mpe[currentTransmitID]->commxmit0;
      switch(cmdValue >> 31)
      {
        case 0:
          break;
        case 1:
          cmdValue &= 0x7FFFFFFF;
          if(cmdValue == 0x2C)
          {
            //write channel mode register: update NuonEnviroment variable only for now
            //To do: modify SetChannelMode to recreate the sound buffer only if the new
            //channel mode buffer size differs from the previous value.
            nuonEnv->nuonAudioChannelMode = nuonEnv->mpe[currentTransmitID]->commxmit1;
          }
          break;
      }
    }
    else if(target == COMM_ID_VDG)
    {
      //vdg target
      cmdValue = nuonEnv->mpe[currentTransmitID]->commxmit0;
      switch(cmdValue >> 31)
      {
        case 0:
          break;
        case 1:
          cmdValue &= 0x7FFFFFFF;
          if((cmdValue >= 0x200) && (cmdValue < 0x300))
          {
            //write VDG clut entry
            vdgCLUT[cmdValue - 0x200] = nuonEnv->mpe[currentTransmitID]->commxmit1;
            SwapScalarBytes(&vdgCLUT[cmdValue - 0x200]);
          }
          break;
      }
    }

    nuonEnv->pendingCommRequests--;
  }

  if(!bLocked)
  {
    currentTransmitID = ((currentTransmitID + 1) & 0x03UL);
  }
}
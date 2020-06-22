#include "basetypes.h"
#include <stdio.h>
#include "byteswap.h"
#include "mpe.h"
#include "comm.h"
#include "NuonEnvironment.h"

extern NuonEnvironment nuonEnv;
extern uint32 vdgCLUT[];

#ifdef LOG_COMM
FILE *commLogFile;
#endif

void DoCommBusController(void)
{
  static uint32 currentTransmitID = 0;

  bool bLocked = false;
  bool bPending = false;

  if(!(nuonEnv.mpe[currentTransmitID].commctl & COMM_LOCKED_BIT))
  {
    for(uint32 i = 0; i < 4; i++)
    {
      const uint32 idx = (currentTransmitID + i) & 0x03UL;
      if(nuonEnv.mpe[idx].commctl & COMM_XMIT_BUFFER_FULL_BIT)
      {
        currentTransmitID = idx;
        bPending = true;
        break;
      }
    }
  }
  else
  {
    bLocked = true;
    bPending = (nuonEnv.mpe[currentTransmitID].commctl & COMM_XMIT_BUFFER_FULL_BIT);
  }

  if(!bPending)
    return;

  const uint32 target = nuonEnv.mpe[currentTransmitID].commctl & COMM_TARGET_ID_BITS;

  if(target < 64) // target is a MPE?
  {
    assert(target < 4);

    if(!(nuonEnv.mpe[target].commctl & (COMM_DISABLED_BITS | COMM_RECV_BUFFER_FULL_BIT)))
    {
      nuonEnv.mpe[currentTransmitID].commctl &= ~(COMM_XMIT_BUFFER_FULL_BIT);
      nuonEnv.mpe[target].commrecv[0] = nuonEnv.mpe[currentTransmitID].commxmit[0];
      nuonEnv.mpe[target].commrecv[1] = nuonEnv.mpe[currentTransmitID].commxmit[1];
      nuonEnv.mpe[target].commrecv[2] = nuonEnv.mpe[currentTransmitID].commxmit[2];
      nuonEnv.mpe[target].commrecv[3] = nuonEnv.mpe[currentTransmitID].commxmit[3];
      nuonEnv.mpe[target].comminfo &= 0xFFUL;
      nuonEnv.mpe[target].comminfo |= (nuonEnv.mpe[currentTransmitID].comminfo << 16);
      nuonEnv.mpe[target].commctl &= ~(COMM_SOURCE_ID_BITS);
      nuonEnv.mpe[target].commctl |= (COMM_RECV_BUFFER_FULL_BIT | (currentTransmitID << 16));

      nuonEnv.mpe[currentTransmitID].TriggerInterrupt(INT_COMMXMIT);
      nuonEnv.mpe[target].TriggerInterrupt(INT_COMMRECV);
      nuonEnv.pendingCommRequests--;

#ifdef LOG_COMM
      fprintf(commLogFile,"Comm packet sent: MPE%ld->MPE%ld, packet contents = {$%lx,$%lx,$%lx,$%lx, comminfo = $%lx\n",
        currentTransmitID,
        target,
        nuonEnv.mpe[currentTransmitID].commxmit[0],
        nuonEnv.mpe[currentTransmitID].commxmit[1],
        nuonEnv.mpe[currentTransmitID].commxmit[2],
        nuonEnv.mpe[currentTransmitID].commxmit[3],
        nuonEnv.mpe[currentTransmitID].comminfo);
      fflush(commLogFile);
#endif
    }
    else
    {
      //xmit failed
      if(nuonEnv.mpe[currentTransmitID].commctl & COMM_XMIT_RETRY_BIT)
      {
#ifdef LOG_COMM
        fprintf(commLogFile,"Comm packet failed: MPE%ld->MPE%ld, will retry\n",currentTransmitID,target);
#endif
      }
      else
      {
        //No Retry

        //Set Transmit Failed bit
        nuonEnv.mpe[currentTransmitID].commctl |= COMM_XMIT_FAILED_BIT;
        //Clear Transmit Buffer full bit
        nuonEnv.mpe[currentTransmitID].commctl &= ~(COMM_XMIT_BUFFER_FULL_BIT);
        nuonEnv.pendingCommRequests--;

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
    nuonEnv.mpe[currentTransmitID].commctl &= (~COMM_XMIT_BUFFER_FULL_BIT);
    nuonEnv.mpe[currentTransmitID].TriggerInterrupt(INT_COMMXMIT);

    if(target == COMM_ID_AUDIO)
    {
      //audio target
      uint32 cmdValue = nuonEnv.mpe[currentTransmitID].commxmit[0];
      switch(cmdValue >> 31)
      {
        case 0:
          break;
        case 1:
          cmdValue &= 0x7FFFFFFF;
          if(cmdValue == 0x2C)
          {
            //write channel mode register: update NuonEnviroment variable only for now
            //!! TODO modify SetChannelMode to recreate the sound buffer only if the new
            //channel mode buffer size differs from the previous value.
            assert((nuonEnv.mpe[currentTransmitID].commxmit[1] & ~(ENABLE_WRAP_INT | ENABLE_HALF_INT)) == (nuonEnv.nuonAudioChannelMode & ~(ENABLE_WRAP_INT | ENABLE_HALF_INT))); // for now we only support a change in these two flags
            nuonEnv.nuonAudioChannelMode = nuonEnv.mpe[currentTransmitID].commxmit[1];
          }
          break;
      }
    }
    else if(target == COMM_ID_VDG)
    {
      //vdg target
      uint32 cmdValue = nuonEnv.mpe[currentTransmitID].commxmit[0];
      switch(cmdValue >> 31)
      {
        case 0:
          break;
        case 1:
          cmdValue &= 0x7FFFFFFF;
          if((cmdValue >= 0x200) && (cmdValue < 0x300))
          {
            //write VDG clut entry
            vdgCLUT[cmdValue - 0x200] = nuonEnv.mpe[currentTransmitID].commxmit[1];
            SwapScalarBytes(&vdgCLUT[cmdValue - 0x200]);
          }
          break;
      }
    }

    nuonEnv.pendingCommRequests--;
  }

  if(!bLocked)
  {
    currentTransmitID = ((currentTransmitID + 1) & 0x03UL);
  }
}

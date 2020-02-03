#include "basetypes.h"
#include <assert.h>
#include "byteswap.h"
#include "comm.h"
#include "dma.h"
#include "mpe.h"
#include "NuonMemoryMap.h"
#include "NuonEnvironment.h"

extern NuonEnvironment nuonEnv;

uint32 MPE::ReadControlRegister(const uint32 address, const uint32 entrypRegs[48])
{
  switch(address >> 4)
  {
    case 0x0:
      //mpectl: bits 0,2,4,6,8,10,12,14 and 23 read as 0
      //bits 16,17,18,19,20,21,22,28,29,30 and 31 are reserved
      //and also forced to 0
      return mpectl & ~(0xF0FF5555UL);
    case 0x1:
      //excepsrc
      return excepsrc;
    case 0x2:
      //excepclr: always reads as zero
      return 0;
    case 0x3:
      //excephalten
      return excephalten & 0x1FFFUL;
    case 0x4:
      return tempCC & 0x7FFUL;
    case 0x5:
      return pcfetch;
    case 0x6:
      return pcroute & 0xFFFFFFFE;
    case 0x7:
      return pcexec;
    case 0x8:
      return entrypRegs[RZ_REG];
    case 0x9:
      return rzi1;
    case 0xA:
      return rzi2;
    case 0xB:
      return intvec1;
    case 0xC:
      return intvec2;
    case 0xD:
      return intsrc;
    case 0xE:
      //intclr: always reads as zero
      return 0;
    case 0xF:
      //intctl
      return (intctl & 0xAA);
    case 0x10:
      //inten1
    case 0x11:
      //inten1set: always reads the same as inten1
      return inten1;
    case 0x12:
      //inten1clr: always reads as zero
      return 0;
    case 0x13:
      //inten2sel
      return (inten2sel & 0x1F);
    case 0x1E:
      return entrypRegs[COUNTER_REG+0] & 0xFFFF;
    case 0x1F:
      return entrypRegs[COUNTER_REG+1] & 0xFFFF;
    case 0x20:
      //rx
      return entrypRegs[INDEX_REG+REG_X];
    case 0x21:
      //ry
      return entrypRegs[INDEX_REG+REG_Y];
    case 0x22:
      return entrypRegs[XYR_REG] & ((0x3FFUL << 16) | 0x3FFUL);
    case 0x23:
      return xybase & 0xFFFFFFFC;
    case 0x24:
      return entrypRegs[XYC_REG] & ~((1UL << 11) | (1UL << 27) | (1UL << 31));
    case 0x25:
      //ru
      return entrypRegs[INDEX_REG+REG_U];
    case 0x26:
      //rv
      return entrypRegs[INDEX_REG+REG_V];
    case 0x27:
      return entrypRegs[UVR_REG] & ((0x3FFUL << 16) | 0x3FFUL);
    case 0x28:
      return uvbase & 0xFFFFFFFC;
    case 0x29:
      return entrypRegs[UVC_REG] & ~((1UL << 11) | (1UL << 27) | (1UL << 31));
    case 0x2A:
      return linpixctl & ((1UL << 28) | (15UL << 20));
    case 0x2B:
      return clutbase & 0xFFFFFFC0;
    case 0x2C:
      return entrypRegs[SVS_REG] & 0x03UL;
    case 0x2D:
      return (((int32)(entrypRegs[ACS_REG] << 25)) >> 25);
    case 0x2E:
      return sp & 0xFFFFFFF0;
    case 0x2F:
      return dabreak;
    case 0x30:
    case 0x31:
    case 0x32:
    case 0x33:
    case 0x34:
    case 0x35:
    case 0x36:
    case 0x37:
    case 0x38:
    case 0x39:
    case 0x3A:
    case 0x3B:
    case 0x3C:
    case 0x3D:
    case 0x3E:
    case 0x3F:
    case 0x40:
    case 0x41:
    case 0x42:
    case 0x43:
    case 0x44:
    case 0x45:
    case 0x46:
    case 0x47:
    case 0x48:
    case 0x49:
    case 0x4A:
    case 0x4B:
    case 0x4C:
    case 0x4D:
    case 0x4E:
    case 0x4F:
      return entrypRegs[(address >> 4) - 0x30];
    case 0x50:
      return odmactl & 0x60;
    case 0x51:
      return odmacptr & 0x207FFFF0UL;
    case 0x60:
      return mdmactl & ~((3UL << 14) | (1UL << 8) | 0xF);
    case 0x61:
      return mdmacptr & 0x207FFFF0UL;
    case 0x7E:
      //comminfo
      return comminfo & ((0xFFUL << 16) | 0xFFUL);
    case 0x7F:
      //commctl
      return commctl & ~((0x3FUL << 24) | (0xFUL << 8));
    case 0x80:
    {
      switch(address & 0x0F)
      {
        //scalar commxmit register
        case 0x00:
          return commxmit0;
        case 0x04:
          return commxmit1;
        case 0x08:
          return commxmit2;
        case 0x0C:
          return commxmit3;
        default:
          return 0;
      }
    }
    case 0x81:
    {
      switch(address & 0x0F)
      {
        //scalar commrecv register
        case 0x00:
          return commrecv0;
        case 0x04:
          return commrecv1;
        case 0x08:
          return commrecv2;
        case 0x0C:
          //commrecv3: reading this register clears recv
          //buffer full bit in commctl
          commctl &= ~COMM_RECV_BUFFER_FULL_BIT;
          return commrecv3;
        default:
          return 0;
      }
    }
    case 0xFF:
    {
      switch(address & 0x0F)
      {
        case 0x00:
        //configa
          return configa;
        case 0x04:
          //configb: reserved
          return configb;
        case 0x08:
          //dcachectl
          return dcachectl;
        case 0x0C:
          //icachectl
          return icachectl;
        default:
          return 0;
      }
    }
    default:
      //no special handling: return control register contents verbatim
      return *(&mpectl + (address >> 4));
  }
}

void MPE::WriteControlRegister(const uint32 address, const uint32 data)
{
  switch(address >> 4)
  {
    case 0x0:
    {
      //Conditionally clear all bits according to their associated bit clear bit

      const uint32 prevGoState = mpectl & MPECTRL_MPEGO;
      const uint32 clearBits = data &
        (MPECTRL_MPEGOCLR | 
         MPECTRL_MPESINGLESTEPCLR |
         MPECTRL_MPEDARDBRKENCLR |
         MPECTRL_MPEDAWRBRKENCLR |
         MPECTRL_MPEIS2XCLR |
         MPECTRL_MPEINTTOHOSTCLR |
         MPECTRL_MPEWASRESETCLR);
      const uint32 setBits = data & ~clearBits;

      mpectl &= ~(clearBits << 1);

      //Even though they should already be clear, explicitly set the bit clear bits to zero
      mpectl &=  
        ~(MPECTRL_MPEGOCLR | 
          MPECTRL_MPESINGLESTEPCLR |
          MPECTRL_MPEDARDBRKENCLR |
          MPECTRL_MPEDAWRBRKENCLR |
          MPECTRL_MPEIS2XCLR |
          MPECTRL_MPEINTTOHOSTCLR |
          MPECTRL_MPEWASRESETCLR);

      mpectl |= (setBits & 0xFFFF);

      //If the cycleType_wren bit is set, write the cycleType bits
      if(setBits & MPECTRL_MPECYCLETYPEWREN)
      {
        mpectl = (mpectl & 0xFFFF) | (setBits & MPECTRL_MPECYCLETYPE);
      }

      //If the resetMpe bit is set, handle the MPE reset
      if(data & MPECTRL_MPERESET)
      {
        //????????
      }

      if(!prevGoState && (mpectl & MPECTRL_MPEGO))
      {
        if(((mpectl & MPECTRL_MPECYCLETYPE) >> 24) == 9)
        {
          pcexec = pcfetch;
          mpectl &= ~MPECTRL_MPECYCLETYPE;
          ecuSkipCounter = 0;
        }
        InvalidateICache();
        nativeCodeCache->Flush();
        invalidateRegionStart = MPE_IRAM_BASE;
        invalidateRegionEnd = MPE_IRAM_BASE+OVERLAY_SIZE-1;
      }

      return;
    }
    case 0x1:
      //excepsrc: writing 0 has no effect, writing 1 sets bit
      excepsrc |= data;
      return;
    case 0x2:
      //excepclr: clear corresponding bits in excepsrc
      excepsrc &= ~data;
      return;
    case 0x3:
      //excephalten
      excephalten = data & 0x1FFFUL;
      return;
    case 0x4:
      cc = data & 0x7FFUL;
      return;
    case 0x5:
      pcfetch = data;
      return;
    case 0x6:
      pcroute = data & 0xFFFFFFFE;
      return;
    case 0x7:
      pcexec = data;
      return;
    case 0x8:
      rz = data;
      return;
    case 0x9:
      rzi1 = data;
      return;
    case 0xA:
      rzi2 = data;
      return;
    case 0xB:
      intvec1 = data & 0xFFFFFFFEUL;
      return;
    case 0xC:
      intvec2 = data & 0xFFFFFFFEUL;
      return;
    case 0xD:
      //intsrc: writing 0 has no effect, writing 1 sets bit
      intsrc |= data;
      if(data)
      {
        Syscall_InterruptTriggered(*this);
      }
      return;
    case 0xE:
      //intclr: clears corresponding bit in intsrc
      if((data & 0x10) && (pcexec > 0x80000000) && (commctl & 0x80000000) && ((pcexec < 0x807604C0) || (pcexec > 0x807604C8)))
      {
        intsrc &= ~data;
      }
      intsrc &= ~data;
      return;
    case 0xF:
      //intctl
      intctl |= (data & 0xAAUL);
      intctl &= ~((data << 1) & 0xAAUL);
      return;
    case 0x10:
      //inten1
      inten1 = data & (~((3UL << 2) | (3UL << 10) | (3UL << 14)));
      return;
    case 0x11:
      //inten1set: sets corresponding bits of inten1
      inten1 |= data;
      return;
    case 0x12:
      //inten1clr: clears corresponding bits of inten1
      inten1 &= ~data;
      return;
    case 0x13:
      //inten2sel
      inten2sel = data & 0x1FUL;
      return;
    case 0x1E:
      //rc0: lower 16 bits only
      rc0 = data & 0xFFFFUL;
      cc |= CC_COUNTER0_ZERO;

      if(rc0 != 0)
      {
        cc &= (~CC_COUNTER0_ZERO);
      }
      return;
    case 0x1F:
      //rc1: lower 16 bits only
      rc1 = data & 0xFFFFUL;
      cc |= CC_COUNTER1_ZERO;

      if(rc1 != 0)
      {
        cc &= (~CC_COUNTER1_ZERO);
      }
      return;
    case 0x20:
      //Rx
      rx = data;
      return;
    case 0x21:
      //Ry
      ry = data;
      return;
    case 0x22:
      xyrange = data;
      return;
    case 0x23:
      //xybase: always on scalar boundary
      xybase = data & 0xFFFFFFFCUL;
      return;
    case 0x24:
      //xyctl
      xyctl = data & ~((1UL << 31) | (1UL << 27) | (1UL << 11));
      return;
    case 0x25:
      //Ru
      ru = data;
      return;
    case 0x26:
      //Rv
      rv = data;
      return;
    case 0x27:
      uvrange = data;
      return;
    case 0x28:
      //uvbase: always on scalar boundary
      uvbase = data & 0xFFFFFFFCUL;
      return;
    case 0x29:
      //uvctl
      uvctl = data & ~((1UL << 31) | (1UL << 27) | (1UL << 11));
      return;
    case 0x2A:
      //linpixctl: bits 20-23 and 28 only
      linpixctl = data & ((0x01UL << 28) | (0xFUL << 20));
      if(data & 0xFUL)
      {
        //M3DL sets the lower four bits in MPR_START, and the value seems
        //to be the pixel type, so this hack allows bits 0-3 to be mapped
        //to linpixctl bits 20-23
        linpixctl |= ((data & 0x0FUL) << 20);
      }
      return;
    case 0x2B:
      //clutbase: always on 64 byte boundary (possibly 1024 byte boundary)
      clutbase = data & 0xFFFFFFC0UL;
      return;
    case 0x2C:
      //svshift: lower 2 bits
      svshift = data & 0x03UL;
      return;
    case 0x2D:
      //acshift: lower 7 bits, sign extended
      acshift = ((int32)(data << 25)) >> 25;
      return;
    case 0x2E:
      //stack pointer: always lies on vector boundary
      sp = data & 0xFFFFFFF0UL;
      return;
    case 0x2F:
      dabreak = data;
      return;
    case 0x30:
    case 0x31:
    case 0x32:
    case 0x33:
    case 0x34:
    case 0x35:
    case 0x36:
    case 0x37:
    case 0x38:
    case 0x39:
    case 0x3A:
    case 0x3B:
    case 0x3C:
    case 0x3D:
    case 0x3E:
    case 0x3F:
    case 0x40:
    case 0x41:
    case 0x42:
    case 0x43:
    case 0x44:
    case 0x45:
    case 0x46:
    case 0x47:
    case 0x48:
    case 0x49:
    case 0x4A:
    case 0x4B:
    case 0x4C:
    case 0x4D:
    case 0x4E:
    case 0x4F:
      regs[(address >> 4) - 0x30] = data;
      return;
    case 0x50:
      //odmactl: bits 0 through 4 are read only and bits
      //7 through 31 are unused

      //force bits 0 through 4 to 0 since DMA transfers
      //will execute immediately so commands are never
      //pending nor does data transfer ever appear active
      odmactl = data & 0x60UL;
      return;
    case 0x51:
      //odmacptr: writing triggers Other BUS DMA
      odmacptr = data & 0x207FFFF0UL;
      //Call GetPointerToMemory to warn if the address is invalid
      nuonEnv.GetPointerToMemory(*this,odmacptr & 0xFFFFFFF0);

      if(odmactl & 0x60UL)
      {
        //other bus DMA is enabled so do it!
        const uint32 * const dmacmd = (uint32 *)(&dtrom[odmacptr & MPE_VALID_MEMORY_MASK]);
        uint32 dmaflags = *dmacmd;
        uint32 baseaddr = *(dmacmd + 1);
        uint32 intaddr = *(dmacmd + 2);
        SwapScalarBytes(&dmaflags);
        SwapScalarBytes(&baseaddr);
        SwapScalarBytes(&intaddr);
        //clear all bits except bits 13, 16 - 23, and 28
        dmaflags &= ((1UL << 28) | (0xFFUL << 16) | (1UL << 13));
        if(((baseaddr >= 0x20700000) && (baseaddr < 0x30000000)) || ((intaddr >= 0x20700000) && (intaddr < 0x30000000)))
        {
          //!! assert??
          //dmacmd = 0;
          //return;
        }

        DMALinear(*this,dmaflags,baseaddr,intaddr);
      }
      return;
    case 0x60:
    {
      //mdmactl;
      //clear pending and active bits since DMA transfers always execute immediately

      uint32 done_cnt_wr = (mdmactl >> 24);
      uint32 done_cnt_rd = (mdmactl >> 16) & 0xFF;

      //clear all existing mdmactl bits except for done_cnt_enable
      mdmactl &= (~(1UL << 9));

      //if done cnt disable bit is to be set
      if(data & (1UL << 8))
      {
        //if the done count enable bit is also to be set
        if(data & (1UL << 9))
        {
          //clear any errors, and reset the read and write counts to zero
          done_cnt_wr = done_cnt_rd = 0;
          mdmactl |= (1UL << 9);
        }
        else
        {
          //clear the enable bit
          mdmactl &= (~(1UL << 9));
        }
      }
      else if(data & (1UL << 9))
      {
        mdmactl |= (1UL << 9);
      }

      //if done_cnt_wr_dec is set and done count enabled
      if((data & (1UL << 11)) && (mdmactl & (1UL << 9)))
      {
        //if the write counter is not at an error state
        if(done_cnt_wr < 0xFEUL)
        {
          //decrement the write counter
          done_cnt_wr--;
        }
      }

      //if done_cnt_rd_dec is set and done count enabled
      if((data & (1UL << 10)) && (mdmactl & (1UL << 9)))
      {
        //if the read counter is not at an error state
        if(done_cnt_rd < 0xFEUL)
        {
          //decrement the read counter
          done_cnt_rd--;
        }
      }

      mdmactl |= ((done_cnt_wr << 24) | (done_cnt_rd << 16) | (data & 0x60));
      return;
    }
    case 0x61:
    {
      //mdmacptr: writing triggers Main BUS DMA
      mdmacptr = data & 0x207FFFF0UL;
      //Call GetPointerToMemory to warn if the address is invalid
      nuonEnv.GetPointerToMemory(*this,data & 0xFFFFFFF0);
do_mdmacmd: // for batch commands
      const uint32* const dmacmd = (uint32 *)(&dtrom[mdmacptr & MPE_VALID_MEMORY_MASK]);
      uint32 dmaflags = *dmacmd;
      uint32 baseaddr = *(dmacmd + 1);
      uint32 intaddr = *(dmacmd + 2);
      SwapScalarBytes(&dmaflags);
      SwapScalarBytes(&baseaddr);
      SwapScalarBytes(&intaddr);
      switch((dmaflags >> 14) & 0x03UL)
      {
        case 0:
          //linear DMA
          switch(baseaddr >> 28)
          {
            case 0x2:
            case 0x4:
            case 0x8:
            case 0xF:
              break;
            default:
              return;
          }
          DMALinear(*this,dmaflags,baseaddr,intaddr);
          if(dmaflags & (1UL << 30)) //!! was data before, but this should lead to potentially endless loops?
          {
            mdmacptr += 16;
            goto do_mdmacmd;
          }
          else
          {
            if(dmaflags & (1UL << 13))
            {
              mdmactl &= (~(0xFFUL << 16));
              //increment done_rd_cnt in mdmactl
              uint32 done_cnt_rd = (mdmactl >> 16) & 0xFF;
              done_cnt_rd++;

              if(done_cnt_rd > 0x1D)
              {
                //overflow
                done_cnt_rd = 0xFE; //!! was wr before but does not make sense
              }

              mdmactl |= ((done_cnt_rd & 0xFF) << 16);
            }
            else
            {
              mdmactl &= (~(0xFFUL << 24));
              //increment done_cnt_wr in mdmactl
              uint32 done_cnt_wr = (mdmactl >> 24);
              done_cnt_wr++;

              if(done_cnt_wr > 0x1D)
              {
                //overflow
                done_cnt_wr = 0xFE;
              }

              mdmactl |= (done_cnt_wr << 24);
            }
          }
          return;
        case 3:
        {
          //bilinear pixel DMA
          const uint32 xptr = intaddr;
          uint32 yptr = *(dmacmd + 3);
          intaddr = *(dmacmd + 4);
          SwapScalarBytes(&yptr);
          SwapScalarBytes(&intaddr);
          DMABiLinear(*this,dmaflags,baseaddr,xptr,yptr,intaddr);
          if(dmaflags & (1UL << 30))
          {
            mdmacptr += 16;
            goto do_mdmacmd;
          }
          else
          {
            if(dmaflags & (1UL << 13))
            {
              mdmactl &= (~(0xFFUL << 16));
              //increment done_rd_cnt in mdmactl
              uint32 done_cnt_rd = (mdmactl >> 16) & 0xFF;
              done_cnt_rd++;

              if(done_cnt_rd > 0x1D)
              {
                //overflow
                done_cnt_rd = 0xFE; //!! was wr before but does not make sense
              }

              mdmactl |= ((done_cnt_rd & 0xFF) << 16);
            }
            else
            {
              mdmactl &= (~(0xFFUL << 24));
              //increment done_cnt_wr in mdmactl
              uint32 done_cnt_wr = (mdmactl >> 24);
              done_cnt_wr++;

              if(done_cnt_wr > 0x1D)
              {
                //overflow
                done_cnt_wr = 0xFE;
              }

              mdmactl |= (done_cnt_wr << 24);
            }
          }
          return;
        }
        default:
          return;
      }

      return;
    }
    case 0x7E:
      //comminfo: only lower 8 bits are writable
      comminfo &= (~0xFFUL);
      comminfo |= (data & 0xFFUL);
      return;
    case 0x7F:
      //commctl:
      commctl &= ~((1UL << 30) | (0x3FFFUL));
      commctl |= (data & ((1UL << 30) | 0x30FFUL));
      return;
    case 0x80:
    {
      //commxmit: scalar write
      switch(address & 0x0F)
      {
        case 0x00:
          commxmit0 = data;
          return;
        case 0x04:
          commxmit1 = data;
          return;
        case 0x08:
          commxmit2 = data;
          return;
        case 0x0C:
          //commxmit3: trigger comm bus xmit request
          commxmit3 = data;
          commctl &= ~(COMM_XMIT_FAILED_BIT);
          commctl |= COMM_XMIT_BUFFER_FULL_BIT;
          nuonEnv.pendingCommRequests++;
          return;
      }
    }
    case 0x81:
      //commrecv: read only
      return;
    case 0xFF:
    {
      switch(address & 0x0F)
      {
        case 0x00:
          //configa: treat as syscall
          ExecuteSyscall(*this, data);
          return;
        case 0x04:
          //configb: read only
          return;
        case 0x08:
          //dcachectl
          dcachectl = data;
          return;
        case 0x0C:
          //icachectl
          icachectl = data;
          return;
      }
    }
    default:
      //no special handling: write control register contents verbatim
      *(&mpectl + (address >> 4)) = data;
      return;
  }
}

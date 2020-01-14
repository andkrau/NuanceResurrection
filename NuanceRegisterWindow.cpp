#include "Basetypes.h"
#include "mpe.h"
#include "NuanceRegisterWindow.h"
#include "NuonEnvironment.h"
#include "QComboBox.h"
#include "QLineEdit.h"
#include "QTextView.h"
#include "QCString.h"
#include "QString.h"
#include "QSpinBox.h"

extern NuonEnvironment *nuonEnv;

void NuanceRegisterWindow::OnMemoryBankSelect(int which)
{
  UpdateDebugDisplay(currMPE);
}

void NuanceRegisterWindow::OnMemoryPageChange(int which)
{
  UpdateDebugDisplay(currMPE);
}

void NuanceRegisterWindow::UpdateDebugDisplay(uint32 currMPE)
{
  QCString tempString(64);
  QCString memString(((8*1024*1024)/16) * 64);
  uint32 currAddress;
  uint32 page;
  uint8 *memPtr;

  //currOffset = (uint32)(ddlbSelectBankOffset->currentItem()) << 20;

  //registeredApp->lock();

  page = (sbMemoryPageUpperNibble->value() << 4) + sbMemoryPageLowerNibble->value();
  currOffset = (page << 16) & 0x7F0000;

  switch(ddlbSelectMemoryBank->currentItem())
  {
    case 0:
      if(nuonEnv->mpe[currMPE])
      {
        memPtr = &(nuonEnv->mpe[currMPE]->dtrom[currOffset]);
        currAddress = 0x20000000UL + currOffset;
        for(uint32 i = 0; i < 0x10000; i = i + 16)
        {
          tempString.sprintf("%8.8lX: %2.2lX %2.2lX %2.2lX %2.2lX %2.2lX %2.2lX %2.2lX %2.2lX %2.2lX %2.2lX %2.2lX %2.2lX %2.2lX %2.2lX %2.2lX %2.2lX\n",
          currAddress, memPtr[0], memPtr[1], memPtr[2], memPtr[3], memPtr[4], memPtr[5], memPtr[6], memPtr[7], 
          memPtr[8], memPtr[9], memPtr[10], memPtr[11], memPtr[12], memPtr[13], memPtr[14], memPtr[15]);
          memString += tempString;
          currAddress += 16;
          memPtr += 16;
        }
        tvMemory->setText(memString);        
      }
      break;
    case 1:
      if(nuonEnv->mainBusDRAM)
      {
        memPtr = &(nuonEnv->mainBusDRAM[currOffset]);
        currAddress = 0x40000000UL + currOffset;
        for(uint32 i = 0; i < 0x10000; i = i + 16)
        {
          tempString.sprintf("%8.8lX: %2.2lX %2.2lX %2.2lX %2.2lX %2.2lX %2.2lX %2.2lX %2.2lX %2.2lX %2.2lX %2.2lX %2.2lX %2.2lX %2.2lX %2.2lX %2.2lX\n",
          currAddress, memPtr[0], memPtr[1], memPtr[2], memPtr[3], memPtr[4], memPtr[5], memPtr[6], memPtr[7], 
          memPtr[8], memPtr[9], memPtr[10], memPtr[11], memPtr[12], memPtr[13], memPtr[14], memPtr[15]);
          memString += tempString;
          currAddress += 16;
          memPtr += 16;
        }
        tvMemory->setText(memString);
      }
      break;
    case 2:
      if(nuonEnv->systemBusDRAM)
      {
        memPtr = &(nuonEnv->systemBusDRAM[currOffset]);
        currAddress = 0x80000000UL + currOffset;
        for(uint32 i = 0; i < 0x10000; i = i + 16)
        {
          tempString.sprintf("%8.8lX: %2.2lX %2.2lX %2.2lX %2.2lX %2.2lX %2.2lX %2.2lX %2.2lX %2.2lX %2.2lX %2.2lX %2.2lX %2.2lX %2.2lX %2.2lX %2.2lX\n",
          currAddress, memPtr[0], memPtr[1], memPtr[2], memPtr[3], memPtr[4], memPtr[5], memPtr[6], memPtr[7], 
          memPtr[8], memPtr[9], memPtr[10], memPtr[11], memPtr[12], memPtr[13], memPtr[14], memPtr[15]);
          memString += tempString;
          currAddress += 16;
          memPtr += 16;
        }
        tvMemory->setText(memString);
      }
      break;
    default:
      break;
  }
  tbR0->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->regs[0]));
  tbR1->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->regs[1]));
  tbR2->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->regs[2]));
  tbR3->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->regs[3]));
  tbR4->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->regs[4]));
  tbR5->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->regs[5]));
  tbR6->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->regs[6]));
  tbR7->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->regs[7]));
  tbR8->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->regs[8]));
  tbR9->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->regs[9]));
  tbR10->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->regs[10]));
  tbR11->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->regs[11]));
  tbR12->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->regs[12]));
  tbR13->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->regs[13]));
  tbR14->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->regs[14]));
  tbR15->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->regs[15]));
  tbR16->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->regs[16]));
  tbR17->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->regs[17]));
  tbR18->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->regs[18]));
  tbR19->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->regs[19]));
  tbR20->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->regs[20]));
  tbR21->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->regs[21]));
  tbR22->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->regs[22]));
  tbR23->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->regs[23]));
  tbR24->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->regs[24]));
  tbR25->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->regs[25]));
  tbR26->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->regs[26]));
  tbR27->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->regs[27]));
  tbR28->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->regs[28]));
  tbR29->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->regs[29]));
  tbR30->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->regs[30]));
  tbR31->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->regs[31]));

  tbMpectl->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->mpectl));
  tbExcepsrc->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->excepsrc));
  tbExcephalten->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->excephalten));
  tbCc->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->cc));
  tbPcfetch->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->pcfetch));
  tbPcroute->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->pcroute));
  tbPcexec->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->pcexec));
  tbRz->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->rz));
  tbRzi1->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->rzi1));
  tbRzi2->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->rzi2));
  tbIntvec1->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->intvec1));
  tbIntvec2->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->intvec2));
  tbIntsrc->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->intsrc));
  tbIntctl->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->intctl));
  tbInten1->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->inten1));
  tbInten2sel->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->inten2sel));

  tbRc0->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->rc0));
  tbRc1->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->rc1));
  tbRx->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->rx));
  tbRy->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->ry));
  tbXyctl->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->xyctl));
  tbXyrange->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->xyrange));
  tbXybase->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->xybase));
  tbRu->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->ru));
  tbRv->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->rv));
  tbUvctl->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->uvctl));
  tbUvrange->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->uvrange));
  tbUvbase->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->uvbase));
  tbLinpixctl->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->linpixctl));
  tbClutbase->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->clutbase));
  tbSvshift->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->svshift));
  tbAcshift->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->acshift));


  tbSp->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->sp));
  tbDabreak->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->dabreak));
  tbOdmactl->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->odmactl));
  tbOdmacptr->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->odmacptr));
  tbMdmactl->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->mdmactl));
  tbMdmacptr->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->mdmacptr));
  tbCommctl->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->commctl));
  tbComminfo->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->comminfo));
  tbCommxmit0->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->commxmit0));
  tbCommxmit1->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->commxmit1));
  tbCommxmit2->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->commxmit2));
  tbCommxmit3->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->commxmit3));
  tbCommrecv0->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->commrecv0));
  tbCommrecv1->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->commrecv1));
  tbCommrecv2->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->commrecv2));
  tbCommrecv3->setText(tempString.sprintf("%8.8X",nuonEnv->mpe[currMPE]->commrecv3));
  //registeredApp->unlock();
}
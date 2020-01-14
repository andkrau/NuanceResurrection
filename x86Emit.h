#ifndef X86EMIT_H
#define X86EMIT_H

#define X86_CC_O (0x0)
#define X86_CC_NO (0x1)
#define X86_CC_B (0x2)
#define X86_CC_NB (0x3)
#define X86_CC_Z (0x4)
#define X86_CC_NZ (0x5)
#define X86_CC_BE (0x6)
#define X86_CC_NBE (0x7)
#define X86_CC_S (0x8)
#define X86_CC_NS (0x9)
#define X86_CC_P (0xA)
#define X86_CC_NP (0xB)
#define X86_CC_L (0xC)
#define X86_CC_NL (0xD)
#define X86_CC_LE (0xE)
#define X86_CC_NLE (0xF)

#define X86_CC_E X86_CC_Z
#define X86_CC_NE X86_CC_NZ

enum x86Reg
{
  x86Reg_al = 0,
  x86Reg_cl,
  x86Reg_dl,
  x86Reg_bl,
  x86Reg_ah,
  x86Reg_ch,
  x86Reg_dh,
  x86Reg_bh,

  x86Reg_ax,
  x86Reg_cx,
  x86Reg_dx,
  x86Reg_bx,
  x86Reg_sp,
  x86Reg_bp,
  x86Reg_si,
  x86Reg_di,

  x86Reg_eax,
  x86Reg_ecx,
  x86Reg_edx,
  x86Reg_ebx,
  x86Reg_esp,
  x86Reg_ebp,
  x86Reg_esi,
  x86Reg_edi,

  x86Reg_es,
  x86Reg_cs,
  x86Reg_ss,
  x86Reg_ds,
  x86Reg_fs,
  x86Reg_gs,
  x86Reg_invalid1,
  x86Reg_invalid2,

  x86Reg_mm0,
  x86Reg_mm1,
  x86Reg_mm2,
  x86Reg_mm3,
  x86Reg_mm4,
  x86Reg_mm5,
  x86Reg_mm6,
  x86Reg_mm7,

  x86Reg_xmm0,
  x86Reg_xmm1,
  x86Reg_xmm2,
  x86Reg_xmm3,
  x86Reg_xmm4,
  x86Reg_xmm5,
  x86Reg_xmm6,
  x86Reg_xmm7,

  x86Reg_cr0,
  x86Reg_cr1,
  x86Reg_cr2,
  x86Reg_cr3,
  x86Reg_cr4,
  x86Reg_cr5,
  x86Reg_cr6,
  x86Reg_cr7,

  x86Reg_dr0,
  x86Reg_dr1,
  x86Reg_dr2,
  x86Reg_dr3,
  x86Reg_dr4,
  x86Reg_dr5,
  x86Reg_dr6,
  x86Reg_dr7,
};

enum x86ModReg
{
  x86ModReg_eax = 0,
  x86ModReg_ecx,
  x86ModReg_edx,
  x86ModReg_ebx,
  x86ModReg_esp,
  x86ModReg_ebp,
  x86ModReg_esi,
  x86ModReg_edi,  
};

enum x86ModType
{
  x86ModType_mem = 0,
  x86ModType_mem_disp8 = 1,
  x86ModType_mem_disp32 = 2,
  x86ModType_reg = 3,
};

enum x86MemPtr
{
  x86MemPtr_byte = 0,
  x86MemPtr_word = 1,
  x86MemPtr_dword = 2,
  x86MemPtr_qword = 3,
};

#define x86ModReg_al x86ModReg_eax
#define x86ModReg_cl x86ModReg_ecx
#define x86ModReg_dl x86ModReg_edx
#define x86ModReg_bl x86ModReg_ebx
#define x86ModReg_ah x86ModReg_esp
#define x86ModReg_ch x86ModReg_ebp
#define x86ModReg_dh x86ModReg_esi
#define x86ModReg_bh x86ModReg_edi

#define x86ModReg_ax x86ModReg_eax
#define x86ModReg_cx x86ModReg_ecx
#define x86ModReg_dx x86ModReg_edx
#define x86ModReg_bx x86ModReg_ebx
#define x86ModReg_sp x86ModReg_esp
#define x86ModReg_bp x86ModReg_ebp
#define x86ModReg_si x86ModReg_esi
#define x86ModReg_di x86ModReg_edi

#define x86ModReg_mm0 x86ModReg_eax
#define x86ModReg_mm1 x86ModReg_ecx
#define x86ModReg_mm2 x86ModReg_edx
#define x86ModReg_mm3 x86ModReg_ebx
#define x86ModReg_mm4 x86ModReg_esp
#define x86ModReg_mm5 x86ModReg_ebp
#define x86ModReg_mm6 x86ModReg_esi
#define x86ModReg_mm7 x86ModReg_edi

#define x86ModReg_xmm0 x86ModReg_eax
#define x86ModReg_xmm1 x86ModReg_ecx
#define x86ModReg_xmm2 x86ModReg_edx
#define x86ModReg_xmm3 x86ModReg_ebx
#define x86ModReg_xmm4 x86ModReg_esp
#define x86ModReg_xmm5 x86ModReg_ebp
#define x86ModReg_xmm6 x86ModReg_esi
#define x86ModReg_xmm7 x86ModReg_edi

enum x86BaseReg
{
  x86BaseReg_eax = 0,
  x86BaseReg_ecx,
  x86BaseReg_edx,
  x86BaseReg_ebx,
  x86BaseReg_esp,
  x86BaseReg_ebp,
  x86BaseReg_esi,
  x86BaseReg_edi,
};

#define x86BaseReg_sib x86BaseReg_esp
#define x86BaseReg_sdword x86BaseReg_ebp

enum x86IndexReg
{
  x86IndexReg_eax = 0,
  x86IndexReg_ecx,
  x86IndexReg_edx,
  x86IndexReg_ebx,
  x86IndexReg_none,
  x86IndexReg_ebp,
  x86IndexReg_esi,
  x86IndexReg_edi,
};

enum x86ScaleVal
{
  x86Scale_1 = 0,
  x86Scale_2,
  x86Scale_4,
  x86Scale_8,
};

void X86Emit_ModRegRM(x86ModType, x86ModReg modReg, uint32 baseReg, x86IndexReg = x86IndexReg_none, x86ScaleVal = x86Scale_1, int32 disp = 0);
uint8 *GetEmitPointer();
void SetEmitPointer(uint8 *ptr);
void AlignEmitPointer(uint8 boundary);

void X86Emit_ADDRR(x86Reg regDest, x86Reg regSrc);
void X86Emit_ADDRM(x86Reg regSrc, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_ADDMR(x86Reg regDest, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_ADDIR(int32 imm, x86Reg regDest);
void X86Emit_ADDIM(int32 imm, x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_PUSHES();
void X86Emit_POPES();
void X86Emit_ORRR(x86Reg regDest, x86Reg regSrc);
void X86Emit_ORRM(x86Reg regSrc, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_ORMR(x86Reg regDest, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_ORIR(int32 imm, x86Reg regDest);
void X86Emit_ORIM(int32 imm, x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_PUSHCS();
void X86Emit_ADCRR(x86Reg regDest, x86Reg regSrc);
void X86Emit_ADCRM(x86Reg regSrc, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_ADCMR(x86Reg regDest, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_ADCIR(int32 imm, x86Reg regDest);
void X86Emit_ADCIM(int32 imm, x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_PUSHSS();
void X86Emit_POPSS();
void X86Emit_SBBRR(x86Reg regDest, x86Reg regSrc);
void X86Emit_SBBRM(x86Reg regSrc, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_SBBMR(x86Reg regDest, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_SBBIR(int32 imm, x86Reg regDest);
void X86Emit_SBBIM(int32 imm, x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_PUSHDS();
void X86Emit_POPDS();
void X86Emit_ANDRR(x86Reg regDest, x86Reg regSrc);
void X86Emit_ANDRM(x86Reg regSrc, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_ANDMR(x86Reg regDest, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_ANDIR(int32 imm, x86Reg regDest);
void X86Emit_ANDIM(int32 imm, x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_ES();
void X86Emit_DAA();
void X86Emit_SUBRR(x86Reg regDest, x86Reg regSrc);
void X86Emit_SUBRM(x86Reg regSrc, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_SUBMR(x86Reg regDest, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_SUBIR(int32 imm, x86Reg regDest);
void X86Emit_SUBIM(int32 imm, x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_CS();
void X86Emit_DAS();
void X86Emit_XORRR(x86Reg regDest, x86Reg regSrc);
void X86Emit_XORRM(x86Reg regSrc, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_XORMR(x86Reg regDest, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_XORIR(int32 imm, x86Reg regDest);
void X86Emit_XORIM(int32 imm, x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_SS();
void X86Emit_AAA();
void X86Emit_CMPRR(x86Reg regDest, x86Reg regSrc);
void X86Emit_CMPRM(x86Reg regSrc, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_CMPMR(x86Reg regDest, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_CMPIR(int32 imm, x86Reg regDest);
void X86Emit_CMPIM(int32 imm, x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_DS();
void X86Emit_AAS();
void X86Emit_INCR(x86Reg reg);
void X86Emit_DECR(x86Reg reg);
void X86Emit_PUSHR(x86Reg reg);
void X86Emit_POPR(x86Reg reg);
void X86Emit_PUSHAW();
void X86Emit_PUSHAD();
#define X86Emit_PUSHA X86Emit_PUSHAD
void X86Emit_POPAW();
void X86Emit_POPAD();
#define X86Emit_POPA X86Emit_POPAD
void X86Emit_FS();
void X86Emit_GS();
void X86Emit_OPSIZE();
void X86Emit_ADSIZE();
void X86Emit_PUSHID(int32 imm);
void X86Emit_PUSHIW(int16 imm);
void X86Emit_IMULIRR(x86Reg regDest, int32 imm, x86Reg regSrc);
void X87Emit_IMULIMR(x86Reg regDest, int32 imm, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_IMULRR(x86Reg regSrc);
void X86Emit_IMULMR(x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_IMULRRR(x86Reg regDest, x86Reg regSrc);
void X86Emit_IMULMRR(x86Reg regDest, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_PUSHIB(int8 imm);
void X86Emit_INSB();
void X86Emit_INSW();
void X86Emit_INSD();
void X86Emit_OUTSB();
void X86Emit_OUTSW();
void X86Emit_OUTSD();
void X86Emit_JCC(uint8 *pTarget, int8 conditionCode);
void X86Emit_JCC_Label(int8 conditionCode, uint32 labelIndex);
void X86Emit_JO(uint8 *pTarget);
void X86Emit_JNO(uint8 *pTarget);
void X86Emit_JB(uint8 *pTarget);
void X86Emit_JNB(uint8 *pTarget);
void X86Emit_JZ(uint8 *pTarget);
void X86Emit_JNZ(uint8 *pTarget);
void X86Emit_JBE(uint8 *pTarget);
void X86Emit_JNBE(uint8 *pTarget);
void X86Emit_JS(uint8 *pTarget);
void X86Emit_JNS(uint8 *pTarget);
void X86Emit_JP(uint8 *pTarget);
void X86Emit_JNP(uint8 *pTarget);
void X86Emit_JL(uint8 *pTarget);
void X86Emit_JNL(uint8 *pTarget);
void X86Emit_JLE(uint8 *pTarget);
void X86Emit_JNLE(uint8 *pTarget);
void X86Emit_TESTRR(x86Reg regDest, x86Reg regSrc);
void X86Emit_TESTRM(x86Reg regSrc, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_TESTIR(uint32 imm, x86Reg regSrc);
void X86Emit_TESTIM(uint32 imm, x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_XCHGRM(x86Reg regSrc, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_MOVRR(x86Reg regDest, x86Reg regSrc);
void X86Emit_LEA(x86Reg regDest, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_POPM(x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_NOP();
void X86Emit_XCHGRR(x86Reg reg1, x86Reg reg2);
void X86Emit_CBW();
void X86Emit_CWDE();
void X86Emit_CWD();
void X86Emit_CDQ();
void X86Emit_CALLI(uint32 offset, uint16 seg);
void X86Emit_JMPI(uint8 *target, uint16 seg);
void X86Emit_JMPI_Label(uint32 labelIndex);
void X86Emit_WAIT();
void X86Emit_PUSHFW();
void X86Emit_PUSHFD();
#define X86Emit_PUSHF X86Emit_PUSHFD
void X86Emit_POPFW();
void X86Emit_POPFD();
#define X86Emit_POPF X86Emit_POPFD
void X86Emit_SAHF();
void X86Emit_LAHF();
void X86Emit_MOVMR(x86Reg regDest, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_MOVRM(x86Reg regSrc, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_MOVIR(int32 imm, x86Reg regDest);
void X86Emit_MOVSB();
void X86Emit_MOVSW();
void X86Emit_MOVSD();
void X86Emit_CMPSB();
void X86Emit_CMPSW();
void X86Emit_CMPSD();
void X86Emit_STOSB();
void X86Emit_STOSW();
void X86Emit_STOSD();
void X86Emit_LODSB();
void X86Emit_LODSW();
void X86Emit_LODSD();
void X86Emit_SCASB();
void X86Emit_SCASW();
void X86Emit_SCASD();
void X86Emit_ROLIR(x86Reg regDest, uint8 shiftCount);
void X86Emit_RORIR(x86Reg regDest, uint8 shiftCount);
void X86Emit_RCLIR(x86Reg regDest, uint8 shiftCount);
void X86Emit_RCRIR(x86Reg regDest, uint8 shiftCount);
void X86Emit_SHLIR(x86Reg regDest, uint8 shiftCount);
void X86Emit_SHLDIRR(x86Reg regDest, x86Reg regSrc, uint8 shiftCount);
void X86Emit_SHLDRRR(x86Reg regDest, x86Reg regSrc);
void X86Emit_SHLDIRM(x86Reg regSrc, x86MemPtr ptrType, uint8 shiftCount, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_SHLDRRM(x86Reg regSrc, x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_SHRDIRR(x86Reg regDest, x86Reg regSrc, uint8 shiftCount);
void X86Emit_SHRDRRR(x86Reg regDest, x86Reg regSrc);
void X86Emit_SHRDIRM(x86Reg regSrc, x86MemPtr ptrType, uint8 shiftCount, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_SHRDRRM(x86Reg regSrc, x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_SHRIR(x86Reg regDest, uint8 shiftCount);
void X86Emit_SALIR(x86Reg regDest, uint8 shiftCount);
void X86Emit_SARIR(x86Reg regDest, uint8 shiftCount);
void X86Emit_ROLIM(x86MemPtr ptrType, uint8 shiftCount, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_RORIM(x86MemPtr ptrType, uint8 shiftCount, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_RCLIM(x86MemPtr ptrType, uint8 shiftCount, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_RCRIM(x86MemPtr ptrType, uint8 shiftCount, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_SHLIM(x86MemPtr ptrType, uint8 shiftCount, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_SHRIM(x86MemPtr ptrType, uint8 shiftCount, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_SALIM(x86MemPtr ptrType, uint8 shiftCount, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_SARIM(x86MemPtr ptrType, uint8 shiftCount, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);

void X86Emit_ROLRM(x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_RORRM(x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_RCLRM(x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_RCRRM(x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_SHLRM(x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_SHRRM(x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_SALRM(x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_SARRM(x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);



void X86Emit_RETN(uint16 iw);
#define X86Emit_RET X86Emit_RETN
void X86Emit_MOVIM(int32 imm, x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_LEAVE();
void X86Emit_RETF(uint16 iw);
void X86Emit_INT3();
void X86Emit_INT(int8 vector);
void X86Emit_INTO();
void X86Emit_IRET();
void X86Emit_ROLRR(x86Reg regDest);
void X86Emit_RORRR(x86Reg regDest);
void X86Emit_RCLRR(x86Reg regDest);
void X86Emit_RCRRR(x86Reg regDest);
void X86Emit_SHLRR(x86Reg regDest);
void X86Emit_SHRRR(x86Reg regDest);
void X86Emit_SALRR(x86Reg regDest);
void X86Emit_SARRR(x86Reg regDest);
void X86Emit_AAM(uint8 divisor);
void X86Emit_AAD(uint8 divisor);
void X86Emit_XLAT();
void X86Emit_ESC0();
void X86Emit_ESC1();
void X86Emit_ESC2();
void X86Emit_ESC3();
void X86Emit_ESC4();
void X86Emit_ESC5();
void X86Emit_ESC6();
void X86Emit_ESC7();
void X86Emit_LOOPNE(uint8 *pTarget);
void X86Emit_LOOPNE_Label(uint32 labelIndex);
#define X86Emit_LOOPNZ X86Emit_LOOPNE
#define X86Emit_LOOPNZ_Label X86Emit_LOOPNE_Label
void X86Emit_LOOPE(uint8 *pTarget);
void X86Emit_LOOPE_Label(uint32 labelIndex);
#define X86Emit_LOOPZ X86Emit_LOOPE
#define X86Emit_LOOPZ_Label X86Emit_LOOPZ_Label
void X86Emit_LOOP(uint8 *pTarget);
void X86Emit_LOOP_Label(uint32 labelIndex);
void X86Emit_JCXZ(uint8 *pTarget);
void X86Emit_JCXZ_Label(uint32 labelIndex);
void X86Emit_JECXZ(uint8 *pTarget);
void X86Emit_JECXZ_Label(uint32 labelIndex);
void X86Emit_INI(x86Reg regDest, uint8 port);
void X86Emit_OUTI(x86Reg regDest, uint8 data);
void X86Emit_INR(x86Reg regDest);
void X86Emit_OUTR(x86Reg regDest);
void X86Emit_LOCK();
void X86Emit_INT1();
void X86Emit_REPNE();
void X86Emit_REPE();
#define X86Emit_REP X86Emit_REPE
void X86Emit_HLT();
void X86Emit_CMC();
void X86Emit_NOTR(x86Reg regDest);
void X86Emit_NOTM(x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_NEGR(x86Reg regDest);
void X86Emit_NEGM(x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_CLC();
void X86Emit_STC();
void X86Emit_CLI();
void X86Emit_STI();
void X86Emit_CLD();
void X86Emit_STD();
void X86Emit_INCM(x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_DECM(x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_CALLNM(uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
#define X86Emit_CALLM X86Emit_CALLNM
void X86Emit_CALLR(x86Reg regSrc);
void X86Emit_CALLFM(uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_JMPR(x86Reg regSrc);
void X86Emit_JMPNM(uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
#define X86Emit_JMPM X86Emit_JMPNM
void X86Emit_JMPFM(uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_PUSHM(x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_INVD(x86Reg reg);
void X86Emit_WBINVD(x86Reg reg);
void X86Emit_UD1(x86Reg reg);
void X86Emit_BSWAP(x86Reg reg);
void X86Emit_SETOR(x86Reg reg);
void X86Emit_SETOM(uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_SETNOR(x86Reg reg);
void X86Emit_SETNOM(uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_SETBR(x86Reg reg);
void X86Emit_SETBM(uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_SETNBR(x86Reg reg);
void X86Emit_SETNBM(uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_SETZR(x86Reg reg);
void X86Emit_SETZM(uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_SETNZR(x86Reg reg);
void X86Emit_SETNZM(uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_SETBER(x86Reg reg);
void X86Emit_SETBEM(uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_SETNBER(x86Reg reg);
void X86Emit_SETNBEM(uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_SETSR(x86Reg reg);
void X86Emit_SETSM(uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_SETNSR(x86Reg reg);
void X86Emit_SETNSM(uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_SETPR(x86Reg reg);
void X86Emit_SETPM(uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_SETNPR(x86Reg reg);
void X86Emit_SETNPM(uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_SETLR(x86Reg reg);
void X86Emit_SETLM(uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_SETNLR(x86Reg reg);
void X86Emit_SETNLM(uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_SETLER(x86Reg reg);
void X86Emit_SETLEM(uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_SETNLER(x86Reg reg);
void X86Emit_SETNLEM(uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_CMOVORR(x86Reg regDest, uint32 regSrc);
void X86Emit_CMOVOMR(x86Reg regDest, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_CMOVNORR(x86Reg regDest, uint32 regSrc);
void X86Emit_CMOVNOMR(x86Reg regDest, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_CMOVBRR(x86Reg regDest, uint32 regSrc);
void X86Emit_CMOVBMR(x86Reg regDest, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_CMOVNBRR(x86Reg regDest, uint32 regSrc);
void X86Emit_CMOVNBMR(x86Reg regDest, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_CMOVZRR(x86Reg regDest, uint32 regSrc);
void X86Emit_CMOVZMR(x86Reg regDest, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_CMOVNZRR(x86Reg regDest, uint32 regSrc);
void X86Emit_CMOVNZMR(x86Reg regDest, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_CMOVBERR(x86Reg regDest, uint32 regSrc);
void X86Emit_CMOVBEMR(x86Reg regDest, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_CMOVNBERR(x86Reg regDest, uint32 regSrc);
void X86Emit_CMOVNBEMR(x86Reg regDest, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_CMOVSRR(x86Reg regDest, uint32 regSrc);
void X86Emit_CMOVSMR(x86Reg regDest, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_CMOVNSRR(x86Reg regDest, uint32 regSrc);
void X86Emit_CMOVNSMR(x86Reg regDest, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_CMOVPRR(x86Reg regDest, uint32 regSrc);
void X86Emit_CMOVPMR(x86Reg regDest, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_CMOVNPRR(x86Reg regDest, uint32 regSrc);
void X86Emit_CMOVNPMR(x86Reg regDest, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_CMOVLRR(x86Reg regDest, uint32 regSrc);
void X86Emit_CMOVLMR(x86Reg regDest, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_CMOVNLRR(x86Reg regDest, uint32 regSrc);
void X86Emit_CMOVNLMR(x86Reg regDest, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_CMOVLERR(x86Reg regDest, uint32 regSrc);
void X86Emit_CMOVLEMR(x86Reg regDest, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_CMOVNLERR(x86Reg regDest, uint32 regSrc);
void X86Emit_CMOVNLEMR(x86Reg regDest, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);

void X86Emit_MOVSXRR(x86Reg regDest, uint32 regSrc);
void X86Emit_MOVZXRR(x86Reg regDest, uint32 regSrc);
void X86Emit_MOVSXMR(x86Reg regDest, x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_MOVZXMR(x86Reg regDest, x86MemPtr ptrType, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);

void X86Emit_MOVQRR(x86Reg regDest, x86Reg regSrc);
void X86Emit_MOVQRM(x86Reg regSrc, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);
void X86Emit_MOVQMR(x86Reg regDest, uint32 base, x86IndexReg index = x86IndexReg_none, x86ScaleVal scale = x86Scale_1, int32 disp = 0);

#endif
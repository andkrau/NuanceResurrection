#ifndef X86EMITTYPES_H
#define X86EMITTYPES_H

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

#endif

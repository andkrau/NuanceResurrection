#include "basetypes.h"
#include <cstdio>
#include <cstring>
#include "mpe.h"
#include "InstructionCache.h"
#include "NuonMemoryMap.h"

const char *GetControlRegister(uint32 which)
{
  switch(which)
  {
    case 0x00:
      return "mpectl";
    case 0x10:
      return "excepsrc";
    case 0x20:
      return "excepclr";
    case 0x30:
      return "excephalten";
    case 0x40:
      return "cc";
    case 0x50:
      return "pcfetch";
    case 0x60:
      return "pcroute";
    case 0x70:
      return "pcexec";
    case 0x80:
      return "rz";
    case 0x90:
      return "rzi1";
    case 0xA0:
      return "rzi2";
    case 0xB0:
      return "intvec1";
    case 0xC0:
      return "intvec2";
    case 0xD0:
      return "intsrc";
    case 0xE0:
      return "intclr";
    case 0xF0:
      return "intctl";
    case 0x100:
      return "inten1";
    case 0x110:
      return "inten1set";
    case 0x120:
      return "inten1clr";
    case 0x130:
      return "inten2sel";
    case 0x1E0:
      return "rc0";
    case 0x1F0:
      return "rcl";
    case 0x200:
      return "rx";
    case 0x210:
      return "ry";
    case 0x220:
      return "xyrange";
    case 0x230:
      return "xybase";
    case 0x240:
      return "xyctl";
    case 0x250:
      return "ru";
    case 0x260:
      return "rv";
    case 0x270:
      return "uvrange";
    case 0x280:
      return "uvbase";
    case 0x290:
      return "uvctl";
    case 0x2A0:
      return "linpixctl";
    case 0x2B0:
      return "clutbase";
    case 0x2C0:
      return "svshift";
    case 0x2D0:
      return "acshift";
    case 0x2E0:
      return "sp";
    case 0x2F0:
      return "dabreak";
    case 0x300:
      return "r0";
    case 0x310:
      return "r1";
    case 0x320:
      return "r2";
    case 0x330:
      return "r3";
    case 0x340:
      return "r4";
    case 0x350:
      return "r5";
    case 0x360:
      return "r6";
    case 0x370:
      return "r7";
    case 0x380:
      return "r8";
    case 0x390:
      return "r9";
    case 0x3A0:
      return "r10";
    case 0x3B0:
      return "r11";
    case 0x3C0:
      return "r12";
    case 0x3D0:
      return "r13";
    case 0x3E0:
      return "r14";
    case 0x3F0:
      return "r15";
    case 0x400:
      return "r16";
    case 0x410:
      return "r17";
    case 0x420:
      return "r18";
    case 0x430:
      return "r19";
    case 0x440:
      return "r20";
    case 0x450:
      return "r21";
    case 0x460:
      return "r22";
    case 0x470:
      return "r23";
    case 0x480:
      return "r24";
    case 0x490:
      return "r25";
    case 0x4A0:
      return "r26";
    case 0x4B0:
      return "r27";
    case 0x4C0:
      return "r28";
    case 0x4D0:
      return "r29";
    case 0x4E0:
      return "r30";
    case 0x4F0:
      return "r31";
    case 0x500:
      return "odmactl";
    case 0x510:
      return "odmacptr";
    case 0x600:
      return "mdmactl";
    case 0x610:
      return "mdmacptr";
    case 0x7E0:
      return "comminfo";
    case 0x7F0:
      return "commctl";
    case 0x800:
      return "commxmit0";
    case 0x804:
      return "commxmit1";
    case 0x808:
      return "commxmit2";
    case 0x80C:
      return "commxmit3";
    case 0x810:
      return "commrecv0";
    case 0x814:
      return "commrecv1";
    case 0x818:
      return "commrecv2";
    case 0x81C:
      return "commrecv3";
    case 0xFF0:
      return "configa";
    case 0xFF4:
      return "configb";
    case 0xFF8:
      return "dcachectl";
    case 0xFFC:
      return "icachectl";
    case 0x1100:
      return "vdmactla";
    case 0x1110:
      return "vdmactlb";
    case 0x1120:
      return "vdmaptra";
    case 0x1130:
      return "vdmaptrb";
    default:
      if((which >= 0x1200) && (which <= 0x1320))
      {
        return "vld-bdu???";
      }
      else
      {
        return "???";
      }
  }
}

uint32 Print_Mirror(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "mirror r%lu, r%lu%s",nuance.fields[FIELD_MEM_FROM],nuance.fields[FIELD_MEM_TO], bNewline ? "\n" : "" );
  return length;
}
uint32 Print_MV_SImmediate(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "mv_s #$%8.8lX, r%lu%s",nuance.fields[FIELD_MEM_FROM],nuance.fields[FIELD_MEM_TO], bNewline ? "\n" : "" );
  return length;
}
uint32 Print_MV_SScalar(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "mv_s r%lu, r%lu%s",nuance.fields[FIELD_MEM_FROM],nuance.fields[FIELD_MEM_TO], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_MV_V(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "mv_v v%lu, v%lu%s",nuance.fields[FIELD_MEM_FROM] >> 2,nuance.fields[FIELD_MEM_TO] >> 2, bNewline ? "\n" : "" );
  return length;
}

uint32 Print_PopVector(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "pop v%lu%s",nuance.fields[FIELD_MEM_TO] >> 2, bNewline ? "\n" : "" );
  return length;
}

uint32 Print_PopVectorRz(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "pop v%lu, rz%s",nuance.fields[FIELD_MEM_TO] >> 2, bNewline ? "\n" : "" );
  return length;
}

uint32 Print_PopScalarRzi1(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "pop r%lu, cc, rz, rzi1%s",nuance.fields[FIELD_MEM_TO], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_PopScalarRzi2(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "pop r%lu, cc, rz, rzi2%s",nuance.fields[FIELD_MEM_TO], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_PushVector(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "push v%lu%s",nuance.fields[FIELD_MEM_FROM] >> 2, bNewline ? "\n" : "" );
  return length;
}

uint32 Print_PushVectorRz(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "push v%lu, rz%s",nuance.fields[FIELD_MEM_FROM] >> 2, bNewline ? "\n" : "" );
  return length;
}

uint32 Print_PushScalarRzi1(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "push r%lu, cc, rz, rzi1%s",nuance.fields[FIELD_MEM_FROM], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_PushScalarRzi2(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "push r%lu, cc, rz, rzi2%s",nuance.fields[FIELD_MEM_FROM], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_LoadScalarControlRegisterAbsolute(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "ld_s %s, r%lu%s",GetControlRegister(nuance.fields[FIELD_MEM_FROM] - MPE_CTRL_BASE),nuance.fields[FIELD_MEM_TO], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_LoadByteAbsolute(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "ld_b $%8.8lX, r%lu%s",nuance.fields[FIELD_MEM_FROM],nuance.fields[FIELD_MEM_TO], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_LoadWordAbsolute(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "ld_w $%8.8lX, r%lu%s",nuance.fields[FIELD_MEM_FROM],nuance.fields[FIELD_MEM_TO], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_LoadScalarAbsolute(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "ld_s $%8.8lX, r%lu%s",nuance.fields[FIELD_MEM_FROM],nuance.fields[FIELD_MEM_TO], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_LoadScalarLinear(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "ld_s (r%lu), r%lu%s",nuance.fields[FIELD_MEM_FROM],nuance.fields[FIELD_MEM_TO], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_LoadVectorAbsolute(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "ld_v $%8.8lX, v%lu%s",nuance.fields[FIELD_MEM_FROM],nuance.fields[FIELD_MEM_TO] >> 2, bNewline ? "\n" : "" );
  return length;
}

uint32 Print_LoadVectorControlRegisterAbsolute(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "ld_v %s, v%lu%s",GetControlRegister(nuance.fields[FIELD_MEM_FROM] - MPE_CTRL_BASE),nuance.fields[FIELD_MEM_TO] >> 2, bNewline ? "\n" : "" );
  return length;
}

uint32 Print_LoadPixelAbsolute(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "ld_p $%8.8lX, v%lu%s",nuance.fields[FIELD_MEM_FROM],nuance.fields[FIELD_MEM_TO] >> 2, bNewline ? "\n" : "" );
  return length;
}

uint32 Print_LoadPixelZAbsolute(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "ld_pz $%8.8lX, v%lu%s",nuance.fields[FIELD_MEM_FROM],nuance.fields[FIELD_MEM_TO] >> 2, bNewline ? "\n" : "" );
  return length;
}

uint32 Print_LoadByteLinear(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "ld_b (r%lu), r%lu%s",nuance.fields[FIELD_MEM_FROM],nuance.fields[FIELD_MEM_TO], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_LoadByteBilinearUV(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "ld_b (uv), r%lu%s",nuance.fields[FIELD_MEM_TO], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_LoadByteBilinearXY(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "ld_b (xy), r%lu%s",nuance.fields[FIELD_MEM_TO], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_LoadWordLinear(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "ld_w (r%lu), r%lu%s",nuance.fields[FIELD_MEM_FROM],nuance.fields[FIELD_MEM_TO], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_LoadWordBilinearUV(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "ld_w (uv), r%lu%s",nuance.fields[FIELD_MEM_TO], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_LoadWordBilinearXY(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "ld_w (xy), r%lu%s",nuance.fields[FIELD_MEM_TO], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_LoadScalarBilinearUV(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "ld_s (uv), r%lu%s",nuance.fields[FIELD_MEM_TO], bNewline ? "\n" : "" );
  return length;
}
uint32 Print_LoadScalarBilinearXY(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "ld_s (xy), r%lu%s",nuance.fields[FIELD_MEM_TO], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_LoadShortVectorAbsolute(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "ld_sv $%8.8lX, v%lu%s",nuance.fields[FIELD_MEM_FROM],nuance.fields[FIELD_MEM_TO] >> 2, bNewline ? "\n" : "" );
  return length;
}

uint32 Print_LoadShortVectorLinear(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "ld_sv (r%lu), v%lu%s",nuance.fields[FIELD_MEM_FROM],nuance.fields[FIELD_MEM_TO] >> 2, bNewline ? "\n" : "" );
  return length;
}

uint32 Print_LoadShortVectorBilinearUV(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "ld_sv (uv), v%lu%s",nuance.fields[FIELD_MEM_TO] >> 2, bNewline ? "\n" : "" );
  return length;
}

uint32 Print_LoadShortVectorBilinearXY(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "ld_sv (xy), v%lu%s",nuance.fields[FIELD_MEM_TO] >> 2, bNewline ? "\n" : "" );
  return length;
}

uint32 Print_LoadVectorLinear(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "ld_v (r%lu), v%lu%s",nuance.fields[FIELD_MEM_FROM],nuance.fields[FIELD_MEM_TO] >> 2, bNewline ? "\n" : "" );
  return length;
}

uint32 Print_LoadVectorBilinearUV(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "ld_v (uv), v%lu%s",nuance.fields[FIELD_MEM_TO] >> 2, bNewline ? "\n" : "" );
  return length;
}

uint32 Print_LoadVectorBilinearXY(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "ld_v (xy), v%lu%s",nuance.fields[FIELD_MEM_TO] >> 2, bNewline ? "\n" : "" );
  return length;
}

uint32 Print_LoadPixelLinear(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "ld_p (r%lu), v%lu%s",nuance.fields[FIELD_MEM_FROM],nuance.fields[FIELD_MEM_TO] >> 2, bNewline ? "\n" : "" );
  return length;
}

uint32 Print_LoadPixelBilinearUV(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "ld_p (uv), v%lu%s",nuance.fields[FIELD_MEM_TO] >> 2, bNewline ? "\n" : "" );
  return length;
}

uint32 Print_LoadPixelBilinearXY(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "ld_p (xy), v%lu%s",nuance.fields[FIELD_MEM_TO] >> 2, bNewline ? "\n" : "" );
  return length;
}

uint32 Print_LoadPixelZLinear(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "ld_pz (r%lu), v%lu%s",nuance.fields[FIELD_MEM_FROM],nuance.fields[FIELD_MEM_TO] >> 2, bNewline ? "\n" : "" );
  return length;
}

uint32 Print_LoadPixelZBilinearUV(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "ld_pz (uv), v%lu%s",nuance.fields[FIELD_MEM_TO] >> 2, bNewline ? "\n" : "" );
  return length;
}

uint32 Print_LoadPixelZBilinearXY(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "ld_pz (xy), v%lu%s",nuance.fields[FIELD_MEM_TO] >> 2, bNewline ? "\n" : "" );
  return length;
}

uint32 Print_StoreScalarImmediate(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "st_s #$%8.8lX, $%8.8lX%s",nuance.fields[FIELD_MEM_FROM],nuance.fields[FIELD_MEM_TO], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_StoreScalarAbsolute(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "st_s r%lu, $%8.8lX%s",nuance.fields[FIELD_MEM_FROM],nuance.fields[FIELD_MEM_TO], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_StoreScalarControlRegisterImmediate(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "st_s #$%8.8lX, %s%s",nuance.fields[FIELD_MEM_FROM],GetControlRegister(nuance.fields[FIELD_MEM_TO] - MPE_CTRL_BASE), bNewline ? "\n" : "");
  return length;
}

uint32 Print_StoreScalarLinear(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "st_s r%lu, (r%lu)%s",nuance.fields[FIELD_MEM_FROM],nuance.fields[FIELD_MEM_TO], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_StoreScalarBilinearUV(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "st_s r%lu, (uv)%s",nuance.fields[FIELD_MEM_TO], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_StoreScalarBilinearXY(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "st_s r%lu, (xy)%s",nuance.fields[FIELD_MEM_TO], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_StoreScalarControlRegisterAbsolute(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "st_s r%lu, %s%s",nuance.fields[FIELD_MEM_FROM],GetControlRegister(nuance.fields[FIELD_MEM_TO] - MPE_CTRL_BASE), bNewline ? "\n" : "");
  return length;
}

uint32 Print_StoreVectorControlRegisterAbsolute(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "st_v v%lu, %s%s",nuance.fields[FIELD_MEM_FROM] >> 2,GetControlRegister(nuance.fields[FIELD_MEM_TO] - MPE_CTRL_BASE), bNewline ? "\n" : "");
  return length;
}

uint32 Print_StorePixelAbsolute(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "st_p v%lu, $%8.8lX%s",nuance.fields[FIELD_MEM_FROM] >> 2,nuance.fields[FIELD_MEM_TO], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_StorePixelZAbsolute(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "st_pz v%lu, $%8.8lX%s",nuance.fields[FIELD_MEM_FROM] >> 2,nuance.fields[FIELD_MEM_TO], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_StoreShortVectorAbsolute(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "st_sv v%lu, $%8.8lX%s",nuance.fields[FIELD_MEM_FROM] >> 2,nuance.fields[FIELD_MEM_TO], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_StoreShortVectorLinear(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "st_sv v%lu, (r%lu)%s",nuance.fields[FIELD_MEM_FROM] >> 2,nuance.fields[FIELD_MEM_TO], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_StoreShortVectorBilinearUV(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "st_sv v%lu, (uv)%s",nuance.fields[FIELD_MEM_FROM] >> 2, bNewline ? "\n" : "" );
  return length;
}

uint32 Print_StoreShortVectorBilinearXY(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "st_sv v%lu, (xy)%s",nuance.fields[FIELD_MEM_FROM] >> 2, bNewline ? "\n" : "" );
  return length;
}

uint32 Print_StoreVectorAbsolute(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "st_v v%lu, $%8.8lX%s",nuance.fields[FIELD_MEM_FROM] >> 2,nuance.fields[FIELD_MEM_TO], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_StoreVectorLinear(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "st_v v%lu, (r%lu)%s",nuance.fields[FIELD_MEM_FROM] >> 2,nuance.fields[FIELD_MEM_TO], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_StoreVectorBilinearUV(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "st_v v%lu, (uv)%s",nuance.fields[FIELD_MEM_FROM] >> 2, bNewline ? "\n" : "" );
  return length;
}

uint32 Print_StoreVectorBilinearXY(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "st_v v%lu, (xy)%s",nuance.fields[FIELD_MEM_FROM] >> 2, bNewline ? "\n" : "" );
  return length;
}

uint32 Print_StorePixelLinear(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "st_p v%lu, (r%lu)%s",nuance.fields[FIELD_MEM_FROM] >> 2,nuance.fields[FIELD_MEM_TO], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_StorePixelBilinearUV(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "st_p v%lu, (uv)%s",nuance.fields[FIELD_MEM_FROM] >> 2, bNewline ? "\n" : "" );
  return length;
}

uint32 Print_StorePixelBilinearXY(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "st_p v%lu, (xy)%s",nuance.fields[FIELD_MEM_FROM] >> 2, bNewline ? "\n" : "" );
  return length;
}

uint32 Print_StorePixelZLinear(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "st_pz v%lu, (r%lu)%s",nuance.fields[FIELD_MEM_FROM] >> 2,nuance.fields[FIELD_MEM_TO], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_StorePixelZBilinearUV(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "st_pz v%lu, (uv)%s",nuance.fields[FIELD_MEM_FROM] >> 2, bNewline ? "\n" : "" );
  return length;
}

uint32 Print_StorePixelZBilinearXY(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "st_pz v%lu, (xy)%s",nuance.fields[FIELD_MEM_FROM] >> 2, bNewline ? "\n" : "" );
  return length;
}

uint32 Print_StoreScalarRegisterConstant(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "stsc r%lu, $%lX, $%lX $%lX%s",
    nuance.fields[FIELD_CONSTANT_ADDRESS],nuance.fields[FIELD_CONSTANT_VALUE],
    nuance.fields[FIELD_CONSTANT_FLAGMASK],nuance.fields[FIELD_CONSTANT_FLAGVALUES], bNewline ? "\n" : "" );
  return length;
}

const char *GetMiscConstantRegister(uint32 which)
{
  switch(which)
  {
    case CONSTANT_REG_RC0:
      return "rc0";
      break;
    case CONSTANT_REG_RC1:
      return "rc1";
      break;
    case CONSTANT_REG_RX:
      return "rx";
      break;
    case CONSTANT_REG_RY:
      return "ry";
      break;
    case CONSTANT_REG_RU:
      return "ru";
      break;
    case CONSTANT_REG_RV:
      return "rv";
      break;
    case CONSTANT_REG_RZ:
      return "rz";
      break;
    case CONSTANT_REG_XYCTL:
      return "xyctl";
      break;
    case CONSTANT_REG_UVCTL:
      return "uvctl";
      break;
    case CONSTANT_REG_XYRANGE:
      return "xyrange";
      break;
    case CONSTANT_REG_UVRANGE:
      return "uvrange";
      break;
    case CONSTANT_REG_ACSHIFT:
      return "acshift";
      break;
    case CONSTANT_REG_SVSHIFT:
      return "svshift";
      break;
    default:
      return "???";
      break;
  }
}

uint32 Print_StoreMiscRegisterConstant(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  uint32 length;
  if(nuance.fields[FIELD_CONSTANT_ADDRESS] != CONSTANT_REG_DISCARD)
  {
    length = sprintf_s(buffer, bufSize, "stsc %s, $%lX, $%lX $%lX%s",
      GetMiscConstantRegister(nuance.fields[FIELD_CONSTANT_ADDRESS]),nuance.fields[FIELD_CONSTANT_VALUE],
      nuance.fields[FIELD_CONSTANT_FLAGMASK],nuance.fields[FIELD_CONSTANT_FLAGVALUES], bNewline ? "\n" : "" );
  }
  else
  {
    length = sprintf_s(buffer, bufSize, "stfc $%lX $%lX%s",
      nuance.fields[FIELD_CONSTANT_FLAGMASK],nuance.fields[FIELD_CONSTANT_FLAGVALUES], bNewline ? "\n" : "" );
  }
  return length;
}

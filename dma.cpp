#include "basetypes.h"
#ifdef ENABLE_EMULATION_MESSAGEBOXES
#include <windows.h>
#endif
#include "bdma_type5.h"
#include "bdma_type8.h"
#include "bdma_type12.h"
#include "byteswap.h"
#include "dma.h"
#include "mpe.h"
#include "NuonMemoryMap.h"
#include "NuonEnvironment.h"

extern NuonEnvironment nuonEnv;
void UnimplementedBilinearDMAHandler(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr);

/* DMA types:

0: Z field of 16+16Z data
1: 4-bit
2: 16-bit
3: 8-bit
4: 32-bit
5: 16+16Z
6: 32+32Z
7: Z field of 32-bit data
8: 32-bit in MPE, 16 bit in DRAM
9: 16+16Z in MPE, 16+16Z triple buffer map C in DRAM
A: 16+16Z in MPE, 16+16Z triple buffer map B in DRAM
B: 16+16Z in MPE, 16+16Z triple buffer map A in DRAM
C: Z field of 16+16Z triple buffer
D: 16+16Z in MPE, 16+16Z double buffer map B in DRAM
E: 16+16Z in MPE, 16+16Z double buffer map A in DRAM
F: Z field of 16+16Z double buffer

When a Z-buffered is active and the Z-comparison field is set to 7 then
the Z-value is neither compared nor updated.  In these cases the MPE data
type is the non-Z equivalent.  Similarly when manipulating the Z-field via
DMA types 0,7,12 and 15, the pixel data is not manipulated when the
Z-comparision field is not equal to 7.  When the pixel type is 12 or 15,
Z-data is always operated on as the DMA controller does not know which
of the framebuffers holds the color data.
*/

constexpr BilinearDMAHandler BilinearDMAHandlers[] =
{
//Pixel Type 0: Allows Z write only of pixel type 5
//Write
  //Horizontal, A = 0, B = 0
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 1, B = 0
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 0, B = 0
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 1, B = 0
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 0, B = 1
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 1, B = 1
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 0, B = 1
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 1, B = 1
  UnimplementedBilinearDMAHandler,
//Read
  //Horizontal, A = 0, B = 0
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 1, B = 0
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 0, B = 0
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 1, B = 0
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 0, B = 1
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 1, B = 1
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 0, B = 1
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 1, B = 1
  UnimplementedBilinearDMAHandler,
//Pixel Type 1: 4-bit
//Write
  //Horizontal, A = 0, B = 0
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 1, B = 0
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 0, B = 0
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 1, B = 0
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 0, B = 1
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 1, B = 1
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 0, B = 1
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 1, B = 1
  UnimplementedBilinearDMAHandler,
//Read
  //Horizontal, A = 0, B = 0
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 1, B = 0
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 0, B = 0
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 1, B = 0
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 0, B = 1
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 1, B = 1
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 0, B = 1
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 1, B = 1
  UnimplementedBilinearDMAHandler,
//Pixel Type 2: 16-bit
//Write
  //Horizontal, A = 0, B = 0
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 1, B = 0
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 0, B = 0
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 1, B = 0
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 0, B = 1
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 1, B = 1
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 0, B = 1
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 1, B = 1
  UnimplementedBilinearDMAHandler,
//Read
  //Horizontal, A = 0, B = 0
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 1, B = 0
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 0, B = 0
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 1, B = 0
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 0, B = 1
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 1, B = 1
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 0, B = 1
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 1, B = 1
  UnimplementedBilinearDMAHandler,
//Pixel Type 3: 8-bit
//Write
  //Horizontal, A = 0, B = 0
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 1, B = 0
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 0, B = 0
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 1, B = 0
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 0, B = 1
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 1, B = 1
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 0, B = 1
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 1, B = 1
  UnimplementedBilinearDMAHandler,
//Read
  //Horizontal, A = 0, B = 0
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 1, B = 0
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 0, B = 0
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 1, B = 0
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 0, B = 1
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 1, B = 1
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 0, B = 1
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 1, B = 1
  UnimplementedBilinearDMAHandler,
//Pixel Type 4: 32-bit
//Write
  //Horizontal, A = 0, B = 0
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 1, B = 0
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 0, B = 0
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 1, B = 0
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 0, B = 1
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 1, B = 1
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 0, B = 1
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 1, B = 1
  UnimplementedBilinearDMAHandler,
//Read
  //Horizontal, A = 0, B = 0
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 1, B = 0
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 0, B = 0
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 1, B = 0
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 0, B = 1
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 1, B = 1
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 0, B = 1
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 1, B = 1
  UnimplementedBilinearDMAHandler,
//Pixel Type 5: 16-bit with 16-bit Z
//Write
  //Horizontal, A = 0, B = 0
  BDMA_Type5_Write_0,
  //Horizontal, A = 1, B = 0
  BDMA_Type5_Write_1,
  //Vertical, A = 0, B = 0
  BDMA_Type5_Write_2,
  //Vertical, A = 1, B = 0
  BDMA_Type5_Write_3,
  //Horizontal, A = 0, B = 1
  BDMA_Type5_Write_4,
  //Horizontal, A = 1, B = 1
  BDMA_Type5_Write_5,
  //Vertical, A = 0, B = 1
  BDMA_Type5_Write_6,
  //Vertical, A = 1, B = 1
  BDMA_Type5_Write_7,
//Read
  //Horizontal, A = 0, B = 0
  BDMA_Type5_Read_0,
  //Horizontal, A = 1, B = 0
  BDMA_Type5_Read_1,
  //Vertical, A = 0, B = 0
  BDMA_Type5_Read_2,
  //Vertical, A = 1, B = 0
  BDMA_Type5_Read_3,
  //Horizontal, A = 0, B = 1
  BDMA_Type5_Read_4,
  //Horizontal, A = 1, B = 1
  BDMA_Type5_Read_5,
  //Vertical, A = 0, B = 1
  BDMA_Type5_Read_6,
  //Vertical, A = 1, B = 1
  BDMA_Type5_Read_7,
//Pixel Type 6: 32-bit with 32-bit Z
//Write
  //Horizontal, A = 0, B = 0
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 1, B = 0
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 0, B = 0
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 1, B = 0
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 0, B = 1
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 1, B = 1
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 0, B = 1
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 1, B = 1
  UnimplementedBilinearDMAHandler,
//Read
  //Horizontal, A = 0, B = 0
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 1, B = 0
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 0, B = 0
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 1, B = 0
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 0, B = 1
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 1, B = 1
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 0, B = 1
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 1, B = 1
  UnimplementedBilinearDMAHandler,
//Pixel Type 7: Allows Z write only of pixel type 6
//Write
  //Horizontal, A = 0, B = 0
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 1, B = 0
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 0, B = 0
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 1, B = 0
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 0, B = 1
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 1, B = 1
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 0, B = 1
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 1, B = 1
  UnimplementedBilinearDMAHandler,
//Read
  //Horizontal, A = 0, B = 0
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 1, B = 0
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 0, B = 0
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 1, B = 0
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 0, B = 1
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 1, B = 1
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 0, B = 1
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 1, B = 1
  UnimplementedBilinearDMAHandler,
//Pixel Type 8: 32 bit pixels in MPE, 16 bit pixels in DRAM
  //Horizontal, A = 0, B = 0
  BDMA_Type8_Write_0,
  //Horizontal, A = 1, B = 0
  BDMA_Type8_Write_1,
  //Vertical, A = 0, B = 0
  BDMA_Type8_Write_2,
  //Vertical, A = 1, B = 0
  BDMA_Type8_Write_3,
  //Horizontal, A = 0, B = 1
  BDMA_Type8_Write_4,
  //Horizontal, A = 1, B = 1
  BDMA_Type8_Write_5,
  //Vertical, A = 0, B = 1
  BDMA_Type8_Write_6,
  //Vertical, A = 1, B = 1
  BDMA_Type8_Write_7,
//Read
  //Horizontal, A = 0, B = 0
  BDMA_Type8_Read_0,
  //Horizontal, A = 1, B = 0
  BDMA_Type8_Read_1,
  //Vertical, A = 0, B = 0
  BDMA_Type8_Read_2,
  //Vertical, A = 1, B = 0
  BDMA_Type8_Read_3,
  //Horizontal, A = 0, B = 1
  BDMA_Type8_Read_4,
  //Horizontal, A = 1, B = 1
  BDMA_Type8_Read_5,
  //Vertical, A = 0, B = 1
  BDMA_Type8_Read_6,
  //Vertical, A = 1, B = 1
  BDMA_Type8_Read_7,
//Pixel Type 9: 16+16Z in MPE, 16+16Z triple buffer C in DRAM (only one set of Z values)
//Write
  //Horizontal, A = 0, B = 0
  BDMA_Type5_Write_0,
  //Horizontal, A = 1, B = 0
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 0, B = 0
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 1, B = 0
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 0, B = 1
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 1, B = 1
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 0, B = 1
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 1, B = 1
  UnimplementedBilinearDMAHandler,
//Read
  //Horizontal, A = 0, B = 0
  BDMA_Type5_Read_0,
  //Horizontal, A = 1, B = 0
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 0, B = 0
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 1, B = 0
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 0, B = 1
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 1, B = 1
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 0, B = 1
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 1, B = 1
  UnimplementedBilinearDMAHandler,
//Pixel Type 10: 16+16Z in MPE, 16+16Z triple buffer B in DRAM (only one set of Z values)
//Write
  //Horizontal, A = 0, B = 0
  BDMA_Type5_Write_0,
  //Horizontal, A = 1, B = 0
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 0, B = 0
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 1, B = 0
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 0, B = 1
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 1, B = 1
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 0, B = 1
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 1, B = 1
  UnimplementedBilinearDMAHandler,
//Read
  //Horizontal, A = 0, B = 0
  BDMA_Type5_Read_0,
  //Horizontal, A = 1, B = 0
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 0, B = 0
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 1, B = 0
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 0, B = 1
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 1, B = 1
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 0, B = 1
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 1, B = 1
  UnimplementedBilinearDMAHandler,
//Pixel Type 11: 16+16Z in MPE, 16+16Z triple buffer A in DRAM (only one set of Z values)
//Write
  //Horizontal, A = 0, B = 0
  BDMA_Type5_Write_0,
  //Horizontal, A = 1, B = 0
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 0, B = 0
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 1, B = 0
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 0, B = 1
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 1, B = 1
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 0, B = 1
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 1, B = 1
  UnimplementedBilinearDMAHandler,
//Read
  //Horizontal, A = 0, B = 0
  BDMA_Type5_Read_0,
  //Horizontal, A = 1, B = 0
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 0, B = 0
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 1, B = 0
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 0, B = 1
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 1, B = 1
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 0, B = 1
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 1, B = 1
  UnimplementedBilinearDMAHandler,
//Pixel Type 12: Allows Z write only of 16+16Z triple buffer in DRAM
//Write
  //Horizontal, A = 0, B = 0
  BDMA_Type12_Write_0,
  //Horizontal, A = 1, B = 0
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 0, B = 0
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 1, B = 0
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 0, B = 1
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 1, B = 1
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 0, B = 1
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 1, B = 1
  UnimplementedBilinearDMAHandler,
//Read
  //Horizontal, A = 0, B = 0
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 1, B = 0
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 0, B = 0
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 1, B = 0
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 0, B = 1
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 1, B = 1
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 0, B = 1
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 1, B = 1
  UnimplementedBilinearDMAHandler,
//Pixel Type 13: 16+16Z in MPE, 16+16Z double buffer B in DRAM (only one set of Z values)
//Write
  //Horizontal, A = 0, B = 0
  BDMA_Type5_Write_0,
  //Horizontal, A = 1, B = 0
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 0, B = 0
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 1, B = 0
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 0, B = 1
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 1, B = 1
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 0, B = 1
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 1, B = 1
  UnimplementedBilinearDMAHandler,
//Read
  //Horizontal, A = 0, B = 0
  BDMA_Type5_Read_0,
  //Horizontal, A = 1, B = 0
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 0, B = 0
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 1, B = 0
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 0, B = 1
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 1, B = 1
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 0, B = 1
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 1, B = 1
  UnimplementedBilinearDMAHandler,
//Pixel Type 14: 16+16Z in MPE, 16+16Z double buffer A in DRAM (only one set of Z values)
//Write
  //Horizontal, A = 0, B = 0
  BDMA_Type5_Write_0,
  //Horizontal, A = 1, B = 0
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 0, B = 0
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 1, B = 0
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 0, B = 1
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 1, B = 1
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 0, B = 1
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 1, B = 1
  UnimplementedBilinearDMAHandler,
//Read
  //Horizontal, A = 0, B = 0
  BDMA_Type5_Read_0,
  //Horizontal, A = 1, B = 0
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 0, B = 0
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 1, B = 0
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 0, B = 1
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 1, B = 1
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 0, B = 1
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 1, B = 1
  UnimplementedBilinearDMAHandler,
//Pixel Type 15: Allows Z write only of 16+16Z double buffer in DRAM
//Write
  //Horizontal, A = 0, B = 0
  BDMA_Type12_Write_0,
  //Horizontal, A = 1, B = 0
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 0, B = 0
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 1, B = 0
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 0, B = 1
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 1, B = 1
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 0, B = 1
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 1, B = 1
  UnimplementedBilinearDMAHandler,
//Read
  //Horizontal, A = 0, B = 0
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 1, B = 0
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 0, B = 0
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 1, B = 0
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 0, B = 1
  UnimplementedBilinearDMAHandler,
  //Horizontal, A = 1, B = 1
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 0, B = 1
  UnimplementedBilinearDMAHandler,
  //Vertical, A = 1, B = 1
  UnimplementedBilinearDMAHandler
};

void UnimplementedBilinearDMAHandler(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL, "UnimplementedBilinearDMAHandler", "Error", MB_OK);
#endif
  return;
}

void DMALinear(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 intaddr)
{
  const bool bRemote = flags & (1UL << 28);
  const bool bDirect = flags & (1UL << 27);
  const bool bDup = (flags & (3UL << 26));
        uint32 length = (flags >> 16) & 0xFF; //Only 1-127 is valid according to docs but field is 8 bits
  const bool bRead = flags & (1UL << 13);

  void *baseMemory;
  if(baseaddr < 0xF0000000)
  {
    if((baseaddr & 0xF0700000) != MPE_CTRL_BASE) // this is the standard case here for the vast majority of linear DMAs and the only path that continues with the different copy cases below
    {
      assert(((baseaddr >> 23) & 0x1FUL) < 4);
      baseMemory = nuonEnv.GetPointerToMemory(nuonEnv.mpe[(baseaddr >> 23) & 0x1FUL], (baseaddr & 0xFFFFFFFC));
    }
    else // handle control register read/write and exit
    {
      if(bRead)
      {
        const uint32 directValue = SwapBytes(nuonEnv.mpe[(baseaddr >> 23) & 0x1FUL].ReadControlRegister((baseaddr & 0x207FFFFC) - MPE_CTRL_BASE,mpe.reg_union));

        if(bRemote)
        {
          if((intaddr & MPE_CTRL_BASE) == MPE_CTRL_BASE)
          {
            nuonEnv.mpe[(intaddr >> 23) & 0x1FUL].WriteControlRegister((intaddr & 0x207FFFFC) - MPE_CTRL_BASE,directValue);
          }
          else
          {
            assert(((intaddr >> 23) & 0x1FUL) < 4);
            uint32* intMemory = (uint32*)nuonEnv.GetPointerToMemory(nuonEnv.mpe[(intaddr >> 23) & 0x1FUL], (intaddr & 0x207FFFFC));
            *intMemory = directValue;
          }
        }
        else
        {
          if((intaddr & MPE_CTRL_BASE) == MPE_CTRL_BASE)
          {
            mpe.WriteControlRegister(intaddr & 0x207FFFFC, directValue);
          }
          else
          {
            uint32* intMemory = (uint32*)nuonEnv.GetPointerToMemory(mpe, (intaddr & 0x207FFFFC));
            *intMemory = directValue;
          }
        }
      }
      else
      {
        uint32 directValue;
        if(bRemote)
        {
          if((intaddr & 0xF0700000) == MPE_CTRL_BASE)
          {
            directValue = nuonEnv.mpe[(intaddr >> 23) & 0x1FUL].ReadControlRegister((intaddr & 0x207FFFFC) - MPE_CTRL_BASE,mpe.reg_union);
          }
          else
          {
            assert(((intaddr >> 23) & 0x1FUL) < 4);
            uint32* intMemory = (uint32*)nuonEnv.GetPointerToMemory(nuonEnv.mpe[(intaddr >> 23) & 0x1FUL], (intaddr & 0x207FFFFC));
            directValue = *intMemory;
          }
        }
        else
        {
          if((intaddr & MPE_CTRL_BASE) == MPE_CTRL_BASE)
          {
            directValue = mpe.ReadControlRegister((intaddr & 0x207FFFFC) - MPE_CTRL_BASE,mpe.reg_union);
          }
          else
          {
            uint32* intMemory = (uint32*)nuonEnv.GetPointerToMemory(mpe, (intaddr & 0x207FFFFC));
            directValue = *intMemory;
          }
        }
        SwapScalarBytes(&directValue);
        nuonEnv.mpe[(baseaddr >> 23) & 0x1FUL].WriteControlRegister((baseaddr & 0x207FFFFC) - MPE_CTRL_BASE,directValue);
      }

      return;
    }
  }
  else // handle flashEEPROM read/write and exit
  {
    uint32* intMemory;
    if(bRemote)
    {
      //internal address is system address (but still in MPE memory)
      assert(((intaddr >> 23) & 0x1FUL) < 4);
      intMemory = (uint32*)nuonEnv.GetPointerToMemory(nuonEnv.mpe[(intaddr >> 23) & 0x1FUL], (intaddr & 0x207FFFFC));
    }
    else
    {
      //internal address is local to MPE
      intMemory = (uint32*)nuonEnv.GetPointerToMemory(mpe, (intaddr & 0x207FFFFC), false);
    }

    if(bRead)
    {
      uint32* const pDest32 = intMemory;
      nuonEnv.flashEEPROM.ReadData(baseaddr - 0xF0000000,pDest32);
      SwapScalarBytes(pDest32);
    }
    else
    {
      const uint32* const pSrc32 = intMemory;
      const uint32 tempScalar = SwapBytes(*pSrc32);
      nuonEnv.flashEEPROM.WriteData(baseaddr - 0xF0000000,tempScalar);
    }
    return;
  }

  //
  // continue with the standard case (see above), everything else returned before
  //

  //wordSize is the size of the atomic data transfer unit as a multiple of a 16-bit word: (word = 1, scalar = 2)

  //source stride specifies the spacing between each atomic data item to be transferred from the source
  //this is normally set to one to indicate contiguous items, but will be set to zero for direct data
  //where the source pointer is not incremented between data transfers

  //dest stride specifies the stride in 32-bit scalars from the start of one written data item to the next
  //for example, alternate word write locations are spaced 32-bits apart, for a stride of one

  uint32 destStride, wordSize;
  bool bByteMode = false;

  switch(flags & 0x07)
  {
    //Determine stride in scalars, or in words for byte mode
    case 0:
      //contiguous scalars
      wordSize = 2;
      destStride = 1;
      break;
    case 1:
      //byte mode
      bByteMode = true;
      wordSize = 1;
      destStride = 1;
      break;
    case 2:
      //alternate scalars
      wordSize = 2;
      destStride = 2;
      break;
    case 3:
      //alternate words
      wordSize = 1;
      destStride = 1;
      break;
    case 4:
      //every fourth scalar
      wordSize = 2;
      destStride = 4;
      break;
    case 5:
      //every fourth word
      wordSize = 1;
      destStride = 2;
      break;
    case 6:
      //every eighth scalar
      wordSize = 2;
      destStride = 8;
      break;
    case 7:
      //every eighth word
      wordSize = 1;
      destStride = 4;
      break;
  }

  uint32 srcStride = 1;
  const void* pSrc;
  void* pDest;
  uint32 directValue;

  if(bRead)
  {
    //Read: base -> internal
    pSrc = baseMemory;

    const bool bFlushCache =
      ((intaddr & MPE_LOCAL_MEMORY_MASK) >= MPE_IRAM_BASE) &&
      ((intaddr & MPE_LOCAL_MEMORY_MASK) < MPE_DTAGS_BASE);

    if(bRemote)
    {
      //internal address is system address (but still in MPE memory)
      assert(((intaddr >> 23) & 0x1FUL) < 4);
      pDest = nuonEnv.GetPointerToMemory(nuonEnv.mpe[(intaddr >> 23) & 0x1FUL], (intaddr & 0x207FFFFC));
      if(bFlushCache)
      {
        //Maintain cache coherency!  This assumes that code will not be
        //dynamically created in the dtrom/dtram section, bypassing the need
        //to flush the cache on data writes.

        /*
        Critical TO DO: Many games such as T3K switch between overlays on a very frequent basis because they cannot fit all of the needed
        code into a single overlay.  For example, T3K switches overlays several times per frame with each overlay rendering a different
        set of playfield objects.  Not only does this cause an unyielding barage of requests to invalidate the interpreter cache and
        native code cache but it also means that the native code cache will fill up with multiple copies of overlay code that is compiled
        and then forgotten when an invalidation occurs.  This causes additional code cache flushes.  In T3K, the code cache is being
        flushed almost non-stop.  This is a serious, serious bottleneck. T3K flushes the cache at an unnerving pace.
        */

        //LARGE_INTEGER timeStart, timeEnd, timeFreq, timeOverhead;
        //QueryPerformanceCounter(&timeStart);
        //QueryPerformanceCounter(&timeEnd);

        //timeOverhead.QuadPart = timeEnd.QuadPart - timeStart.QuadPart;

        //QueryPerformanceFrequency(&timeFreq);
        //QueryPerformanceCounter(&timeStart);
        //nuonEnv.mpe[(intaddr >> 23) & 0x1FUL].InvalidateICacheRegion((intaddr & 0xF07FFFFF), (intaddr & 0xF07FFFFF) + (length << 2) - 1);
        //nuonEnv.mpe[(intaddr >> 23) & 0x1FUL].InvalidateICache();
        //nuonEnv.mpe[(intaddr >> 23) & 0x1FUL].nativeCodeCache->FlushRegion(intaddr & 0xF07FFFFF, (intaddr & 0xF07FFFFF) + (length << 2) - 1);

        nuonEnv.mpe[(intaddr >> 23) & 0x1FUL].UpdateInvalidateRegion(intaddr & MPE_LOCAL_MEMORY_MASK, length*2*wordSize);
        //nuonEnv.mpe[(intaddr >> 23) & 0x1FUL].UpdateInvalidateRegion(MPE_IRAM_BASE, MPE::overlayLengths[(intaddr >> 23) & 0x1FUL]);
        //nuonEnv.mpe[(intaddr >> 23) & 0x1FUL].nativeCodeCache->FlushRegion(0x20300000, (intaddr & 0xF07FFFFF) + ((length - 1) << 2));
        //QueryPerformanceCounter(&timeEnd);
        //char tempBuf[128];
        //sprintf(tempBuf,"DMA caused cache invalidations: %lf seconds wasted.",((double)(timeEnd.QuadPart - timeStart.QuadPart - timeOverhead.QuadPart)) / ((double)timeFreq.QuadPart));
        //MessageBox(NULL,tempBuf,"DMALinear",MB_OK);
      }
    }
    else
    {
      //internal address is local to MPE
      pDest = nuonEnv.GetPointerToMemory(mpe, (intaddr & 0x207FFFFC), false);
      if(bFlushCache)
      {
        //Maintain cache coherency!  This assumes that code will not be
        //dynamically created in the dtrom/dtram section, bypassing the need
        //to flush the cache on data writes.
        //mpe.InvalidateICacheRegion(intaddr, intaddr + (length << 2) - 1);
        //mpe.InvalidateICache();
        //mpe.nativeCodeCache->FlushRegion(intaddr, intaddr + (length << 2) - 1);
        mpe.UpdateInvalidateRegion(intaddr & MPE_LOCAL_MEMORY_MASK, length*2*wordSize);
        //mpe.UpdateInvalidateRegion(MPE_IRAM_BASE,MPE::overlayLengths[mpe.mpeIndex]);
        //mpe.nativeCodeCache->FlushRegion(0x20300000, intaddr + ((length - 1) << 2));
      }
    }
  }
  else
  {
    //Write: internal -> base
    pDest = baseMemory;

    if(bDup)
    {
      if(bDirect)
      {
        directValue = intaddr;
#ifdef LITTLE_ENDIAN
        //swap back to big endian format
        SwapScalarBytes(&directValue);
#else
        if(wordSize == 1)
        {
          directValue >>= 16;
        }
#endif
        pSrc = (void *)&directValue;
      }
      else
      {
        //Dup but not Direct: read scalar from memory, no need to swap
        if(bRemote)
        {
          //internal address is system address (but still in MPE memory)
          assert(((intaddr >> 23) & 0x1FUL) < 4);
          directValue = *((uint32 *)nuonEnv.GetPointerToMemory(nuonEnv.mpe[(intaddr >> 23) & 0x1FUL], (intaddr & 0x207FFFFC)));
        }
        else
        {
          //internal address is local to MPE
          directValue = *((uint32 *)nuonEnv.GetPointerToMemory(mpe, (intaddr & 0x207FFFFC), false));
        }

        //change srcStride to 0 to keep pSrc pointed at directValue
        srcStride = 0;
        pSrc = (void *)&directValue;

#ifndef LITTLE_ENDIAN
        if(wordSize == 1)
        {
          directValue >>= 16;
          pSrc = (void *)((uint8 *)&directValue + 2);
        }
#endif
      }
    }
    else
    {
      if(bRemote)
      {
        //internal address is system address (but still in MPE memory)
        assert(((intaddr >> 23) & 0x1FUL) < 4);
        pSrc = nuonEnv.GetPointerToMemory(nuonEnv.mpe[(intaddr >> 23) & 0x1FUL], (intaddr & 0x207FFFFC));
      }
      else
      {
        //internal address is local to MPE
        pSrc = nuonEnv.GetPointerToMemory(mpe, (intaddr & 0x207FFFFC), false);
      }
    }
  }

  const bool cw = (((flags >> 3) & 0x3) == 3); //Contiguous words

  if(!bByteMode || cw)
  {
    if(wordSize == 2)
    {
      const uint32* pSrc32 = (uint32*)pSrc;
      uint32* pDest32 = (uint32*)pDest;
      while(length--)
      {
        *pDest32 = *pSrc32;
        pSrc32 += srcStride;
        pDest32 += destStride;
      }
    }
    else
    {
      const uint16* pSrc16 = (uint16*)pSrc;
      uint16* pDest16 = (uint16*)pDest;
      while(length--)
      {
        *pDest16 = *pSrc16;
        pSrc16 += srcStride;
        pDest16 += destStride;
      }
    }
  }
  else
  {
    uint16 mask;

    switch((flags >> 3) & 0x3)
    {
      case 0: //illegal, but fall through to case 1 anyway
        assert(false);
      case 1: //Byte 1 only (LSB bits [7:0] of each word)
#ifdef LITTLE_ENDIAN
        mask = 0xFF00;
#else
        mask = 0x00FF;
#endif
        break;
      case 2: //Byte 0 only (MSB bits [15:8] of each word)
#ifdef LITTLE_ENDIAN
        mask = 0x00FF;
#else
        mask = 0xFF00;
#endif
        break;
      //case 3: //Contiguous words (handled above)
      //  mask = 0xFFFF;
      //  break;
    }

    const uint16* pSrc16 = (uint16*)pSrc;
    uint16* pDest16 = (uint16*)pDest;
    while(length--)
    {
       *pDest16 = (*pDest16 & ~mask) | (*pSrc16 & mask);
       pSrc16 += srcStride;
       pDest16 += destStride;
    }
  }
}

void DMALinear(MPE &mpe)
{
  const uint32 flags = mpe.regs[0];
  const uint32 baseaddr = mpe.regs[1];
  const uint32 intaddr = mpe.regs[2];

  //For the BIOS call, simulate the latency of the call assuming
  //40 cycles of setup time (copying to command buffer, determining
  //which bus to write to, etc) plus one cycle per long transferred
  nuonEnv.cycleCounter += (40 + (flags >> 16) & 0xFF); //Only 1-127 is valid according to docs but field is 8 bits

  DMALinear(mpe,flags,baseaddr,intaddr);
}

void DMABiLinear(MPE &mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
  uint32 whichRoutine;

  //pixel flags + backwards flag a (PPPP):(xxxA) 
  whichRoutine = flags & 0xF1;
  //read flag (PPPP):(RxxA)
  whichRoutine |= ((flags >> (13 - 3)) & 0x08UL);
  //vertical flag and backwards flag b (PPPP):(RBVA)
  whichRoutine |= ((flags >> (9 - 2)) & 0x06UL);

  switch(whichRoutine >> 4)
  {
    case 0:
#ifdef ENABLE_EMULATION_MESSAGEBOXES
      MessageBox(NULL,"whichRoutine>>4 not found","DMABiLinear Error",MB_OK);
#endif
      //whichRoutine = 0;
      return;
      break;
    case 1:
      whichRoutine = 0;
      //return;
      break;
    case 2:
      whichRoutine = 0;
      break;
    case 3:
      whichRoutine = 0;
      //return;
      break;
    case 4:
      switch(whichRoutine & 0x0FUL)
      {
        case 0:
        case 8:
          break;
        default:
          whichRoutine = 0;
          break;
      }
      break;
    case 5:
      switch(whichRoutine & 0x0FUL)
      {
        case 0:
        case 8:
          BilinearDMAHandlers[whichRoutine](mpe,flags,baseaddr,xinfo,yinfo,intaddr);
          return;
          break;
        default:
          whichRoutine = 0;
          break;
      }
      break;
    case 6:
      BilinearDMAHandlers[whichRoutine](mpe,flags,baseaddr,xinfo,yinfo,intaddr);
      return;
      break;
    case 8:
      BilinearDMAHandlers[whichRoutine](mpe,flags,baseaddr,xinfo,yinfo,intaddr);
      return;
      break;
    case 9:
    case 10:
    case 11:
      BilinearDMAHandlers[whichRoutine](mpe,flags,baseaddr,xinfo,yinfo,intaddr);
      return;
      break;
    case 12:
      BilinearDMAHandlers[whichRoutine](mpe,flags,baseaddr,xinfo,yinfo,intaddr);
      return;
      break;
    case 13:
    case 14:
      BilinearDMAHandlers[whichRoutine](mpe,flags,baseaddr,xinfo,yinfo,intaddr);
      return;
      break;
    case 15:
      BilinearDMAHandlers[whichRoutine](mpe,flags,baseaddr,xinfo,yinfo,intaddr);
      return;
      break;
    default:
#ifdef ENABLE_EMULATION_MESSAGEBOXES
      MessageBox(NULL,"whichRoutine>>4 not found","DMABiLinear Error",MB_OK);
#endif
      //whichRoutine = 0;
      return;
      break;
  }

  //const bool bBatch = flags & (1UL << 30);
  const bool bChain = flags & (1UL << 29);

  if(bChain)
  {
#ifdef ENABLE_EMULATION_MESSAGEBOXES
    MessageBox(NULL,"Chained DMA not supported","DMABiLinear Error",MB_OK);
#endif
    return;
  }

  const bool bRemote = flags & (1UL << 28);
  const bool bDirect = flags & (1UL << 27);
  const bool bDup = flags & (3UL << 26); //bDup = dup | direct
  //const bool bTrigger = flags & (1UL << 25);
  const bool bRead = flags & (1UL << 13);
        int32 xsize = (flags >> 13) & 0x7F8UL;
  //const uint32 type = (flags >> 14) & 0x03UL;
  //const uint32 mode = flags & 0xFFFUL;
  const uint32 zcompare = (flags >> 1) & 0x07UL;
  const uint32 pixtype = (flags >> 4) & 0x0FUL;
  const uint32 bva = ((flags >> 7) & 0x06UL) | (flags & 0x01UL);
  const uint32 sdramBase = baseaddr & 0x7FFFFFFEUL;
  const uint32 mpeBase = intaddr & 0x7FFFFFFCUL;
        uint32 xlen = (xinfo >> 16) & 0x3FFUL;
        uint32 xpos = xinfo & 0x7FFUL;
  const uint32 ylen = (yinfo >> 16) & 0x3FFUL;
  const uint32 ypos = yinfo & 0x7FFUL;
        bool bCompareZ = false;
        bool bUpdateZ = false;

  uint32 wordsize/*, pixsize*/;
  switch(pixtype)
  {
    //4 bit, 8 bit, 16 bit, and 16 bit Z-field only modes
    case 0:
      //16 bit Z field only
      wordsize = 1;
      break;
    case 1:
      //4 bit
      wordsize = 1;
      //pixsize = 1;
      xlen >>= 2;
      xsize >>= 2;
      xpos >>= 2;
      break;
    case 2:
      //16 bit
      wordsize = 1;
      //pixsize = 1;
      break;
    case 3:
      //8 bit
      wordsize = 1;
      //pixsize = 1;
      xlen >>= 1;
      xsize >>= 1;
      xpos >>= 1;
      break;
    //32 bit, 32 bit + 32 bit Z, and 32 bit Z-field only modes
    case 4:
      wordsize = 2;
      //pixsize = 2;
      break;
    case 5:
    {
      wordsize = 2;
      //pixsize = 1;

      bUpdateZ = (zcompare != 7);
      if(bUpdateZ)
      {
        //pixel+Z write (16 + 16Z)
        bCompareZ = (zcompare != 0);
      }
      else
      {
        //pixel only write (16 bit)
        bCompareZ = false;
      }
      break;
    }
    case 6:
    {
      wordsize = 2;
      //pixsize = 2;

      bUpdateZ = (zcompare != 7);
      if(bUpdateZ)
      {
        //pixel+Z write (32 + 32Z)
        bCompareZ = (zcompare != 0);
      }
      else
      {
        //pixel only write (32 bit)
        bCompareZ = false;
      }
      break;
    }
    case 7:
      wordsize = 2;
      break;
    //16 bit + 16 bit Z, single, double and triple buffer modes
    case 8:
      wordsize = 1;
      //pixsize = 1;
      break;
    case 9:
    {
      wordsize = 2;
      //pixsize = 1;

      bUpdateZ = (zcompare != 7);
      if(bUpdateZ)
      {
        //pixel+Z write (16 + 16Z)
        bCompareZ = (zcompare != 0);
      }
      else
      {
        //pixel only write (16 bit)
        bCompareZ = false;
      }
      break;
    }
    case 10:
    {
      wordsize = 2;
      //pixsize = 1;

      bUpdateZ = (zcompare != 7);
      if(bUpdateZ)
      {
        //pixel+Z write (16 + 16Z)
        bCompareZ = (zcompare != 0);
      }
      else
      {
        //pixel only write (16 bit)
        bCompareZ = false;
      }
      break;
    }
    case 11:
    {
      wordsize = 2;
      //pixsize = 1;

      bUpdateZ = (zcompare != 7);
      if(bUpdateZ)
      {
        //pixel+Z write (16 + 16Z)
        bCompareZ = (zcompare != 0);
      }
      else
      {
        //pixel only write (16 bit)
        bCompareZ = false;
      }
      break;
    }
    case 12:
      wordsize = 1;
      //pixsize = 1;
      break;
    case 13:
    {
      wordsize = 2;
      //pixsize = 1;

      bUpdateZ = (zcompare != 7);
      if(bUpdateZ)
      {
        //pixel+Z write (16 + 16Z)
        bCompareZ = (zcompare != 0);
      }
      else
      {
        //pixel only write (16 bit)
        bCompareZ = false;
      }
      break;
    }
    case 14:
    {
      wordsize = 2;

      bUpdateZ = (zcompare != 7);
      if(bUpdateZ)
      {
        //pixel+Z write (16 + 16Z)
        bCompareZ = (zcompare != 0);
      }
      else
      {      
        bCompareZ = false;
      }
      break;
    }
    case 15:
      wordsize = 1;
      //pixsize = 1;
      break;
  }

  void* intMemory;
  if(bRemote)
  {
    //internal address is system address (but still in MPE memory)
    assert(((mpeBase >> 23) & 0x1FUL) < 4);
    intMemory = nuonEnv.GetPointerToMemory(nuonEnv.mpe[(mpeBase >> 23) & 0x1FUL], mpeBase & 0x207FFFFF, false);
  }
  else
  {
    //internal address is local to MPE
    intMemory = nuonEnv.GetPointerToMemory(mpe, mpeBase, false);
  }

  //base address is always a system address (absolute)

#ifdef ENABLE_EMULATION_MESSAGEBOXES
  if((sdramBase < 0x40000000) || (sdramBase > 0x407FFFFF))
  {
    char msgBuf[512];
    sprintf_s(msgBuf, sizeof(msgBuf), "sdramBase is out of range on MPE%u: 0x%lx\n",mpe.mpeIndex,sdramBase);
    MessageBox(NULL,msgBuf,"DMABiLinear error",MB_OK);
  }
  //else
  //{
  //}
#endif

  assert(((sdramBase >> 23) & 0x1FUL) < 4);
  void* const baseMemory = nuonEnv.GetPointerToMemory(nuonEnv.mpe[(sdramBase >> 23) & 0x1FUL], sdramBase, false);

  uint32 srcAStart, srcBStart, destAStart, destBStart, aCountInit, bCount, srcOffset, destOffset;
  int32 srcAStep, srcBStep, destAStep, destBStep;

  uint32 directValue = intaddr;
  const void* pSrc;
  void* pDest;

  if(bRead)
  {
    pSrc = baseMemory;
    pDest = intMemory;
    srcOffset = ypos * (uint32)xsize + xpos;
    destOffset = 0;

    destAStart = 0;
    destAStep = 1;
    destBStart = 0;
    destBStep = xlen;

    switch(bva)
    {
      case 0:
        //BVA = 000 (horizontal DMA, x increment, y increment)
        srcAStart = 0;
        srcAStep = 1;
        srcBStart = 0;
        srcBStep = xsize;
        aCountInit = xlen;
        bCount = ylen;
        break;
      case 1:
        //BVA = 001 (horizontal DMA, x decrement, y increment)
        srcAStart = xlen - 1;
        srcAStep = -1;
        srcBStart = 0;
        srcBStep = xsize;
        aCountInit = xlen;
        bCount = ylen;
        break;
      case 2:
        //BVA = 010 (vertical DMA, y increment, x increment)
        srcAStart = 0;
        srcAStep = xsize;
        srcBStart = 0;
        srcBStep = 1;
        aCountInit = ylen;
        bCount = xlen;
        break;
      case 3:
        //BVA = 011 (vertical DMA, y decrement, x increment)
        srcAStart = (ylen - 1) * xsize;
        srcAStep = -xsize;
        srcBStart = 0;
        srcBStep = 1;
        aCountInit = ylen;
        bCount = xlen;
        break;
      case 4:
        //BVA = 100 (horizontal DMA, x increment, y decrement)
        srcAStart = 0;
        srcAStep = 1;
        srcBStart = (ylen - 1) * xsize;
        srcBStep = -xsize;
        aCountInit = xlen;
        bCount = ylen;
        break;
      case 5:
        //BVA = 101 (horizontal DMA, x decrement, y decrement)
        srcAStart = xlen - 1;
        srcAStep = -1;
        srcBStart = (ylen - 1) * xsize;
        srcBStep = -xsize;
        aCountInit = xlen;
        bCount = ylen;
        break;
      case 6:
        //BVA = 110 (vertical DMA, y increment, x decrement)
        srcAStart = 0;
        srcAStep = xsize;
        srcBStart = xlen - 1;
        srcBStep = -1;
        aCountInit = ylen;
        bCount = xlen;
        break;
      case 7:
        //BVA = 111 (vertical DMA, y decrement, x decrement)
        srcAStart = (ylen - 1) * xsize;
        srcAStep = -xsize;
        srcBStart = xlen - 1;
        srcBStep = -1;
        aCountInit = ylen;
        bCount = xlen;
        break;
    }
  }
  else
  {
    pSrc = intMemory;
    pDest = baseMemory;

    if(bDup)
    {
      if(bDirect)
      {
        //Direct and Dup: intaddr is data.
        directValue = intaddr;
#ifdef LITTLE_ENDIAN
        //swap back to big endian format
        SwapScalarBytes(&directValue);
#else
        if(wordsize == 1)
        {
          directValue >>= 16;
        }
#endif
      }
      else
      {
        //Dup but not Direct: read scalar from memory, no need to swap
        directValue = *((uint32 *)intMemory);
#ifndef LITTLE_ENDIAN
        if(wordsize == 1)
        {
          directValue >>= 16;
        }
#endif
      }

      pSrc = (void *)&directValue;
      srcAStep = 0;
      srcBStep = 0;
    }
    else
    {
      srcAStep = 1;
      srcBStep = xlen;
    }

    srcAStart = 0;
    srcBStart = 0;
    srcOffset = 0;
    destOffset = ypos * (uint32)xsize + xpos;

    switch(bva)
    {
      case 0:
        //BVA = 000 (horizontal DMA, x increment, y increment)
        destAStart = 0;
        destAStep = 1;
        destBStart = 0;
        destBStep = xsize;
        aCountInit = xlen;
        bCount = ylen;
        break;
      case 1:
        //BVA = 001 (horizontal DMA, x decrement, y increment)
        destAStart = xlen - 1;
        destAStep = -1;
        destBStart = 0;
        destBStep = xsize;
        aCountInit = xlen;
        bCount = ylen;
        break;
      case 2:
        //BVA = 010 (vertical DMA, y increment, x increment)
        destAStart = 0;
        destAStep = xsize;
        destBStart = 0;
        destBStep = 1;
        aCountInit = ylen;
        bCount = xlen;
        break;
      case 3:
        //BVA = 011 (vertical DMA, y decrement, x increment)
        destAStart = (ylen - 1) * xsize;
        destAStep = -xsize;
        destBStart = 0;
        destBStep = 1;
        aCountInit = ylen;
        bCount = xlen;
        break;
      case 4:
        //BVA = 100 (horizontal DMA, x increment, y decrement)
        destAStart = 0;
        destAStep = 1;
        destBStart = (ylen - 1) * xsize;
        destBStep = -xsize;
        aCountInit = xlen;
        bCount = ylen;
        break;
      case 5:
        //BVA = 101 (horizontal DMA, x decrement, y decrement)
        destAStart = xlen - 1;
        destAStep = -1;
        destBStart = (ylen - 1) * xsize;
        destBStep = -xsize;
        aCountInit = xlen;
        bCount = ylen;
        break;
      case 6:
        //BVA = 110 (vertical DMA, y increment, x decrement)
        destAStart = 0;
        destAStep = xsize;
        destBStart = xlen - 1;
        destBStep = -1;
        aCountInit = ylen;
        bCount = xlen;
        break;
      case 7:
        //BVA = 111 (vertical DMA, y decrement, x decrement)
        destAStart = (ylen - 1) * xsize;
        destAStep = -xsize;
        destBStart = xlen - 1;
        destBStep = -1;
        aCountInit = ylen;
        bCount = xlen;
        break;
    }
  }

  if(wordsize == 2)
  {
    /*if(!bRead)
    {
      if((GetPixBaseAddr(sdramBase,destOffset,2) >= nuonEnv.mainChannelLowerLimit) && (GetPixBaseAddr(sdramBase,destOffset,2) <= nuonEnv.mainChannelUpperLimit) ||
         (GetPixBaseAddr(sdramBase,(destOffset+((xsize - 1)*ylen)+xlen),2) >= nuonEnv.mainChannelLowerLimit) && (GetPixBaseAddr(sdramBase,(destOffset+((xsize - 1)*ylen)+xlen),2) <= nuonEnv.mainChannelUpperLimit))
      {
        nuonEnv.bMainBufferModified = true;
      }
      else if((GetPixBaseAddr(sdramBase,destOffset,2) >= nuonEnv.overlayChannelLowerLimit) && (GetPixBaseAddr(sdramBase,destOffset,2) <= nuonEnv.overlayChannelUpperLimit) ||
         (GetPixBaseAddr(sdramBase,(destOffset+((xsize - 1)*ylen)+xlen),2) >= nuonEnv.overlayChannelLowerLimit) && (GetPixBaseAddr(sdramBase,(destOffset+((xsize - 1)*ylen)+xlen),2) <= nuonEnv.overlayChannelUpperLimit))
      {
        nuonEnv.bOverlayBufferModified = true;
      }
    }*/

    const uint32* pSrc32 = ((uint32*)pSrc) + (srcOffset+srcAStart+srcBStart);
    uint32* pDest32 = ((uint32*)pDest) + (destOffset+destAStart+destBStart);

    const bool bCompareZ2 = bCompareZ && (zcompare > 0) && (zcompare < 7);

    while(bCount--)
    {
      uint32 srcA = 0;
      uint32 destA = 0;
      uint32 aCount = aCountInit;

      if(!bCompareZ2)
      {
      while(aCount--)
      {
        pDest32[destA] = pSrc32[srcA];

        srcA += srcAStep;
        destA += destAStep;
      }
      }
      else
      while(aCount--)
      {
        bool bZTestResult = false;

        //if(bCompareZ2)
        {
          const uint16 ztarget = SwapBytes(((uint16 *)&pDest32[destA])[1]);
          const uint16 ztransfer = SwapBytes(((uint16 *)&pSrc32[srcA])[1]);

          switch(zcompare)
          {
            //case 0x0:
              //bZTestResult = false;
              //break;
            case 0x1:
              bZTestResult = (ztarget < ztransfer);
              break;
            case 0x2:
              bZTestResult = (ztarget == ztransfer);
              break;
            case 0x3:
              bZTestResult = (ztarget <= ztransfer);
              break;
            case 0x4:
              bZTestResult = (ztarget > ztransfer);
              break;
            case 0x5:
              bZTestResult = (ztarget != ztransfer);
              break;
            case 0x6:
              bZTestResult = (ztarget >= ztransfer);
              break;
            //case 0x7:
              //bZTestResult = false;
              //break;
          }
        }

        if(!bZTestResult)
          pDest32[destA] = pSrc32[srcA];

        srcA += srcAStep;
        destA += destAStep;
      }

      pSrc32 += srcBStep;
      pDest32 += destBStep;
    }
  }
  else
  {
    /*if(!bRead)
    {
      if((GetPixBaseAddr(sdramBase,destOffset,1) >= nuonEnv.mainChannelLowerLimit) && (GetPixBaseAddr(sdramBase,destOffset,1) <= nuonEnv.mainChannelUpperLimit) ||
         (GetPixBaseAddr(sdramBase,(destOffset+((xsize - 1)*ylen)+xlen),1) >= nuonEnv.mainChannelLowerLimit) && (GetPixBaseAddr(sdramBase,(destOffset+((xsize - 1)*ylen)+xlen),1) <= nuonEnv.mainChannelUpperLimit))
      {
        nuonEnv.bMainBufferModified = true;
      }
      else if((GetPixBaseAddr(sdramBase,destOffset,1) >= nuonEnv.overlayChannelLowerLimit) && (GetPixBaseAddr(sdramBase,destOffset,1) <= nuonEnv.overlayChannelUpperLimit) ||
         (GetPixBaseAddr(sdramBase,(destOffset+((xsize - 1)*ylen)+xlen),2) >= nuonEnv.overlayChannelLowerLimit) && (GetPixBaseAddr(sdramBase,(destOffset+((xsize - 1)*ylen)+xlen),1) <= nuonEnv.overlayChannelUpperLimit))
      {
        nuonEnv.bOverlayBufferModified = true;
      }
    }*/

    const uint16* pSrc16 = ((uint16*)pSrc) + (srcOffset+srcAStart+srcBStart);
    uint16* pDest16 = ((uint16*)pDest) + (destOffset+destAStart+destBStart);

    while(bCount--)
    {
      uint32 srcA = 0;
      uint32 destA = 0;
      uint32 aCount = aCountInit;

      while(aCount--)
      {
        pDest16[destA] = pSrc16[srcA];

        srcA += srcAStep;
        destA += destAStep;
      }

      pSrc16 += srcBStep;
      pDest16 += destBStep;
    }
  }
}

void DMABiLinear(MPE &mpe)
{
  const uint32 flags = mpe.regs[0];
  const uint32 baseaddr = mpe.regs[1];
  const uint32 xinfo = mpe.regs[2];
  const uint32 yinfo = mpe.regs[3];
  const uint32 intaddr = mpe.regs[4];

  //For the BIOS call, simulate the latency of the call assuming
  //25 cycles of setup time plus 1 cycle per pixel transferred (xlen * ylen)
  nuonEnv.cycleCounter += (25 + (((xinfo >> 16) & 0x3FFUL) * ((yinfo >> 16) & 0x3FFUL)));

  DMABiLinear(mpe,flags,baseaddr,xinfo,yinfo,intaddr);
}

void DMADo(MPE &mpe)
{
  const uint32 ctrl = mpe.regs[0];
  const uint32 cmdBlock = mpe.regs[1] & 0x3FFFFFF0UL;
  //const uint32 waitFlag = mpe.regs[2];

  if(ctrl == 0x20500500)
  {
    mpe.odmacptr = cmdBlock;

    if(mpe.odmactl & 0x60UL)
    {
      //other bus DMA is enabled so do it!
      const uint32* const cmdptr = (uint32 *)nuonEnv.GetPointerToMemory(mpe,cmdBlock);
            uint32 dmaflags = SwapBytes(cmdptr[0]);
      const uint32 baseaddr = SwapBytes(cmdptr[1]);
      const uint32 intaddr = SwapBytes(cmdptr[2]);
      //clear all bits except bits 13, 16 - 23, and 28
      dmaflags &= ((1UL << 13) | (0xFFUL << 16) | (1UL << 28));
      DMALinear(mpe,dmaflags,baseaddr,intaddr);
    }
  }
  else if(ctrl == 0x20500600)
  {
    //mdmacptr
    mpe.mdmacptr = cmdBlock;

    while(true) {
    const uint32* const cmdptr = (uint32 *)nuonEnv.GetPointerToMemory(mpe,cmdBlock,false);
    const uint32 dmaflags = SwapBytes(cmdptr[0]);
    const uint32 baseaddr = SwapBytes(cmdptr[1]);
          uint32 intaddr = SwapBytes(cmdptr[2]);

    switch((dmaflags >> 14) & 0x03UL)
    {
      case 0:
        //linear DMA
        DMALinear(mpe,dmaflags,baseaddr,intaddr);
        break;
      case 3:
      {
        //bilinear pixel DMA
        const uint32 xptr = intaddr;
        const uint32 yptr = SwapBytes(cmdptr[3]);
        intaddr = SwapBytes(cmdptr[4]);
        DMABiLinear(mpe,dmaflags,baseaddr,xptr,yptr,intaddr);
        break;
      }
      default:
#ifdef ENABLE_EMULATION_MESSAGEBOXES
        MessageBox(NULL, "(dmaflags >> 14) & 0x03UL not found", "DMADo Error", MB_OK);
#endif
        return;
    }

    if(dmaflags & (1UL << 30)) // batch? -> repeat the loop
      mpe.mdmacptr += 16;
    else
      return;
    }
  }
}

// as none of the games seem to use waitFlag == 0 above, AND all DMA emulation code is immediate at the moment, just return
void DMAWait(MPE &mpe)
{
  return;
}

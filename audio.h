//---------------------------------------------------------------------------

#ifndef _AUDIO_H
#define _AUDIO_H
//---------------------------------------------------------------------------

class MPE;

#define RATE_44_1_KHZ  (0x0001UL)
#define RATE_88_2_KHZ  (0x0002UL)
#define RATE_22_05_KHZ (0x0004UL)

#define RATE_48_KHZ	(0x0010UL)
#define RATE_96_KHZ	(0x0020UL)
#define RATE_24_KHZ	(0x0040UL)

#define RATE_32_KHZ	(0x0100UL)
#define RATE_64_KHZ	(0x0200UL)
#define RATE_16_KHZ	(0x0400UL)


#define STREAM_TWO_16_BIT (0x0000UL)
#define STREAM_FOUR_16_BIT (0x0008UL)
#define STREAM_TWO_32_BIT (0x0010UL)
#define STREAM_EIGHT_16_BIT (0x0018UL)
#define STREAM_EIGHT_32_BIT (0x2010UL)
#define STREAM_FOUR_32_BIT (0x2018UL)

#define BUFFER_SIZE_1K (0x0020UL)
#define BUFFER_SIZE_2K (0x0040UL)
#define BUFFER_SIZE_4K (0x0060UL)
#define BUFFER_SIZE_8K (0x0080UL)
#define BUFFER_SIZE_16K (0x00A0UL)
#define BUFFER_SIZE_32K (0x00C0UL)
#define BUFFER_SIZE_64K (0x00E0UL)

#define ENABLE_AUDIO_DMA (0x0001UL)
#define ENABLE_WRAP_INT	(0x0100UL)
#define ENABLE_HALF_INT	(0x0200UL)
#define ENABLE_SAMP_INT	(0x0400UL)
#define ENABLE_DMA_SKIP	(0x0800UL)
#define ENABLE_DMA_STALL (0x1000UL)

void AudioMute(MPE *);
void AudioReset(MPE *);
void AudioQuerySampleRates(MPE *);
void AudioSetSampleRate(MPE *);
void AudioQueryChannelMode(MPE *);
void AudioSetChannelMode(MPE *);
void AudioSetDMABuffer(MPE *);

#endif

#ifndef _AUDIO_H
#define _AUDIO_H

class MPE;

#define RATE_44_1_KHZ  (0x0001U)
#define RATE_88_2_KHZ  (0x0002U)
#define RATE_22_05_KHZ (0x0004U)

#define RATE_48_KHZ	(0x0010U)
#define RATE_96_KHZ	(0x0020U)
#define RATE_24_KHZ	(0x0040U)

#define RATE_32_KHZ	(0x0100U)
#define RATE_64_KHZ	(0x0200U)
#define RATE_16_KHZ	(0x0400U)


#define STREAM_TWO_16_BIT (0x0000U)
#define STREAM_FOUR_16_BIT (0x0008U)
#define STREAM_TWO_32_BIT (0x0010U)
#define STREAM_EIGHT_16_BIT (0x0018U)
#define STREAM_EIGHT_32_BIT (0x2010U)
#define STREAM_FOUR_32_BIT (0x2018U)

#define BUFFER_SIZE_1K (0x0020U)
#define BUFFER_SIZE_2K (0x0040U)
#define BUFFER_SIZE_4K (0x0060U)
#define BUFFER_SIZE_8K (0x0080U)
#define BUFFER_SIZE_16K (0x00A0U)
#define BUFFER_SIZE_32K (0x00C0U)
#define BUFFER_SIZE_64K (0x00E0U)

#define ENABLE_AUDIO_DMA (0x0001U)
#define ENABLE_WRAP_INT	(0x0100U)
#define ENABLE_HALF_INT	(0x0200U)
#define ENABLE_SAMP_INT	(0x0400U)
#define ENABLE_DMA_SKIP	(0x0800U)
#define ENABLE_DMA_STALL (0x1000U)
#define ENABLE_LATER_COUNT_WAIT (0x100000U)

void AudioMute(MPE& mpe);
void AudioReset(MPE& mpe);
void AudioQuerySampleRates(MPE &mpe);
void AudioSetSampleRate(MPE &mpe);
void AudioQueryChannelMode(MPE &mpe);
void AudioSetChannelMode(MPE &mpe);
void AudioSetDMABuffer(MPE &mpe);

#endif

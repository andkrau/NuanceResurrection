#ifndef commH
#define commH

#define COMM_RECV_BUFFER_FULL_BIT (1UL << 31)
#define COMM_RECV_DISABLE_BIT (1UL << 30)
#define COMM_DISABLED_BITS (3UL << 30)
#define COMM_SOURCE_ID_BITS (0xFFUL << 16)
#define COMM_XMIT_BUFFER_FULL_BIT (1UL << 15)
#define COMM_XMIT_FAILED_BIT (1UL << 14)
#define COMM_XMIT_RETRY_BIT (1UL << 13)
#define COMM_LOCKED_BIT (1UL << 12)
#define COMM_TARGET_ID_BITS (0xFFUL)

#define COMM_ID_VDG (0x41)
#define COMM_ID_AUDIO (0x43)

void DoCommBusController(void);

#endif

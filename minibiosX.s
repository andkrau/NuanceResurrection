.nooptimize

;nise audioisr = $203004C4
ptraudioisr = $20100000
;nise commhandler = $203005E8?
ptraudiocommhandler = $20100004
stack_pointer_initial_value = $20100be0

.section minid
.origin $20100be0
dmacmdbuffer0: .ds.v 1
;.origin $20100bf0
dmacmdbuffer1: .ds.v 1
;.origin $20100c00
comminfoc3variable: .dc.s $00000000
.ds.s 2
commhookhandler: .dc.s $00000000
;.origin $20100c10
;level2 handler inserts packets here
comm_packet_queue1_tail: .dc.s $20101c20
comm_packet_queue1_head: .dc.s $20101c20
;.origin $20100c18
comm_packet_queue2_tail: .dc.s $20101da0
;commrecv reads from here
comm_packet_queue2_head: .dc.s $20101da0
;.origin $20100c20
comm_packet_queue1_start: .ds.v 24
;.origin $20100da0
comm_packet_beyond_queue1:
comm_packet_queue2_start: .ds.v 10
;.origin $20100e40
beyond_comm_packet_queue2:

.section minic
.nooptimize
.origin $20300a80

minibios_entry_point:

;disable all level 1 interrupt sources
st_s #$00000000,inten1
;clear all interrupts
st_s #$FFFFFFFF,intclr
;v0 = commrecv (clear commrecv.recvbufferfull bit)
ld_v commrecv,v0

comm_c3_rzi1:
st_s #stack_pointer_initial_value,sp
st_s #$00000020,odmactl
st_s #$00000020,mdmactl
;set up the level 1 interrupt handler
st_s #minilevel1wrapper,intvec1
mv_s #$c,r0
st_s r0,intclr
;enable intsrc2 and intsrc3
st_s r0,inten1set
;enable halting on all exceptions
st_s #$FFFFFFFF,excephalten
;enable software interrupt 
st_s #$1,inten1set
;set up level 2 handler and branch to minispinwait
{
st_s #minilevel2handler,intvec2
bra minispinwait2
}
;select commrecv as level 2 interrupt source
st_s #4,inten2sel
;clear hw and sw masks for level 1 and level 2
st_s #$55,intctl

comm_c4_rzi1:
;store r27 at memory location pointed to by r26 (r27 = first scalar of $c4 comm packet, r26 = second scalar of $c4 comm packet)
st_s r27,(r26)

;.origin $20300acc
minispinwait:
nop
bra minispinwait
nop
nop

;.origin $20300ad6
comm_c0:
minireadlinear:
;START OF MINIREADLINEAR ROUTINE
push v0,rz
mv_v v1,v2
;R10 = ADJUSTED NUMBER OF BYTES TO READ (ADJUSTED SUCH THAT DIVIDING BY FOUR WILL YIELD CORRECT SCALAR COUNT IN ALL CASES)
add #$3,r10
;R10 = (ADJUSTED NUMBER OF BYTES TO READ) >> 2 = NUMBER OF SCALARS TO READ
lsr #$2,r10

read_loop:
;R10 IS REMAINING SCALAR COUNT

;MOVE BASEADDR PARAMETER TO R1
mv_s r9,r1
;MOVE INTADDR PARAMETER TO R2
mv_s r8,r2
;R3 = PARAMS = WAIT FOR DMA ($80000000)
mv_s #$80000000,r3
;SCALAR_READ_COUNT = SCALAR_REMAINING_COUNT
mv_s r10,r0
;COMPARE REMAINING NUMBER OF SCALARS TO 64
cmp #$00000040,r10
;BRANCH TO DO_READ IF REMAINING NUMBER OF SCALARS IS LESS THAN OR EQUAL TO 64
bra le,do_read,nop

;SCALAR_READ_COUNT = 64
mv_s #$00000040,r0

do_read:
;BASEADDR = BASEADDR + (SCALAR_READ_COUNT << 2)
add r0,>>#-2,r9
;CALL MINIDMALINEARREAD ROUTINE
jsr minidmalinearread
;SCALAR_REMAINING_COUNT = SCALAR_REMAINING_COUNT - SCALAR_READ_COUNT
sub r0,r10
;INTADDR = INTADDR + (SCALAR_READ_COUNT << 2)
add r0,>>#-2,r8
;COMPARE SCALAR_REMAINING_COUNT TO ZERO
cmp #$00000000,r10
;IF SCALAR_REMAINING_COUNT IS NOT ZERO, BRANCH TO READ_LOOP
bra ne,read_loop,nop
pop v0, rz
nop
;RETURN TO CALLING ROUTINE USING STORED RETURN ADDRESS IN R11
rts nop

comm_c2:
rts
st_s r4,rzi1
;ENABLE ALL EXCEPTION HALT BITS EXCEPT EXCEPHALTEN_HALT
st_s #$fffffffe,excephalten

exceptionhandler:
;CLEAR INTSRC EXCEPTION INTERRUPT BIT
st_s #$00000001,intclr
;CLEAR EXCEPTSRC EXCEPTION INTERRUPT BIT
st_s #$00000001,excepclr

comm_c3:
rts
;STORE ZERO INTO COMMINFOC3VARIABLE ($20101C00)
st_s #$00000000,comminfoc3variable
st_s #comm_c3_rzi1,rzi1

comm_c4:
st_s #comm_c4_rzi1,rzi1
rts
mv_s r4,r26
mv_s r5,r27

minidmalineardirect_wait:
;MINIDMALINEARDIRECT_WAIT(NUMSCALARS,BASEADDR,INTADDR,PARAMS)
push v1
;R4 = R0, R3 = 0
{
mv_s r0,r4
sub r3,r3
}
;R3 = $80000000 (PARAMS = WAIT FOR DMA COMPLETION, USE DMA BUFFER 0)
or #$00000001,<>#1,r3
;BRANCH TO WAIT_FOR_MDMA_PENDING
bra wait_for_mdma_pending
;R0 = R0 << 16
asl #16,r0
;R0 = R0 | $8000000 (R0 = (NUMBER OF SCALARS << 16) | DIRECT)
or #$00000001,<>#5,r0

minidmalinearreadextraflags_wait:
;MINIDMALINEARREADEXTRAFLAGS_WAIT(NUMSCALARS,BASEADDR,INTADDR,EXTRAFLAGS)
;R0 = R0 << 16
asl #16,r0
;R0 = (NUMSCALARS << 16) | DMA_READ
or #$00000001,<>#-13,r0
;OR EXTRAFLAGS INTO R0 DMAFLAGS 
or r3,r0

minidmalinear_wait:
;MINIDMALINEAR_WAIT(DMAFLAGS,BASEADDR,INTADDR)
;BRANCH TO COMMON_DMA
bra common_dma
;R0 = 0
sub r3,r3
;R3 = $80000000 (PARAMS = WAIT FOR DMA COMPLETION)
or #$00000001,<>#1,r3

;START OF JUMPTABLE DMA ROUTINES

minidmalinearread_wait:
;MINIDMALINEARREAD_WAIT(NUMSCALARS,BASEADDR,INTADDR,PARAMS)
bra minidmalinearread

minidmalinearwrite_wait:
;MINIDMALINEARWRITE_WAIT(NUMSCALARS,BASEADDR,INTADDR,PARAMS)
;PARAMS = 0
sub r3,r3
;SET WAIT BIT OF PARAMS
or #$00000001,<>#1,r3

minidmalinearwrite:
;MINIDMALINEARWRITE(NUMSCALARS,BASEADDR,INTADDR,PARAMS)
;BRANCH TO COMMON_DMA
bra common_dma

minidmalinearread:
;MINIDMALINEARREAD(NUMSCALARS,BASEADDR,INTADDR,PARAMS)
;R4 = NUMSCALARS
mv_s r0,r4
;R0 = DMAFLAGS = NUMSCALARS << 16
asl #16,r0
;RETURN IF DMAFLAGS IS ZERO
rts eq,nop
;DMAFLAGS = DMAFLAGS | DMA_READ_BIT
or #$00000001,<>#-13,r0

common_dma:
push v1

wait_for_mdma_pending:
ld_s mdmactl,r4
ld_s odmactl,r5
;TEST PENDING BIT OF MDMACTL
btst #$00000004,r4
;IF PENDING BIT IS SET, BRANCH BACK TO WAIT_FOR_MDMA_PENDING, TEST PENDING BIT OF ODMACTL
{
bra ne,wait_for_mdma_pending,nop
btst #$00000004,r5
}

wait_for_odma_pending:
;IF PENDING BIT IS SET, BRANCH BACK TO WAIT_FOR_MDMA_PENDING, TEST BIT 31 OF EXTERNAL ADDRESS (R1)
{
bra ne,wait_for_mdma_pending,nop
btst #$0000001f,r1
}

;R5 = ADDRESS OF MDMACTL REGISTER
mv_s #$20500600,r5
;IF BIT 31 OF EXTERNAL ADDRESS (R1 = $80000000 OR GREATER), BRANCH TO DO_DMA:
bra eq,do_dma,nop
;R5 = ADDRESS OF ODMACTL REGISTER
mv_s #$20500500,r5

do_dma:
;R4 = BIT 0 OF DMA PARAMETERS (BIT 31 = WAIT BIT, BIT 0 = CMD BUFFER SELECT BIT)
and #$00000001,r3,r4
;R4 = R4 * 16 (RESULT IS EITHER 0 OR 16)
asl #4,r4
;R4 = DMA CMD BUFFER BASE 0 + SELECTED BUFFER OFFSET (RESULT IS $20101BE0 OR $20101BF0)
add #dmacmdbuffer0,r4
;STORE DMA COMMAND DATA INTO SELECTED COMMAND BUFFER
st_v v0,(r4)
;R5 = ADDRESS OF ODMACPTR/MDMACPTR
add #$00000010,r5
;STORE STARTING ADDRESS OF SELECTED BUFFER INTO ODMACPTR/MDMACPTR TO TRIGGER THE DMA OPERATION
st_s r4,(r5)
;R5 = ADDRESS OF ODMACTL/MDMACTL
sub #$00000010,r5
;TEST BIT 31 OF R3 (WAIT FOR DMA)
btst #$0000001f,r3
;IF NOT SET, BRANCH TO EPILOGUE
bra eq,dma_epilogue,nop

wait_for_dma:
;R4 = ODMACTL/MDMACTL
ld_s (r5),r4
nop
;R4 = NUMBER OF PENDING/ACTIVE COMMANDS
bits #$00000004,>>#0,r4
;IF COMMANDS ARE PENDING OR ACTIVE, BRANCH BACK TO WAIT_FOR_DMA
bra ne,wait_for_dma,nop

dma_epilogue:
{
rts nop
pop v1
}

miniwaitmdma:
bra wait_for_dma
push v1
mv_s #$20500500,r5

;ODMAWAIT(VOID)
miniwaitodma:
bra wait_for_dma
push v1
mv_s #$20500600,r5

;DONT MOVE ANYTHING BEYOND THIS POINT!  SOME PROGRAMS SUCH AS LIBNISE ASSUME THE JUMPTABLE 
;STARTS AT $20301FE0 INSTEAD OF USING AN OFFSET FROM INTVEC1

;.origin $20300be2
minilevel1handler:
ld_s rz,r13
push v1
ld_s intsrc,r15
ld_s inten1,r14
push v0
;R15 = INTSRC & INTEN1
and r14,r15
;TEST BIT 3 OF (INTSRC & INTEN1)
btst #$00000003,r15
;CALL INTSRC3_HANDLER IF BIT 3 IS SET
jsr ne,intsrc3handler,nop
;TEST BIT 2 OF (INTSRC & INTEN1)
btst #$00000002,r15
;CALL INTSRC_2_HANDLER IF BIT 2 IS SET
jsr ne,intsrc2handler,nop
;R1 = PTRAUDIOISR
ld_s ptraudioisr,r1
;TEST AUDIO BIT (27) OF (INTSRC & INTEN1)
btst #$0000001b,r15
;CALL AUDIO HANDLER IF AUDIO BIT IS SET
jsr ne,(r1),nop
;TEST EXCEPTION BIT (0) OF (INTSRC & INTEN1)
btst #$00000000,r15
;CALL HANDLER IF EXCEPTION BIT IS SET
jsr ne,exceptionhandler,nop
;EPILOGUE
pop v0
pop v1
st_s r13,rz
pop v2
{
st_s r12,cc
rti rzi1
}
pop v3
nop

;.origin $20300c1a
minilevel2handler:
;START OF MINIBIOS LEVEL2 INTERRUPT HANDLER
push v0
push v1
push v2
push r31,cc,rzi2,rz
;r31 = comm_packet_queue1_tail
ld_s comm_packet_queue1_tail,r31
;CALLED ROUTINE SHOULD RETURN TO EPILOGUE
st_s #minilevel2_epilogue,rz
;R0 = COMMCTL
ld_s commctl,r0
;R1 = COMMINFO
ld_s comminfo,r1
;TEST RECVBUFFER FULL BIT OF COMMCTL
btst #$0000001f,r0
;BRANCH TO COMMEXCEPTION IF RECVBUFFER FULL BIT IS NOT SET
bra eq,minicommexception1,nop
;R0 = RECV SOURCE ID, CLEAR BIT 4 OF INTSRC (COMMRECV)
{
bits #$00000007,>>#16,r0
st_s #$00000010,intclr
}
;V1 = COMMRECV, COMPARE RECV SOURCE ID TO $4A (CODED DATA INTERFACE)
{
ld_v commrecv,v1
cmp #$0000004a,r0
}
;R1 = RECV COMMINFO, BRANCH TO CDIHANDLER ($20301E34) IF RECV SOURCE ID WAS $4A (CODED DATA INTERFACE)
{
bits #$00000007,>>#16,r1
jmp eq,cdihandler,nop
}
;COMPARE RECV SOURCE ID TO $03
cmp #$00000003,r0
;BRANCH TO NOT_MPE3 IF RECV SOURCE IS IS NOT $03, COMPARE RECV COMMINFO TO $D0
{
bra ne,not_mpe3
cmp #$000000d0,r1
}
;BRANCH TO comminfod0handler ($20301DE6) IF RECV SOURCE IS MPE3 AND RECV COMMINFO IS $D0
jmp eq,comm_d0,nop

not_mpe3:
;STORE FIRST VECTOR OF PACKET DATA TO QUEUE1 TAIL POINTER, INCREMENT TAIL POINTER BY $10
{
st_v v0,(r31)
add #$00000010,r31
}
;STORE SECOND VECTOR OF PACKET DATA TO QUEUE1 TAIL POINTER, INCREMENT TAIL POINTER BY $10
{
st_v v1,(r31)
add #$00000010,r31
}
;R0 = QUEUE1 HEAD POINTER ($20101C14), COMPARE UPDATED QUEUE1 TAIL POINTER TO START OF QUEUE2 ($20101DA0)
{
ld_s comm_packet_queue1_head,r0
cmp #comm_packet_queue2_start,r31
}
;BRANCH TO COMPARE_UPDATED_QUEUE_POINTER IF UPDATED QUEUE POINTER IS LESS THAN START OF QUEUE2 ($20101DA0)
bra lt,compare_queue1_tail_pointer,nop

queue1_tail_wrap:
;SET UPDATED QUEUE1 TAIL POINTER TO POINT TO THE START OF QUEUE1 ($20101C20)
mv_s #comm_packet_queue1_start,r31

compare_queue1_tail_pointer:
;COMPARE UPDATED QUEUE1 TAIL POINTER TO QUEUE1 HEAD POINTER ($20301C14)
cmp r0,r31
;BRANCH TO QUEUE1_OVERFLOW IF QUEUE1 TAIL POINTER IS EQUAL TO QUEUE1 HEAD 
;POINTER: SETS COMMRECV EXCEPTION BIT AND BRANCHES TO INTSRC_3_HANDLER TO 
;COPY ALL PACKETS TO QUEUE2
bra eq,minicommexception2,nop

;STORE UPDATED QUEUE TAIL POINTER BACK TO MEMORY ($20101C10)
st_s r31,comm_packet_queue1_tail
;TRIGGER INTSRC3 INTERRUPT
st_s #$00000008,intsrc

minilevel2_epilogue:
pop r31,cc,rzi2,rz
pop v2
{
pop v1
rti rzi2
}
pop v0
nop

minicommexception2:
st_s #$4,excepsrc
minicommexception1:
;SET COMMRECV EXCEPTION BIT IN EXCEPSRC
st_s #$4,excepsrc

intsrc3handler:
;CLEAR THE INTSRC3 BIT IN THE INTSRC REGISTER
st_s #$00000008,intclr
push v3,rz

intsrc3_packet_loop:
;LOAD V2 WITH VECTOR STARTING AT $20101C10 (QUEUE1TAIL:QUEUE1HEAD:QUEUE2TAIL:QUEUE2HEAD)
ld_v comm_packet_queue1_tail,v2
;RZ = INTSRC3_HANDLE_PACKET_LOOP
st_s #intsrc3_packet_loop,rz
;COMPARE CURRENT QUEUE1 TAIL POINTER WITH THE QUEUE1 HEAD POINTER
cmp r9,r8
;IF THE CURRENT QUEUE1 TAIL POSITION EQUALS THE QUEUE1 HEAD POSITION, BRANCH TO EPILOGUE
bra eq,intsrc3_epilogue,nop

;V0 = VECTOR FROM QUEUE1 HEAD, INCREMENT QUEUE1 HEAD POINTER BY ONE VECTOR
{
ld_v (r9),v0
add #$00000010,r9
}
;V0 = VECTOR FROM QUEUE1 HEAD, INCREMENT QUEUE1 HEAD POINTER BY ONE VECTOR
{
ld_v (r9),v1
add #$00000010,r9
}
;COMPARE UPDATE QUEUE1 HEAD POINTER TO START OF QUEUE2 ($20101DA0), R3 = CONTENTS OF MINICOMMHOOK VARIABLE
{
ld_s commhookhandler,r3
cmp #comm_packet_queue2_start,r9
}
;BRANCH IF UPDATED QUEUE1 ENTRY ADDRESS IS NOT EQUAL TO THE START OF QUEUE2 ($20101DA0)
bra ne,intsrc3_update_queue1_head,nop

queue1_head_wrap:
;ADJUST QUEUE1 HEAD POINTER TO START OF QUEUE1 ($20101C20)
mv_s #comm_packet_queue1_start,r9

intsrc3_update_queue1_head:
;STORE UPDATED QUEUE1 HEAD POINTER BACK TO MEMORY ($20101C14)
st_s r9,comm_packet_queue1_head
;COMPARE SENDER ID TO $03
cmp #$00000003,r0
;IF SENDER ID IS NOT $03, BRANCH TO INTSRC3_NOT_MPE3:
bra ne,intsrc3_not_mpe3,nop

;R2 = SCALAR AT $20101004 (PTRAUDIOCOMMHANDLER)
ld_s ptraudiocommhandler,r2
;R3 = COMMINFO & $F0
and #$000000f0,r1,r3
;COMPARE (COMMINFO & $F0) TO $A0
{
cmp #$000000a0,r3
ld_s commhookhandler,r3
}
;JUMP TO PTRAUDIOCOMMHANDLER IF (COMMINFO & $F0) EQUALS $A0, COMPARE COMMINFO TO $C0
{
jmp eq,(r2),nop
cmp #$000000c0,r1
}
;JUMP TO C0 HANDLER IF COMMINFO IS $C0, COMPARE COMMINFO TO $C2
{
jmp eq,comm_c0,nop
cmp #$000000c2,r1
}
;JUMP TO C2 HANDLER IF COMMINFO IS $C2, COMPARE COMMINFO TO $C3
{
jmp eq,comm_c2,nop
cmp #$000000c3,r1
}
;JUMP TO C3 HANDLER IF COMMINFO IS $C3, COMPARE COMMINFO TO $C4
{
jmp eq,comm_c3,nop
cmp #$000000c4,r1
}
;JUMP TO C4 HANDLER IF COMMINFO IS $C4
jmp eq,comm_c4,nop

intsrc3_not_mpe3:
;COMPARE MINICOMMHOOK TO ZERO
cmp #$00000000,r3
;JUMP TO MINICOMMHOOK IF MINICOMMHOOK IS NON-ZERO
jmp ne,(r3),nop

;R8 = QUEUE2 TAIL INCREMENTED BY TWO VECTORS
add #$00000020,r10,r8
;COMPARE (QUEUE2 TAIL INCREMENTED BY TWO VECTORS) TO BEYOND_QUEUE2 ($20101E40)
cmp #beyond_comm_packet_queue2,r8
;IF (QUEUE2 TAIL INCREMENTED BY TWO VECTORS) IS NOT EQUAL TO BEYOND_QUEUE2, BRANCH 
;TO CHECK_FOR_REMAINING_PACKETS
bra ne,intsrc3_check_for_remaining_packets,nop

queue2_wrap:
;QUEUE2 TAIL POINTER = START OF QUEUE2
mv_s #comm_packet_queue2_start,r8

intsrc3_check_for_remaining_packets:
;COMPARE QUEUE2 TAIL POINTER TO QUEUE2 HEAD POINTER
cmp r8,r11
;IF QUEUE2 TAIL POINTER EQUALS QUEUE2 HEAD POINTER, BRANCH TO INTSRC3_AFTER_STORE (queue2 is out of room)
bra eq,intsrc3_after_store,nop

;STORE FIRST VECTOR OF QUEUE1 PACKET TO QUEUE2 TAIL
{
st_v v0,(r10)
add #$00000010,r10
}
;STORE SECOND VECTOR OF QUEUE1 PACKET TO QUEUE2 TAIL
st_v v1,(r10)

;STORE UPDATED QUEUE2 TAIL POINTER BACK TO MEMORY ($20101C1C)
st_s r8,comm_packet_queue2_tail

intsrc3_after_store:
bra intsrc3_packet_loop,nop

intsrc3_epilogue:
pop v3,rz
nop
rts nop

minicommsend:
push v2

wait_for_xmit_buffer:
ld_s commctl,r8
ld_s comminfo,r7
;TEST XMIT BUFFER FULL BIT
btst #$0000000f,r8
;IF XMIT BUFFER FULL BIT IS SET, BRANCH BACK TO WAIT_FOR_XMIT_BUFFER
bra ne,wait_for_xmit_buffer,nop

trigger_xmit:
;COMMINFO = R5, R10 = COMMCTL
{
st_s r5,comminfo
copy r8,r10
}
;COMMCTL = R4 (TARGET ID BITS), TRANSMIT BUFFER FAIL BIT OF R10
{
st_s r4,commctl
bits #$00000000,>>#14,r10
}
;COMMXMIT = V0, R8 = PREVIOUS COMMCTL VALUE | (TRANSMIT BUFFER FAIL BIT << 5)
{
st_v v0,commxmit
or r10,>>#-5,r8
}
check_for_transmit_failed:
;R11 = COMMCTL
ld_s commctl,r11
nop
;TEST TRANSMIT FAILED BIT OF R11
btst #$0000000e,r11
;BRANCH BACK TO TRIGGER_XMIT IF TRANSMIT FAILED BIT IS SET, TEST BIT 5 OF TRANSMIT TARGET ID (SET IF TRANSMIT WAS INTERRUPTED AND FAILED IN THE INTERRUPT)
{
bra ne,trigger_xmit,nop
btst #$00000005,r11
}
;BRANCH BACK TO TRIGGER_XMIT IF TARGET ID IS HARDWARE TARGET (ID >= 32) AND TRANSMIT FAILED BIT IS NOT SET
;TEST TRANSMIT BUFFER FULL BIT OF R11
{
bra ne,trigger_xmit,nop
btst #$0000000f,r11
}
;BRANCH BACK TO CHECK_FOR_TRANSMIT FAILED IF TRANSMIT BUFFER FULL BIT IS STILL SET
bra ne,check_for_transmit_failed,nop

;RESTORE COMMCTL VALUE (IN CASE WE EXECUTE THIS FROM WITHIN AN INTERRUPT): IF THE TRANSMIT FAILED BIT WAS SET PRIOR
;TO THE INTERRUPT, BIT 5 WILL BE OF TARGET ID WILL BE SET (BECAUSE THE FAILED BIT IS READ ONLY)
{
st_s r8,commctl
rts
}
pop v2
st_s r7,comminfo

;.origin $20300da2
minicommrecv:
;BRANCH TO COMMRECV_COMMON
bra commrecv_common
;R6 = FAILURE ADDRESS = CHECK_FOR_AVAILABLE_PACKET
mv_s #check_for_available_packet,r6
nop

;.origin $20300dac
minicommrecvquery:
;R6 FAILURE ADDRESS = EPILOGUE
mv_s #minicommrecvquery_epilogue,r6

commrecv_common:
push v2
check_for_available_packet:
;LOAD V2 WITH VECTOR AT $20101C10
ld_v comm_packet_queue1_tail,v2
nop
;COMPARE QUEUE2 TAIL POINTER (R10:$20101C18) WITH QUEUE2 HEAD POINTER (R11:$20101C1C)
cmp r10,r11
;R4 = -1, JUMP TO FAILURE ADDRESS IF SCALARS ARE EQUAL (NO NEW PACKET, R6 WILL BE EPILOGUE FOR COMMRECVQUERY, RETRY FOR COMMRECV)
{
jmp eq,(r6),nop
mv_s #$ffffffff,r4
}
;V1 = FIRST PACKET DATA VECTOR FROM CURRENT QUEUE2 HEAD
ld_v (r11),v1
add #$00000010,r11
;V0 = SECOND PACKET DATA VECTOR FROM CURRENT QUEUE2 HEAD
ld_v (r11),v0
;R11 = NEW QUEUE2 HEAD (OLD HEAD + $20)
add #$00000010,r11
;COMPARE NEW QUEUE2 HEAD TO $20101E40
cmp #beyond_comm_packet_queue2,r11
;BRANCH TO NO_ADJUST IF NEW QUEUE2 HEAD IS NOT EQUAL TO $2010E40
bra ne,no_adjust2,nop
;ADJUST NEW QUEUE2 HEAD TO $20101DA0
mv_s #comm_packet_queue2_start,r11

no_adjust2:
;STORE UPDATED QUEUE HEAD POINTER BACK INTO MEMORY ($20101C1C)
st_s r11,comm_packet_queue2_head

minicommrecvquery_epilogue:
pop v2
rts nop

minicommhook:
;R1 = SCALAR AT $20101C0C (COMMHOOK_HANDLER)
ld_s commhookhandler,r1
rts
;STORE NEW COMMHOOK HANDLER ADDRESS BACK TO MEMORY
st_s r0,commhookhandler
;RETURN OLD COMMHOOK_HANDLER ADDRESS
copy r1,r0

;I don't know what this is used for, so its left verbatim
comm_d0:
ld_s $20101FF0,r0
copy r5,r7
cmp #$00000000,r0
bra ne,comm_d0_part2,nop
sub r5,r5
st_v v1,$20101FC0
mv_s #$20101E40,r0
{
st_s r0,$20101FE0
sub r0,r0
}
{
st_s r0,$20101FE4
rts 
}
st_s r0,$20101FE8
st_s #$0000000D,$20101FF0

comm_d0_part2:
mv_s r4,r1
mv_s #$00000009,r0
st_s #$00000000,$20101FF0
jmp minicommsend
mv_s #$00000003,r4
mv_s #$000000D1,r5

minicommrecvwait:
push v7, rz
`commrecv_wait_loop:
//syscall
st_s #((1 << 24)|1), configa
jsr minicommrecvquery,nop
cmp #-1, r4
bra eq, `commrecv_wait_loop,nop
pop v7, rz
nop
rts nop

minispinwait2:
mv_s #$FFFFFFFF, r0
//syscall: WaitForInterrupt (any interrupt)
st_s #(1 << 24)|0, configa
;This is the only code not copied directly from the n501 minibios... it should never be called anyway
bra minispinwait2, nop

cdihandler:
rts nop

;This is the only code not copied directly from the n501 minibios... it should never be called anyway
intsrc2handler:
{
st_s #$00000004,intclr
rts nop
}
.section somejumptab
.origin $20301fc0

;The usage of these two entries is unknown so don't eliminate them!
unknown_jumptable:
{
mv_s #-1, r0
rts nop
}
{
mv_s #-1, r0
rts nop
}
.section djumptab
.origin $20300fc8
dma_jumptable:
jmp minidmalinearread, nop
jmp minidmalinearread_wait, nop
jmp minidmalinearwrite, nop
jmp minidmalinearwrite_wait, nop
jmp miniwaitodma, nop
jmp miniwaitmdma, nop

.section jumptabX
.origin $20300fe0
minibios_jumptable:
;intvec - 20
jmp minicommhook, nop
;intvec - 16
jmp minicommsend, nop
;intvec - 12 ($20300fe8)
jmp minicommrecvquery, nop
;intvec - 8 ($20300fec)
jmp minicommrecvwait, nop
;intvec - 4 ($20300ff0)
{
  mv_s #5, r0
  rts nop
}

.origin $20300ff4
minilevel1wrapper:
{
push v3
jmp minilevel1handler
}
ld_s cc,r12
push v2
nop
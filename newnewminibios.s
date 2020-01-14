.nooptimize

ptraudioisr = $20101000
ptraudiocommhandler = $20101004
stack_pointer_initial_value = $20101be0

.section minid
.origin $20101be0
dmacmdbuffer0: .ds.v 1
.origin $20101bf0
dmacmdbuffer1: .ds.v 1
.origin $20101c00
comminfoc3variable: .dc.s $00000000
.ds.s 2
commhookhandler: .dc.s $00000000
comm_packet_queue1_tail: .dc.s $20101c20
comm_packet_queue1_head: .dc.s $20101c20
comm_packet_queue2_tail: .dc.s $20101da0
comm_packet_queue2_head: .dc.s $20101da0
.origin $20101c20
comm_packet_queue1_start: .ds.v 24
.origin $20101da0
beyond_comm_packet_queue1:
comm_packet_queue2_start: .ds.v 10
.origin $20101e40
beyond_comm_packet_queue2:

.section minic
.nooptimize
.origin $20301a90

comm_c3_rzi1:
;LOAD INTVEC1 WITH MINILEVEL1WRAPPER ADDRESS
st_s #minilevel1wrapper,intvec1
;LOAD INTVEC2 WITH MINILEVEL2HANDLER ADDRESS
st_s #minilevel2handler,intvec2
;SELECT COMMRECV INTERRUPTS AS THE LEVEL2 INTERRUPT TYPE
st_s #$00000004,inten2sel
;SET INTEN1 TO $c3 (HAHA), DO SOME FUNKY CHICKEN DANCE WITH MUL_SV
{
mul_sv ru,v0,>>#$00000010,v0
st_s r0,inten1
}
;SET UP STACK POINTER
st_s #$20101be0,sp
;SET ODMA TO PRIORITY LEVEL 1
st_s #$00000020,odmactl
;SET MDMA TO PRIORITY LEVEL 1
st_s #$00000020,mdmactl
;BRANCH TO SPINWAIT
bra minispinwait
;CLEAR ALL INTERRUPT MASKS
st_s #$00000055,intctl
;ENABLE HALT FOR ALL EXCEPTIONS
st_s #$ffffffff,excephalten

comm_c4_rzi1:
;store r27 at memory location pointed to by r26 (r27 = first scalar of $c4 comm packet, r26 = second scalar of $c4 comm packet)
st_s r27,(r26)

minispinwait:
bra minispinwait
nop
nop

;.origin $20301ad0
comminfo_c0_handler:
minireadlinear:
;START OF MINIREADLINEAR ROUTINE
mv_v v1,v2
;SAVE RETURN ADDRESS IN R11
ld_s rz,r11
;R10 = ADJUSTED NUMBER OF BYTES TO READ (ADJUSTED SUCH THAT DIVIDING BY FOUR WILL YIELD CORRECT SCALAR COUNT IN ALL CASES)
add #$00000003,r10
;R10 = (ADJUSTED NUMBER OF BYTES TO READ) >> 2 = NUMBER OF SCALARS TO READ
lsr #$00000002,r10

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

;RETURN TO CALLING ROUTINE USING STORED RETURN ADDRESS IN R11
jmp (r11),nop

comminfo_c2_handler:
rts
st_s r4,rzi1
;ENABLE ALL EXCEPTION HALT BITS EXCEPT EXCEPHALTEN_HALT
st_s #$fffffffe,excephalten

exceptionhandler:
;CLEAR INTSRC EXCEPTION INTERRUPT BIT
st_s #$00000001,intclr
;CLEAR EXCEPTSRC EXCEPTION INTERRUPT BIT
st_s #$00000001,excepclr

comminfo_c3_handler:
rts
;STORE ZERO INTO COMMINFOC3VARIABLE ($20101C00)
st_s #$00000000,comminfoc3variable
st_s #comm_c3_rzi1,rzi1

comminfo_c4_handler:
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

.origin $20301bda
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

.origin $20301c12
minilevel2handler:
;START OF MINIBIOS LEVEL2 INTERRUPT HANDLER
push v0
push v1
push v2
push r31,cc,rzi2,rz
;CLEAR BIT 3 OF INTSRC
st_s #$00000008,intclr
;V2 = MINI_COMM_QUEUE1_TAIL:MINI_COMM_QUEUE1_HEAD:MINI_COMM_QUEUE2_TAIL:MINI_COMM_QUEUE2_HEAD
ld_v comm_packet_queue1_tail,v2
;CALLED ROUTINE SHOULD RETURN TO EPILOGUE
st_s #minilevel2_epilogue,rz
;R0 = COMMCTL
ld_s commctl,r0
;R1 = COMMINFO
ld_s comminfo,r1
;TEST RECVBUFFER FULL BIT OF COMMCTL
btst #$0000001f,r0
;BRANCH TO EPILOGUE IF RECVBUFFER FULL BIT IS NOT SET
bra eq,minilevel2_epilogue,nop
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
bra ne,not_mpe3
cmp #$000000d0,r1
;BRANCH TO comminfod0handler ($20301DE6) IF RECV SOURCE IS MPE3 AND RECV COMMINFO IS $D0
jmp eq,comminfod0handler,nop
{
and #$f0,r1,r3
ld_s ptraudiocommhandler,r2
}
{
;COMPARE (COMMINFO & $F0) TO $A0, R3 = COMMHOOKHANDLER
cmp #$a0,r3
ld_s commhookhandler,r3
}
{
;JUMP TO PTRAUDIOCOMMHANDLER IF (COMMINFO & $F0) IS $A0, COMPARE RECV COMMINFO TO $C0
jmp eq,(r2),nop
cmp #$000000c0,r1
}
;JUMP TO C0MMINFO_C0_HANDLER IF COMMINFO IS $C0, COMPARE RECV COMMINFO TO $C2
{
jmp eq,comminfo_c0_handler,nop
cmp #$000000c2,r1
}
;JUMP TO C0MMINFO_C2_HANDLER IF COMMINFO IS $C2, COMPARE RECV COMMINFO TO $C3
{
jmp eq,comminfo_c2_handler,nop
cmp #$000000c3,r1
}
;JUMP TO C0MMINFO_C3_HANDLER IF COMMINFO IS $C3, COMPARE RECV COMMINFO TO $C4
{
jmp eq,comminfo_c3_handler,nop
cmp #$000000c4,r1
}
;JUMP TO COMMINFO_C4_HANDLER IF COMMINFO IS $C4
jmp eq,comminfo_c4_handler,nop

not_mpe3:

;PACKET WAS EITHER NOT FROM MPE3 OR THE COMMINFO WAS NOT $A0-$AF,$C0,$C2,OR $C4
;COMPARE COMMHOOK_HANDLER TO ZERO
cmp #$00000000,r3
;JUMP TO THE COMMHOOK HANDLER IF THE POINTER IS NOT ZERO
jmp ne,(r3),nop
;R8 = NEXT QUEUE2 HEAD = CURRENT QUEUE2 HEAD + $20
add #$00000020,r10,r8
;COMPARE NEXT QUEUE2 HEAD ADDRESS TO $20101E40
cmp #beyond_comm_packet_queue2,r8
;BRANCH IF NEXT QUEUE2 HEAD ADDRESS IS NOT $20101E40
bra ne,no_adjust,nop
;ADJUST NEXT QUEUE2 TAIL ADDRESS TO $20101DA0
mv_s #comm_packet_queue2,r8

no_adjust:
;COMPARE NEXT QUEUE2 TAIL ADDRESS TO CURRENT QUEUE2 HEAD ADDRESS
cmp r8,r11
;BRANCH TO EPILOGUE IF THE NEXT QUEUE1 TAIL ADDRESS EQUAL THE CURRENT HEAD ADDRESS
bra eq,minilevel2_epilogue,nop

;STORE FIRST VECTOR OF QUEUE1 PACKET DATA TO CURRENT QUEUE2 HEAD ENTRY
{
st_v v0,(r10)
add #$00000010,r10
}
;STORE SECOND VECTOR OF QUEUE1 PACKET DATA TO CURRENT QUEUE2 HEAD ENTRY
st_v v1,(r10)
;STORE UPDATED QUEUE2 TAIL BACK INTO MEMORY AT $20101C18
st_s r8,comm_packet_queue2_tail

minilevel2_epilogue:
pop r31,cc,rzi2,rz
pop v2
{
pop v1
rti rzi2
}
pop v0
nop

;.origin $20301c96
intsrc3handler:
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
;BRANCH BACK TO TRIGGER_XMIT IF TRANSMIT FAILED BIT IS SET, TEST BIT 5 OF TRANSMIT TARGET ID (IS IT GREATER THAN 31?)
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
;TO THE INTERRUPT, 32 WILL BE ADDED TO THE TARGET ID (FOR WHATEVER REASON)
{
st_s r8,commctl
rts
}
pop v2
st_s r7,comminfo

;.origin $20301d96
minicommrecv:
;BRANCH TO COMMRECV_COMMON
bra commrecv_common
;R6 = FAILURE ADDRESS = CHECK_FOR_AVAILABLE_PACKET
mv_s #check_for_available_packet,r6
nop

;.origin $20301da0
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
;COMPARE NEW QUEUE2 HEAD TO BEYOND_COMM_PACKET_QUEUE2 ($20101E40)
cmp #beyond_comm_packet_queue2,r11
;BRANCH TO NO_ADJUST IF NEW QUEUE2 HEAD IS NOT EQUAL TO BEYOND_COMM_PACKET_QUEUE2
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

comminfod0handler:
rts nop

cdihandler:
rts nop

intsrc2handler:
{
st_s #$00000004,intclr
rts nop
}

.section djumptab
.origin $20301fc8
dma_jumptable:
jmp minidmalinearread, nop
jmp minidmalinearread_wait, nop
jmp minidmalinearwrite, nop
jmp minidmalinearwrite_wait, nop
jmp miniwaitodma, nop
jmp miniwaitmdma, nop

.section jumptabX
.origin $20301fe0
minibios_jumptable:
jmp minicommhook, nop
jmp minicommsend, nop
jmp minicommrecvquery, nop
jmp minicommrecv, nop
{
  mv_s #5, r0
  rts nop
}

.origin $20301ff4
minilevel1wrapper:
{
push v3
jmp minilevel1handler
}
ld_s cc,r12
push v2
nop

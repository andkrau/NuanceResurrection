;r0 = test status register
;r1 = stack pointer (scratch buffer location, 256 bytes minimum)
;r2 = test number
;r3 = saved return address for C calling convention
;r4 - r26: available for test purposes
;r27 = branch target register
;r28 = result register
;r29 = expected result register
;r30 = result flags register
;r31 = expected flags register

;Status of $FEEDF00D indicates that testing is in progress
;Status of $DEADBEEF indicates that a instruction test failed
;Status of $DEADF00D most likely indicates that the implementation
;  of jmp ne, <label>, nop is broken
;Status of $70031337 indicates that all tests passed successfully
;
;The tests attempt to exercise each Nuon instruction form in terms
;of flag setting and operation results.  Flags are always tested
;before the results of the operation are tested.

noflags = $00
zf = $01
cf = $02
vf = $04
nf = $08
mvf = $10
c0zf = $20
c1zf = $40
modgef = $80
modmif = $100
cp0f = $200
cp1f = $400
;Don't include the coprocessor bits or the reserved bits in comparisons
allflags = (zf+cf+vf+nf+mvf+c0zf+c1zf+modgef+modmif)

testStatusReg = r0
scratchBufferReg = r1
testNumberReg = r2
returnAddressReg = r3
branchTargetReg = r27
resultValueReg = r28
expectedResultReg = r29
resultFlagsReg = r30
expectedFlagsReg = r31

.nooptimize

.macro SetTestNumber testNum
	mv_s #testNum, testNumberReg
.mend

.macro SetStatus newStatus
	mv_s #newStatus, testStatusReg
.mend

.macro LoadTestReg value, testReg
	mv_s #value, testReg
.mend

.macro LoadControlReg value, controlReg
  st_s #value, controlReg
  nop
.mend

.macro LoadCounterRegs reg0val, reg1val
	st_s #reg0val, rc0
	st_s #reg1val, rc1
	nop
.mend

.macro LoadFlags value
	st_s #value, cc
	nop
.mend

.macro ReadIndexRegs regX,regY,regU,regV
	ld_s rx, regX
	ld_s ry, regY
	ld_s ru, regU
	ld_s rv, regV
	nop
.mend

.macro LoadIndexReg value, indexReg
	st_s #value, indexReg
	nop
.mend

.macro LoadIndexRegs xval,yval,uval,vval
  st_s #xval, rx
  st_s #yval, ry
  st_s #uval, ru
  st_s #vval, rv
  nop
.mend

.macro ReadCounterRegs regrc0,regrc1
	ld_s rc0, regrc0
	ld_s rc1, regrc1
	nop
.mend

.macro StoreResult testReg
	mv_s testReg, resultValueReg
.mend

;the TestFlags macro automatically adjusts the expectedFlags
;input parameter to account for the current state of the rc0 and
;rc1 counter registers.  If the adjusted expectedFlags do not match
;the value in the resultFlagsReg register, the error handler is
;called

;.macro TestFlags expectedFlags
;	ld_s cc, resultFlagsReg
;	nop
;	mv_s resultFlagsReg, expectedFlagsReg
;	and #(c0zf+c1zf), expectedFlagsReg
;	or #(expectedFlags & ~(c0zf+c1zf)), expectedFlagsReg
;	cmp expectedFlagsReg, resultFlagsReg
;	jmp ne, error, nop
;.mend

.macro TestFlags expectedFlags
	ld_s cc, resultFlagsReg
	nop
  and #allflags, resultFlagsReg
	mv_s #(expectedFlags & allflags), expectedFlagsReg
	cmp expectedFlagsReg, resultFlagsReg
	mv_s #error, branchTargetReg
	jmp ne, (branchTargetReg), nop
.mend

;the TestFlagsExact macro works nearly the same as the TestFlags
;macro except that the expectedFlags parameter is not modified,
;allowing for the c0z and c1z flags to be tested

.macro TestFlagsExact expectedFlags
	ld_s cc, resultFlagsReg
	nop
  and #allflags, resultFlagsReg
	mv_s #(expectedFlags & allflags), expectedFlagsReg
	cmp expectedFlagsReg, resultFlagsReg
	mv_s #error, branchTargetReg
	jmp ne, (branchTargetReg), nop
.mend

.macro TestResult expectedResult
  mv_s #expectedResult, expectedResultReg
	cmp #expectedResult, resultValueReg
	mv_s #error, branchTargetReg
	jmp ne, (branchTargetReg), nop
.mend

.export _nuontest

.text
.align.v

_nuontest:

{
copy r0, scratchBufferReg
ld_s rz, returnAddressReg
}
{
st_v v7, (scratchBufferReg)
add #16, scratchBufferReg
}
{
st_v v6, (scratchBufferReg)
add #16, scratchBufferReg
}
{
st_v v5, (scratchBufferReg)
add #16, scratchBufferReg
}
{
st_v v4, (scratchBufferReg)
add #16, scratchBufferReg
}
{
st_v v3, (scratchBufferReg)
}

SetStatus $DEADBEEF

test_abs:
`test_abs:

;abs(0), expect r4 = 0, zero flag set
SetTestNumber 1
LoadTestReg $0,r4
LoadFlags cf+vf+nf
abs r4
StoreResult r4
TestFlags zf
TestResult $0

;abs($80000000), expect r4 = $80000000, negative, carry and overflow flags set
SetTestNumber 2
LoadTestReg $80000000,r4
LoadFlags zf
abs r4
StoreResult r4
TestFlags cf+vf+nf
TestResult $80000000

;abs($FFFFFFFF), expect r4 = $00000001, carry flag set
SetTestNumber 3
LoadTestReg $FFFFFFFF,r4
LoadFlags zf+vf+nf
abs r4
StoreResult r4
TestFlags cf
TestResult $1
cmp #$1, r1

;abs($7FFFFFFF), expect r4 = $7FFFFFFF, no flags set
SetTestNumber 4
LoadTestReg $7FFFFFFF,r4
LoadFlags zf+vf+nf+cf
abs r4
StoreResult r4
TestFlags noflags
TestResult $7FFFFFFF

`test_addm:
;addm Ri, Rj, Rk
SetTestNumber 5
LoadTestReg 25,r4
LoadTestReg 32,r5
LoadTestReg 0,r6
LoadFlags allflags
addm r4,r5,r6
StoreResult r6
TestFlags allflags
TestResult 57

LoadTestReg 18,r4
LoadTestReg 62,r5
LoadTestReg 255,r6
LoadFlags noflags
addm r4,r5,r6
StoreResult r6
TestFlags noflags
TestResult 80

`test_subm:
;subm #32 - #25, expect #7
SetTestNumber 6
LoadTestReg 25,r4
LoadTestReg 32,r5
LoadTestReg 0,r6
LoadFlags allflags
subm r4,r5,r6
StoreResult r6
TestFlags allflags
TestResult 7

LoadTestReg 18,r4
LoadTestReg 62,r5
LoadTestReg 255,r6
LoadFlags noflags
subm r4,r5,r6
StoreResult r6
TestFlags noflags
TestResult 44

`test_not:
;not($FFFFFFFF): expect r4 = #$0, zf set, cf unchanged, vf cleared
SetTestNumber 7
LoadTestReg $FFFFFFFF,r4
LoadFlags vf+nf
not r4
StoreResult r4
TestFlags zf
TestResult $0

LoadTestReg $FFFFFFFF,r4
LoadFlags vf+nf+cf
not r4
StoreResult r4
TestFlags zf+cf
TestResult $0

;not(0): expect r4 = #$FFFFFFFF, nf set, cf unchanged, vf cleared
SetTestNumber 8
LoadTestReg $0,r4
LoadFlags vf+zf
not r4
StoreResult r4
TestFlags nf
TestResult $FFFFFFFF

LoadTestReg $0,r4
LoadFlags vf+zf+cf
not r4
StoreResult r4
TestFlags nf+cf
TestResult $FFFFFFFF

;not($80000000): expect r4 = #$7FFFFFFF, cf unchanged, vf cleared
SetTestNumber 9
LoadTestReg $80000000,r4
LoadFlags vf+zf+nf
not r4
StoreResult r4
TestFlags noflags
TestResult $7FFFFFFF

LoadTestReg $80000000,r4
LoadFlags vf+zf+nf+cf
not r4
StoreResult r4
TestFlags cf
TestResult $7FFFFFFF

`test_neg:
;neg($FFFFFFFF): expect r4 = #$1, cf set
SetTestNumber 10
LoadTestReg $FFFFFFFF,r4
LoadFlags vf+zf+nf
neg r4
StoreResult r4
TestFlags cf
TestResult $1

;neg($0): expect r4 = #$00000000, zf set
SetTestNumber 11
LoadTestReg $0,r4
LoadFlags vf+cf+zf
neg r4
StoreResult r4
TestFlags zf
TestResult $0

LoadTestReg $0,r4
LoadFlags nf+vf+cf
neg r4
StoreResult r4
TestFlags zf
TestResult $0

;neg($80000000): expect r1 = #$80000000, nf set, vf set, cf set
SetTestNumber 12
LoadTestReg $80000000, r4
LoadFlags zf
neg r4
StoreResult r4
TestFlags nf+vf+cf
TestResult $80000000

;neg($7FFFFFFF): expect r4 = #$80000001, nf set, cf set
SetTestNumber 13
LoadTestReg $7FFFFFFF,r4
LoadFlags zf+vf
neg r4
StoreResult r4
TestFlags nf+cf
TestResult $80000001

`test_copy:

;copy r4=0 to r5, expect r5 = $0, zf set, vf cleared, cf unchanged
SetTestNumber 14
LoadTestReg $0,r4
LoadTestReg $FFFFFFFF,r5
LoadFlags nf+vf
copy r4, r5
StoreResult r5
TestFlags zf
TestResult $0

LoadTestReg $0,r4
LoadTestReg $FFFFFFFF,r5
LoadFlags nf+vf+cf
copy r4, r5
StoreResult r5
TestFlags cf+zf
TestResult $0

;copy r4=$80000000 to r5, expect r5 = $80000000, nf set, vf cleared, cf unchanged
SetTestNumber 15
LoadTestReg $80000000,r4
LoadTestReg $7FFFFFFF,r5
LoadFlags vf+zf
copy r4, r5
StoreResult r5
TestFlags nf
TestResult $80000000

LoadTestReg $80000000,r4
LoadTestReg $7FFFFFFF,r5
LoadFlags vf+cf+zf
copy r4, r5
StoreResult r5
TestFlags nf+cf
TestResult $80000000

;copy r4=$7FFFFFFF to r5, expect r5 = $7FFFFFFF, vf cleared, cf unchanged
SetTestNumber 16
LoadTestReg $7FFFFFFF,r4
LoadTestReg $80000000,r5
LoadFlags vf
copy r4, r5
StoreResult r5
TestFlags noflags
TestResult $7FFFFFFF

LoadTestReg $7FFFFFFF,r4
LoadTestReg $80000000,r5
LoadFlags vf+cf
copy r4, r5
StoreResult r5
TestFlags cf
TestResult $7FFFFFFF

`test_mv_s:

;mv_s Sj, Sk
SetTestNumber 17
LoadTestReg $5A5A5A5A,r4
LoadTestReg $0,r5
LoadFlags noflags
mv_s r4, r5
StoreResult r5
TestFlags noflags
TestResult $5A5A5A5A

;mv_s #n, Sk
SetTestNumber 18
LoadTestReg $F,r4
LoadTestReg $0,r5
LoadFlags allflags
mv_s r4, r5
StoreResult r5
TestFlags allflags
TestResult $F

;mv_s #nnn, Sk ((16 <= nnn <= 2047) or (-2048 <= nnnn <= -17)
SetTestNumber 19
LoadTestReg $FF,r4
LoadTestReg $0,r5
LoadFlags allflags
mv_s r4, r5
StoreResult r5
TestFlags allflags
TestResult $FF

;mv_s #nnnn, Sk ((2048 <= nnn <= (2^31 - 1)) or (-(2^31) <= nnnn <= -2049)
SetTestNumber 20
LoadTestReg $76543210,r4
LoadTestReg $0,r5
LoadFlags allflags
mv_s r4, r5
StoreResult r5
TestFlags allflags
TestResult $76543210

test_mv_v:
`test_mv_v:

;mv_v Vj, Vk
SetTestNumber 21
LoadTestReg $5A5A5A5A,r4
LoadTestReg $A5A5A5A5,r5
LoadTestReg $89ABCDEF,r6
LoadTestReg $01234567,r7
LoadTestReg $0,r8
LoadTestReg $0,r9
LoadTestReg $0,r10
LoadTestReg $0,r11
LoadFlags noflags
mv_v v1, v2
StoreResult r4
TestFlags noflags
TestResult $5A5A5A5A
StoreResult r5
TestResult $A5A5A5A5
StoreResult r6
TestResult $89ABCDEF
StoreResult r7
TestResult $01234567

LoadTestReg $5A5A5A5A,r4
LoadTestReg $A5A5A5A5,r5
LoadTestReg $89ABCDEF,r6
LoadTestReg $01234567,r7
LoadTestReg $0,r8
LoadTestReg $0,r9
LoadTestReg $0,r10
LoadTestReg $0,r11
LoadFlags allflags
mv_v v1, v2
StoreResult r4
TestFlags allflags
TestResult $5A5A5A5A
StoreResult r5
TestResult $A5A5A5A5
StoreResult r6
TestResult $89ABCDEF
StoreResult r7
TestResult $01234567

`test_mvr:

;mvr Sj, RI: expect rx = $12121212, ry = ru = rz = 0
SetTestNumber 22
LoadTestReg $12121212,r4
LoadIndexRegs $0,$0,$0,$0
LoadFlags allflags
mvr r4, rx
LoadTestReg $0,r4
ReadIndexRegs r4,r5,r6,r7
StoreResult r4
TestFlags allflags
TestResult $12121212
StoreResult r5
TestResult $0
StoreResult r6
TestResult $0
StoreResult r7
TestResult $0

;mvr Sj, RI: expect ry = $12121212, rx = ru = rz = 0
LoadTestReg $12121212,r5
LoadIndexRegs $FFFFFFFF,$0,$FFFFFFFF,$FFFFFFFF
LoadFlags noflags
mvr r5, ry
LoadTestReg $0,r5
ReadIndexRegs r4,r5,r6,r7
StoreResult r4
TestFlags noflags
TestResult $FFFFFFFF
StoreResult r5
TestResult $12121212
StoreResult r6
TestResult $FFFFFFFF
StoreResult r7
TestResult $FFFFFFFF

;mvr Sj, RI: expect ru = $12121212, rx = ry = rz = 0
LoadTestReg $12121212,r6
LoadIndexRegs $0,$0,$0,$0
LoadFlags noflags
mvr r6, ru
LoadTestReg $0,r6
ReadIndexRegs r4,r5,r6,r7
StoreResult r4
TestFlags noflags
TestResult $0
StoreResult r5
TestResult $0
StoreResult r6
TestResult $12121212
StoreResult r7
TestResult $0

;mvr Sj, RI: expect rv = $12121212, rx = ry = ru = 0
LoadTestReg $12121212,r7
LoadIndexRegs $0,$0,$0,$0
LoadFlags noflags
mvr r7, rv
LoadTestReg $0,r7
ReadIndexRegs r4,r5,r6,r7
StoreResult r4
TestFlags noflags
TestResult $0
StoreResult r5
TestResult $0
StoreResult r6
TestResult $0
StoreResult r7
TestResult $12121212

;mvr #nnnn, RI: expect rx = $89ABCDEF, ry = ru = rv = 0
SetTestNumber 23
LoadIndexRegs $0,$0,$0,$0
LoadFlags noflags
mvr #$89ABCDEF, rx
LoadTestReg $0,r4
ReadIndexRegs r4,r5,r6,r7
StoreResult r4
TestFlags noflags
TestResult $89ABCDEF
StoreResult r5
TestResult $0
StoreResult r6
TestResult $0
StoreResult r7
TestResult $0

`test_dec:

;dec rc0: expect rc0 = $CDEE, rc1 = $5670, cc = $0
SetTestNumber 24
LoadCounterRegs $CDEF,$5670
LoadFlags c0zf+c1zf
dec rc0
ReadCounterRegs r4,r5
StoreResult r4
TestFlagsExact c1zf
TestResult $CDEE
StoreResult r5
TestResult $5670

;dec rc1: expect rc0 = $CDEF, rc1 = $566F, c0zf unchanged, c1zf clear
SetTestNumber 25
LoadCounterRegs $CDEF,$5670
LoadFlags c0zf+c1zf
dec rc1
ReadCounterRegs r4,r5
StoreResult r4
TestFlagsExact c0zf
TestResult $CDEF
StoreResult r5
TestResult $566F

;dec rc0, rc1: expect rc0 = $CDEE, rc1 = $566F, cc = $0
SetTestNumber 26
LoadCounterRegs $CDEF,$5670
LoadFlags c0zf+c1zf
{
dec rc1
dec rc0
}
ReadCounterRegs r4,r5
StoreResult r4
TestFlagsExact noflags
TestResult $CDEE
StoreResult r5
TestResult $566F

;dec rc0, rc1: expect rc0 = $0, rc1 = $0, cc = $60
SetTestNumber 27
LoadCounterRegs $1,$1
LoadFlags noflags
{
dec rc1
dec rc0
}
ReadCounterRegs r4,r5
StoreResult r4
TestFlagsExact c0zf+c1zf
TestResult $0
StoreResult r5
TestResult $0

;dec rc0, rc1: both counters already zero, expect rc0 = $0, rc1 = $0, cc = $60
SetTestNumber 28
LoadCounterRegs $0,$0
LoadFlags noflags
{
dec rc1
dec rc0
}
ReadCounterRegs r4,r5
StoreResult r4
TestFlagsExact c0zf+c1zf
TestResult $0
StoreResult r5
TestResult $0

`test_addr:
SetTestNumber 29

;addr Si, RI
LoadTestReg $12345678,r4
LoadIndexReg $89ABCDEF,rx
LoadFlags allflags
addr r4, rx
ReadIndexRegs r4,r5,r6,r7
StoreResult r4
TestFlags allflags
TestResult $9BE02467

LoadTestReg $82345678,r4
LoadIndexReg $7DCBA988,ry
LoadFlags noflags
addr r4, ry
ReadIndexRegs r4,r5,r6,r7
StoreResult r5
TestFlags noflags
TestResult $0

;addr #nnnn, RI

SetTestNumber 30

LoadIndexReg $89ABCDEF,ru
LoadFlags allflags
addr #$12345678, ru
ReadIndexRegs r4,r5,r6,r7
StoreResult r6
TestFlags allflags
TestResult $9BE02467

LoadIndexReg $7DCBA988,rv
LoadFlags noflags
addr #$82345678, rv
ReadIndexRegs r4,r5,r6,r7
StoreResult r7
TestFlags noflags
TestResult $0

;addr #(n << 16), RI

SetTestNumber 31

LoadIndexReg $89ABCDEF,rx
LoadFlags allflags
addr #(15 << 16), rx
ReadIndexRegs r4,r5,r6,r7
StoreResult r4
TestFlags allflags
TestResult $89BACDEF

LoadIndexReg $7DCBA988,ry
LoadFlags noflags
addr #(-16 << 16), ry
ReadIndexRegs r4,r5,r6,r7
StoreResult r5
TestFlags noflags
TestResult $7DBBA988


;modulo RI

`test_modulo:

;less than zero (rx = -10.FFFF, xrange = 521): expect rx = 511.FFFF, modmi set, modge cleared

SetTestNumber 32
;xrange = 521, yrange = 1022
LoadControlReg ((521 << 16) | (1022 << 0)), xyrange
;urange = 10, vrange = 11
LoadControlReg ((10 << 16) | (11 << 0)), uvrange

LoadIndexRegs ((-10 << 16) | $FFFF),(20 << 16),(30 << 16),(40 << 16)
LoadFlags modgef
modulo rx
ReadIndexRegs r4,r5,r6,r7
StoreResult r4
TestFlags modmif
TestResult ((511 << 16) | $FFFF)

;equal to range (ry = 1022.FFFF, yrange = 1022): expect ry = 0.FFFF, modmi cleared, modge set

SetTestNumber 33
;xrange = 521, yrange = 1022
LoadControlReg ((521 << 16) | (1022 << 0)), xyrange
;urange = 10, vrange = 11
LoadControlReg ((10 << 16) | (11 << 0)), uvrange

LoadIndexRegs (-10 << 16),((1022 << 16) | 0xFFFF),(30 << 16),(40 << 16)
LoadFlags modmif
modulo ry
ReadIndexRegs r4,r5,r6,r7
StoreResult r5
TestFlags modgef
TestResult $0000FFFF

;greater than range (ru = 15.0, urange = 10): expect ru = 5.0, modmi cleared, modge set

SetTestNumber 34
;xrange = 521, yrange = 1022
LoadControlReg ((521 << 16) | (1022 << 0)), xyrange
;urange = 10, vrange = 11
LoadControlReg ((10 << 16) | (11 << 0)), uvrange

LoadIndexRegs 0,0,(15 << 16),0
LoadFlags (allflags & ~modgef)
modulo ru
ReadIndexRegs r4,r5,r6,r7
StoreResult r6
TestFlags (allflags & ~modmif)
TestResult (5 << 16)

;within range (rv = 3.0, vrange = 11): expect rv = 3.0, modmi and modge cleared

SetTestNumber 35
;xrange = 521, yrange = 1022
LoadControlReg ((521 << 16) | (1022 << 0)), xyrange
;urange = 10, vrange = 11
LoadControlReg ((10 << 16) | (11 << 0)), uvrange

LoadIndexRegs (-1 << 16),(-1 << 16),(-1 << 16),(3 << 16)
LoadFlags (modmif|modgef)
modulo rv
ReadIndexRegs r4,r5,r6,r7
StoreResult r7
TestFlags noflags
TestResult (3 << 16)

;range RI

`test_range:

;less than zero (rx = -10.0, xrange = 521): expect rx = -10.0, modmi set, modge cleared

SetTestNumber 36
;xrange = 521, yrange = 1022
LoadControlReg ((521 << 16) | (1022 << 0)), xyrange
;urange = 10, vrange = 11
LoadControlReg ((10 << 16) | (11 << 0)), uvrange

LoadIndexRegs (-10 << 16),(20 << 16),(30 << 16),(40 << 16)
LoadFlags modgef
range rx
ReadIndexRegs r4,r5,r6,r7
StoreResult r4
TestFlags modmif
TestResult (-10 << 16)

;equal to range (ry = 1022.0, yrange = 1022): expect ry = 1022.0, modmi cleared, modge set

SetTestNumber 37
;xrange = 521, yrange = 1022
LoadControlReg ((521 << 16) | (1022 << 0)), xyrange
;urange = 10, vrange = 11
LoadControlReg ((10 << 16) | (11 << 0)), uvrange

LoadIndexRegs (-10 << 16),(1022 << 16),(30 << 16),(40 << 16)
LoadFlags modmif
range ry
ReadIndexRegs r4,r5,r6,r7
StoreResult r5
TestFlags modgef
TestResult (1022 << 16)

;greater than range (ru = 15.0, urange = 10): expect ru = 5.0, modmi cleared, modge set

SetTestNumber 38
;xrange = 521, yrange = 1022
LoadControlReg ((521 << 16) | (1022 << 0)), xyrange
;urange = 10, vrange = 11
LoadControlReg ((10 << 16) | (11 << 0)), uvrange

LoadIndexRegs 0,0,(15 << 16),0
LoadFlags (allflags & ~modgef)
range ru
ReadIndexRegs r4,r5,r6,r7
StoreResult r6
TestFlags (allflags & ~modmif)
TestResult (15 << 16)

;within range (rv = 3.0, vrange = 11): expect rv = 3.0, modmi and modge cleared

SetTestNumber 39
;xrange = 521, yrange = 1022
LoadControlReg ((521 << 16) | (1022 << 0)), xyrange
;urange = 10, vrange = 11
LoadControlReg ((10 << 16) | (11 << 0)), uvrange

LoadIndexRegs (-1 << 16),(-1 << 16),(-1 << 16),(3 << 16)
LoadFlags (modmif|modgef)
range rv
ReadIndexRegs r4,r5,r6,r7
StoreResult r7
TestFlags noflags
TestResult (3 << 16)

`test_msb:

;msb Si, Sk
SetTestNumber 40
LoadTestReg $0,r4
LoadTestReg $FFFFFFFF,r5
LoadFlags noflags
msb r4, r5
StoreResult r5
TestFlags zf
TestResult $0

LoadTestReg $FFFFFFFF,r4
LoadTestReg $FFFFFFFF,r5
LoadFlags (allflags & ~zf)
msb r4, r5
StoreResult r5
TestFlags allflags
TestResult $0

LoadTestReg $0007F00F,r4
LoadTestReg $FFFFFFFF,r5
LoadFlags noflags
msb r4, r5
StoreResult r5
TestFlags noflags
TestResult 19

LoadTestReg $FFFFF90C,r4
LoadTestReg $FFFFFFFF,r5
LoadFlags noflags
msb r4, r5
StoreResult r5
TestFlags noflags
TestResult 11

`test_sat:

;sat #n, Si, Sk

;positive saturation; expect r5 = $7FFF, nf and zf cleared, cf and vf unchanged
SetTestNumber 41
LoadTestReg $04F08310,r4
LoadTestReg $FFFF8000,r5
LoadFlags zf+nf+cf+vf
sat #16, r4, r5
StoreResult r5
TestFlags cf+vf
TestResult $7FFF

;negative saturation; expect r5 = $FFFF8000, nf set, zf cleared
LoadTestReg $FF000000,r4
LoadTestReg $00007FFF,r5
LoadFlags zf
sat #16, r4, r5
StoreResult r5
TestFlags nf
TestResult $FFFF8000

;no saturation: expect r5 = $FFFFFFFD, nf set, zf cleared
LoadTestReg $FFFFFFFD,r4
LoadTestReg $00000002,r5
LoadFlags zf
sat #3, r4, r5
StoreResult r5
TestFlags nf
TestResult $FFFFFFFD

;no saturation: expect r5 = $0, nf cleared, zf set
LoadTestReg $0,r4
LoadTestReg $FFFFFFFF,r5
LoadFlags nf
sat #3, r4, r5
StoreResult r5
TestFlags zf
TestResult $0

`test_bits:

;bits #n, >>#m, Sk

;bits #31, >>#0, $89ABCDEF, expect r4 =$89ABCDEF , nf set, zf cleared
SetTestNumber 42
LoadTestReg $89ABCDEF,r4
LoadFlags (allflags & ~nf)
bits #31, >>#0, r4
StoreResult r4
TestFlags (allflags & ~zf)
TestResult $89ABCDEF

;bits #31, >>#31, $7FFFFFFF, expect r4 = 0 , zf set
LoadTestReg $7FFFFFFF,r4
LoadFlags noflags
bits #31, >>#31, r4
StoreResult r4
TestFlags zf
TestResult 0

;bits #5, >>#8, $FFFFF5FF, expect r4 = $00000035, nf cleared, zf cleared
LoadTestReg $FFFFF5FF,r4
LoadFlags nf+zf
bits #5, >>#8, r4
StoreResult r4
TestFlags noflags
TestResult $35

;bits #n, >>Si, Sk

;bits #31, >>#0, $89ABCDEF, expect r4 =$89ABCDEF , nf set, zf cleared
SetTestNumber 43
LoadTestReg $89ABCDEF,r4
LoadTestReg 0, r5
LoadFlags (allflags & ~nf)
bits #31, >>r5, r4
StoreResult r4
TestFlags (allflags & ~zf)
TestResult $89ABCDEF

;bits #31, >>#31, $7FFFFFFF, expect r4 = 0 , zf set
LoadTestReg $7FFFFFFF,r4
LoadTestReg 31,r5
LoadFlags noflags
bits #31, >>r5, r4
StoreResult r4
TestFlags zf
TestResult 0

;bits #5, >>#8, $FFFFF5FF, expect r4 = $00000035, nf cleared, zf cleared
LoadTestReg $FFFFF5FF,r4
LoadTestReg 8, r5
LoadFlags nf+zf
bits #5, >>r5, r4
StoreResult r4
TestFlags noflags
TestResult $35

`test_add_p:

;add_p Vi,Vj,Vk

;add_p v1,v2,v3
;v1 = ($1234FFFE, $89ABCDEF, $34561234, $54321000)
;v2 = ($77890001, $76543211, $0000FFFF, $ABCDEFFF)
;v3 = ($FFFFFFFF, $0000FFFF, $FFFFFFFF, $A5A5A5A5)
;expect v3 = ($89BD0000, $FFFF0000, $34560000, $A5A5A5A5)
SetTestNumber 44
LoadTestReg $1234FFFE,r4
LoadTestReg $89ABCDEF,r5
LoadTestReg $34561234,r6
LoadTestReg $54321000,r7
LoadTestReg $77890001,r8
LoadTestReg $76543211,r9
LoadTestReg $0000FFFF,r10
LoadTestReg $ABCDEFFF,r11
LoadTestReg $FFFFFFFF,r12
LoadTestReg $0000FFFF,r13
LoadTestReg $FFFFFFFF,r14
LoadTestReg $A5A5A5A5,r15
LoadFlags (allflags)
add_p v1,v2,v3
StoreResult r12
TestFlags (allflags)
TestResult $89BD0000
StoreResult r13
TestResult $FFFF0000
StoreResult r14
TestResult $34560000
StoreResult r15
TestResult $A5A5A5A5

`test_add_sv:

;add_sv Vi,Vk

;add_sv v1,v2
;v1 = ($1234FFFE, $89ABCDEF, $34561234, $54321000)
;v2 = ($77890001, $76543211, $0000FFFF, $ABCDEFFF)
;expect v2 = ($89BD0000, $FFFF0000, $34560000, $FFFF0000)
SetTestNumber 45
LoadTestReg $1234FFFE,r4
LoadTestReg $89ABCDEF,r5
LoadTestReg $34561234,r6
LoadTestReg $54321000,r7
LoadTestReg $77890001,r8
LoadTestReg $76543211,r9
LoadTestReg $0000FFFF,r10
LoadTestReg $ABCDEFFF,r11
LoadFlags (allflags)
add_sv v1,v2
StoreResult r8
TestFlags (allflags)
TestResult $89BD0000
StoreResult r9
TestResult $FFFF0000
StoreResult r10
TestResult $34560000
StoreResult r11
TestResult $FFFF0000

;add_sv Vi,Vj,Vk

;add_sv v1,v2,v3
;v1 = ($1234FFFE, $89ABCDEF, $34561234, $54321000)
;v2 = ($77890001, $76543211, $0000FFFF, $ABCDEFFF)
;v3 = ($FFFFFFFF, $0000FFFF, $FFFFFFFF, $0000FFFF)
;expect v3 = ($89BD0000, $FFFF0000, $34560000, $FFFF0000)
SetTestNumber 46
LoadTestReg $1234FFFE,r4
LoadTestReg $89ABCDEF,r5
LoadTestReg $34561234,r6
LoadTestReg $54321000,r7
LoadTestReg $77890001,r8
LoadTestReg $76543211,r9
LoadTestReg $0000FFFF,r10
LoadTestReg $ABCDEFFF,r11
LoadTestReg $FFFFFFFF,r12
LoadTestReg $0000FFFF,r13
LoadTestReg $FFFFFFFF,r14
LoadTestReg $0000FFFF,r15
LoadFlags (allflags)
add_sv v1,v2,v3
StoreResult r12
TestFlags (allflags) 
TestResult $89BD0000
StoreResult r13
TestResult $FFFF0000
StoreResult r14
TestResult $34560000
StoreResult r15
TestResult $FFFF0000

test_sub_p:
`test_sub_p:

;sub_p Vi,Vj,Vk

;sub_p v1,v2,v3
;v1 = ($1234FFFE, $89ABCDEF, $34561234, $54321000)
;v2 = ($77890001, $76543211, $0000FFFF, $ABCDEFFF)
;v3 = ($FFFFFFFF, $0000FFFF, $FFFFFFFF, $A5A5A5A5)
;expect v3 = ($65550000, $ECA90000, $CBAA0000, $A5A5A5A5) 
SetTestNumber 47
LoadTestReg $1234FFFE,r4
LoadTestReg $89ABCDEF,r5
LoadTestReg $34561234,r6
LoadTestReg $54321000,r7
LoadTestReg $77890001,r8
LoadTestReg $76543211,r9
LoadTestReg $0000FFFF,r10
LoadTestReg $ABCDEFFF,r11
LoadTestReg $FFFFFFFF,r12
LoadTestReg $0000FFFF,r13
LoadTestReg $FFFFFFFF,r14
LoadTestReg $A5A5A5A5,r15
LoadFlags (allflags)
sub_p v1,v2,v3
StoreResult r12
TestFlags (allflags) 
TestResult $65550000
StoreResult r13
TestResult $ECA90000
StoreResult r14
TestResult $CBAA0000
StoreResult r15
TestResult $A5A5A5A5

test_sub_sv:
`test_sub_sv:

;sub_sv Vi,Vk

;sub_sv v1,v2
;v1 = ($1234FFFE, $89ABCDEF, $34561234, $54321000)
;v2 = ($77890001, $76543211, $0000FFFF, $ABCDEFFF)
;expect v2 = ($65550000, $ECA90000, $CBAA0000, $579BA5A5)
SetTestNumber 48
LoadTestReg $1234FFFE,r4
LoadTestReg $89ABCDEF,r5
LoadTestReg $34561234,r6
LoadTestReg $54321000,r7
LoadTestReg $77890001,r8
LoadTestReg $76543211,r9
LoadTestReg $0000FFFF,r10
LoadTestReg $ABCDEFFF,r11
LoadFlags (allflags)
sub_sv v1,v2
StoreResult r8
TestFlags (allflags)
TestResult $65550000
StoreResult r9
TestResult $ECA90000
StoreResult r10
TestResult $CBAA0000
StoreResult r11
TestResult $579B0000

;sub_sv Vi,Vj,Vk

;sub_sv v1,v2,v3
;v1 = ($1234FFFE, $89ABCDEF, $34561234, $54321000)
;v2 = ($77890001, $76543211, $0000FFFF, $ABCDEFFF)
;v3 = ($FFFFFFFF, $0000FFFF, $FFFFFFFF, $A5A5A5A5)
;expect v3 = ($65550000, $ECA90000, $CBAA0000, $579BA5A5)
SetTestNumber 49
LoadTestReg $1234FFFE,r4
LoadTestReg $89ABCDEF,r5
LoadTestReg $34561234,r6
LoadTestReg $54321000,r7
LoadTestReg $77890001,r8
LoadTestReg $76543211,r9
LoadTestReg $0000FFFF,r10
LoadTestReg $ABCDEFFF,r11
LoadTestReg $FFFFFFFF,r12
LoadTestReg $0000FFFF,r13
LoadTestReg $FFFFFFFF,r14
LoadTestReg $A5A5A5A5,r15
LoadFlags (allflags)
sub_sv v1,v2,v3
StoreResult r12
TestFlags (allflags)
TestResult $65550000
StoreResult r13
TestResult $ECA90000
StoreResult r14
TestResult $CBAA0000
StoreResult r15
TestResult $579B0000

`test_ls:

;ls >>Sj, Si, Sk

;ls >>5, $84A5A51E, r6: expect r6 = $4252D28, nf, zf, cf and vf cleared
SetTestNumber 50
LoadTestReg 5,r4
LoadTestReg $84A5A51E,r5
LoadTestReg $0,r6
LoadFlags (allflags)
ls >>r4, r5, r6
StoreResult r6
TestFlags (allflags & ~(nf+vf+cf+zf))
TestResult $04252D28

;ls >>31, $700FF00F, r6: expect r6 = $0, nf, vf cleared, zf, cf set
LoadTestReg 31,r4
LoadTestReg $700FF00F,r5
LoadTestReg $FFFFFFFF,r6
LoadFlags (vf+nf)
ls >>r4, r5, r6
StoreResult r6
TestFlags cf+zf
TestResult 0

;ls >>-32, $7FFFFFFF, r6: expect r6 = $0, nf, cf, vf cleared, zf set
LoadTestReg -32,r4
LoadTestReg $7FFFFFFF,r5
LoadTestReg $FFFFFFFF,r6
LoadFlags (vf+cf+nf)
ls >>r4, r5, r6
StoreResult r6
TestFlags zf
TestResult $0

;ls >>-31, $80000001, r6: expect r6 = $80000000, vf, zf cleared, cf, nf set
LoadTestReg -31,r4
LoadTestReg $80000001,r5
LoadTestReg $7FFFFFFE,r6
LoadFlags (allflags & ~nf)
ls >>r4, r5, r6
StoreResult r6
TestFlags (allflags & ~(zf+vf))
TestResult $80000000

;ls >>-2, $80000001, r6: expect r6 = $4, vf, zf, nf cleared, cf set
LoadTestReg -2,r4
LoadTestReg $80000001,r5
LoadTestReg $7FFFFFFE,r6
LoadFlags (allflags & ~cf)
ls >>r4, r5, r6
StoreResult r6
TestFlags (allflags & ~(zf+vf+nf))
TestResult $4

`test_as:

;as >>Sj, Si, Sk

;as >>5, $84A5A51E, r6: expect r6 = $FC252D28, zf, cf and vf cleared, nf set
SetTestNumber 51
LoadTestReg 5,r4
LoadTestReg $84A5A51E,r5
LoadTestReg $0,r6
LoadFlags (allflags & ~nf)
as >>r4, r5, r6
StoreResult r6
TestFlags (allflags & ~(vf+cf+zf))
TestResult $FC252D28

;as >>31, $0FFFFF01, r6: expect r6 = $0, nf, vf cleared, zf, cf set
LoadTestReg 31,r4
LoadTestReg $0FFFFF01,r5
LoadTestReg $FFFFFFFF,r6
LoadFlags (nf+vf)
as >>r4, r5, r6
StoreResult r6
TestFlags cf+zf
TestResult 0

;as >>-32, $7FFFFFFF, r6: expect r6 = $0, nf, cf, vf cleared, zf set
LoadTestReg -32,r4
LoadTestReg $7FFFFFFF,r5
LoadTestReg $FFFFFFFF,r6
LoadFlags (nf+cf+vf)
as >>r4, r5, r6
StoreResult r6
TestFlags zf
TestResult $0

;as >>-31, $80000001, r6: expect r6 = $80000000, vf, zf cleared, cf, nf set
LoadTestReg -31,r4
LoadTestReg $80000001,r5
LoadTestReg $7FFFFFFE,r6
LoadFlags (allflags & ~(cf+nf))
as >>r4, r5, r6
StoreResult r6
TestFlags (allflags & ~(zf+vf))
TestResult $80000000

;as >>-2, $80000001, r6: expect r6 = $4, vf, zf, nf cleared, cf set
LoadTestReg -2,r4
LoadTestReg $80000001,r5
LoadTestReg $7FFFFFFE,r6
LoadFlags (allflags & ~cf)
as >>r4, r5, r6
StoreResult r6
TestFlags (allflags & ~(zf+vf+nf))
TestResult $4

`test_lsr:

;lsr #m, Si, Sk

;lsr #5, $84A5A51E, r5: expect r5 = $4252D28, nf, zf, cf and vf cleared
SetTestNumber 52
LoadTestReg $84A5A51E,r4
LoadTestReg $0,r5
LoadFlags (allflags)
lsr #5, r4, r5
StoreResult r5
TestFlags (allflags & ~(nf+vf+cf+zf))
TestResult $04252D28

;lsr #31, $700FF00F, r5: expect r6 = $0, nf, vf cleared, zf, cf set
LoadTestReg $700FF00F,r4
LoadTestReg $FFFFFFFF,r5
LoadFlags (nf+vf)
lsr #31, r4, r5
StoreResult r5
TestFlags cf+zf
TestResult 0

;lsr #30, $3FFFFFFE, r5: expect r5 = $0, nf, cf, vf cleared, zf set
LoadTestReg $3FFFFFFE,r4
LoadTestReg $FFFFFFFF,r5
LoadFlags (vf+cf+nf)
lsr #30, r4, r5
StoreResult r5
TestFlags zf
TestResult 0

;lsr #0, $80000000, r5: expect r5 = $80000000, vf, zf, cf cleared, nf set
LoadTestReg $80000000,r4
LoadTestReg $7FFFFFFF,r5
LoadFlags (allflags & ~nf)
lsr #0, r4, r5
StoreResult r5
TestFlags (allflags & ~(zf+vf+cf))
TestResult $80000000

;lsr #0, $80000001, r5: expect r5 = $4, vf, zf cleared, nf,cf set
LoadTestReg $80000001,r4
LoadTestReg $7FFFFFFE,r5
LoadFlags (allflags & ~(nf+cf))
lsr #0, r4, r5
StoreResult r5
TestFlags (allflags & ~(zf+vf))
TestResult $80000001

;lsr #m, Sk

;lsr #5, $84A5A51E: expect r4 = $4252D28, nf, zf, cf and vf cleared
SetTestNumber 53
LoadTestReg $84A5A51E,r4
LoadFlags (allflags)
lsr #5, r4
StoreResult r4
TestFlags (allflags & ~(nf+vf+cf+zf))
TestResult $04252D28

;lsr #31, $700FF00F: expect r4 = $0, nf, vf cleared, zf, cf set
LoadTestReg $700FF00F,r4
LoadFlags (zf+cf)
lsr #31, r4
StoreResult r4
TestFlags cf+zf
TestResult 0

;lsr #30, $3FFFFFFE: expect r4 = $0, nf, cf, vf cleared, zf set
LoadTestReg $3FFFFFFE,r4
LoadFlags (vf+cf+nf)
lsr #30, r4
StoreResult r4
TestFlags zf
TestResult 0

;lsr #0, $80000000, r4: expect r4 = $80000001, vf, zf, cf cleared, nf set
LoadTestReg $80000000,r4
LoadFlags (allflags & ~nf)
lsr #0, r4
StoreResult r4
TestFlags (allflags & ~(zf+vf+cf))
TestResult $80000000

;lsr #0, $80000001: expect r4 = $4, vf, zf cleared, nf,cf set
LoadTestReg $80000001,r4
LoadFlags (allflags & ~(nf+cf))
lsr #0, r4
StoreResult r4
TestFlags (allflags & ~(zf+vf))
TestResult $80000001

`test_asr:

;asr #m, Si, Sk

;asr #5, $84A5A51E, r5: expect r5 = $FC252D28, zf, cf and vf cleared, nf set
SetTestNumber 54
LoadTestReg $84A5A51E,r4
LoadTestReg $0,r5
LoadFlags (allflags & ~nf)
asr #5, r4, r5
StoreResult r5
TestFlags (allflags & ~(vf+cf+zf))
TestResult $FC252D28

;asr #31, $700FF00F, r5: expect r6 = $0, nf, vf cleared, zf, cf set
LoadTestReg $700FF00F,r4
LoadTestReg $FFFFFFFF,r5
LoadFlags (nf+vf)
asr #31, r4, r5
StoreResult r5
TestFlags cf+zf
TestResult 0

;asr #30, $3FFFFFFE, r5: expect r5 = $0, nf, cf, vf cleared, zf set
LoadTestReg $3FFFFFFE,r4
LoadTestReg $FFFFFFFF,r5
LoadFlags (nf+cf+vf)
asr #30, r4, r5
StoreResult r5
TestFlags zf
TestResult 0

;asr #0, $80000000, r5: expect r5 = $80000001, vf, zf, cf cleared, nf set
LoadTestReg $80000000,r4
LoadTestReg $7FFFFFFE,r5
LoadFlags (allflags)
asr #0, r4, r5
StoreResult r5
TestFlags (allflags & ~(zf+vf+cf))
TestResult $80000000

;asr #0, $80000001, r5: expect r5 = $4, vf, zf cleared, nf,cf set
LoadTestReg $80000001,r4
LoadTestReg $7FFFFFFE,r5
LoadFlags (allflags)
asr #0, r4, r5
StoreResult r5
TestFlags (allflags & ~(zf+vf))
TestResult $80000001

;asr #m, Sk

;asr #5, $84A5A51E: expect r4 = $FC252D28, zf, cf and vf cleared, nf set
SetTestNumber 55
LoadTestReg $84A5A51E,r4
LoadFlags (allflags & ~nf)
asr #5, r4
StoreResult r4
TestFlags (allflags & ~(vf+cf+zf))
TestResult $FC252D28

;asr #31, $700FF00F: expect r4 = $0, nf, vf cleared, zf, cf set
LoadTestReg $700FF00F,r4
LoadFlags (nf+vf)
asr #31, r4
StoreResult r4
TestFlags cf+zf
TestResult 0

;asr #30, $3FFFFFFE: expect r4 = $0, nf, cf, vf cleared, zf set
LoadTestReg $3FFFFFFE,r4
LoadFlags (vf+cf+nf)
asr #30, r4
StoreResult r4
TestFlags zf
TestResult 0

;asr #0, $80000000, r4: expect r4 = $80000001, vf, zf, cf cleared, nf set
LoadTestReg $80000000,r4
LoadFlags (allflags & ~nf)
asr #0, r4
StoreResult r4
TestFlags (allflags & ~(zf+vf+cf))
TestResult $80000000

;asr #0, $80000001: expect r4 = $4, vf, zf cleared, nf,cf set
LoadTestReg $80000001,r4
LoadFlags (allflags & ~(nf+cf))
asr #0, r4
StoreResult r4
TestFlags (allflags & ~(zf+vf))
TestResult $80000001

test_lsl:
`test_lsl:

;lsl #m, Si, Sk

;lsl #5, $74A5A51E, r5: expect r5 = $94B4A3C0, zf, cf and vf cleared, nf set
SetTestNumber 56
LoadTestReg $74A5A51E,r4
LoadTestReg $0,r5
LoadFlags (allflags & ~nf)
lsl #5, r4, r5
StoreResult r5
TestFlags (allflags & ~(vf+cf+zf))
TestResult $94B4A3C0

;lsl #31, $F00FF00E, r5: expect r5 = $0, nf, vf cleared, zf, cf set
LoadTestReg $F00FF00E,r4
LoadTestReg $FFFFFFFF,r5
LoadFlags (vf+nf)
lsl #31, r4, r5
StoreResult r5
TestFlags cf+zf
TestResult 0

;lsl #30, $3FFFFFFC, r5: expect r5 = $0, nf, cf, vf cleared, zf set
LoadTestReg $3FFFFFFC,r4
LoadTestReg $FFFFFFFF,r5
LoadFlags (vf+cf+nf)
lsl #30, r4, r5
StoreResult r5
TestFlags zf
TestResult 0

;lsl #0, $80000000, r5: expect r5 = $80000000, vf, zf cleared, cf and nf set
LoadTestReg $80000000,r4
LoadTestReg $7FFFFFFE,r5
LoadFlags (allflags & ~(nf+cf))
lsl #0, r4, r5
StoreResult r5
TestFlags (allflags & ~(zf+vf))
TestResult $80000000

;lsl #0, $80000001, r5: expect r5 = $4, vf, zf cleared, nf,cf set
LoadTestReg $80000001,r4
LoadTestReg $7FFFFFFE,r5
LoadFlags (allflags & ~(nf+cf))
lsl #0, r4, r5
StoreResult r5
TestFlags (allflags & ~(zf+vf))
TestResult $80000001

;lsl #m, Sk

;lsl #5, $74A5A51E: expect r4 = $94B4A3C0, zf, cf and vf cleared, nf set
SetTestNumber 57
LoadTestReg $74A5A51E,r4
LoadFlags (allflags & ~nf)
lsl #5, r4
StoreResult r4
TestFlags (allflags & ~(vf+cf+zf))
TestResult $94B4A3C0

;lsl #31, $F00FF00E: expect r4 = $0, nf, vf cleared, zf, cf set
LoadTestReg $F00FF00E,r4
LoadFlags (noflags)
lsl #31, r4
StoreResult r4
TestFlags cf+zf
TestResult 0

;lsl #30, $3FFFFFFC: expect r4 = $0, nf, cf, vf cleared, zf set
LoadTestReg $3FFFFFFC,r4
LoadFlags (noflags)
lsl #30, r4
StoreResult r4
TestFlags zf
TestResult 0

;lsl #0, $80000000, r4: expect r4 = $80000001, vf, zf cleared, cf, nf set
LoadTestReg $80000000,r4
LoadFlags (allflags)
lsl #0, r4
StoreResult r4
TestFlags (allflags & ~(zf+vf))
TestResult $80000000

;lsl #0, $80000001: expect r4 = $4, vf, zf cleared, nf,cf set
LoadTestReg $80000001,r4
LoadFlags (allflags)
lsl #0, r4
StoreResult r4
TestFlags (allflags & ~(zf+vf))
TestResult $80000001

test_rot:
`test_rot:

;rot <>Sj, Si, Sk

;rot <>5, $84A5A51E, r6: expect r6 = $F4252D28, zf, vf cleared, nf set, cf unchanged
SetTestNumber 58
LoadTestReg 5,r4
LoadTestReg $84A5A51E,r5
LoadTestReg $0BDAD2D7,r6
LoadFlags (allflags & ~nf)
rot <>r4, r5, r6
StoreResult r6
TestFlags (allflags & ~(vf+zf))
TestResult $F4252D28

;rot <>31, $0FFFFF01, r6: expect r6 = $1FFFFE02, nf, zf, vf cleared, cf unchanged
LoadTestReg 31,r4
LoadTestReg $0FFFFF01,r5
LoadTestReg $E00001FD,r6
LoadFlags (nf+zf+vf)
rot <>r4, r5, r6
StoreResult r6
TestFlags noflags
TestResult $1FFFFE02

;rot <>-32, $7FFFFFFF, r6: expect r6 = $7FFFFFFF, nf, vf, zf cleared, cf unchanged
LoadTestReg -32,r4
LoadTestReg $7FFFFFFF,r5
LoadTestReg $80000000,r6
LoadFlags (nf+cf+vf+zf)
rot <>r4, r5, r6
StoreResult r6
TestFlags cf
TestResult $7FFFFFFF

;rot <>-31, $80000001, r6: expect r6 = $C0000000, vf, zf cleared, nf set, cf unchanged
LoadTestReg -31,r4
LoadTestReg $80000001,r5
LoadTestReg $3FFFFFFF,r6
LoadFlags (allflags & ~(cf+nf))
rot <>r4, r5, r6
StoreResult r6
TestFlags (allflags & ~(cf+zf+vf))
TestResult $C0000000

;rot <>-2, $80000001, r6: expect r6 = $6, vf, zf, nf cleared, cf unchanged
LoadTestReg -2,r4
LoadTestReg $80000001,r5
LoadTestReg $FFFFFFF9,r6
LoadFlags (allflags)
rot <>r4, r5, r6
StoreResult r6
TestFlags (allflags & ~(zf+vf+nf))
TestResult $6

;rot <>0, $0, r6: expect r6 = $0, vf, nf cleared, zf set, cf unchanged
LoadTestReg 0,r4
LoadTestReg $0,r5
LoadTestReg $FFFFFFFF,r6
LoadFlags (allflags & ~(zf+cf))
rot <>r4, r5, r6
StoreResult r6
TestFlags (allflags & ~(vf+nf+cf))
TestResult $0

;rot #m, Si, Sk

;rot #5, $84A5A51E, r6: expect r6 = $F4252D28, zf, vf cleared, nf set, cf unchanged
SetTestNumber 59
LoadTestReg $84A5A51E,r5
LoadTestReg $0BDAD2D7,r6
LoadFlags (allflags & ~nf)
rot #5, r5, r6
StoreResult r6
TestFlags (allflags & ~(vf+zf))
TestResult $F4252D28

;rot #31, $0FFFFF01, r6: expect r6 = $1FFFFE02, nf, zf, vf cleared, cf unchanged
LoadTestReg $0FFFFF01,r5
LoadTestReg $E00001FD,r6
LoadFlags (nf+zf+vf)
rot #31, r5, r6
StoreResult r6
TestFlags noflags
TestResult $1FFFFE02

;rot #-32, $7FFFFFFF, r6: expect r6 = $7FFFFFFF, nf, vf, zf cleared, cf unchanged
LoadTestReg $7FFFFFFF,r5
LoadTestReg $80000000,r6
LoadFlags (nf+cf+vf+zf)
rot #-32, r5, r6
StoreResult r6
TestFlags cf
TestResult $7FFFFFFF

;rot #-31, $80000001, r6: expect r6 = $C0000000, vf, zf cleared, nf set, cf unchanged
LoadTestReg $80000001,r5
LoadTestReg $3FFFFFFF,r6
LoadFlags (allflags & ~(cf+nf))
rot #-31, r5, r6
StoreResult r6
TestFlags (allflags & ~(cf+zf+vf))
TestResult $C0000000

;rot #-2, $80000001, r6: expect r6 = $6, vf, zf, nf cleared, cf unchanged
LoadTestReg $80000001,r5
LoadTestReg $FFFFFFF9,r6
LoadFlags (allflags)
rot #-2, r5, r6
StoreResult r6
TestFlags (allflags & ~(zf+vf+nf))
TestResult $6

;rot #0, $0, r6: expect r6 = $0, vf, nf cleared, zf set, cf unchanged
LoadTestReg $0,r5
LoadTestReg $FFFFFFFF,r6
LoadFlags (allflags & ~(zf+cf))
rot #0, r5, r6
StoreResult r6
TestFlags (allflags & ~(vf+nf+cf))
TestResult $0

`test_mirror:

;mirror 0b00000001001000110100010101100111, r5
;expect r5 = 0b11100110101000101100010010000000, flags unchanged
SetTestNumber 60
LoadTestReg 0b00000001001000110100010101100111,r4
LoadTestReg 0b00011001010111010011101101111111,r5
LoadFlags allflags
mirror r4, r5
StoreResult r5
TestFlags allflags
TestResult 0b11100110101000101100010010000000

;mirror 0b11100110101000101100010010000000, r5
;expect r5 = 0b00000001001000110100010101100111, flags unchanged
LoadTestReg 0b11100110101000101100010010000000,r4
LoadTestReg 0b11111110110111001011101010011000,r5
LoadFlags noflags
mirror r4, r5
StoreResult r5
TestFlags noflags
TestResult 0b00000001001000110100010101100111

test_and:
`test_and:

;and Si, Sk
;r4 = $89ABCDEF, r5 = $FFFFFFFF, expect r5 = $89ABCDEF, nf set, zf amd vf cleared, cf unchanged
SetTestNumber 61
LoadTestReg $89ABCDEF,r4
LoadTestReg $FFFFFFFF,r5
LoadFlags (allflags & ~nf)
and r4, r5
StoreResult r5
TestFlags (allflags & ~(zf+vf))
TestResult $89ABCDEF

;r4 = $00000000, r5 = $12345670, expect r5 = 0, nf and vf cleared, zf set, cf unchanged
LoadTestReg 0,r4
LoadTestReg $12345670,r5
LoadFlags nf+vf
and r4, r5
StoreResult r5
TestFlags zf
TestResult 0

;r4 = $3210ABCD, r5 = $5882300E, expect r5 = $1000200C, nf, zf and vf cleared, cf unchanged
LoadTestReg $3210ABCD,r4
LoadTestReg $5882300E,r5
LoadFlags nf+vf+zf
and r4, r5
StoreResult r5
TestFlags noflags
TestResult $1000200C

;and Si, Sj, Sk
;r4 = $89ABCDEF, r5 = $FFFFFFFF, expect r6 = $89ABCDEF, nf set, zf amd vf cleared, cf unchanged
SetTestNumber 62
LoadTestReg $89ABCDEF,r4
LoadTestReg $FFFFFFFF,r5
LoadFlags (allflags & ~nf)
and r4, r5, r6
StoreResult r6
TestFlags (allflags & ~(zf+vf))
TestResult $89ABCDEF

;r4 = $00000000, r5 = $12345670, expect r6 = 0, nf and vf cleared, zf set, cf unchanged
LoadTestReg 0,r4
LoadTestReg $12345670,r5
LoadFlags nf+vf
and r4, r5, r6
StoreResult r6
TestFlags zf
TestResult 0

;r4 = $3210ABCD, r5 = $5882300E, expect r6 = $1000200C, nf, zf and vf cleared, cf unchanged
LoadTestReg $3210ABCD,r4
LoadTestReg $5882300E,r5
LoadFlags nf+vf+zf
and r4, r5, r6
StoreResult r6
TestFlags noflags
TestResult $1000200C

;and #n, Sj, Sk
;n = -16, r5 = $FFFFFFFF, expect r6 = $FFFFFFF0, nf set, zf amd vf cleared, cf unchanged
SetTestNumber 63
LoadTestReg $FFFFFFFF,r5
LoadFlags (allflags & ~nf)
and #-16, r5, r6
StoreResult r6
TestFlags (allflags & ~(zf+vf))
TestResult $FFFFFFF0

;n = 00000000, r5 = $12345670, expect r6 = 0, nf and vf cleared, zf set, cf unchanged
LoadTestReg $12345670,r5
LoadFlags nf+vf
and #0, r5, r6
StoreResult r6
TestFlags zf
TestResult 0

;n = 15, r5 = $5882300E, expect r6 = $E, nf, zf and vf cleared, cf unchanged
LoadTestReg $3210ABCD,r4
LoadTestReg $5882300E,r5
LoadFlags nf+vf+zf
and #15, r5, r6
StoreResult r6
TestFlags noflags
TestResult $E

;and #nnnn, Sj, Sk
;nnnn = $89ABCDEF, r5 = $FFFFFFFF, expect r6 = $89ABCDEF, nf set, zf amd vf cleared, cf unchanged
SetTestNumber 64
LoadTestReg $FFFFFFFF,r5
LoadFlags (allflags & ~nf)
and #$89ABCDEF, r5, r6
StoreResult r6
TestFlags (allflags & ~(zf+vf))
TestResult $89ABCDEF

;nnnn = $A5A5FFFF, r5 = $5A5A0000, expect r6 = 0, nf and vf cleared, zf set, cf unchanged
LoadTestReg $5A5A0000,r5
LoadFlags nf+vf
and #$A5A5FFFF, r5, r6
StoreResult r6
TestFlags zf
TestResult 0

;nnnn = $3210ABCD, r5 = $5882300E, expect r6 = $1000200C, nf, zf and vf cleared, cf unchanged
LoadTestReg $5882300E,r5
LoadFlags nf+vf+zf
and #$3210ABCD, r5, r6
StoreResult r6
TestFlags noflags
TestResult $1000200C

;and #n, <>#m, Sk
;n = -3, m = -4, r6 = $89ABCDEF, expect r6 = $89ABCDCF, nf set, zf amd vf cleared, cf unchanged
SetTestNumber 65
LoadTestReg $89ABCDEF,r6
LoadFlags (allflags & ~nf)
and #-3, <>#-4, r6
StoreResult r6
TestFlags (allflags & ~(zf+vf))
TestResult $89ABCDCF

;n = 15, m = 12, r6 = $FF0FFFFF, expect r6 = 0, nf and vf cleared, zf set, cf unchanged
LoadTestReg $FF0FFFFF,r6
LoadFlags nf+vf
and #15, <>#12, r6
StoreResult r6
TestFlags zf
TestResult 0

;n = -1, m = 0, r6 = $1000200C, expect r6 = $1000200C, nf, zf and vf cleared, cf unchanged
LoadTestReg $1000200C,r6
LoadFlags nf+vf+zf
and #-1, <>#0, r6
StoreResult r6
TestFlags noflags
TestResult $1000200C

;and #n, >>Sj, Sk
;n = -3, r5 = -4, r6 = $A55AA55A, expect r6 = $A55AA550, nf set, zf amd vf cleared, cf unchanged
SetTestNumber 66
LoadTestReg -4,r5
LoadTestReg $A55AA55A,r6
LoadFlags (allflags & ~nf)
and #-3, >>r5, r6
StoreResult r6
TestFlags (allflags & ~(zf+vf))
TestResult $A55AA550

;n = 15, r5 = 2, r6 = $C, expect r6 = 0, nf and vf cleared, zf set, cf unchanged
LoadTestReg 2,r5
LoadTestReg $C,r6
LoadFlags nf+vf
and #15, >>r5, r6
StoreResult r6
TestFlags zf
TestResult 0

;n = -1, r5 = -28, r6 = $1FFFFFFF, expect r6 = $10000000, nf, zf and vf cleared, cf unchanged
LoadTestReg -28,r5
LoadTestReg $1FFFFFFF,r6
LoadFlags nf+vf+zf
and #-1, >>r5, r6
StoreResult r6
TestFlags noflags
TestResult $10000000

;and #nnnn, >>Sj, Sk
;nnnn = $F00FF00F, r5 = -16, r6 = $A55AA55A, expect r6 = $A00A0000, nf set, zf amd vf cleared, cf unchanged
SetTestNumber 67
LoadTestReg -16,r5
LoadTestReg $A55AA55A,r6
LoadFlags (allflags & ~nf)
and #$FFFFF00F, >>r5, r6
StoreResult r6
TestFlags (allflags & ~(zf+vf))
TestResult $A00A0000

;nnnn = $A5A5FFFF, r5 = 16, r6 = $FFFF5A5A, expect r6 = 0, nf and vf cleared, zf set, cf unchanged
LoadTestReg 16,r5
LoadTestReg $FFFF5A5A,r6
LoadFlags nf+vf
and #$A5A5FFFF, >>r5, r6
StoreResult r6
TestFlags zf
TestResult 0

;nnnn = $0F0FFFFF, r5 = -20, r6 = $1FFFFFFF, expect r6 = $1FF00000, nf, zf and vf cleared, cf unchanged
LoadTestReg -20,r5
LoadTestReg $1FFFFFFF,r6
LoadFlags nf+vf+zf
and #$0F0FFFFF, >>r5, r6
StoreResult r6
TestFlags noflags
TestResult $1FF00000

;and Si, >>Sj, Sk
;r4 = -3, r5 = -4, r6 = $A55AA55A, expect r6 = $A55AA550, nf set, zf amd vf cleared, cf unchanged
SetTestNumber 68
LoadTestReg -3,r4
LoadTestReg -4,r5
LoadTestReg $A55AA55A,r6
LoadFlags (allflags & ~nf)
and r4, >>r5, r6
StoreResult r6
TestFlags (allflags & ~(zf+vf))
TestResult $A55AA550

;r4 = 15, r5 = 2, r6 = $C, expect r6 = 0, nf and vf cleared, zf set, cf unchanged
LoadTestReg 15,r4
LoadTestReg 2,r5
LoadTestReg $C,r6
LoadFlags nf+vf
and r4, >>r5, r6
StoreResult r6
TestFlags zf
TestResult 0

;r4 = -1, r5 = -28, r6 = $1FFFFFFF, expect r6 = $10000000, nf, zf and vf cleared, cf unchanged
LoadTestReg -1,r4
LoadTestReg -28,r5
LoadTestReg $1FFFFFFF,r6
LoadFlags nf+vf+zf
and r4, >>r5, r6
StoreResult r6
TestFlags noflags
TestResult $10000000

;and Si, >>#m, Sk
;r4 = -3, m = -4, r6 = $A55AA55A, expect r6 = $A55AA550, nf set, zf amd vf cleared, cf unchanged
SetTestNumber 69
LoadTestReg -3,r4
LoadTestReg $A55AA55A,r6
LoadFlags (allflags & ~nf)
and r4, >>#-4, r6
StoreResult r6
TestFlags (allflags & ~(zf+vf))
TestResult $A55AA550

;r4 = 15, m = 2, r6 = $C, expect r6 = 0, nf and vf cleared, zf set, cf unchanged
LoadTestReg 15,r4
LoadTestReg $C,r6
LoadFlags nf+vf
and r4, >>#2, r6
StoreResult r6
TestFlags zf
TestResult 0

;r4 = -1, m = -16, r6 = $1FFFFFFF, expect r6 = $1FFF0000, nf, zf and vf cleared, cf unchanged
LoadTestReg -1,r4
LoadTestReg $1FFFFFFF,r6
LoadFlags nf+vf+zf
and r4, >>#-16, r6
StoreResult r6
TestFlags noflags
TestResult $1FFF0000

;and Si, <>Sj, Sk
;r4 = -3, r5 = -4, r6 = $89ABCDEF, expect r6 = $89ABCDCF, nf set, zf amd vf cleared, cf unchanged
SetTestNumber 70
LoadTestReg -3,r4
LoadTestReg -4,r5
LoadTestReg $89ABCDEF,r6
LoadFlags (allflags & ~nf)
and r4, <>r5, r6
StoreResult r6
TestFlags (allflags & ~(zf+vf))
TestResult $89ABCDCF

;r4 = 15, r5 = 12, r6 = $FF0FFFFF, expect r6 = 0, nf and vf cleared, zf set, cf unchanged
LoadTestReg 15,r4
LoadTestReg 12,r5
LoadTestReg $FF0FFFFF,r6
LoadFlags nf+vf
and r4, <>r5, r6
StoreResult r6
TestFlags zf
TestResult 0

;r4 = -1, r5 = 32, r6 = $1000200C, expect r6 = $1000200C, nf, zf and vf cleared, cf unchanged
LoadTestReg -1,r4
LoadTestReg 32,r5
LoadTestReg $1000200C,r6
LoadFlags nf+vf+zf
and r4, <>r5, r6
StoreResult r6
TestFlags noflags
TestResult $1000200C

test_ftst:
`test_ftst:

;ftst Si, Sq
;r4 = $89ABCDEF, r5 = $FFFFFFFF, expect r5 = $FFFFFFFF, nf set, zf amd vf cleared, cf unchanged
SetTestNumber 71
LoadTestReg $89ABCDEF,r4
LoadTestReg $FFFFFFFF,r5
LoadFlags (allflags & ~nf)
ftst r4, r5
StoreResult r5
TestFlags (allflags & ~(zf+vf))
TestResult $FFFFFFFF

;r4 = $0, r5 = $12345670, expect r5 = $12345670, nf and vf cleared, zf set, cf unchanged
LoadTestReg 0,r4
LoadTestReg $12345670,r5
LoadFlags nf+vf
ftst r4, r5
StoreResult r5
TestFlags zf
TestResult $12345670

;r4 = $3210ABCD, r5 = $5882300E, expect r5 = $5882300E, nf, zf and vf cleared, cf unchanged
LoadTestReg $3210ABCD,r4
LoadTestReg $5882300E,r5
LoadFlags nf+vf+zf
ftst r4, r5
StoreResult r5
TestFlags noflags
TestResult $5882300E

;ftst #n, Sj
;n = -16, r5 = $FFFFFFFF, expect r5 = $FFFFFFFF, nf set, zf amd vf cleared, cf unchanged
SetTestNumber 72
LoadTestReg $FFFFFFFF,r5
LoadFlags (allflags & ~nf)
ftst #-16, r5
StoreResult r5
TestFlags (allflags & ~(zf+vf))
TestResult $FFFFFFFF

;n = $0, r5 = $12345670, expect r5 = $12345670, nf and vf cleared, zf set, cf unchanged
LoadTestReg $12345670,r5
LoadFlags nf+vf
ftst #0, r5
StoreResult r5
TestFlags zf
TestResult $12345670

;n = 15, r5 = $5882300E, expect r5 = $5882300E, nf, zf and vf cleared, cf unchanged
LoadTestReg $5882300E,r5
LoadFlags nf+vf+zf
ftst #15, r5
StoreResult r5
TestFlags noflags
TestResult $5882300E

;ftst #nnnn, Sj
;nnnn = $89ABCDEF, r5 = $FFFFFFFF, expect r5 = $FFFFFFFF, nf set, zf amd vf cleared, cf unchanged
SetTestNumber 73
LoadTestReg $FFFFFFFF,r5
LoadFlags (allflags & ~nf)
ftst #$89ABCDEF, r5
StoreResult r5
TestFlags (allflags & ~(zf+vf))
TestResult $FFFFFFFF

;nnnn = $A5A5FFFF, r5 = $5A5A0000, expect r5 = $5A5A0000, nf and vf cleared, zf set, cf unchanged
LoadTestReg $5A5A0000,r5
LoadFlags nf+vf
ftst #$A5A5FFFF, r5
StoreResult r5
TestFlags zf
TestResult $5A5A0000

;nnnn = $3210ABCD, r5 = $5882300E, expect r5 = $5882300E, nf, zf and vf cleared, cf unchanged
LoadTestReg $5882300E,r5
LoadFlags nf+vf+zf
ftst #$3210ABCD, r5
StoreResult r5
TestFlags noflags
TestResult $5882300E

;ftst #n, <>#m, Sq
;n = -3, m = -4, r6 = $89ABCDEF, expect r6 = $89ABCDEF, nf set, zf amd vf cleared, cf unchanged
SetTestNumber 74
LoadTestReg $89ABCDEF,r6
LoadFlags (allflags & ~nf)
ftst #-3, <>#-4, r6
StoreResult r6
TestFlags (allflags & ~(zf+vf))
TestResult $89ABCDEF

;n = 15, m = 12, r6 = $FF0FFFFF, expect r6 = $FF0FFFFF, nf and vf cleared, zf set, cf unchanged
LoadTestReg $FF0FFFFF,r6
LoadFlags nf+vf
ftst #15, <>#12, r6
StoreResult r6
TestFlags zf
TestResult $FF0FFFFF

;n = -1, m = 0, r6 = $1000200C, expect r6 = $1000200C, nf, zf and vf cleared, cf unchanged
LoadTestReg $1000200C,r6
LoadFlags nf+vf+zf
ftst #-1, <>#0, r6
StoreResult r6
TestFlags noflags
TestResult $1000200C

;ftst #n, >>Sj, Sq
;n = -3, r5 = -4, r6 = $A55AA55A, expect r6 = $A55AA55A, nf set, zf amd vf cleared, cf unchanged
SetTestNumber 75
LoadTestReg -4,r5
LoadTestReg $A55AA55A,r6
LoadFlags (allflags & ~nf)
ftst #-3, >>r5, r6
StoreResult r6
TestFlags (allflags & ~(zf+vf))
TestResult $A55AA55A

;n = 15, r5 = 2, r6 = $C, expect r6 = $C, nf and vf cleared, zf set, cf unchanged
LoadTestReg 2,r5
LoadTestReg $C,r6
LoadFlags nf+vf
ftst #15, >>r5, r6
StoreResult r6
TestFlags zf
TestResult $C

;n = -1, r5 = -28, r6 = $1FFFFFFF, expect r6 = $1FFFFFFF, nf, zf and vf cleared, cf unchanged
LoadTestReg -28,r5
LoadTestReg $1FFFFFFF,r6
LoadFlags nf+vf+zf
ftst #-1, >>r5, r6
StoreResult r6
TestFlags noflags
TestResult $1FFFFFFF

;ftst #nnnn, >>Sj, Sq
;nnnn = $F00FF00F, r5 = -16, r6 = $A55AA55A, expect r6 = $A55AA55A, nf set, zf amd vf cleared, cf unchanged
SetTestNumber 76
LoadTestReg -16,r5
LoadTestReg $A55AA55A,r6
LoadFlags (allflags & ~nf)
ftst #$FFFFF00F, >>r5, r6
StoreResult r6
TestFlags (allflags & ~(zf+vf))
TestResult $A55AA55A

;nnnn = $A5A5FFFF, r5 = 16, r6 = $FFFF5A5A, expect r6 = $FFFF5A5A, nf and vf cleared, zf set, cf unchanged
LoadTestReg 16,r5
LoadTestReg $FFFF5A5A,r6
LoadFlags nf+vf
ftst #$A5A5FFFF, >>r5, r6
StoreResult r6
TestFlags zf
TestResult $FFFF5A5A

;nnnn = $0F0FFFFF, r5 = -20, r6 = $1FFFFFFF, expect r6 = $1FFFFFFF, nf, zf and vf cleared, cf unchanged
LoadTestReg -20,r5
LoadTestReg $1FFFFFFF,r6
LoadFlags nf+vf+zf
ftst #$0F0FFFFF, >>r5, r6
StoreResult r6
TestFlags noflags
TestResult $1FFFFFFF

;ftst Si, >>Sj, Sk
;r4 = -3, r5 = -4, r6 = $A55AA55A, expect r6 = $A55AA55A, nf set, zf amd vf cleared, cf unchanged
SetTestNumber 77
LoadTestReg -3,r4
LoadTestReg -4,r5
LoadTestReg $A55AA55A,r6
LoadFlags (allflags & ~nf)
ftst r4, >>r5, r6
StoreResult r6
TestFlags (allflags & ~(zf+vf))
TestResult $A55AA55A

;r4 = 15, r5 = 2, r6 = $C, expect r6 = $C, nf and vf cleared, zf set, cf unchanged
LoadTestReg 15,r4
LoadTestReg 2,r5
LoadTestReg $C,r6
LoadFlags nf+vf
ftst r4, >>r5, r6
StoreResult r6
TestFlags zf
TestResult $C

;r4 = -1, r5 = -28, r6 = $1FFFFFFF, expect r6 = $1FFFFFFF, nf, zf and vf cleared, cf unchanged
LoadTestReg -1,r4
LoadTestReg -28,r5
LoadTestReg $1FFFFFFF,r6
LoadFlags nf+vf+zf
ftst r4, >>r5, r6
StoreResult r6
TestFlags noflags
TestResult $1FFFFFFF

;ftst Si, >>#m, Sk
;r4 = -3, m = -4, r6 = $A55AA55A, expect r6 = $A55AA55A, nf set, zf amd vf cleared, cf unchanged
SetTestNumber 78
LoadTestReg -3,r4
LoadTestReg $A55AA55A,r6
LoadFlags (allflags & ~nf)
ftst r4, >>#-4, r6
StoreResult r6
TestFlags (allflags & ~(zf+vf))
TestResult $A55AA55A

;r4 = 15, m = 2, r6 = $C, expect r6 = $C, nf and vf cleared, zf set, cf unchanged
LoadTestReg 15,r4
LoadTestReg $C,r6
LoadFlags nf+vf
ftst r4, >>#2, r6
StoreResult r6
TestFlags zf
TestResult $C

;r4 = -1, m = -16, r6 = $1FFFFFFF, expect r6 = $1FFFFFFF, nf, zf and vf cleared, cf unchanged
LoadTestReg -1,r4
LoadTestReg $1FFFFFFF,r6
LoadFlags nf+vf+zf
ftst r4, >>#-16, r6
StoreResult r6
TestFlags noflags
TestResult $1FFFFFFF

;ftst Si, <>Sj, Sq
;r4 = -3, r5 = -4, r6 = $89ABCDEF, expect r6 = $89ABCDEF, nf set, zf amd vf cleared, cf unchanged
SetTestNumber 79
LoadTestReg -3,r4
LoadTestReg -4,r5
LoadTestReg $89ABCDEF,r6
LoadFlags (allflags & ~nf)
ftst r4, <>r5, r6
StoreResult r6
TestFlags (allflags & ~(zf+vf))
TestResult $89ABCDEF

;r4 = 15, r5 = 12, r6 = $FF0FFFFF, expect r6 = $FF0FFFFF, nf and vf cleared, zf set, cf unchanged
LoadTestReg 15,r4
LoadTestReg 12,r5
LoadTestReg $FF0FFFFF,r6
LoadFlags nf+vf
ftst r4, <>r5, r6
StoreResult r6
TestFlags zf
TestResult $FF0FFFFF

;r4 = -1, r5 = 32, r6 = $1000200C, expect r6 = $1000200C, nf, zf and vf cleared, cf unchanged
LoadTestReg -1,r4
LoadTestReg 32,r5
LoadTestReg $1000200C,r6
LoadFlags nf+vf+zf
ftst r4, <>r5, r6
StoreResult r6
TestFlags noflags
TestResult $1000200C

test_btst:
`test_btst:

;btst #m, Sj

;Nuon hardware bug!  In the case of BTST #31, $80000000, the negative flag is
;not set correctly.

;m = 31, r6 = $80000000, expect r6 = $80000000, nf set, zf amd vf cleared, cf unchanged
SetTestNumber 80
LoadTestReg $80000000,r6
LoadFlags (allflags & ~nf)
btst #31, r6
StoreResult r6
;TestFlags (allflags & ~(zf+vf))
TestFlags (allflags & ~(nf+zf+vf))
TestResult $80000000

;m = 6, $FFFFFFBF, expect r6 = $FFFFFFBF, nf and vf cleared, zf set, cf unchanged
LoadTestReg $FFFFFFBF,r6
LoadFlags nf+vf
btst #6, r6
StoreResult r6
TestFlags zf
;TestFlags nf+zf
TestResult $FFFFFFBF

;m = 3, r6 = $00000008, expect r6 = $00000008, nf, zf and vf cleared, cf unchanged
LoadTestReg $8,r6
LoadFlags nf+vf+zf
btst #3,r6
StoreResult r6
TestFlags noflags
;TestFlags nf
TestResult $8

;Test to confirm Nuon hardware bug.  This test shows that BTST #31, Sk, with
;Sk containing _any_ negative number, will always cause the negative flag to be
;cleared

;m = 31, $F00FF00F, expect r6 = $F00FF00F, nf cleared, vf, zf  cleared, cf unchanged
LoadTestReg $F00FF00F,r6
LoadFlags nf+vf+zf
btst #31, r6
StoreResult r6
TestFlags noflags
TestResult $F00FF00F

;Test to make sure there is no bug with btst #31 when testing a positive number
;m = 31, $7FFFFFFF, expect r6 = $7FFFFFFF, nf, vf, zf  cleared, cf unchanged
LoadTestReg $7FFFFFFF,r6
LoadFlags nf+vf
btst #31, r6
StoreResult r6
TestFlags zf
TestResult $7FFFFFFF

`test_or:

;or Si, Sk
;r4 = $89ABCDEF, r5 = $FFFFFFFF, expect r5 = $FFFFFFFF, nf set, zf amd vf cleared, cf unchanged
SetTestNumber 81
LoadTestReg $89ABCDEF,r4
LoadTestReg $FFFFFFFF,r5
LoadFlags (allflags & ~nf)
or r4, r5
StoreResult r5
TestFlags (allflags & ~(zf+vf))
TestResult $FFFFFFFF

;r4 = $00000000, r5 = $00000000, expect r5 = 0, nf and vf cleared, zf set, cf unchanged
LoadTestReg 0,r4
LoadTestReg 0,r5
LoadFlags nf+vf
or r4, r5
StoreResult r5
TestFlags zf
TestResult 0

;r4 = $3210ABCD, r5 = $5882300E, expect r5 = $7A92BBCF, nf, zf and vf cleared, cf unchanged
LoadTestReg $3210ABCD,r4
LoadTestReg $5882300E,r5
LoadFlags nf+vf+zf
or r4, r5
StoreResult r5
TestFlags noflags
TestResult $7A92BBCF

;or Si, Sj, Sk
;r4 = $89ABCDEF, r5 = $FFFFFFFF, expect r6 = $FFFFFFFF, nf set, zf amd vf cleared, cf unchanged
SetTestNumber 82
LoadTestReg $89ABCDEF,r4
LoadTestReg $FFFFFFFF,r5
LoadFlags (allflags & ~nf)
or r4, r5, r6
StoreResult r6
TestFlags (allflags & ~(zf+vf))
TestResult $FFFFFFFF

;r4 = $00000000, r5 = 0, expect r6 = 0, nf and vf cleared, zf set, cf unchanged
LoadTestReg 0,r4
LoadTestReg 0,r5
LoadFlags nf+vf
or r4, r5, r6
StoreResult r6
TestFlags zf
TestResult 0

;r4 = $3210ABCD, r5 = $5882300E, expect r6 = $1000200C, nf, zf and vf cleared, cf unchanged
LoadTestReg $3210ABCD,r4
LoadTestReg $5882300E,r5
LoadFlags nf+vf+zf
or r4, r5, r6
StoreResult r6
TestFlags noflags
TestResult $7A92BBCF

;or #n, Sj, Sk
;n = -16, r5 = $FFFFFFFF, expect r6 = $FFFFFFFF, nf set, zf amd vf cleared, cf unchanged
SetTestNumber 83
LoadTestReg $FFFFFFFF,r5
LoadFlags (allflags & ~nf)
or #-16, r5, r6
StoreResult r6
TestFlags (allflags & ~(zf+vf))
TestResult $FFFFFFFF

;n = 00000000, r5 = 0, expect r6 = 0, nf and vf cleared, zf set, cf unchanged
LoadTestReg 0,r5
LoadFlags nf+vf
or #0, r5, r6
StoreResult r6
TestFlags zf
TestResult 0

;n = 15, r5 = $5882300E, expect r6 = $5882300F, nf, zf and vf cleared, cf unchanged
LoadTestReg $3210ABCD,r4
LoadTestReg $5882300E,r5
LoadFlags nf+vf+zf
or #15, r5, r6
StoreResult r6
TestFlags noflags
TestResult $5882300F

;or #nnnn, Sj, Sk
;nnnn = $89ABCDEF, r5 = $FFFFFFFF, expect r6 = $FFFFFFFF, nf set, zf amd vf cleared, cf unchanged
SetTestNumber 84
LoadTestReg $FFFFFFFF,r5
LoadFlags (allflags & ~nf)
or #$89ABCDEF, r5, r6
StoreResult r6
TestFlags (allflags & ~(zf+vf))
TestResult $FFFFFFFF

;nnnn = $3210ABCD, r5 = $5882300E, expect r6 = $7A92BBCF, nf, zf and vf cleared, cf unchanged
LoadTestReg $5882300E,r5
LoadFlags nf+vf+zf
or #$3210ABCD, r5, r6
StoreResult r6
TestFlags noflags
TestResult $7A92BBCF

;or #n, <>#m, Sk
;n = -3, m = -4, r6 = $89ABCDEF, expect r6 = $FFFFFFFF, nf set, zf amd vf cleared, cf unchanged
SetTestNumber 85
LoadTestReg $89ABCDEF,r6
LoadFlags (allflags & ~nf)
or #-3, <>#-4, r6
StoreResult r6
TestFlags (allflags & ~(zf+vf))
TestResult $FFFFFFFF

;n = 15, m = 12, r6 = $FF0FFFFF, expect r6 = $FFFFFFFF, zf and vf cleared, nf set, cf unchanged
LoadTestReg $FF0FFFFF,r6
LoadFlags zf+vf
or #15, <>#12, r6
StoreResult r6
TestFlags nf
TestResult $FFFFFFFF

;n = 3, m = 0, r6 = $1000200C, expect r6 = $1000200F, nf, zf and vf cleared, cf unchanged
LoadTestReg $1000200C,r6
LoadFlags nf+vf+zf
or #3, <>#0, r6
StoreResult r6
TestFlags noflags
TestResult $1000200F

;or #n, >>Sj, Sk
;n = -3, r5 = -4, r6 = $A55AA55A, expect r6 = $FFFFFFDA, nf set, zf amd vf cleared, cf unchanged
SetTestNumber 86
LoadTestReg -4,r5
LoadTestReg $A55AA55A,r6
LoadFlags (allflags & ~nf)
or #-3, >>r5, r6
StoreResult r6
TestFlags (allflags & ~(zf+vf))
TestResult $FFFFFFDA

;n = 15, r5 = 4, r6 = $0, expect r6 = $0, nf and vf cleared, zf set, cf unchanged
LoadTestReg 4,r5
LoadTestReg $0,r6
LoadFlags nf+vf
or #15, >>r5, r6
StoreResult r6
TestFlags zf
TestResult 0

;n = -9, r5 = -28, r6 = $1FFFFFFF, expect r6 = $7FFFFFFF, nf, zf and vf cleared, cf unchanged
LoadTestReg -28,r5
LoadTestReg $1FFFFFFF,r6
LoadFlags nf+vf+zf
or #-9, >>r5, r6
StoreResult r6
TestFlags noflags
TestResult $7FFFFFFF

;or #nnnn, >>Sj, Sk
;nnnn = $FFFFF00F, r5 = -16, r6 = $A55AA55A, expect r6 = $F55FA55A, nf set, zf amd vf cleared, cf unchanged
SetTestNumber 87
LoadTestReg -16,r5
LoadTestReg $A55AA55A,r6
LoadFlags (allflags & ~nf)
or #$FFFFF00F, >>r5, r6
StoreResult r6
TestFlags (allflags & ~(zf+vf))
TestResult $F55FA55A

;nnnn = $0000FFFF, r5 = 16, r6 = 0, expect r6 = 0, nf and vf cleared, zf set, cf unchanged
LoadTestReg 16,r5
LoadTestReg 0,r6
LoadFlags nf+vf
or #$0000FFFF, >>r5, r6
StoreResult r6
TestFlags zf
TestResult 0

;nnnn = $0F0FF7FF, r5 = -20, r6 = $1FF00000, expect r6 = $7FFFFFFF, nf, zf and vf cleared, cf unchanged
LoadTestReg -20,r5
LoadTestReg $1FFFFFFF,r6
LoadFlags nf+vf+zf
or #$0F0FF7FF, >>r5, r6
StoreResult r6
TestFlags noflags
TestResult $7FFFFFFF

;or Si, >>Sj, Sk
;r4 = -3, r5 = -4, r6 = $A55AA55A, expect r6 = $FFFFFFDA, nf set, zf amd vf cleared, cf unchanged
SetTestNumber 88
LoadTestReg -3,r4
LoadTestReg -4,r5
LoadTestReg $A55AA55A,r6
LoadFlags (allflags & ~nf)
or r4, >>r5, r6
StoreResult r6
TestFlags (allflags & ~(zf+vf))
TestResult $FFFFFFDA

;r4 = 15, r5 = 4, r6 = $0, expect r6 = 0, nf and vf cleared, zf set, cf unchanged
LoadTestReg 15,r4
LoadTestReg 4,r5
LoadTestReg $0,r6
LoadFlags nf+vf
or r4, >>r5, r6
StoreResult r6
TestFlags zf
TestResult 0

;r4 = -9, r5 = -28, r6 = $1FFFFFFF, expect r6 = $7FFFFFFF, nf, zf and vf cleared, cf unchanged
LoadTestReg -9,r4
LoadTestReg -28,r5
LoadTestReg $1FFFFFFF,r6
LoadFlags nf+vf+zf
or r4, >>r5, r6
StoreResult r6
TestFlags noflags
TestResult $7FFFFFFF

;or Si, >>#m, Sk
;r4 = -3, m = -4, r6 = $A55AA55A, expect r6 = $FFFFFFDA, nf set, zf amd vf cleared, cf unchanged
SetTestNumber 89
LoadTestReg -3,r4
LoadTestReg $A55AA55A,r6
LoadFlags (allflags & ~nf)
or r4, >>#-4, r6
StoreResult r6
TestFlags (allflags & ~(zf+vf))
TestResult $FFFFFFDA

;r4 = 15, m = 4, r6 = $0, expect r6 = 0, nf and vf cleared, zf set, cf unchanged
LoadTestReg 15,r4
LoadTestReg 0,r6
LoadFlags nf+vf
or r4, >>#4, r6
StoreResult r6
TestFlags zf
TestResult 0

;r4 = -1, m = 3, r6 = $1FFFFFFF, expect r6 = $1FFFFFFF, nf, zf and vf cleared, cf unchanged
LoadTestReg -1,r4
LoadTestReg $1FFFFFFF,r6
LoadFlags nf+vf+zf
or r4, >>#3, r6
StoreResult r6
TestFlags noflags
TestResult $1FFFFFFF

;or Si, <>Sj, Sk
;r4 = -3, r5 = -4, r6 = $89ABCDEF, expect r6 = $FFFFFFFF, nf set, zf amd vf cleared, cf unchanged
SetTestNumber 90
LoadTestReg -3,r4
LoadTestReg -4,r5
LoadTestReg $89ABCDEF,r6
LoadFlags (allflags & ~nf)
or r4, <>r5, r6
StoreResult r6
TestFlags (allflags & ~(zf+vf))
TestResult $FFFFFFFF

;r4 = 15, r5 = 12, r6 = $FF0FFFFF, expect r6 = $FFFFFFFF, zf and vf cleared, nf set, cf unchanged
LoadTestReg 15,r4
LoadTestReg 12,r5
LoadTestReg $FF0FFFFF,r6
LoadFlags zf+vf
or r4, <>r5, r6
StoreResult r6
TestFlags nf
TestResult $FFFFFFFF

;r4 = -2, r5 = 32, r6 = $1, expect r6 = $FFFFFFFF, zf and vf cleared, nf set, cf unchanged
LoadTestReg -2,r4
LoadTestReg 32,r5
LoadTestReg $1,r6
LoadFlags vf+zf
or r4, <>r5, r6
StoreResult r6
TestFlags nf
TestResult $FFFFFFFF

`test_eor:

;eor Si, Sk
;r4 = $79ABCDEF, r5 = $FFFFFFFF, expect r5 = $86543210, nf set, zf amd vf cleared, cf unchanged
SetTestNumber 91
LoadTestReg $79ABCDEF,r4
LoadTestReg $FFFFFFFF,r5
LoadFlags (allflags & ~nf)
eor r4, r5
StoreResult r5
TestFlags (allflags & ~(zf+vf))
TestResult $86543210

;r4 = $FFFFFFFF, r5 = $FFFFFFFF, expect r5 = 0, nf and vf cleared, zf set, cf unchanged
LoadTestReg $FFFFFFFF,r4
LoadTestReg $FFFFFFFF,r5
LoadFlags nf+vf
eor r4, r5
StoreResult r5
TestFlags zf
TestResult 0

;r4 = $3210ABCD, r5 = $5882300E, expect r5 = $6A929BC3, nf, zf and vf cleared, cf unchanged
LoadTestReg $3210ABCD,r4
LoadTestReg $5882300E,r5
LoadFlags nf+vf+zf
eor r4, r5
StoreResult r5
TestFlags noflags
TestResult $6A929BC3

;eor Si, Sj, Sk
;r4 = $79ABCDEF, r5 = $FFFFFFFF, expect r6 = $86543210, nf set, zf amd vf cleared, cf unchanged
SetTestNumber 92
LoadTestReg $79ABCDEF,r4
LoadTestReg $FFFFFFFF,r5
LoadFlags (allflags & ~nf)
eor r4, r5, r6
StoreResult r6
TestFlags (allflags & ~(zf+vf))
TestResult $86543210

;r4 = $FFFFFFFF, r5 = $FFFFFFFF, expect r6 = 0, nf and vf cleared, zf set, cf unchanged
LoadTestReg $FFFFFFFF,r4
LoadTestReg $FFFFFFFF,r5
LoadFlags nf+vf
eor r4, r5, r6
StoreResult r6
TestFlags zf
TestResult 0

;r4 = $3210ABCD, r5 = $5882300E, expect r6 = $6A929BC3, nf, zf and vf cleared, cf unchanged
LoadTestReg $3210ABCD,r4
LoadTestReg $5882300E,r5
LoadFlags nf+vf+zf
eor r4, r5, r6
StoreResult r6
TestFlags noflags
TestResult $6A929BC3

;eor #n, Sk
;n = -16, r6 = $F, expect r6 = $FFFFFFFF, nf set, zf amd vf cleared, cf unchanged
SetTestNumber 93
LoadTestReg $F,r6
LoadFlags (allflags & ~nf)
eor #-16, r6
StoreResult r6
TestFlags (allflags & ~(zf+vf))
TestResult $FFFFFFFF

;n = 00000000, r6 = 0, expect r6 = 0, nf and vf cleared, zf set, cf unchanged
LoadTestReg 0,r6
LoadFlags nf+vf
eor #0, r6
StoreResult r6
TestFlags zf
TestResult 0

;n = 15, r6 = $5882300E, expect r6 = $58823001, nf, zf and vf cleared, cf unchanged
LoadTestReg $5882300E,r6
LoadFlags nf+vf+zf
eor #15, r6
StoreResult r6
TestFlags noflags
TestResult $58823001

;eor #n, Sj, Sk
;n = -16, r5 = $F, expect r6 = $FFFFFFFF, nf set, zf amd vf cleared, cf unchanged
SetTestNumber 94
LoadTestReg $F,r5
LoadFlags (allflags & ~nf)
eor #-16, r5, r6
StoreResult r6
TestFlags (allflags & ~(zf+vf))
TestResult $FFFFFFFF

;n = 00000000, r5 = 0, expect r6 = 0, nf and vf cleared, zf set, cf unchanged
LoadTestReg 0,r5
LoadFlags nf+vf
eor #0, r5, r6
StoreResult r6
TestFlags zf
TestResult 0

;n = 15, r5 = $5882300E, expect r6 = $58823001, nf, zf and vf cleared, cf unchanged
LoadTestReg $5882300E,r5
LoadFlags nf+vf+zf
eor #15, r5, r6
StoreResult r6
TestFlags noflags
TestResult $58823001

;eor #nnnn, Sk
;nnnn = $79ABCDEF, r6 = $FFFFFFFF, expect r6 = $86543210, nf set, zf amd vf cleared, cf unchanged
SetTestNumber 95
LoadTestReg $FFFFFFFF,r6
LoadFlags (allflags & ~nf)
eor #$79ABCDEF, r6
StoreResult r6
TestFlags (allflags & ~(zf+vf))
TestResult $86543210

;nnnn = $3210ABCD, r5 = $5882300E, expect r6 = $6A929BC3, nf, zf and vf cleared, cf unchanged
LoadTestReg $5882300E,r6
LoadFlags nf+vf+zf
eor #$3210ABCD, r6
StoreResult r6
TestFlags noflags
TestResult $6A929BC3

;eor #nnnn, Sj, Sk
;nnnn = $79ABCDEF, r5 = $FFFFFFFF, expect r6 = $86543210, nf set, zf amd vf cleared, cf unchanged
SetTestNumber 96
LoadTestReg $FFFFFFFF,r5
LoadFlags (allflags & ~nf)
eor #$79ABCDEF, r5, r6
StoreResult r6
TestFlags (allflags & ~(zf+vf))
TestResult $86543210

;nnnn = $3210ABCD, r5 = $5882300E, expect r6 = $6A929BC3, nf, zf and vf cleared, cf unchanged
LoadTestReg $5882300E,r5
LoadFlags nf+vf+zf
eor #$3210ABCD, r5, r6
StoreResult r6
TestFlags noflags
TestResult $6A929BC3

;eor #n, <>#m, Sk
;n = -3, m = -4, r6 = $79ABCDEF, expect r6 = $86543230, nf set, zf amd vf cleared, cf unchanged
SetTestNumber 97
LoadTestReg $79ABCDEF,r6
LoadFlags (allflags & ~nf)
eor #-3, <>#-4, r6
StoreResult r6
TestFlags (allflags & ~(zf+vf))
TestResult $86543230

;n = 15, m = 12, r6 = $FF0FFFFF, expect r6 = $FFFFFFFF, zf and vf cleared, nf set, cf unchanged
LoadTestReg $FF0FFFFF,r6
LoadFlags zf+vf
eor #15, <>#12, r6
StoreResult r6
TestFlags nf
TestResult $FFFFFFFF

;n = 3, m = 0, r6 = $1000200C, expect r6 = $1000200F, nf, zf and vf cleared, cf unchanged
LoadTestReg $1000200C,r6
LoadFlags nf+vf+zf
eor #3, <>#0, r6
StoreResult r6
TestFlags noflags
TestResult $1000200F

;eor #n, >>Sj, Sk
;n = -3, r5 = -4, r6 = $755AA55A, expect r6 = $8AA55A8A, nf set, zf amd vf cleared, cf unchanged
SetTestNumber 98
LoadTestReg -4,r5
LoadTestReg $755AA55A,r6
LoadFlags (allflags & ~nf)
eor #-3, >>r5, r6
StoreResult r6
TestFlags (allflags & ~(zf+vf))
TestResult $8AA55A8A

;n = 15, r5 = 4, r6 = $0, expect r6 = $0, nf and vf cleared, zf set, cf unchanged
LoadTestReg 4,r5
LoadTestReg $0,r6
LoadFlags nf+vf
eor #15, >>r5, r6
StoreResult r6
TestFlags zf
TestResult 0

;n = -9, r5 = -28, r6 = $0FFFFFFF, expect r6 = $7FFFFFFF, nf, zf and vf cleared, cf unchanged
LoadTestReg -28,r5
LoadTestReg $0FFFFFFF,r6
LoadFlags nf+vf+zf
eor #-9, >>r5, r6
StoreResult r6
TestFlags noflags
TestResult $7FFFFFFF

;eor #nnnn, >>Sj, Sk
;nnnn = $FFFFF00F, r5 = -16, r6 = $755AA55A, expect r6 = $8555A55A, nf set, zf amd vf cleared, cf unchanged
SetTestNumber 99
LoadTestReg -16,r5
LoadTestReg $755AA55A,r6
LoadFlags (allflags & ~nf)
eor #$FFFFF00F, >>r5, r6
StoreResult r6
TestFlags (allflags & ~(zf+vf))
TestResult $8555A55A

;nnnn = $0000FFFF, r5 = 16, r6 = 0, expect r6 = 0, nf and vf cleared, zf set, cf unchanged
LoadTestReg 16,r5
LoadTestReg 0,r6
LoadFlags nf+vf
eor #$0000FFFF, >>r5, r6
StoreResult r6
TestFlags zf
TestResult 0

;nnnn = $0F0FF7FF, r5 = -20, r6 = $0FFFFFFF, expect r6 = $700FFFFF, nf, zf and vf cleared, cf unchanged
LoadTestReg -20,r5
LoadTestReg $0FFFFFFF,r6
LoadFlags nf+vf+zf
eor #$0F0FF7FF, >>r5, r6
StoreResult r6
TestFlags noflags
TestResult $700FFFFF

;eor Si, >>Sj, Sk
;r4 = -3, r5 = -4, r6 = $755AA55A, expect r6 = $8AA55A8A, nf set, zf amd vf cleared, cf unchanged
SetTestNumber 100
LoadTestReg -3,r4
LoadTestReg -4,r5
LoadTestReg $755AA55A,r6
LoadFlags (allflags & ~nf)
eor r4, >>r5, r6
StoreResult r6
TestFlags (allflags & ~(zf+vf))
TestResult $8AA55A8A

;r4 = 15, r5 = 4, r6 = $0, expect r6 = 0, nf and vf cleared, zf set, cf unchanged
LoadTestReg 15,r4
LoadTestReg 4,r5
LoadTestReg $0,r6
LoadFlags nf+vf
eor r4, >>r5, r6
StoreResult r6
TestFlags zf
TestResult 0

;r4 = -9, r5 = -28, r6 = $0FFFFFFF, expect r6 = $7FFFFFFF, nf, zf and vf cleared, cf unchanged
LoadTestReg -9,r4
LoadTestReg -28,r5
LoadTestReg $0FFFFFFF,r6
LoadFlags nf+vf+zf
eor r4, >>r5, r6
StoreResult r6
TestFlags noflags
TestResult $7FFFFFFF

;eor Si, >>#m, Sk
;r4 = -3, m = -4, r6 = $755AA55A, expect r6 = $8AA55A8A, nf set, zf amd vf cleared, cf unchanged
SetTestNumber 101
LoadTestReg -3,r4
LoadTestReg $755AA55A,r6
LoadFlags (allflags & ~nf)
eor r4, >>#-4, r6
StoreResult r6
TestFlags (allflags & ~(zf+vf))
TestResult $8AA55A8A

;r4 = 15, m = 4, r6 = $0, expect r6 = 0, nf and vf cleared, zf set, cf unchanged
LoadTestReg 15,r4
LoadTestReg 0,r6
LoadFlags nf+vf
eor r4, >>#4, r6
StoreResult r6
TestFlags zf
TestResult 0

;r4 = -1, m = 3, r6 = $7FFFFFFF, expect r6 = $60000000, nf, zf and vf cleared, cf unchanged
LoadTestReg -1,r4
LoadTestReg $7FFFFFFF,r6
LoadFlags nf+vf+zf
eor r4, >>#3, r6
StoreResult r6
TestFlags noflags
TestResult $60000000

;eor Si, <>Sj, Sk
;r4 = -3, r5 = -4, r6 = $79ABCDEF, expect r6 = $86543230, nf set, zf amd vf cleared, cf unchanged
SetTestNumber 102
LoadTestReg -3,r4
LoadTestReg -4,r5
LoadTestReg $79ABCDEF,r6
LoadFlags (allflags & ~nf)
eor r4, <>r5, r6
StoreResult r6
TestFlags (allflags & ~(zf+vf))
TestResult $86543230

;r4 = 15, r5 = 12, r6 = $FF0FFFFF, expect r6 = $FFFFFFFF, zf and vf cleared, nf set, cf unchanged
LoadTestReg 15,r4
LoadTestReg 12,r5
LoadTestReg $FF0FFFFF,r6
LoadFlags zf+vf
eor r4, <>r5, r6
StoreResult r6
TestFlags nf
TestResult $FFFFFFFF

;r4 = -2, r5 = 32, r6 = $1, expect r6 = $FFFFFFFF, zf and vf cleared, nf set, cf unchanged
LoadTestReg -2,r4
LoadTestReg 32,r5
LoadTestReg $1,r6
LoadFlags vf+zf
eor r4, <>r5, r6
StoreResult r6
TestFlags nf
TestResult $FFFFFFFF

allpass:

;Return $0 to indicate success
SetStatus $0
mv_s #success, branchTargetReg
jmp (branchTargetReg), nop
SetStatus $DEADF00D
SetStatus $DEADF00D

error:
;Return test number that failed
mv_s testNumberReg, testStatusReg

success:
mv_s #doquit, branchTargetReg
jmp (branchTargetReg), nop
SetStatus $DEADF00D
SetStatus $DEADF00D

doquit:

st_s returnAddressReg, rz
nop
;save test results
mv_s expectedFlagsReg, r4
mv_s resultFlagsReg, r5
mv_s expectedResultReg, r6
mv_s resultValueReg, r7
{
ld_v (scratchBufferReg), v3
sub #16, scratchBufferReg
}
{
ld_v (scratchBufferReg), v4
sub #16, scratchBufferReg
}
{
ld_v (scratchBufferReg), v5
sub #16, scratchBufferReg
}
{
ld_v (scratchBufferReg), v6
sub #16, scratchBufferReg
}
ld_v (scratchBufferReg), v7
{
st_s r4, (scratchBufferReg)
add #4, scratchBufferReg
}
{
st_s r5, (scratchBufferReg)
add #4, scratchBufferReg
}
{
st_s r6, (scratchBufferReg)
add #4, scratchBufferReg
}
{
st_s r7, (scratchBufferReg)
rts nop
}



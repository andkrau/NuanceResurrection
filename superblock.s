;packet 1
mv_s #$20100000, r1
{
;packet 2
add r2, r2
mul r2, r3
}
;packet 3
bra ne, exit
;packet 4
bra mvs, overflow
;packet 5
nop
;packet 6
bra always
;should not be present
nop 
;packet 7
{
addm r3, r4, r5
mv_v v4, v5
}

exit:
nop

overflow:
nop

always:
nop
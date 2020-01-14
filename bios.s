.nocache
.nooptimize

commxmit1 = $20500804
commxmit2 = $20500808
commxmit3 = $2050080C
_TimeElapsed = $80000070
bios_stack_low = $807FDC80
bios_stack_high = $807FFC80

NUMRECVHANDLERS = 68

.section biosvars
.origin $80000800
environ:
.dc.s riff_environment
mediawaiting:
.dc.s 0
lostpackets:
.dc.s 0
intnesting:
.dc.s 0
compatibility_mode:
.dc.s 0

.section jumptab
.origin $80000000
jmp riff_commsend, nop
jmp riff_commsendinfo, nop
jmp riff_commrecvinfo, nop
jmp riff_commrecvinfoquery, nop
jmp riff_commsendrecv, nop
jmp riff_commsendrecvinfo, nop
;6
jmp riff_nullroutine, nop
;7
jmp riff_nullroutine, nop
;8
jmp riff_nullroutine, nop
;9
jmp riff_nullroutine, nop
;10
jmp riff_nullroutine, nop
;11
jmp riff_nullroutine, nop
;12
jmp riff_nullroutine, nop
;13
jmp riff_nullroutine, nop
;14
jmp riff_nullroutine, nop
;15
jmp riff_timetosleep, nop
;16
jmp riff_nullroutine, nop
;17
jmp riff_nullroutine, nop
;18
jmp riff_nullroutine, nop
;19
jmp riff_nullroutine, nop
;20
jmp riff_nullroutine, nop
;21
jmp riff_nullroutine, nop
;22
jmp riff_vidsync, nop
;23
jmp riff_nullroutine, nop
;24
jmp riff_nullroutine, nop
;25
jmp riff_nullroutine, nop
;26
jmp riff_nullroutine, nop
;27
jmp riff_nullroutine, nop
;28
jmp riff_nullroutine, nop
;29
jmp riff_nullroutine, nop
;30
jmp riff_nullroutine, nop
;31
jmp riff_nullroutine, nop
;32
jmp riff_nullroutine, nop
;33
jmp riff_nullroutine, nop
;34
jmp riff_nullroutine, nop
;35
jmp riff_nullroutine, nop
;36
jmp riff_nullroutine, nop
;37
jmp riff_nullroutine, nop
;38
jmp riff_nullroutine, nop
;39
jmp riff_nullroutine, nop
;40
jmp riff_nullroutine, nop
;41
jmp riff_nullroutine, nop
;42
jmp riff_nullroutine, nop
;43
jmp riff_nullroutine, nop
;44
jmp riff_nullroutine, nop
;45
jmp riff_nullroutine, nop
;46
jmp riff_nullroutine, nop
;47
jmp riff_nullroutine, nop
;48
jmp riff_nullroutine, nop
;49
jmp riff_nullroutine, nop
;50
jmp riff_nullroutine, nop
;51
jmp riff_nullroutine, nop
;52
jmp riff_nullroutine, nop
;53
jmp riff_nullroutine, nop
;54
jmp riff_nullroutine, nop
;55
jmp riff_nullroutine, nop
;56
jmp riff_nullroutine, nop
;57
jmp riff_nullroutine, nop
;58
jmp riff_nullroutine, nop
;59
jmp riff_nullroutine, nop
;60
jmp riff_nullroutine, nop
;61
jmp riff_nullroutine, nop
;62
jmp riff_nullroutine, nop
;63
jmp riff_nullroutine, nop
;64
jmp riff_nullroutine, nop
;65
jmp riff_nullroutine, nop
;66
jmp riff_nullroutine, nop
;67
jmp riff_nullroutine, nop
;68
jmp riff_nullroutine, nop
;69
jmp riff_nullroutine, nop
;70
jmp riff_nullroutine, nop
;71
jmp riff_nullroutine, nop
;72
jmp riff_nullroutine, nop
;73
jmp riff_nullroutine, nop
;74
jmp riff_nullroutine, nop
;75
jmp riff_nullroutine, nop
;76
jmp riff_nullroutine, nop
;77
jmp riff_nullroutine, nop
;78
jmp riff_nullroutine, nop
;79
jmp riff_nullroutine, nop
;80
jmp riff_nullroutine, nop
;81
jmp riff_commsenddirect, nop
;82
jmp riff_comm_recv, nop
;83
jmp riff_comm_query, nop
;84
jmp riff_nullroutine, nop
;85
jmp riff_nullroutine, nop
;86
jmp riff_nullroutine, nop
;87
jmp riff_nullroutine, nop
;88
jmp riff_nullroutine, nop
;89
jmp riff_nullroutine, nop
;90
jmp riff_nullroutine, nop
;91
jmp riff_nullroutine, nop
;92
jmp riff_nullroutine, nop
;93
jmp riff_nullroutine, nop
;94
jmp riff_nullroutine, nop
;95
jmp riff_nullroutine, nop
;96
jmp riff_nullroutine, nop
;97
jmp riff_nullroutine, nop
;98
jmp riff_nullroutine, nop
;99
jmp riff_nullroutine, nop
;100
jmp riff_nullroutine, nop
;101
jmp riff_nullroutine, nop
;102
jmp riff_nullroutine, nop
;103
jmp riff_nullroutine, nop
;104
jmp riff_nullroutine, nop
;105
jmp riff_nullroutine, nop
;106
jmp riff_mpewait, nop
;107
jmp riff_nullroutine, nop
;108
jmp riff_nullroutine, nop
;109
jmp riff_nullroutine, nop
;110
jmp riff_nullroutine, nop
;111
jmp riff_biosgetinfo, nop
nop
nop

.section commrecv_vars
.origin $800009D0
comm_queue1_head:
.dc.s commrecv_queue1 
comm_queue1_tail:
.dc.s commrecv_queue1
comm_queue2_head:
.dc.s commrecv_queue2 
comm_queue2_tail:
.dc.s commrecv_queue2

.section commrecv_queues
.origin $80000DC0
commrecv_queue1:
.ds.v 128
commrecv_queue2:
.ds.v 128

.section haltab
.origin $80006000
;DACGETSUPPORTEDSAMPLERATES
jmp riff_halstub, nop
;DACINSTALL
jmp riff_halstub, nop
;DACMUTE
jmp riff_halstub, nop
;DACRESET
jmp riff_halstub, nop
;DACSETDEEMPHASIS
jmp riff_halstub, nop
;DACSETSAMPLERATE
jmp riff_halstub, nop
;DACSETSAMPLEWIDTH
jmp riff_halstub, nop
;DENCBLACKLEVEL
jmp riff_halstub, nop
;DENCCANCELAPS
jmp riff_halstub, nop
;DENCCOLORBARS
jmp riff_halstub, nop
;DENCINSTALL
jmp riff_halstub, nop
;DENCSETAPS
jmp riff_halstub, nop
;DENCWIDESCREEN
jmp riff_halstub, nop
;DENC_CC_EVENFIELD
jmp riff_halstub, nop
;DENC_CC_ODDFIELD
jmp riff_halstub, nop
;DENC_CC_ON
jmp riff_halstub, nop
;DENC_CC_ENABLE
jmp riff_halstub, nop
;DENC_CC_NTSC
jmp riff_halstub, nop
;DENC_CC_PAL
jmp riff_halstub, nop
;FLASHERASE
jmp riff_halstub, nop
;FLASHGETINFO
jmp riff_halstub, nop
;FLASHGETSTATUS
jmp riff_halstub, nop
;FLASHINSTALL
jmp riff_halstub, nop
;FLASHREAD
jmp riff_halstub, nop
;FLASHRESET
jmp riff_halstub, nop
;FLASHTIMECLIENT
jmp riff_halstub, nop
;FLASHWRITE
jmp riff_halstub, nop
;HALCOLDBOOT
jmp riff_halstub, nop
;HALCOMMSEND
jmp riff_halstub, nop
;HALDMABILINEAR
jmp riff_halstub, nop
;HALDMALINEAR
jmp riff_halstub, nop
;HALDISABLETIMECLIENT
jmp riff_halstub, nop
;HALENABLETIMECLIENT ($80006100)
jmp riff_halstub, nop
;HALEXAMINEPACKET ($80006108)
jmp riff_halstub, nop
;HALGETSCRATCHBUFFER ($80006110) 
jmp riff_halstub, nop
;HALGETTIMER
jmp riff_halstub, nop
;HALREADINDMA ($80006120)
jmp riff_halstub, nop 
;HALREADSCALARDMA
jmp riff_halstub, nop
;HALREGISTERCOMMANDHANDLER ($80006130)
jmp riff_halstub, nop 
;HALREGISTERTIMECLIENT
jmp riff_halstub, nop
;HALRUNCOMPONENTS ($80006140)
jmp riff_halstub, nop
;HALSETUP ($8006148)
jmp riff_halstub, nop
;HALWRITEOUTDMA
jmp riff_halstub, nop
;HALWRITESCALARDMA
jmp riff_halstub, nop
;HALBYTECOPY
jmp riff_halstub, nop
;I2CCANCELREQUEST
jmp riff_halstub, nop
;I2CCANCELREQUESTINT
jmp riff_halstub, nop
;I2CGETSTATUS
jmp riff_halstub, nop
;I2CGETSTATUSINT
jmp riff_halstub, nop
;I2CINSTALL
jmp riff_halstub, nop
;I2CMASTERREAD
jmp riff_halstub, nop
;I2CMASTERINT
jmp riff_halstub, nop
;I2CMASTERWRITE
jmp riff_halstub, nop
;I2CMASTERWRITEINT
jmp riff_halstub, nop
;LOADCOMPONENT
rts
nop
mv_s #$ffffffff,r0
nop
;NVGETSIZE
jmp riff_halstub, nop
;NVGETSTATUS
jmp riff_halstub, nop
;NVINSTALL
jmp riff_halstub, nop
;NVREAD
jmp riff_halstub, nop
;NVRESET
jmp riff_halstub, nop
;NVTIMECLIENT
jmp riff_halstub, nop
;NVWRITE
jmp riff_halstub, nop
;RUNCOMPONENTS
rts
nop
mv_s #$ffffffff,r0
nop
;SIINSTALL
jmp riff_halstub, nop
;SIGETSTATUS
jmp riff_halstub, nop
;SIRESET
jmp riff_halstub, nop
;SILOADSETTINGS
jmp riff_halstub, nop
;SISTORESETTINGS
jmp riff_halstub, nop
;SIGETSETTINGLENGTH
jmp riff_halstub, nop
;SIGETSETTING
jmp riff_halstub, nop
;SISETSETTING
jmp riff_halstub, nop
;NPINSTALL
jmp riff_halstub, nop
;NPDELAY
jmp riff_halstub, nop
;NPREAD
jmp riff_halstub, nop
;NPWRITE
jmp riff_halstub, nop
;NPWRITEDIRECT
jmp riff_halstub, nop
;NPWRITECMD
jmp riff_halstub, nop
;NPNEWDRIVER
jmp riff_halstub, nop
;TRAYINSTALL
jmp riff_halstub, nop
;TRAYNUMOFDISC
jmp riff_halstub, nop
;TRAYGETSTATUS
jmp riff_halstub, nop
;TRAYOPEN
jmp riff_halstub, nop
;TRAYCLOSE
jmp riff_halstub, nop
;TRAYSELECT
jmp riff_halstub, nop
;IRINSTALL
jmp riff_halstub, nop
;IRRESET
jmp riff_halstub, nop
;IRFLUSH
jmp riff_halstub, nop
;IRSETPOLLRATE
jmp riff_halstub, nop
;IRSETMODE
jmp riff_halstub, nop
;IRGETMODE
jmp riff_halstub, nop
;IRGETSTATUS
jmp riff_halstub, nop
;IRGETKEY
jmp riff_halstub, nop
;FLINSTALL
jmp riff_halstub, nop
;FLRESET
jmp riff_halstub, nop
;FLSETMODE
jmp riff_halstub, nop
;FLGETMODE
jmp riff_halstub, nop
;FLSENDRAW
jmp riff_halstub, nop
;FLUPDATE
jmp riff_halstub, nop
;DRIVEINSTALL
jmp riff_halstub, nop
;DRIVESEEKPHYSICALD????
jmp riff_halstub, nop
;DRIVESEEKABSOLUTEC????
jmp riff_halstub, nop
;DRIVESTAYATLOCATION
jmp riff_halstub, nop
;DRIVESTOP
jmp riff_halstub, nop
;DRIVEDETECTDISKTYPE
jmp riff_halstub, nop
;DRIVECONFIGURECDI
jmp riff_halstub, nop
;DRIVEREADSTATUS
jmp riff_halstub, nop
;DRIVEGETSTATUS
jmp riff_halstub, nop
;DRIVECONTROL
jmp riff_halstub, nop
;I2CRESET
jmp riff_halstub, nop
;I2CMASTERWRITETHEN????
jmp riff_halstub, nop
;HALTIMECLIENTSLEEP
jmp riff_halstub, nop
;DRIVEGETDETECTEDDISKTYPE
jmp riff_halstub, nop
;DRIVESETLAYERENDS(ECTOR?)
jmp riff_halstub, nop
;DRIVEFREEZECDI
jmp riff_halstub, nop
;NPCONFIGURING
jmp riff_halstub, nop
;DACSETI2SMODE
jmp riff_halstub, nop
;HALRUNSINGLECOMPONENT
jmp riff_halstub, nop
;HALGETVERSION
jmp riff_halstub, nop
;NPGREEDY
jmp riff_halstub, nop
;POWERINSTALL
jmp riff_halstub, nop
;POWERGETCAPS
jmp riff_halstub, nop
;POWEROFF
jmp riff_halstub, nop
;POWERSLEEP
jmp riff_halstub, nop
;POWERRESET
jmp riff_halstub, nop
;FLASHERASEMULTIPLE
jmp riff_halstub, nop
;HALIOCTL
jmp riff_halstub, nop
;FLDISPLAYMESSAGE
jmp riff_halstub, nop
;CLOCKINSTALL
jmp riff_halstub, nop
;CLOCKRESET
jmp riff_halstub, nop
;CLOCKSETTIME
jmp riff_halstub, nop
;CLOCKGETTIME
jmp riff_halstub, nop
;TRAYSETMODE
jmp riff_halstub, nop
;DENCENABLEAPS
jmp riff_halstub, nop
;HALSYNCCACHE
jmp riff_halstub, nop
;UARTCANCELREQUEST
jmp riff_halstub, nop
;UARTCANCELREQUESTINT
jmp riff_halstub, nop
;UARTGETSTATUS
jmp riff_halstub, nop
;UARTGETSTATUSINT
jmp riff_halstub, nop
;UARTINSTALL
jmp riff_halstub, nop
;UARTMASTERREAD
jmp riff_halstub, nop
;UARTMASTERREADINT
jmp riff_halstub, nop
;UARTMASTERWRITE
jmp riff_halstub, nop
;UARTMASTERWRITEINT
jmp riff_halstub, nop
;DEBUGINSTALL
jmp riff_halstub, nop
;DEBUGPUTSTRING
jmp riff_halstub, nop
;DEBUGGETSTRING
jmp riff_halstub, nop
;DEBUGSETUPPACKETLO?????
jmp riff_halstub, nop
;DEBUGADDPACKET
jmp riff_halstub, nop
;DEBUGGETPACKET
jmp riff_halstub, nop
;DEBUGPRINTPACKETLO?????
jmp riff_halstub, nop
;SYSTEMSETREGISTER
jmp riff_halstub, nop
;SYSTEMGETREGISTER
jmp riff_halstub, nop
;SYSTEMINITREGISTER
jmp riff_halstub, nop
;DRIVESETDISKMODE
jmp riff_halstub, nop
;DEBUGPUTCHAR
jmp riff_halstub, nop
;HALCOMMSENDASYNC
jmp riff_halstub, nop
;DENCGETVIDEOSYSTEM
jmp riff_halstub, nop
;DENCSETVIDEOSYSTEM
jmp riff_halstub, nop
;DENCGETATTRIBUTE
jmp riff_halstub, nop
;DENCSETATTRIBUTE
jmp riff_halstub, nop
;DRIVETRAYNOTIFICATION
jmp riff_halstub, nop
;UARTMASTERWRITETHEN
jmp riff_halstub, nop
;IRGETEXTCONTROLEVENT
jmp riff_halstub, nop
;SILOADDEFAULTSETTINGS
jmp riff_halstub, nop
;IRGENERATEEXTCONTROLEVENT
jmp riff_halstub, nop
;IRKBDFLUSH
jmp riff_halstub, nop
;IRKBDGETSTATE
jmp riff_halstub, nop
;IRKBDSETSTATE
jmp riff_halstub, nop
;IRKBDGETSTATUS
jmp riff_halstub, nop
;IRKBDSETMODE
jmp riff_halstub, nop
;IRKBDGETMODE
jmp riff_halstub, nop
;IRKBDGETKEY
jmp riff_halstub, nop
;IRKBDGETTRANSLATION
jmp riff_halstub, nop
;IRKBDSETTRANSLATION
jmp riff_halstub, nop
;DRIVERUNCDI
jmp riff_halstub, nop
;DRIVEPREPARECSS
jmp riff_halstub, nop
;DRIVERRUNCSS
jmp riff_halstub, nop
;DRIVEREADCSSKEY
jmp riff_halstub, nop
;DRIVESETCSSKEY
jmp riff_halstub, nop
;DRIVEWRITECSSKEY
jmp riff_halstub, nop
;HALLOCKALLTIMECLIENTS
jmp riff_halstub, nop
;HALUNLOCKALLTIMECLIENTS
jmp riff_halstub, nop
;HALSHUTDOWN
jmp riff_halstub, nop
;HALREGISTERSHUTDOWN
jmp riff_halstub, nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop
;??
rts
nop
mv_s #$ffffffff,r0
nop

.section biosinfo
.origin $80760000
riff_biosinfostruct:
;Major Revision
.dc.w 1
;Minor Revision
.dc.w 3
;VMLabs Revision
.dc.w 0
;OEM Revision
.dc.w 0
;Pointer to Bios Info String
.dc.s riff_biosinfostring
;Pointer to Bios Date String
.dc.s riff_biosdatestring

;HAL Revision
.dc.s 0

.origin $80760014
riff_biosinfostring:
.asciiz "Nuance BIOS Version 2.2.0"
riff_biosdatestring:
.asciiz "March 20, 2004"
env_system:
.asciiz "SYSTEM=Nuance Emulator"
env_mainram:
.asciiz "MAINRAM=8388608"
env_sysram:
.asciiz "SYSRAM=8388608"
biosinfo_end:

.section biosfuncs
.origin $80760080

;**********************************************************************
;* _CommSendInfo: r0 = target, r1 = info, r2 = pointer to xmit packet
;**********************************************************************
riff_commsendinfo:
{
copy r0, r4
mv_s r1, r5
}
{
mv_s r2, r1
bra commsend_loadpacket, nop
}

;**********************************************************************
;* _CommSend: 
;r0 = target
;r1 = pPacket
;**********************************************************************
riff_commsend:
sub r5, r5
commsend_loadpacket:
{
copy r0, r4
mv_s r1, r3
}
{
ld_s (r3), r0
add #4, r3
}
{
ld_s (r3), r1
add #4, r3
}
{
ld_s (r3), r2
add #4, r3
}
ld_s (r3), r3
nop

;fall through to CommSendDirect

;**********************************************************************
;* _CommSendDirect: r0-r3: packet data, r4 = target, r5 = info
;**********************************************************************
riff_commsenddirect:
csd_wait_for_xmit_clear:
ld_s commctl,r6
nop
btst #15,r6
;if commctl.xmitbufferfull, branch back to wait_for_xmit_clear
bra ne,csd_wait_for_xmit_clear,nop

csd_xmit_failed:
;r4 = target & 0xFF
{
st_s r5, comminfo
bits #7, >>#0, r4
}

;store target bits into commctl
st_s r4,commctl

;transmit packet
st_v v0,commxmit

csd_wait_for_xmit_complete:
ld_s commctl,r6
nop
btst #14,r6

;if failed bit is set, branch back to xmit_failed
{
bra ne,csd_xmit_failed,nop
btst #5,r6
}

;if bit 5 of commctl is set, the xmit was interrupted and failed in the ISR so branch back to to xmit_failed
{
bra ne,csd_xmit_failed,nop
btst #15,r6
}

;if xmit buffer is clear, return from routine
rts eq,nop

;if xmit buffer is not clear, branch back to wait_for_xmit_complete
bra csd_wait_for_xmit_complete,nop

;**********************************************************************
;* CommRecvInfoQuery
;r0 = pInfo
;r1 = pPacket
;**********************************************************************

riff_commrecvinfoquery:
{
copy r0, r2
mv_s #bios_commrecv_packet_available, r0
}
ld_s (r0), r0
nop
sub #1, r0
rts mi, nop

mv_s #bios_commrecv_info, r0
mv_s #bios_commrecv_packet, r3
ld_s (r0), r0
ld_v (r3), v1
st_s r0, (r2)
{
sub r0, r0
mv_s #bios_commrecv_packet_available, r2
}
mv_s #bios_commrecv_id, r3
ld_s (r3),r3
{
st_s r4, (r1)
add #4, r1
}
{
st_s r5, (r1)
add #4, r1
}
{
st_s r6, (r1)
add #4, r1
}
st_s r7, (r1)
{
st_s r0, (r2)
copy r3, r0
}
rts nop

;**********************************************************************
;* CommRecvInfo
;r0 = pInfo
;r1 = pPacket
;**********************************************************************
riff_commrecvinfo:
ld_s rz, r29
sub #$10, r31
st_v v7, (r31)

mv_s r0, r28
mv_s r1, r30

wait_for_comm_recv_packet:
jsr riff_commrecvinfoquery,nop
cmp #-1, r0
bra ne, got_comm_recv_info_packet, nop
mv_s r28, r0
mv_s r30, r1
bra wait_for_comm_recv_packet, nop

got_comm_recv_info_packet:
;epilogue:
ld_v (r31), v7
nop
st_s r29, rz
add #$10, r31
rts nop

;**********************************************************************
;* CommRecvInfoWait: 
;r0 = pInfo
;r1 = pPacket
;**********************************************************************

riff_commrecvinfowait:
push v7, rz
`commrecvinfo_wait_loop:
//syscall
st_s #((1 << 24)|1), configa
mv_s r0, r28
mv_s r1, r30
jsr riff_commrecvinfoquery,nop
cmp #-1, r0
mv_s r28, r0
mv_s r30, r1
bra eq, `commrecvinfo_wait_loop,nop
pop v7, rz
nop
rts nop

;**********************************************************************
;* _comm_recv
;v0: packet
;r4: senderID
;r5: info
;**********************************************************************
riff_comm_recv:
mv_s #bios_commrecv_packet_available, r1
mv_s #bios_commrecv_packet, r5

riff_comm_recv_wait:
ld_s (r1), r0
nop
cmp #0, r0
bra ne, comm_recv_packet_arrived, nop
;syscall: WaitForCommPacket
st_s #(1 << 24)|1, configa
bra riff_comm_recv_wait, nop

comm_recv_packet_arrived:
;r2 = 0
eor r2, r2
;bios_commrecv_packet_available = 0
st_s r2, (r1)
;v0 = packet data
{
ld_v (r5), v0
add #16, r5
}
;r4 = commid
{
ld_s (r5), r4
add #4, r5
}
;r5 = comminfo
ld_s (r5), r5
nop
rts nop

;**********************************************************************
;* _comm_query
;**********************************************************************
riff_comm_query:
mv_s #bios_commrecv_packet_available, r0
ld_s (r0), r0
nop
sub #1, r0
rts nop

;**********************************************************************
;* CommSendRecvInfo: R0 = target, R1 = pPacket, R2 = info
;**********************************************************************
riff_commsendrecvinfo:
;**********************************************************************
;* CommSendRecv: R0 = target, R1 = pPacket, R2 = info
;**********************************************************************
riff_commsendrecv:
ld_s rz, r29
sub #$10, r31
st_v v7, (r31)
{
copy r1, r2
mv_s r2, r1
}

mv_s #commsendrecv_target, r3
st_s r0, (r3)
jsr riff_commsendinfo, nop

wait_for_recv_target_response:
mv_s #commsendrecv_target, r3
ld_s (r3), r3
nop
btst #31, r3
bra eq, wait_for_recv_target_response, nop

mv_s #commsendrecv_packet, r3
ld_v (r3), v1
nop
st_v v1, (r1)
;epilogue:
ld_v (r31), v7
nop
st_s r29, rz
add #$10, r31
rts nop

;**********************************************************************
;* VidSync
;**********************************************************************
riff_vidsync:

{
mv_s #riff_vidfieldcounter, r3
copy r0, r4
}
;r0 = field counter at start of testing
ld_s (r3), r0

cmp #-1, r4

;----------------------------------------------------------------------
; VidSync(-2): return without doing anything
;----------------------------------------------------------------------

vidsync_neg2:

;vidsync(-2): return without doing anything
rts lt, nop

;----------------------------------------------------------------------
; VidSync(-1): return without doing anything
;----------------------------------------------------------------------

{
;vidsync(-1): just branch to vidsync_complete to return field counter
bra eq, vidsync_complete, nop
}

test_for_vidsync_0:

{
mv_s #riff_vidfieldcounter, r3
;r1 = video field at start of testing
copy r0, r1
}
{
cmp #0, r4
;r0 = current vidsync counter
ld_s (r3), r0
}
bra ne, vidsync_n, nop 

;----------------------------------------------------------------------
; VidSync(0): return when current field counter is not equal to the 
; video config counter
;----------------------------------------------------------------------

;vidsync(0)
vidsync_0:

mv_s #riff_lastvidconfigcounter, r1
mv_s #riff_vidfieldcounter, r3
;r1 = vidsync counter at last vidconfig
ld_s (r1), r1
nop
;compare current field counter to last video config counter, reload current field counter into r0
{
cmp r1, r0
ld_s (r3), r0
}
rts ne, nop
bra vidsync_0, nop

;----------------------------------------------------------------------
; VidSync(n > 0): return after n video fields have passed
; r1: field counter at start of vidsync
; r2: current field counter
; r3: current field counter - counter at start of vidsync = number of vidsyncs that have occurred)
; r4: n
;----------------------------------------------------------------------

vidsync_n:

;r3 = current field counter - counter at start of vidsync = number of vidsyncs that have occurred)
mv_s #riff_vidfieldcounter, r2
ld_s (r2), r2
nop
sub r1, r2, r3
cmp r3, r4
;if the requested number of video fields have not passed yet, branch back to the start of the loop
bra gt, vidsync_n, nop

vidsync_complete:
mv_s #riff_vidfieldcounter, r0
ld_s (r0), r0
rts nop

;**********************************************************************
;* MPEWait
;**********************************************************************
riff_mpewait:
ld_s rz, r29
sub #$10, r31
st_v v7, (r31)

wait_for_halt:

;call BIOS ReadRegister routine
jsr $80000358
{
mv_s #addrof(mpectl), r1
copy r0, r4
}
nop

btst #1, r0
{
bra ne, wait_for_halt, nop
mv_s r4, r0
}

;check for an exception

;call BIOS ReadRegister routine
mv_s #addrof(excephalten), r1
jsr $80000358, nop

{
mv_s r0, r2
copy r4, r0
}

;call BIOS ReadRegister routine
mv_s #addrof(excepsrc), r1
jsr $80000358, nop

{
and r1, r2
mv_s #-1, r0
}
;if (excepsrc & excephalten) is non-zero, an exception caused the
;halt and MPEWait should return -1
bra ne, mpewait_epilogue, nop

;no exception returned: return value in remote MPEs r0

;call BIOS ReadRegister routine
{
copy r4, r0
mv_s #addrof(r0), r1
}

jsr $80000358, nop

mpewait_epilogue:

;epilogue:
ld_v (r31), v7
nop
st_s r29, rz
rts
add #$10, r31
nop

;**********************************************************************
;* TimeToSleep: R0 = msec
;
;r0: current msecs
;r1: 0
;r2: msec
;r30: target msecs
;**********************************************************************
riff_timetosleep:
ld_s rz, r29
sub #$10, r31
st_v v7, (r31)

;r28 = target_elapsed_msec
mv_s r0, r28

;Call BIOS TimeElapsed routine (native implementation so r0-r11 dont need to be preserved
mv_s #0, r0
mv_s #0, r1
jsr _TimeElapsed, nop

;r30 = initial_msec
{
copy r0, r30
mv_s #0, r0
}

let_time_pass:
;Call BIOS TimeElapsed routine
jsr _TimeElapsed,nop

;r1 = elapsed_msec = current_msec - initial_msec
sub r30, r0, r1
//compare elapsed_msec to target_elapsed_msec
cmp r28, r1
mv_s #0, r0
mv_s #0, r1
;if elapsed_msec < target_elapsed_msec, branch to let_time_pass
bra lt, let_time_pass, nop

time_elapsed:
ld_v (r31), v7
nop
st_s r29, rz
add #$10, r31
rts nop

;**********************************************************************
;* MPERunMediaMPE
;r0 = which
;r1 = entryPoint
;r2 = return address
;**********************************************************************
mperunmpemediampe:
push v3
mv_s r1, r0
mv_s r2, r12
mv_s r0, r4
mv_s #$c2, r5
jsr riff_commsenddirect, nop
st_s r12, rz
pop v3
rts nop

;**********************************************************************
;* Default ISRs
;**********************************************************************

riff_defaultint16handler:
rts
st_s #(1 << 16), intclr

riff_defaultint1handler:
rts
st_s #(1 << 1), intclr

riff_defaultint0handler:
rts
st_s #(1 << 0), intclr

riff_defaultinthandler:
rts nop

riff_defaultrecvhandler:
mv_s #bios_commrecv_id, r3
st_s r0, (r3)
mv_s #bios_commrecv_info, r3
st_s r1, (r3)
mv_s #bios_commrecv_packet, r3
st_v v1, (r3)
mv_s #bios_commrecv_packet_available, r3
mv_s #1, r2
st_s r2, (r3)
mv_s #-1, r0
rts nop

;**********************************************************************
;* VidTimer ISR
;**********************************************************************

video_isr:
mv_s #(riff_intvectors + 31*4), r0
ld_s (r0), r0
;clear video interrupt
st_s #(1 << 31), intclr
or r0, r0
jmp ne, (r0), nop
rts nop

;**********************************************************************
;* Level1 ISR
;**********************************************************************
riff_level1handler:
push v0
push v1
push v2
push v3
push v4
push v5
push r29, cc, rzi1, rz

save_regs:

ld_s rc0, r17
ld_s intctl, r8
ld_s rc1, r18

btst #3, r8
;return from interrupt if sw1 mask is set
bra ne, level1handler_epilogue, nop


;wait for pending comm transmit to finish
level1_wait_xmit_clear1:
ld_s commctl, r0
nop
btst #15, r0
bra ne, level1_wait_xmit_clear1, nop

;r17 = saved_rc0
;r18 = saved_rc1
ld_s commctl, r19
bits #0, >>#14, r0
;r20 = saved_comminfo
ld_s comminfo, r20
;r21 = saved_acshift
ld_s acshift, r21
;r22 = saved_sp
ld_s sp, r22
;r19 = saved_commctl | (xmit_failed << 5), in case a pending comm transfer was interrupted and failed
or r0, >>#-5, r19

cmp #bios_stack_low, r22
bra ge, increment_int_nesting, nop

st_s #bios_stack_high, sp

increment_int_nesting:
mv_s #intnesting, r0
ld_s (r0), r1
nop
add #1, r1
st_s r1, (r0)

ld_s inten1, r14
ld_s intsrc, r15
nop
;R15 = (inten1 & intsrc)
and r14, r15 

// INTSRC_NONZERO LOOP: there may be some enabled level1 interrupt source bits set so they should be checked and processed
l1_intsrc_nonzero:

;Clear video interrupt source and branch if no video interrupt
mv_s #riff_video_isr, r1
btst #31, r15
{
bra eq, dont_call_vid_isr, nop
bclr #31, r15
}

// ISR LOOP: use MSB to calculate the ISR vector addresses that need to be fetched and called.
level1_isr_loop:
;load ISR address from vector
ld_s (r1), r1
nop
;Call user ISR 
jsr (r1), nop

// This is an entry point to the ISR loop only when the video ISR was not called.
dont_call_vid_isr:
{
msb r15, r0
mv_s #riff_intvectors, r1
}
sub #1, r0
;r1 = riff_intvectors + (which << 2)
add r0, >>#-2, r1
;r0 = -which
neg r0
;r15 = (intsrc & inten1) xor (1 << which)
eor #1, >>r0, r15
;compare -which to 1
cmp #1, r0
;if -which is not 1 then which was not -1 so branch back and call the user ISR
bra ne, level1_isr_loop, nop

ld_s intsrc, r0
ld_s inten1, r15
nop
and r0, r15
;if (intsrc & inten1) is non-zero, branch back to l1_intsrc_nonzero
bra ne, l1_intsrc_nonzero, nop

//END OF ISR LOOP

;wait for pending comm transmit to finish
level1_wait_xmit_clear2:
ld_s commctl, r0
nop
btst #15, r0
bra ne, level1_wait_xmit_clear2, nop

clear_sw1_mask:

;clear sw1 mask
st_s #4, intctl

mv_s #isr_exit_hook_address, r0
mv_s #intnesting, r1
ld_s (r0), r0
ld_s (r1), r1
nop
cmp #1, r1
bra gt, decrement_intnesting, nop

;call the ISR exit hook if it exists
or r0, r0
jsr ne, (r0), nop

decrement_intnesting:
mv_s #intnesting, r0
ld_s (r0), r1
nop
sub #1, r1
st_s r1, (r0)

st_s r17, rc0
st_s r18, rc1
st_s r19, commctl
st_s r20, comminfo
st_s r21, acshift
st_s r22, sp

level1handler_epilogue:
pop r29, cc, rzi1, rz
pop v5
pop v4
pop v3
pop v2
pop v1
pop v0
rti t, (rzi1), nop

;**********************************************************************
;* Level2 ISR
;**********************************************************************
riff_level2handler:
push v7
push v4
push v3
push v2
push v1
push v0
;r13 = saved_rc0
ld_s rc0, r13
;r14 = saved_rc1
ld_s rc1, r14
push r29, cc, rzi2, rz
;r12 = saved_sp
ld_s sp, r12
nop

cmp #bios_stack_low, r12
bra ge, `check_for_commrecv, nop

st_s #bios_stack_high, sp

`check_for_commrecv:
;avoid cache bug
mv_s #commsendrecv_target, r2
ld_s commctl, r0
;r1 = comminfo
ld_s comminfo, r1
{
ld_s (r2),r2
btst #31, r0
}
bra eq, comm_error, nop

{
;clear recvbufferfull interrupt
st_s #(1 << 4), intclr
;r0 = recvID
bits #7, >>#16, r0
}

;r1 = comminfo
bits #7, >>#16, r1

{
ld_v commrecv, v1
;Compare commctl.recvID to commsendrecv_target
cmp r0, r2
}
;If commctl.recvID equals commsendrecv_target, branch to matched_commsendrecv_target_id
bra eq, matched_commsendrecv_target_id, nop

mv_s #riff_recvhandlercount, r18
;r18 = riff_numrecvhandlers
ld_s (r18),r18
nop
lsl #2, r18, r19
;add #(riff_recvhandlers - 4), r19
mv_s #(riff_recvhandlers), r19
ld_s (r19), r16
;test riff_numrecvhandlers against zero
or r18, r18
;branch to calldefaultrecvhandler if riff_recvhandlercount is zero
bra eq, calldefaultrecvhandler, nop

recvhandler_loop:

;call next recv handler
jsr (r16), nop

add #4, r19
cmp #-1, r0

{
;if handler returned -1, packet was processed, so skip to epilogue, count = count - 1
bra eq, level2_epilogue, nop
sub #1, r18
}
{
;if more recv handlers, branch back to recvhandler_loop
bra ne, recvhandler_loop, nop
ld_s (r19), r16
}

calldefaultrecvhandler:
jsr t, riff_defaultrecvhandler, nop

level2_epilogue:
st_s r12, sp
st_s r13, rc0
st_s r14, rc1
pop r29, cc, rzi2, rz
pop v0
pop v1
pop v2
pop v3
pop v4
pop v7
rti t, (rzi2), nop

matched_commsendrecv_target_id:
mv_s #commsendrecv_packet, r2
st_v v1, (r2)
mv_s #-1, r17
mv_s #commsendrecv_target, r2
;commsendrecv_target = -1
st_s r17, (r2)
bra level2_epilogue, nop

comm_error:
st_s #$f0025084, rx
halt
nop
nop
bra level2_epilogue, nop

riff_nullroutine:
rts nop
;**********************************************************************
;* BiosGetInfo
;**********************************************************************
riff_biosgetinfo:
mv_s #riff_biosinfostruct, r0
rts nop

.section bios_stack_start
.origin $807FDC80
.section bios_stack_end
.origin $807FFC80
.dc.s $DEADBEEF

.section halstub
.origin $807FFC88
riff_halstub:
rts nop
rts nop
rts nop
rts nop


.section biosdata
.origin $807FFC90
isr_exit_hook_address:
.dc.s 0
.dc.s 0
.dc.s 0
.dc.s 0

;$807FFCA0
commsendrecv_target:
.dc.s -1
commsendrecv_info:
.dc.s -1
reserved0:
.dc.s 0
reserved1:
.dc.s 0

;$807FFCB0
commsendrecv_packet:
.ds.v 1

;$807FFCC0
reserved2:
.ds.v 1

;$807FFCD0
bios_commrecv_packet:
.ds.v 1
bios_commrecv_id:
.dc.s 0
bios_commrecv_info:
.dc.s 0
bios_commrecv_packet_available:
.dc.s 0
bios_commrecv_packet_dropped:
.dc.s 0

;$807FFCF0
riff_environment:
.dc.s env_system
.dc.s env_mainram
.dc.s env_sysram
.dc.s 0

;$807FFD00
riff_scratchmem:
.ds.s 47
;$807FFDBC
riff_cyclecounter:
.dc.s 0
;$807FFDC0 (current field counter value)
riff_vidfieldcounter:
.dc.s 0
;$807FFDC4 (field counter value at previous call to vidconfig/vidsetup/vidchangebase/vidsetscroll)
riff_lastvidconfigcounter:
.dc.s 0
;$807FFDC8 (number of installed commrecv handlers)
riff_recvhandlercount:
.dc.s 0
riff_video_isr:
;$807FFDCC (BIOS video isr vector: contains address BIOS video ISR)
.dc.s video_isr
;$807FFDD0 (BIOS and user ISR vector)
riff_intvectors: 
.ds.s 32
;$807FFE50 (user commrecv handlers: called by level1 handler)
riff_recvhandlers:
.ds.s NUMRECVHANDLERS


.section halts
.origin $807FFF60
riff_halt_instructions:
halt
halt
halt
halt
halt
halt
halt
halt

.section controllerdata
.origin $807FFF70
riff_controllerdata:
.ds.s (4 * 9)

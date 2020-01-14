#
# Makefile for Filetest
#
# Copyright (c) 1996-2001 VM Labs, Inc.  All rights reserved.
# NOTICE: VM Labs permits you to use, modify, and distribute this file
# in accordance with the terms of the VM Labs license agreement
# accompanying it. If you have received this file from a source other
# than VM Labs, then your use, modification, or distribution of it
# requires the prior written permission of VM Labs.

###############################

include $(VMLABS)/util/nuon_build_tools.mk

###############################

DEFINES =
CFLAGS = -g -O3 -Wall $(DEFINES)
LDFLAGS = 

###############################

OBJS = instructiontest.o nuontest.o

LIBS = -lterm -lmml2d -lmutil -lm 

NUON.CD: cd_app.cof
	CreateNuonCD

cd_app.cof: instructiontest.cof
	coffpack -o cd_app.cof instructiontest.cof

instructiontest.cof: $(OBJS)
	$(CC) $(LDFLAGS) -o instructiontest.cof $(OBJS) $(LIBS)

###############################

clean:
	-$(RM) *.o
	-$(RM) NUON.CD

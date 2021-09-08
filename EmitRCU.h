#ifndef EMITRCU_H
#define EMITRCU_H

#include "mpe.h"

NativeEmitHandlerProto Emit_DECRc0;
NativeEmitHandlerProto Emit_DECRc1;
NativeEmitHandlerProto Emit_DECBoth;
NativeEmitHandlerProto Emit_DEC;
NativeEmitHandlerProto Emit_ADDRImmediateOnly;
NativeEmitHandlerProto Emit_ADDRImmediate;
NativeEmitHandlerProto Emit_ADDRScalarOnly;
NativeEmitHandlerProto Emit_ADDRScalar;
NativeEmitHandlerProto Emit_MVRImmediateOnly;
NativeEmitHandlerProto Emit_MVRImmediate;
NativeEmitHandlerProto Emit_MVRScalarOnly;
NativeEmitHandlerProto Emit_MVRScalar;
NativeEmitHandlerProto Emit_RangeOnly;
NativeEmitHandlerProto Emit_Range;
NativeEmitHandlerProto Emit_ModuloOnly;
NativeEmitHandlerProto Emit_Modulo;

#endif

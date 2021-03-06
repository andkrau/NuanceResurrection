#ifndef EMIT_MUL_H
#define EMIT_MUL_H

#include "mpe.h"

NativeEmitHandlerProto Emit_ADDM;
NativeEmitHandlerProto Emit_ADDMImmediate;
NativeEmitHandlerProto Emit_SUBM;
NativeEmitHandlerProto Emit_SUBMImmediateReverse;
NativeEmitHandlerProto Emit_MULScalarShiftAcshift;
NativeEmitHandlerProto Emit_MULScalarShiftRightImmediate;
NativeEmitHandlerProto Emit_MULScalarShiftLeftImmediate;
NativeEmitHandlerProto Emit_MULImmediateShiftAcshift;
NativeEmitHandlerProto Emit_MULScalarShiftScalar;
NativeEmitHandlerProto Emit_MULImmediateShiftScalar;
NativeEmitHandlerProto Emit_MULImmediateShiftRightImmediate;
NativeEmitHandlerProto Emit_MULImmediateShiftLeftImmediate;
NativeEmitHandlerProto Emit_MUL_SVImmediateShiftImmediate;
NativeEmitHandlerProto Emit_MUL_SVScalarShiftImmediate;
NativeEmitHandlerProto Emit_MUL_SVScalarShiftSvshift;
NativeEmitHandlerProto Emit_MUL_SVRuShiftImmediate;
NativeEmitHandlerProto Emit_MUL_SVRuShiftSvshift;
NativeEmitHandlerProto Emit_MUL_SVRvShiftImmediate;
NativeEmitHandlerProto Emit_MUL_SVRvShiftSvshift;
NativeEmitHandlerProto Emit_MUL_SVVectorShiftImmediate;
NativeEmitHandlerProto Emit_MUL_SVVectorShiftSvshift;
NativeEmitHandlerProto Emit_MUL_PImmediateShiftImmediate;
NativeEmitHandlerProto Emit_MUL_PScalarShiftImmediate;
NativeEmitHandlerProto Emit_MUL_PScalarShiftSvshift;
NativeEmitHandlerProto Emit_MUL_PRuShiftImmediate;
NativeEmitHandlerProto Emit_MUL_PRuShiftSvshift;
NativeEmitHandlerProto Emit_MUL_PRvShiftImmediate;
NativeEmitHandlerProto Emit_MUL_PRvShiftSvshift;
NativeEmitHandlerProto Emit_MUL_PVectorShiftImmediate;
NativeEmitHandlerProto Emit_MUL_PVectorShiftSvshift;
NativeEmitHandlerProto Emit_DOTPScalarShiftImmediate;
NativeEmitHandlerProto Emit_DOTPScalarShiftSvshift;
NativeEmitHandlerProto Emit_DOTPVectorShiftImmediate;
NativeEmitHandlerProto Emit_DOTPVectorShiftSvshift;

#endif

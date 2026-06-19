#ifndef EMBEDDED_SHADERS_H
#define EMBEDDED_SHADERS_H

#include <cstring>

// Embeds the GLSL shader sources into the binary so the libretro core still has
// a YCrCbA->RGB shader when no .vs/.fs files sit next to it (GetModuleFileName
// resolves to the RetroArch binary, not the core).
//
// The .vs/.fs files remain the single source of truth: each one is guarded with
//
//   #ifdef EMBED_HLSL
//   R"NUONSHADER(
//   #endif
//   ...shader code...
//   #ifdef EMBED_HLSL
//   )NUONSHADER"
//   #endif
//
// so the same file is valid both as standalone GLSL (the guards are skipped by
// the GLSL preprocessor) and, when included here with EMBED_HLSL defined, as a
// C++ raw-string literal. The embedded copy therefore can never drift from the
// on-disk source and there is no generated header.
//
// Caveat handled by ShaderProgram::InstallShaderSourceFromFile: because the raw
// string opens right after the leading "#ifdef EMBED_HLSL", it captures that
// guard's closing "#endif" at the very start and the trailing "#ifdef
// EMBED_HLSL" at the very end. The embedded source is therefore wrapped in
// "#if 1 ... #endif" before being handed to glShaderSource so those stray
// directives stay balanced for the GLSL preprocessor.
namespace {
#define EMBED_HLSL
static const char * const g_emb_video_generic_vs =
#include "video_generic.vs"
;
static const char * const g_emb_video_m32_o32_fs =
#include "video_m32_o32.fs"
;
static const char * const g_emb_video_m32_o32_crt_fs =
#include "video_m32_o32_crt.fs"
;
#undef EMBED_HLSL
}

static inline const char *GetEmbeddedShaderSource(const char *filename)
{
  if(!filename) return nullptr;
  if(strcmp(filename, "video_generic.vs") == 0) return g_emb_video_generic_vs;
  if(strcmp(filename, "video_m32_o32.fs") == 0) return g_emb_video_m32_o32_fs;
  if(strcmp(filename, "video_m32_o32_crt.fs") == 0) return g_emb_video_m32_o32_crt_fs;
  return nullptr;
}

#endif // EMBEDDED_SHADERS_H

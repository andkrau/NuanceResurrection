#ifdef EMBED_HLSL
R"NUONSHADER(
#endif
/*
Nuance generic vertex shader for all pixel modes:

This vertex shader simply passes in the texture coordinates set by
glMultiTexCoord and then sets gl_Position with the transformed vertex
coordinate.
*/

#ifdef GL_ES
precision highp float;
#endif

// Explicit attributes and varyings rather than gl_MultiTexCoord/gl_TexCoord and
// the fixed-function matrix: those are compatibility-profile built-ins that
// OpenGL ES does not have at all, and the client feeds these the same values it
// used to pass through glMultiTexCoord2fv.
attribute vec4 a_position;
attribute vec2 a_texcoord0;
attribute vec2 a_texcoord1;
attribute vec2 a_texcoord2;

uniform mat4 u_mvp;

varying vec2 v_texcoord0;
varying vec2 v_texcoord1;
varying vec2 v_texcoord2;

void main(void)
{
  v_texcoord0 = a_texcoord0;
  v_texcoord1 = a_texcoord1;
  v_texcoord2 = a_texcoord2;
  gl_Position = u_mvp * a_position;
}
#ifdef EMBED_HLSL
)NUONSHADER"
#endif

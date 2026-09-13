#pragma once

// One place that decides which GL headers the core gets. Desktop builds go
// through GLEW, which loads the entry points at runtime; OpenGL ES has no
// loader - the platform library exports everything - and including glew.h
// there fails outright ("gl2.h included before glew.h") before it gets as far
// as the entry points it declares that GLES does not have.
#if defined(NUANCE_GLES)
// ES 3.0: GL_RGBA8, GL_RG8 and GL_RG, which the video buffers are declared
// with and ES 2 does not have. gl3.h includes the ES 2 entry points as well.
#if defined(__APPLE__)
// Apple puts the same headers inside the OpenGLES framework under names of
// its own; there is no GLES3/gl3.h to find on iOS or tvOS.
#include <OpenGLES/ES3/gl.h>
#include <OpenGLES/ES3/glext.h>
#else
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#endif
#else
#include <GL/glew.h>
#endif

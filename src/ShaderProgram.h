#ifndef SHADERPROGRAM_H
#define SHADERPROGRAM_H

#include "basetypes.h"
#include "gl_headers.h"

class ShaderProgram final
{
public:
  ShaderProgram();
  ~ShaderProgram();

  bool Initialize();

  bool Uninitalize();

  bool InstallShaderSourceFromFile(const char * const filename, GLenum type);
  bool InstallShaderSourceFromMemory(char **sourceStrings, uint32 count, const int *lengths, GLenum type);
  bool CompileShader(GLenum type);
  bool Link();
  void PrintInfoLog(GLuint obj, const char *msg);
  bool AttachShader(GLenum type);
  bool DetachShader(GLenum type);
  bool CompileAndLinkShaders();
  bool StartShaderProgram();
  bool StopShaderProgram();
  GLuint GetProgramObject() { return hProgramObject; }
  GLuint GetVertexShaderObject() { return hVertexShaderObject; }
  GLuint GetFragmentShaderObject() { return hFragmentShaderObject; }

private:
  GLuint hVertexShaderObject;
  GLuint hFragmentShaderObject;
  GLuint hProgramObject;
  bool bVertexShaderObjectAttached;
  bool bFragmentShaderObjectAttached;
  bool bVertexShaderCodeLoaded;
  bool bFragmentShaderCodeLoaded;
};

#endif

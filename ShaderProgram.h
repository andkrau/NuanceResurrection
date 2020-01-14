#ifndef SHADERPROGRAM_H
#define SHADERPROGRAM_H

#include "BaseTypes.h"
#include "external\glew-2.1.0\include\GL\glew.h"

class ShaderProgram
{
public:
  ShaderProgram();
  ~ShaderProgram();

  bool ShaderProgram::Initialize();

  bool ShaderProgram::Uninitalize();

  bool InstallShaderSourceFromFile(char *filename, GLenum type);
  bool InstallShaderSourceFromMemory(char **sourceStrings, uint32 count, const int *lengths, GLenum type);
  bool CompileShader(GLenum type);
  bool Link();
  void PrintInfoLog(GLhandleARB obj, const char *msg);
  bool AttachShader(GLenum type);
  bool DetatchShader(GLenum type);
  bool CompileAndLinkShaders();
  bool StartShaderProgram();
  bool StopShaderProgram();
  GLhandleARB GetProgramObject() { return hProgramObject; }
  GLhandleARB GetVertexShaderObject() { return hVertexShaderObject; }
  GLhandleARB GetFragmentShaderObject() { return hFragmentShaderObject; }

protected:
  GLhandleARB hVertexShaderObject;
  GLhandleARB hFragmentShaderObject;
  GLhandleARB hProgramObject;
  bool bVertexShaderObjectAttached;
  bool bFragmentShaderObjectAttached;
  bool bVertexShaderCodeLoaded;
  bool bFragmentShaderCodeLoaded;
};
#endif

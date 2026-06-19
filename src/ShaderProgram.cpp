#include <string>
#include <cstdio>
#include <cstring>
#include "ShaderProgram.h"
#include "embedded_shaders.h"
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include "linux_compat.h"
#endif

ShaderProgram::ShaderProgram()
{
  hProgramObject = 0;
  hVertexShaderObject = 0;
  hFragmentShaderObject = 0;
  bVertexShaderObjectAttached = false;
  bVertexShaderCodeLoaded = false;
  bFragmentShaderObjectAttached = false;
  bFragmentShaderCodeLoaded = false;
}

ShaderProgram::~ShaderProgram()
{
  Uninitalize();
}

bool ShaderProgram::Initialize()
{  
  if(!hProgramObject)
  {
    hProgramObject = glCreateProgramObjectARB();
    if(!hProgramObject)
    {
      return false;
    }

    hVertexShaderObject = glCreateShaderObjectARB(GL_VERTEX_SHADER);
    hFragmentShaderObject = glCreateShaderObjectARB(GL_FRAGMENT_SHADER);
  }

  return true;
}

bool ShaderProgram::Uninitalize()
{
  if(hProgramObject)
  {
    if(hVertexShaderObject)
    {
      glDeleteObjectARB(hVertexShaderObject);
    }

    if(hFragmentShaderObject)
    {
      glDeleteObjectARB(hFragmentShaderObject);
    }

    glDeleteObjectARB(hProgramObject);
    hProgramObject = 0;
    hVertexShaderObject = 0;
    hFragmentShaderObject = 0;
    bVertexShaderObjectAttached = false;
    bFragmentShaderObjectAttached = false;
  }

  return true;
}

void ShaderProgram::PrintInfoLog(GLhandleARB obj, const char *msg)
{
  int32 blen = 0;   /* length of buffer to allocate */
  glGetObjectParameterivARB(obj, GL_OBJECT_INFO_LOG_LENGTH_ARB , &blen);
  if(blen > 1)
  {
    GLchar *infoLog = new GLchar[blen];
    int32 slen = 0;   /* strlen actually written to buffer */
    glGetInfoLogARB(obj, blen, &slen, infoLog);
    MessageBox(NULL,infoLog,msg,MB_OK);
    delete [] infoLog;
  }
}

bool ShaderProgram::InstallShaderSourceFromFile(const char * const filename, GLenum type)
{
  bool bStatus = false;

  FILE *inFile;
  errno_t err = fopen_s(&inFile,filename,"rb");
  if(err != 0)
  {
    char tmp[1024];
    GetModuleFileName(NULL, tmp, 1024);
    std::string tmps(tmp);
    size_t idx = tmps.find_last_of("/\\");
    if (idx != std::string::npos)
      tmps = tmps.substr(0, idx+1);
    err = fopen_s(&inFile,(tmps + filename).c_str(),"rb");
  }

  GLchar *buffer = nullptr;
  GLint length = 0;

  if(err == 0)
  {
    fseek(inFile,0,SEEK_END);
    length = ftell(inFile);

    buffer = new GLchar[length+1];
    buffer[length] = '\0';
    fseek(inFile,0,SEEK_SET);
    fread(buffer,sizeof(char),length,inFile);
    fclose(inFile);
  }
  else
  {
    // No shader file on disk (e.g. the libretro core has no .vs/.fs files next
    // to it - GetModuleFileName resolves to the RetroArch binary, not the core).
    // Fall back to the source compiled into the binary so rendering still works.
    const char * const embedded = GetEmbeddedShaderSource(filename);
    if(embedded)
    {
      // The embedded source is the .vs/.fs file turned into a C++ raw string.
      // Because the raw string opens right after the file's leading
      // "#ifdef EMBED_HLSL", it captures that guard's closing "#endif" at the
      // start and the trailing "#ifdef EMBED_HLSL" at the end. Wrap the whole
      // thing in "#if 1 ... #endif" so those stray directives stay balanced for
      // the GLSL preprocessor: the leading #endif closes our #if 1, and the
      // trailing #ifdef is closed by our appended #endif.
      static const char prefix[] = "#if 1\n";
      static const char suffix[] = "\n#endif\n";
      const size_t embLen = strlen(embedded);
      length = (GLint)(sizeof(prefix)-1 + embLen + sizeof(suffix)-1);
      buffer = new GLchar[length+1];
      memcpy(buffer, prefix, sizeof(prefix)-1);
      memcpy(buffer + sizeof(prefix)-1, embedded, embLen);
      memcpy(buffer + sizeof(prefix)-1 + embLen, suffix, sizeof(suffix)); // incl. '\0'
    }
  }

  if(buffer)
  {
    const GLchar **pBuffer = (const GLchar **)(&buffer);

    if(type == GL_FRAGMENT_SHADER)
    {
      if(hFragmentShaderObject)
      {
        glShaderSourceARB(hFragmentShaderObject,1,pBuffer,&length);
        bFragmentShaderCodeLoaded = true;
        bStatus = true;
      }
    }
    else if(type == GL_VERTEX_SHADER)
    {
      if(hVertexShaderObject)
      {
        glShaderSourceARB(hVertexShaderObject,1,pBuffer,NULL);
        bVertexShaderCodeLoaded = true;
        bStatus = true;
      }
    }

    delete [] buffer;
  }

  return bStatus;
}

bool ShaderProgram::InstallShaderSourceFromMemory(char **sourceStrings, uint32 count, const int *lengths, GLenum type)
{
  bool bStatus = false;

  if(type == GL_FRAGMENT_SHADER)
  {
    if(hFragmentShaderObject)
    {
      glShaderSourceARB(hFragmentShaderObject,count,(const char **)sourceStrings,lengths);
      bFragmentShaderCodeLoaded = true;
      bStatus = true;
    }
  }
  else if(type == GL_VERTEX_SHADER)
  {
    if(hVertexShaderObject)
    {
      glShaderSourceARB(hVertexShaderObject,1,(const char **)sourceStrings,lengths);
      bVertexShaderCodeLoaded = true;
      bStatus = true;
    }
  } 

  return bStatus;
}

bool ShaderProgram::CompileShader(GLenum type)
{
  bool bStatus = false;
  GLint bCompiled = GL_FALSE;

  if(type == GL_FRAGMENT_SHADER)
  {
    if(hFragmentShaderObject && bFragmentShaderCodeLoaded)
    {
      glCompileShaderARB(hFragmentShaderObject);
      glGetObjectParameterivARB(hFragmentShaderObject, GL_OBJECT_COMPILE_STATUS_ARB, &bCompiled);
      bStatus = bCompiled;
    }
  }
  else if(type == GL_VERTEX_SHADER)
  {
    if(hVertexShaderObject && bVertexShaderCodeLoaded)
    {
      glCompileShaderARB(hVertexShaderObject);
      glGetObjectParameterivARB(hVertexShaderObject, GL_OBJECT_COMPILE_STATUS_ARB, &bCompiled);
      bStatus = bCompiled;
    }
  }

  return bStatus;
}

bool ShaderProgram::Link()
{
  if(!hProgramObject || !(bVertexShaderObjectAttached || bFragmentShaderObjectAttached))
    return false;

  glLinkProgramARB(hProgramObject);
  GLint bLinked = GL_FALSE;
  glGetObjectParameterivARB(hProgramObject, GL_OBJECT_LINK_STATUS_ARB, &bLinked);
  const bool bStatus = bLinked;

  return bStatus;
}

bool ShaderProgram::AttachShader(GLenum type)
{
  if(!hProgramObject)
  {
    return false;
  }

  if(type == GL_FRAGMENT_SHADER)
  {
    if(!hFragmentShaderObject)
    {
      return false;
    }

    if(!bFragmentShaderObjectAttached)
    {
      bFragmentShaderObjectAttached = true;
      glAttachObjectARB(hProgramObject,hFragmentShaderObject);
    }
  }
  else if(type == GL_VERTEX_SHADER)
  {
    if(!hVertexShaderObject)
    {
      return false;
    }

    if(!bVertexShaderObjectAttached)
    {
      bVertexShaderObjectAttached = true;
      glAttachObjectARB(hProgramObject,hVertexShaderObject);
    }
  } 

  return true;
}

bool ShaderProgram::DetachShader(GLenum type)
{
  if(!hProgramObject)
  {
    return false;
  }

  if(type == GL_FRAGMENT_SHADER)
  {
    if(!hFragmentShaderObject)
    {
      return false;
    }

    if(bFragmentShaderObjectAttached)
    {
      bFragmentShaderObjectAttached = false;
      glDetachObjectARB(hProgramObject,hFragmentShaderObject);
    }
  }
  else if(type == GL_VERTEX_SHADER)
  {
    if(!hVertexShaderObject)
    {
      return false;
    }

    if(bVertexShaderObjectAttached)
    {
      bVertexShaderObjectAttached = false;
      glDetachObjectARB(hProgramObject,hVertexShaderObject);
    }
  } 

  return true;
}

bool ShaderProgram::CompileAndLinkShaders()
{
  if(!hProgramObject || !(bFragmentShaderCodeLoaded || bVertexShaderCodeLoaded))
    return false;

  GLint bCompiled;
  if(hFragmentShaderObject && bFragmentShaderCodeLoaded)
  {
    glCompileShaderARB(hFragmentShaderObject);
    glGetObjectParameterivARB(hFragmentShaderObject, GL_OBJECT_COMPILE_STATUS_ARB, &bCompiled);
    if(!bCompiled)
    {
      PrintInfoLog(hFragmentShaderObject,"Fragment Shader Compile Error");
      return false;
    }
  }

  if(hVertexShaderObject && bVertexShaderCodeLoaded)
  {
    glCompileShaderARB(hVertexShaderObject);
    glGetObjectParameterivARB(hVertexShaderObject, GL_OBJECT_COMPILE_STATUS_ARB, &bCompiled);
    if(!bCompiled)
    {
      PrintInfoLog(hVertexShaderObject,"Vertex Shader Compile Error");
      return false;
    }
  }

  glLinkProgramARB(hProgramObject);
  GLint bLinked;
  glGetObjectParameterivARB(hProgramObject, GL_OBJECT_LINK_STATUS_ARB, &bLinked);
  const bool bStatus = bLinked;
  if(!bLinked)
  {
    PrintInfoLog(hProgramObject,"Shader Program Link Error");
  }
  return bStatus;
}

bool ShaderProgram::StartShaderProgram()
{
  glUseProgramObjectARB(hProgramObject);
  return true;
}

bool ShaderProgram::StopShaderProgram()
{
  glUseProgramObjectARB(0);
  return true;
}

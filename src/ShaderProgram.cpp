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
    hProgramObject = glCreateProgram();
    if(!hProgramObject)
    {
      return false;
    }

    hVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
    hFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
  }

  return true;
}

bool ShaderProgram::Uninitalize()
{
  if(hProgramObject)
  {
    if(hVertexShaderObject)
    {
      glDeleteShader(hVertexShaderObject);
    }

    if(hFragmentShaderObject)
    {
      glDeleteShader(hFragmentShaderObject);
    }

    glDeleteProgram(hProgramObject);
    hProgramObject = 0;
    hVertexShaderObject = 0;
    hFragmentShaderObject = 0;
    bVertexShaderObjectAttached = false;
    bFragmentShaderObjectAttached = false;
  }

  return true;
}

void ShaderProgram::PrintInfoLog(GLuint obj, const char *msg)
{
  // The ARB extension had one call for both; the core API this now uses - and
  // the only one OpenGL ES has - splits them, so ask the object which it is.
  const bool bIsShader = glIsShader(obj) == GL_TRUE;
  int32 blen = 0;   /* length of buffer to allocate */
  if(bIsShader)
    glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &blen);
  else
    glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &blen);
  if(blen > 1)
  {
    GLchar *infoLog = new GLchar[blen];
    int32 slen = 0;   /* strlen actually written to buffer */
    if(bIsShader)
      glGetShaderInfoLog(obj, blen, &slen, infoLog);
    else
      glGetProgramInfoLog(obj, blen, &slen, infoLog);
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
        glShaderSource(hFragmentShaderObject,1,pBuffer,&length);
        bFragmentShaderCodeLoaded = true;
        bStatus = true;
      }
    }
    else if(type == GL_VERTEX_SHADER)
    {
      if(hVertexShaderObject)
      {
        glShaderSource(hVertexShaderObject,1,pBuffer,NULL);
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
      glShaderSource(hFragmentShaderObject,count,(const char **)sourceStrings,lengths);
      bFragmentShaderCodeLoaded = true;
      bStatus = true;
    }
  }
  else if(type == GL_VERTEX_SHADER)
  {
    if(hVertexShaderObject)
    {
      glShaderSource(hVertexShaderObject,1,(const char **)sourceStrings,lengths);
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
      glCompileShader(hFragmentShaderObject);
      glGetShaderiv(hFragmentShaderObject, GL_COMPILE_STATUS, &bCompiled);
      bStatus = bCompiled;
    }
  }
  else if(type == GL_VERTEX_SHADER)
  {
    if(hVertexShaderObject && bVertexShaderCodeLoaded)
    {
      glCompileShader(hVertexShaderObject);
      glGetShaderiv(hVertexShaderObject, GL_COMPILE_STATUS, &bCompiled);
      bStatus = bCompiled;
    }
  }

  return bStatus;
}

bool ShaderProgram::Link()
{
  if(!hProgramObject || !(bVertexShaderObjectAttached || bFragmentShaderObjectAttached))
    return false;

  glLinkProgram(hProgramObject);
  GLint bLinked = GL_FALSE;
  glGetProgramiv(hProgramObject, GL_LINK_STATUS, &bLinked);
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
      glAttachShader(hProgramObject,hFragmentShaderObject);
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
      glAttachShader(hProgramObject,hVertexShaderObject);
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
      glDetachShader(hProgramObject,hFragmentShaderObject);
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
      glDetachShader(hProgramObject,hVertexShaderObject);
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
    glCompileShader(hFragmentShaderObject);
    glGetShaderiv(hFragmentShaderObject, GL_COMPILE_STATUS, &bCompiled);
    if(!bCompiled)
    {
      PrintInfoLog(hFragmentShaderObject,"Fragment Shader Compile Error");
      return false;
    }
  }

  if(hVertexShaderObject && bVertexShaderCodeLoaded)
  {
    glCompileShader(hVertexShaderObject);
    glGetShaderiv(hVertexShaderObject, GL_COMPILE_STATUS, &bCompiled);
    if(!bCompiled)
    {
      PrintInfoLog(hVertexShaderObject,"Vertex Shader Compile Error");
      return false;
    }
  }

  glLinkProgram(hProgramObject);
  GLint bLinked;
  glGetProgramiv(hProgramObject, GL_LINK_STATUS, &bLinked);
  const bool bStatus = bLinked;
  if(!bLinked)
  {
    PrintInfoLog(hProgramObject,"Shader Program Link Error");
  }
  return bStatus;
}

bool ShaderProgram::StartShaderProgram()
{
  glUseProgram(hProgramObject);
  return true;
}

bool ShaderProgram::StopShaderProgram()
{
  glUseProgram(0);
  return true;
}

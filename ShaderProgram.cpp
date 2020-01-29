#include <stdio.h>
#include "ShaderProgram.h"
#include "Windows.h"
#include <assert.h>

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

  FILE *inFile = fopen(filename,"rb");

  if(inFile)
  {
    fseek(inFile,0,SEEK_END);
    GLint length = ftell(inFile);

    GLchar *buffer = new GLchar[length+1];
    const GLchar **pBuffer = (const GLchar **)(&buffer);
    buffer[length] = '\0';
    fseek(inFile,0,SEEK_SET);
    fread(buffer,sizeof(char),length,inFile);

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
    fclose(inFile);
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
  bool bStatus = false;
  GLint bLinked = GL_FALSE;

  if(!hProgramObject || !(bVertexShaderObjectAttached || bFragmentShaderObjectAttached))
  {
    return false;
  }

  glLinkProgramARB(hProgramObject);
  glGetObjectParameterivARB(hProgramObject, GL_OBJECT_LINK_STATUS_ARB, &bLinked);
  bStatus = bLinked;

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
  bool bStatus = false;
  GLint bLinked, bCompiled;

  if(!hProgramObject || !(bFragmentShaderCodeLoaded || bVertexShaderCodeLoaded))
  {
    return false;
  }

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
  glGetObjectParameterivARB(hProgramObject, GL_OBJECT_LINK_STATUS_ARB, &bLinked);
  bStatus = bLinked;
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

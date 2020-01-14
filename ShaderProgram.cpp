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

    hVertexShaderObject = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
    hFragmentShaderObject = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
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

    if(hVertexShaderObject)
    {
      glDeleteObjectARB(hVertexShaderObject);
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
  int32 slen = 0;   /* strlen actually written to buffer */
  GLcharARB *infoLog;
  glGetObjectParameterivARB(obj, GL_OBJECT_INFO_LOG_LENGTH_ARB , &blen);
  if(blen > 1)
  {
    if((infoLog = new GLcharARB[blen]) == 0) 
    {
      MessageBox(NULL,"OpenGLShader could not allocate InfoLog buffer","Error",MB_OK);
      return;
    }
    glGetInfoLogARB(obj, blen, &slen, infoLog);
    MessageBox(NULL,infoLog,msg,MB_OK);
    delete [] infoLog;
 }
}

bool ShaderProgram::InstallShaderSourceFromFile(char *filename, GLenum type)
{
  FILE *inFile;
  GLint length;
  GLcharARB *buffer;
  const GLcharARB **pBuffer = (const GLcharARB **)(&buffer);
  bool bStatus = false;

  inFile = fopen(filename,"rb");

  if(inFile)
  {
    fseek(inFile,0,SEEK_END);
    length = ftell(inFile);

    buffer = new GLcharARB[length+1];
    buffer[length] = '\0';
    fseek(inFile,0,SEEK_SET);
    fread(buffer,sizeof(char),length,inFile);

    if(type == GL_FRAGMENT_SHADER_ARB)
    {
      if(hFragmentShaderObject)
      {
        glShaderSourceARB(hFragmentShaderObject,1,pBuffer,&length);
        bFragmentShaderCodeLoaded = true;
        bStatus = true;
      }
    }
    else if(type == GL_VERTEX_SHADER_ARB)
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

  if(type == GL_FRAGMENT_SHADER_ARB)
  {
    if(hFragmentShaderObject)
    {
      glShaderSourceARB(hFragmentShaderObject,count,(const char **)sourceStrings,lengths);
      bFragmentShaderCodeLoaded = true;
      bStatus = true;
    }
  }
  else if(type == GL_VERTEX_SHADER_ARB)
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

  if(type == GL_FRAGMENT_SHADER_ARB)
  {
    if(hFragmentShaderObject && bFragmentShaderCodeLoaded)
    {
      glCompileShaderARB(hFragmentShaderObject);
      glGetObjectParameterivARB(hFragmentShaderObject, GL_OBJECT_COMPILE_STATUS_ARB, &bCompiled);
      bStatus = bCompiled;
    }
  }
  else if(type == GL_VERTEX_SHADER_ARB)
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
  bool status = false;
  GLint bCompiled = GL_FALSE;

  if(!hProgramObject)
  {
    return false;
  }

  if(type == GL_FRAGMENT_SHADER_ARB)
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
  else if(type == GL_VERTEX_SHADER_ARB)
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

bool ShaderProgram::DetatchShader(GLenum type)
{
  bool status = false;
  GLint bCompiled = GL_FALSE;

  if(!hProgramObject)
  {
    return false;
  }

  if(type == GL_FRAGMENT_SHADER_ARB)
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
  else if(type == GL_VERTEX_SHADER_ARB)
  {
    if(!hVertexShaderObject)
    {
      return false;
    }

    if(!bVertexShaderObjectAttached)
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
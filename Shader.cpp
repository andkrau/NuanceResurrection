#include <cstdio>
#include "Shader.h"

void ShaderProgram::ShaderProgram()
{
  hProgramObject = 0;
  hVertexShaderObject = 0;
  hFragmentShaderObject = 0;
  bVertexShaderObjectAttached = false;
  bFragmentShaderObjectAttached = false;
}

void ShaderProgram::~ShaderProgram()
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

    hVertexShaderObject = glCreateShaderObject(GL_ARB_VERTEX_SHADER);
    hFragmentShaderObject = glCreateShaderObject(GL_ARB_FRAGMENT_SHADER);
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
    hShaderProgram = 0;
    hVertexShaderProgram = 0;
    hFragmentShaderProgram = 0;
    bVertexShaderObjectAttached = false;
    bFragmentShaderObjectAttached = false;
  }

  return true;
}

bool ShaderProgram::InstallShaderSourceFromFile(uchar *filename, GLshaderType type)
{
  FILE *inFile;
  uint32 length;
  uchar *buffer;
  int32 nextChar;
  bool bStatus = false;


  inFile = fopen(filename,"r");

  if(inFile)
  {
    fseek(inFile,0,SEEK_END);
    length = ftell(inFile) + 1;

    buffer = new uchar[length];
    fseek(inFile,0,SEEK_SET);
    fread(buffer,1,sizeof(buffer),inFile);

    if(type == GL_FRAGMENT_SHADER_ARB)
    {
      if(hFragmentShaderObject)
      {
        glShaderSourceARB(hFragmentShaderObject,1,&buffer,&length);
        bStatus = true;
      }
    }
    else if(type == GL_VERTEX_SHADER_ARB)
    {
      if(hVertexShaderObject)
      {
        glShaderSourceARB(hVertexShaderObject,1,&buffer,&length);
        bStatus = true;
      }
    }

    delete [] buffer;
  }

  return bStatus;
}

bool ShaderProgram::InstallShaderSourceFromMemory(uchar **sourceStrings, uint32 count, const int *lengths, GLshaderType type)
{
  bool bStatus = false;

  if(type == GL_FRAGMENT_SHADER_ARB)
  {
    if(hFragmentShaderObject)
    {
      glShaderSourceARB(hFragmentShaderObject,count,sourceStrings,lengths);
      bStatus = true;
    }
  }
  else if(type == GL_VERTEX_SHADER_ARB)
  {
    if(hVertexShaderObject)
    {
      glShaderSourceARB(hVertexShaderObject,1,&buffer,&length);
      bStatus = true;
    }
  }

  return bStatus;
}

bool ShaderProgram::CompileShader(GLshaderType type)
{
  bool bStatus = false;
  GLboolean bCompiled = GL_FALSE;

  if(type == GL_FRAGMENT_SHADER_ARB)
  {
    if(hFragmentShaderObject)
    {
      glCompileShaderARB(hFragmentShaderObject);
      glGetObjectParameterivARB(hFragmentShaderObject, GL_OBJECT_COMPILE_STATUS_ARB, &bCompiled);
      bStatus = bCompiled;
    }
  }
  else if(type == GL_VERTEX_SHADER_ARB)
  {
    if(hVertexShaderObject)
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
  GLboolean linked = GL_FALSE;

  if(!hProgramObject)
  {
    return false;
  }

  glLinkProgramARB(hProgramObject);
  glGetObjectParameterivARB(hProgramObject, GL_OBJECT_LINK_STATUS_ARB, &bLinked);
  bStatus = bLinked;

  return bStatus;
}

bool ShaderProgram::AttachShader(GLshaderType type)
{
  bool status = false;
  GLboolean bCompiled = GL_FALSE;

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

bool ShaderProgram::DetatchShader(GLshaderType type)
{
  bool status = false;
  GLboolean bCompiled = GL_FALSE;

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
}

bool ShaderProgram::CompileAndLinkShaders()
{
  bool bStatus = false;

  if(!hProgramObject)
  {
    return false;
  }

  if(hFragmentShaderObject)
  {
    glCompileShaderARB(hFragmentShaderObject);
    glGetObjectParameterivARB(hFragmentShaderObject, GL_OBJECT_COMPILE_STATUS_ARB, &bCompiled);
    if(!bCompiled)
    {
      return false;
    }
  }

  if(hVertexShaderObject)
  {
    glCompileShaderARB(hVertexShaderObject);
    glGetObjectParameterivARB(hVertexShaderObject, GL_OBJECT_COMPILE_STATUS_ARB, &bCompiled);
    if(!bCompiled)
    {
      return false;
    }
  }

  glLinkProgramARB(hProgramObject);
  glGetObjectParameterivARB(hProgramObject, GL_OBJECT_LINK_STATUS_ARB, &bLinked);
  status = bLinked;

  return bStatus;
}

bool ShaderProgram::StartShaderProgram()
{
  glUseProgramObjectARB(hProgramObject);
}

bool ShaderProgram::StopShaderProgram()
{
  glUseProgramObjectARB(0);
}

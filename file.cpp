#include "basetypes.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <io.h>
#include <stdlib.h>
#include <windows.h>
#include "byteswap.h"
#include "file.h"
#include "mpe.h"
#include "NuonEnvironment.h"

extern NuonEnvironment nuonEnv;
extern HWND hWndStdErr;
extern HWND hWndStdOut;

/*
int _FileOpen(const char *path, int access, int mode, int *errnum);
int _FileClose( int fd, int *errnum );
int _FileIoctl(int fd, int request, void *argp, int *errnum);
int _FileFstat(int fd, struct stat *buf, int *errnum);
int _FileLseek(int fd, int offset, int whence, int *errnum);
int _FileIsatty ( int fd, int *errnum );
int _FileStat(const char *path, struct stat *buf, int *errnum);
int _FileWrite(int fd, char *buf, int len, int *errnum);
int _FileRead(int fd, char *buf, int len, int *errnum);
int _FileLink(const char *oldpath, const char *newpath, int *errnum);
int _FileLstat(const char *file_name, struct stat *buf, int *errnum);
int _FileUnlink(const char *pathname, int *errnum);

struct stat
{
  dev_t		st_dev; //short
  ino_t		st_ino; //unsigned long
  mode_t	st_mode; //int
  short		st_nlink; //short
  uid_t		st_uid; //short
  gid_t		st_gid; //short
  dev_t		st_rdev; //short
  off_t		st_size; //long
  time_t	st_atime; //long
  int	st_spare1;
  time_t	st_mtime; //long
  int	st_spare2;
  time_t	st_ctime; //long
  int	st_spare3;
  long		st_blksize;
  long		st_blocks;
  long	st_spare4[2];
};
*/

int fdesc;

int fileDescriptors[16] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};

uint32 ConvertFlags(uint32 nuonFlags)
{
  uint32 result = nuonFlags & (_O_RDONLY|_O_WRONLY|O_RDWR|_O_APPEND);

  if(nuonFlags & _FCREAT)
  {
    result |= O_CREAT;
  }
  if(nuonFlags & _FTRUNC)
  {
    result |= O_TRUNC;
  }
  if(nuonFlags & _FEXCL)
  {
    result |= O_EXCL;
  }
  if(nuonFlags & _FBINARY)
  {
    result |= O_BINARY;
  }

  return (result | O_BINARY) & ~O_TEXT;
}

int FindFileDescriptorIndex(int fd)
{
  uint32 i = 0;

  for(i = 0; i < 16; i++)
  {
    if(fileDescriptors[i] == fd)
    {
      return i;
    }
  }

  return -1;
}

uint32 ConvertIFMT(uint32 hostIFMT)
{
  uint32 result = 0;

  //Convert MSVC file type flags to newlib format (fstat)
  if(hostIFMT & _S_IFCHR)
  {
    result |= NUON_IFCHR;
  }
  if(hostIFMT & _S_IFIFO)
  {
    result |= NUON_IFIFO;
  }
  if(hostIFMT & _S_IFREG)
  {
    result |= NUON_IFREG;
  }
  if(hostIFMT & _S_IREAD)
  {
    result |= NUON_S_IRUSR;
  }
  if(hostIFMT & _S_IWRITE)
  {
    result |= NUON_S_IWUSR;
  }
  if(hostIFMT & _S_IEXEC)
  {
    result |= NUON_S_IXUSR;
  }

  return result;
}

bool IsValidFileDescriptor(int fd)
{
  uint32 i;

  if(fd < 3)
  {
    for(i = fd - 3; i < 16; i++)
    {
      if(fileDescriptors[i] == fd)
      {
        return true;
      }
    }
  }

  return false;  
}

void ConvertSeparatorCharacters(char *pathname)
{
  if(pathname)
  {
    while(*pathname)
    {
      if(*pathname == '/')
      {
        *pathname = '\\';
      }
      
      pathname++;
    }
  }
}

void FileOpen(MPE &mpe)
{
  uint32 path = mpe.regs[0];
  uint32 access = mpe.regs[1];
  uint32 mode = mpe.regs[2];
  uint32 errnum = mpe.regs[3];
  uint32 *pErr;
  char name[513];

  char *pPath = (char *)nuonEnv.GetPointerToMemory(mpe,path);
  int index, fd;

  if((index = FindFileDescriptorIndex(-1)) >= 0)
  {
    access = ConvertFlags(access);
    //Windows will set the read-only flag after opening the file if _S_IWRITE is not specified within the mode bits
    mode = ConvertFlags(mode) | _S_IWRITE;

    if(!strncmp("/iso9660/",pPath,9))
    {
      pPath += 9;
    }
    else if(!strncmp("/udf/",pPath,5))
    {
      pPath += 5;
    }

    strcpy(name,nuonEnv.GetDVDBase());
    strcat(name,pPath);
    
    ConvertSeparatorCharacters(name);

    fd = _open(name,access,mode);
    if(fd == -1)
    {
      goto Error;
    }
    fileDescriptors[index] = fd;
    mpe.regs[0] = fd;
  }
  else
  {
Error:
    pErr = (uint32 *)nuonEnv.GetPointerToMemory(mpe,errnum);
    *pErr = errno;
    SwapScalarBytes(pErr);
    mpe.regs[0] = -1;
  }
  
}

void FileClose(MPE &mpe)
{
  int32 fd = mpe.regs[0];
  uint32 errnum = mpe.regs[1];
  uint32 *pErr;

  int result, index;

  if((index = FindFileDescriptorIndex(fd)) >= 0)
  {
    result = _close(fileDescriptors[index]);
    if(result == -1)
    {
      goto Error;
    }

    fileDescriptors[index] = -1;
    mpe.regs[0] = 0;
  }
  else
  {
Error:
    pErr = (uint32 *)nuonEnv.GetPointerToMemory(mpe, errnum);
    *pErr = EINVAL;
    SwapScalarBytes(pErr);
    mpe.regs[0] = -1;
  }
}

void FileRead(MPE &mpe)
{
  uint32 fd = mpe.regs[0];
  uint32 buf = mpe.regs[1];
  uint32 len = mpe.regs[2];
  uint32 errnum = mpe.regs[3];
  uint32 *pErr;
  int32 index, result;
  void *pBuf;

  if((index = FindFileDescriptorIndex(fd)) >= 0)
  {
    pBuf = nuonEnv.GetPointerToMemory(mpe, buf);
    result = _read(fileDescriptors[index], pBuf, len);
    if(result == -1)
    {
      goto Error;
    }
      mpe.regs[0] = result;
  }
  else
  {
Error:
    pErr = (uint32 *)nuonEnv.GetPointerToMemory(mpe, errnum);
    *pErr = EBADF;
    SwapScalarBytes(pErr);
    mpe.regs[0] = -1;
  }
}

void FileWrite(MPE &mpe)
{
  uint32 fd = mpe.regs[0];
  uint32 buf = mpe.regs[1];
  uint32 len = mpe.regs[2];
  uint32 errnum = mpe.regs[3];
  uint32 *pErr;
  int32 index, result;

  char *pBuf = (char *)nuonEnv.GetPointerToMemory(mpe,buf);

  if((fd == NUON_FD_STDOUT) || (fd == NUON_FD_STDERR))
  {
    nuonEnv.WriteFile(mpe,fd,buf,len);
    mpe.regs[0] = len;
  }
  else
  {
    if((index = FindFileDescriptorIndex(fd)) >= 0)
    {
      result = _write(fileDescriptors[index], pBuf, len);
      if(result == -1)
      {
        goto Error;
      }
      mpe.regs[0] = result;
    }
    else
    {
  Error:
      pErr = (uint32 *)nuonEnv.GetPointerToMemory(mpe, errnum);
      *pErr = errno;
      SwapScalarBytes(pErr);
      mpe.regs[0] = -1;
    }
  }
}

void FileIoctl(MPE &mpe)
{
  uint32 fd = mpe.regs[0];
  uint32 request = mpe.regs[1];
  uint32 argp = mpe.regs[2];
  uint32 errnum = mpe.regs[3];

  MessageBox(NULL,"This handler does nothing","Unimplemented File Call: FileIoctl",MB_OK);
}

void FileFstat(MPE &mpe)
{
  int32 index, result;
  uint32 *pErr;
  struct _stat32 st;
  uint32 fd = mpe.regs[0];
  uint32 buf = mpe.regs[1];
  uint32 errnum = mpe.regs[2];

  nuon_stat *pBuf = (nuon_stat *)nuonEnv.GetPointerToMemory(mpe,buf);

  if(fd <= NUON_FD_STDERR)
  {
    mpe.regs[0] = -1;

    switch(fd)
    {
      case NUON_FD_STDIN:
        pBuf->st_mode = NUON_IFCHR | NUON_S_IRUSR | NUON_S_IRGRP | NUON_S_IROTH;
        pBuf->st_dev = pBuf->st_rdev = NUON_FD_STDIN;
        SwapWordBytes((uint16 *)&pBuf->st_dev);
        SwapWordBytes((uint16 *)&pBuf->st_rdev);
        SwapScalarBytes((uint32 *)&pBuf->st_mode);
        mpe.regs[0] = 0;
        break;
      case NUON_FD_STDOUT:
        pBuf->st_mode = NUON_IFCHR | NUON_S_IWUSR | NUON_S_IWGRP | NUON_S_IWOTH;
        pBuf->st_dev = pBuf->st_rdev = NUON_FD_STDOUT;
        SwapWordBytes((uint16 *)&pBuf->st_dev);
        SwapWordBytes((uint16 *)&pBuf->st_rdev);
        SwapScalarBytes((uint32 *)&pBuf->st_mode);
        mpe.regs[0] = 0;
        break;
      case NUON_FD_STDERR:
        pBuf->st_mode = NUON_IFCHR | NUON_S_IWUSR | NUON_S_IWGRP | NUON_S_IWOTH;
        pBuf->st_dev = pBuf->st_rdev = NUON_FD_STDERR;
        SwapWordBytes((uint16 *)&pBuf->st_dev);
        SwapWordBytes((uint16 *)&pBuf->st_rdev);
        SwapScalarBytes((uint32 *)&pBuf->st_mode);
        mpe.regs[0] = 0;
        break;
    }
  }
  else
  {
    if((index = FindFileDescriptorIndex(fd)) >= 0)
    {
      result = _fstat32(fd, &st);
      if(result == -1)
      {
        goto Error;
      }
      
      pBuf->st_atime = st.st_atime;
      pBuf->st_blksize = 1;
      pBuf->st_blocks = st.st_size;
      pBuf->st_ctime = st.st_ctime;
      pBuf->st_dev = st.st_dev;
      pBuf->st_gid = st.st_gid;
      pBuf->st_ino = st.st_ino;
      pBuf->st_mode = ConvertIFMT(st.st_mode);
      pBuf->st_mtime = st.st_mtime;
      pBuf->st_nlink = st.st_nlink;
      pBuf->st_rdev = st.st_rdev;
      pBuf->st_size = st.st_size;
      pBuf->st_uid = st.st_uid;

      SwapWordBytes((uint16 *)&pBuf->st_dev);
      SwapWordBytes((uint16 *)&pBuf->st_nlink);
      SwapWordBytes((uint16 *)&pBuf->st_gid);
      SwapWordBytes((uint16 *)&pBuf->st_uid);
      SwapWordBytes((uint16 *)&pBuf->st_rdev);
      SwapScalarBytes((uint32 *)&pBuf->st_atime);
      SwapScalarBytes((uint32 *)&pBuf->st_ctime);
      SwapScalarBytes((uint32 *)&pBuf->st_mtime);
      SwapScalarBytes((uint32 *)&pBuf->st_ino);
      SwapScalarBytes((uint32 *)&pBuf->st_mode);      
      SwapScalarBytes((uint32 *)&pBuf->st_size);
      SwapScalarBytes((uint32 *)&pBuf->st_blocks);      
      SwapScalarBytes((uint32 *)&pBuf->st_blksize);      
      mpe.regs[0] = 0;
    }
    else
    {
  Error:
      pErr = (uint32 *)nuonEnv.GetPointerToMemory(mpe, errnum);
      *pErr = errno;
      SwapScalarBytes(pErr);
      mpe.regs[0] = -1;
    }
  }
}

void FileStat(MPE &mpe)
{
  uint32 path = mpe.regs[0];
  uint32 buf = mpe.regs[1];
  uint32 errnum = mpe.regs[2];

  char *pPath = (char *)nuonEnv.GetPointerToMemory(mpe,path);

  MessageBox(NULL,pPath,"Unimplemented File Call: FileStat",MB_OK);

  mpe.regs[0] = 0;
}

void FileIsatty(MPE &mpe)
{
  uint32 path = mpe.regs[0];
  uint32 errnum = mpe.regs[1];

  char *pPath = (char *)nuonEnv.GetPointerToMemory(mpe,path);

  MessageBox(NULL,pPath,"Unimplemented File Call: FileIsatty",MB_OK);
}

void FileLseek(MPE &mpe)
{
  uint32 fd = mpe.regs[0];
  uint32 offset = mpe.regs[1];
  uint32 whence = mpe.regs[2];
  uint32 errnum = mpe.regs[3];
  int32 index, result;
  uint32 *pErr;

  if((index = FindFileDescriptorIndex(fd)) >= 0)
  {
    result = _lseek(fd, offset, whence);
    if(result == -1)
    {
      goto Error;
    }
    mpe.regs[0] = result;
  }
  else
  {
Error:
    pErr = (uint32 *)nuonEnv.GetPointerToMemory(mpe, errnum);
    *pErr = EBADF;
    SwapScalarBytes(pErr);
    mpe.regs[0] = -1;
  }
}

void FileLink(MPE &mpe)
{
  uint32 oldpath = mpe.regs[0];
  uint32 newpath = mpe.regs[1];
  uint32 errnum = mpe.regs[2];

  MessageBox(NULL,"This handler does nothing","Unimplemented File Call: FileLink",MB_OK);
}

void FileLstat(MPE &mpe)
{
  uint32 file_name = mpe.regs[0];
  uint32 buf = mpe.regs[1];
  uint32 errnum = mpe.regs[2];

  char *pFilename = (char *)nuonEnv.GetPointerToMemory(mpe,file_name);

  MessageBox(NULL,pFilename,"Unimplemented File Call: FileLstat",MB_OK);
}

void FileUnlink(MPE &mpe)
{
  uint32 pathname = mpe.regs[0];
  uint32 errnum = mpe.regs[1];

  char *pPathname = (char *)nuonEnv.GetPointerToMemory(mpe,pathname);

  MessageBox(NULL,pPathname,"Unimplemented File Call: FileUnlink",MB_OK);
}

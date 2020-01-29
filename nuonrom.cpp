#include <io.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include "byteswap.h"
#include "coff.h"
#include "nuonrom.h"

bool MPE::LoadNuonRomFile(const char * const filename)
{
  char linebuf[64];
  char intbuf[5] = "\0\0\0\0";
  int handle, bytesRead;
  uint32 offset, length;
  long where;

  handle = _open(filename,O_RDONLY|O_BINARY,0);
  if(handle >= 0)
  {
check_for_bles:
    bytesRead = _read(handle, &linebuf, 16);
    if(bytesRead == 16)
    {
      if(strncmp(linebuf,"Bles",4) == 0)
      {
        where = _tell(handle) - 16;
        //This is a BLES file, not a NUONROM-DISK file.
        if((linebuf[4] == 0) && linebuf[5] == 1)
        {
          //Version 1?  COFF offset value is at offset 0x52
          bytesRead = _lseek(handle, 0x52 - 0x10, SEEK_CUR);
          
          _read(handle, &linebuf, 2);
          intbuf[0] = linebuf[1];
          intbuf[1] = linebuf[0];
          offset = *((unsigned __int32 *)intbuf);
          _lseek(handle,where,SEEK_SET);
          goto load_coff_file;
        }
      }
      if(strncmp(linebuf,"NUONROM-DISK",12) == 0)
      {
        //skip to line containing "cd_app.cof"
        bytesRead = _read(handle, &linebuf, 48);
        if(bytesRead == 48)
        {
          bytesRead = _read(handle, &linebuf, 16);
          if(bytesRead == 16)
          {
            if(strncmp(linebuf,"cd_app.cof",10) == 0)
            {
              bytesRead = _read(handle, &offset, 4);
              SwapScalarBytes(&offset);
              bytesRead = _read(handle, &length, 4);
              SwapScalarBytes(&length);
              bytesRead = _read(handle, &linebuf, 8);
              _lseek(handle, 0, SEEK_SET);

              if(bytesRead == 8)
              {
                //seek to the file offset point
load_coff_file:
                bytesRead = _lseek(handle, offset, SEEK_CUR);
                if(bytesRead != -1)
                {
                  return LoadCoffFile(filename,true,handle);
                }
                else
                {
                  goto failure;
                }
              }
            }
            else if(strncmp(linebuf,"nuon.run",8) == 0)
            {
              bytesRead = _read(handle, &offset, 4);
              SwapScalarBytes(&offset);
              bytesRead = _read(handle, &length, 4);
              SwapScalarBytes(&length);
              bytesRead = _read(handle, &linebuf, 8);
              if(bytesRead == 8)
              {
                bytesRead = _lseek(handle, offset, SEEK_SET);
                if(bytesRead != -1)
                {
                  goto check_for_bles;
                }
                else
                {
                  goto failure;
                }
              }
              else
              {
                goto failure;
              }
            }
            else
            {
              goto failure;
            }
          }
          else
          {
            goto failure;
          }
        }
        else
        {
          goto failure;
        }
      }
      else
      {
        goto failure;
      }
    }
    else
    {
      goto failure;
    }
  }

failure:
  if (handle >= 0)
    _close(handle);

  return false;
}

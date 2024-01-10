#include <cstdio>

//struct __sFILE {
//  unsigned char *_p;	/* current position in (some) buffer */
//  int	_r;		/* read space left for getc() */
//  int	_w;		/* write space left for putc() */
//  short	_flags;		/* flags, below; this FILE is free if 0 */
//  short	_file;		/* fileno, if Unix descriptor, else -1 */
//  struct __sbuf _bf;	/* the buffer (at least 1 byte, if !NULL) */
//  int	_lbfsize;	/* 0 or -_bf._size, for inline putc */

//  /* operations */
//  _PTR	_cookie;	/* cookie passed to io functions */

//  int	_EXFUN((*_read),(_PTR _cookie, char *_buf, int _n));
//  int	_EXFUN((*_write),(_PTR _cookie, const char *_buf, int _n));
//  _fpos_t _EXFUN((*_seek),(_PTR _cookie, _fpos_t _offset, int _whence));
//  int	_EXFUN((*_close),(_PTR _cookie));

  /* separate buffer for long sequences of ungetc() */
//  struct __sbuf _ub;	/* ungetc buffer */
//  unsigned char *_up;	/* saved _p when _p is doing ungetc data */
//  int	_ur;		/* saved _r when _r is counting ungetc data */

  /* tricks to meet minimum requirements even when malloc() fails */
//  unsigned char _ubuf[3];	/* guarantee an ungetc() buffer */
//  unsigned char _nbuf[1];	/* guarantee a getc() buffer */

  /* separate buffer for fgetline() when line crosses buffer boundary */
//  struct __sbuf _lb;	/* buffer for fgetline() */

  /* Unix stdio files get aligned to block boundaries on fseek() */
//  int	_blksize;	/* stat.st_blksize (may be != _bf._size) */
//  int	_offset;	/* current lseek offset */

//  struct _reent *_data;
//};


int main(int argc, char *argv[])
{
  FILE *f;
  FILE *f2;
  char buf[512];
  long pos = 0;
  int result;

  f = fopen("filetest.txt","wb");
  f2 = fopen("test.bin","rb");
  fprintf(f, " before ftell -> r: %ld, w: %ld, ur: %ld, flags: 0x%lx, lbfsize: %ld, offset: %ld, seek: 0x%lx\n", f2->_r, f2->_w, f2->_ur, (long)f2->_flags, f2->_lbfsize, f2->_offset, f2->_seek);
  pos = ftell(f2);
  fprintf(f, "f2 pos: %ld\n", pos);
  fprintf(f, " before fseek -> r: %ld, w: %ld, ur: %ld, flags: 0x%lx, lbfsize: %ld, offset: %ld, seek: 0x%lx\n", f2->_r, f2->_w, f2->_ur, (long)f2->_flags, f2->_lbfsize, f2->_offset, f2->_seek);
  result = fseek(f2,0,SEEK_END);
  pos = ftell(f2);
  fprintf(f, "f2 pos: %ld\n", pos);
  if(result == -1)
  {
    fputs("Could not perform fseek to end of file\n",f);
  }
  fprintf(f, " after fseek -> r: %ld, w: %ld, ur: %ld, flags: 0x%lx, lbfsize: %ld, offset: %ld, seek: 0x%lx\n", f2->_r, f2->_w, f2->_ur, (long)f2->_flags, f2->_lbfsize, f2->_offset, f2->_seek);
  pos = ftell(f2);
  fprintf(f, "f2 pos: %ld\n", pos);
  fprintf(f, " after ftell -> r: %ld, w: %ld, ur: %ld, flags: 0x%lx, lbfsize: %ld, offset: %ld, seek: 0x%lx\n", f2->_r, f2->_w, f2->_ur, (long)f2->_flags, f2->_lbfsize, f2->_offset, f2->_seek);
  fprintf(f, "f2 pos: %ld\n", pos);
  fclose(f);
  fclose(f2);
  return 1;
}
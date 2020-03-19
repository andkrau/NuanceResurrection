#ifndef fileH
#define fileH

#define NUON_FD_STDIN (0UL)
#define NUON_FD_STDOUT (1UL)
#define NUON_FD_STDERR (2UL)

class MPE;

void FileOpen(MPE &mpe);
void FileClose(MPE &mpe);
void FileRead(MPE &mpe);
void FileWrite(MPE &mpe);
void FileIoctl(MPE &mpe);
void FileFstat(MPE &mpe);
void FileStat(MPE &mpe);
void FileIsatty(MPE &mpe);
void FileLseek(MPE &mpe);
void FileLink(MPE &mpe);
void FileLstat(MPE &mpe);
void FileUnlink(MPE &mpe);

#define	_FREAD      0x0001	/* read enabled */
#define	_FWRITE     0x0002	/* write enabled */
#define	_FNDELAY    0x0004	/* non blocking I/O (4.2 style) */
#define	_FAPPEND    0x0008	/* append (writes guaranteed at the end) */
#define	_FMARK      0x0010	/* internal; mark during gc() */
#define	_FDEFER     0x0020	/* internal; defer for next gc pass */
#define	_FASYNC     0x0040	/* signal pgrp when data ready */
#define	_FSHLOCK    0x0080	/* BSD flock() shared lock present */
#define	_FEXLOCK    0x0100	/* BSD flock() exclusive lock present */
#define	_FCREAT     0x0200	/* open with file create */
#define	_FTRUNC     0x0400	/* open with truncation */
#define	_FEXCL      0x0800	/* error on open if file exists */
#define	_FNBIO      0x1000	/* non blocking I/O (sys5 style) */
#define	_FSYNC      0x2000	/* do all writes synchronously */
#define	_FNONBLOCK  0x4000	/* non blocking I/O (POSIX style) */
#define	_FNOCTTY    0x8000	/* don't assign a ctty on this open */

#define _FBINARY    0x10000
#define _FTEXT      0x20000

#define	NUON_IFMT  0x0170000	/* type of file */
#define	NUON_IFDIR 0x0040000	/* directory */
#define	NUON_IFCHR 0x0020000	/* character special */
#define	NUON_IFBLK 0x0060000	/* block special */
#define	NUON_IFREG 0x0100000	/* regular */
#define	NUON_IFLNK 0x0120000	/* symbolic link */
#define	NUON_IFSOCK	0x0140000	/* socket */
#define	NUON_IFIFO 0x0010000	/* fifo */

#define	NUON_S_IRWXU 0x0000700	/* rwx, owner */
#define	NUON_S_IRUSR 0x0000400	/* read permission, owner */
#define	NUON_S_IWUSR 0x0000200	/* write permission, owner */
#define	NUON_S_IXUSR 0x0000100	/* execute/search permission, owner */
#define	NUON_S_IRWXG 0x0000070	/* rwx, group */
#define	NUON_S_IRGRP 0x0000040	/* read permission, group */
#define	NUON_S_IWGRP 0x0000020	/* write permission, grougroup */
#define	NUON_S_IXGRP 0x0000010	/* execute/search permission, group */
#define	NUON_S_IRWXO 0x0000007	/* rwx, other */
#define	NUON_S_IROTH 0x0000004	/* read permission, other */
#define	NUON_S_IWOTH 0x0000002	/* write permission, other */
#define	NUON_S_IXOTH 0x0000001	/* execute/search permission, other */
#define NUON_S_BLKSIZE 1024 /* size of a block */

struct nuon_stat
{
  __int16	st_dev; //short: drive number of disk or handle if file is on a device
  unsigned __int32	st_ino; //unsigned long
  __int32 st_mode; //int: bitmasked info about file mode
  __int16	st_nlink; //short
  __int16	st_uid; //short
  __int16	st_gid; //short
  __int16 st_rdev; //short
  __int32 st_size; //long: size of file in bytes
  __int32 st_atime; //long
  __int32 st_spare1;
  __int32 st_mtime; //long
  __int32 st_spare2;
  __int32 st_ctime; //long
  __int32 st_spare3;
  __int32 st_blksize;
  __int32 st_blocks;
  __int32 st_spare4[2];
};

#endif

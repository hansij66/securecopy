#ifndef _CRCSUM_H
#define _CRCSUM_H 1

#include <ftw.h>

#define MAX_BUF_LEN  (65536)
#define RESET_TEXT()  printf("\033[0;0m")

#define RESET		0
#define BRIGHT 		1
#define DIM             2
#define UNDERLINE 	3
#define BLINK		4
#define REVERSE		7
#define HIDDEN		8

#define BLACK 		0
#define RED             1
#define GREEN		2
#define YELLOW		3
#define BLUE		4
#define MAGENTA		5
#define CYAN		6

#define VERBOSE         0x01
#define STORE	        0x02
#define CHECK	        0x04
#define REMOVE	        0x10
#define RECURSE	        0x20
#define OVERWRITE	0x40
#define PRINT	        0x80
#define UPDATE          0x100
#define DEBUG           0x200
#define OMIT		0x400

// For option -e OMIT
#define MAXEXT		20


typedef unsigned long long t_crc64;
//typedef uint64_t t_crc64;

struct s_crc64
{
   t_crc64 crc;
   t_crc64 t;
//   time_t  t; this is not constant size accross 32 and 64bit OSses, resulting in xattr errors when copying files
};

t_crc64  CalculateCRC64(const char *filename);
int      processFile(const char *filename);
int      nftw_callback(const char *, const struct stat *, int, struct FTW *);
int      processCmdLine(const char *filename);
uint64_t crc64(uint64_t crc, const unsigned char *s, uint64_t l);
t_crc64  getCRC(const char *filename);
int      presentCRC64(const char *filename);
int      removeCRC(const char *filename);
int      bPutCRC(const char *filename, t_crc64 *crc, bool bError);
t_crc64  getCRC(const char *filename);
void     textcolor(int attr, int fg, int bg);
void     printStatus(const char* filename, const char* status, int fg);
int      IsPresentCRC64(const char *file);
int      IsFreshCRC(const char *filename);

#endif //define _CRCSUM_H

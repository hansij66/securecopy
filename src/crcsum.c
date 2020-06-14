/*
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    This software is based on Checkit version 0.2.0
    Dennis Katsonis; E-Mail dennisk (at) netspace.net.au
*/

#define _GNU_SOURCE  // asprintf

// This caused build issues on Debian Jessie. Not sure why. Not sure if disabling is an issue too
//#define _XOPEN_SOURCE 500  // nftw


// to overcome issue with file size & stat limitations on 32 bit systems
#define _FILE_OFFSET_BITS 64


#include <sys/types.h>
#include <attr/xattr.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <ftw.h>
#include <libgen.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <sys/statfs.h>
#include <stdint.h>
#include <time.h>

#include "config.h"

#include <unistd.h>

#include "crc64.h"
#include "crcsum.h"

int  flags = 0;
int  processed = 0;
int  failed = 0;
int  ext_counter = 0;
char arr_ext[MAXEXT][5];



////////////////////////////////////////////////////////////
// Check if CRC64 attribute is present.
// Returns
//   true:  success
//   false: if requested attribute is not present
////////////////////////////////////////////////////////////
int IsPresentCRC64(const char *file)
{
  char  buf[4096];
  char* current_attr;
  int   x;

  // size = listxattr(file, buff, 0);
  // use to dynamically allocate buf size

  x = listxattr(file, buf, 4096);
  current_attr = buf;

  if (x > 0)
  {
    do
    {
      if (strcmp(current_attr, "user.crc64") == 0)
      {
        return true;
      }
      else
      {
        current_attr += (strlen(current_attr) + 1);
      }
    }
    while ((current_attr - buf) < x);
  }
  else if (x == -1)
  {
    perror("IsPresentCRC64:listxattr");
  }
  return false;
}


////////////////////////////////////////////////////////////
// Check if CRC is fresh
// RETURN
//   true:  fresh
//   false: stale or CRC not present or on any stat(file) error
////////////////////////////////////////////////////////////
int IsFreshCRC(const char *filename)
{
  //int     attribute_format;
  struct  s_crc64 crc; //crc, t
  struct  stat    stat_file;

  if ( stat(filename, &stat_file) == -1 )
  {
    printStatus(filename, "ERROR", RED);
    perror("IsFreshCRC:stat");
    return false;
  }

  //attribute_format = IsPresentCRC64(filename);

  if ( IsPresentCRC64(filename) )
  {
    if ((getxattr(filename, "user.crc64", &crc, sizeof(crc)) == -1))
    {
      //printStatus(filename, "ERROR", RED);
      //perror("IsFreshCRC:getxattr");
      return false;
    }
    else
    {
      return (crc.t == stat_file.st_mtime);
    }
  }
  return 0;
}


//////////////////////////////////////////////////////////// 
// Removes the CRC
////////////////////////////////////////////////////////////
int removeCRC(const char *filename)
{
  if ( IsPresentCRC64(filename) )
  {
    if ((removexattr(filename, "user.crc64")) == -1)
    {
      printStatus(filename, "ERROR", RED);
      perror("removeCRC:removexattr");
      failed++;
      return false;
    }
    else
    {
      if (flags & VERBOSE)
      {
        printStatus(filename,"CRC removed", GREEN);
      }
    }
  }
  processed++;
  return true;
}

////////////////////////////////////////////////////////////
// Write CRC - depending on flags
//
//   filename: file under subject
//   *crc     : pointer to a t_crc64 where crc value will be stored; pointer can be NULL (and value will not be stored)
//              if *crc != null (crc value), the provided crc value will be stored; otherwise it will be calculated
//   bError   : ignore errors writing to xattr (in case of read only files)
//
// return:
//   false   : failed
//   true    : success
int bPutCRC(const char *filename, t_crc64 *crc, bool bError)
{
  struct  stat    stat_file;
  struct  s_crc64 s_crc; //crc, t

  if ( stat(filename, &stat_file) == -1 )
  {
    printStatus(filename, "ERROR", RED);
    perror("bPutCRC:stat");
    return false;
  }

  if (crc != NULL && *crc != 0)
  {
    // pointer to a crc value is there AND a non zero crc value is provided
    // use provided crc value
    s_crc.crc = *crc;
    s_crc.t   = stat_file.st_mtime;
  }

  if (crc != NULL && *crc == 0)
  {
    // pointer to a crc value is there AND a zero crc value is provided
    // Calculate crc value & return value
    s_crc.crc = CalculateCRC64(filename);
    s_crc.t   = stat_file.st_mtime;
    *crc      = s_crc.crc;
  }

  if (crc == NULL)
  {
    // Calculate crc value
    s_crc.crc = CalculateCRC64(filename);
    s_crc.t   = stat_file.st_mtime;
  }

  if ( (setxattr(filename, "user.crc64", &s_crc, sizeof(s_crc), 0 )) == -1 )
  {
    // print error when requested. In some cases (in expected read only file) it is OK to ignore error
    if (bError)
    {
      printStatus(filename, "ERROR", RED);
      perror("bPutCRC:setxattr");
    }
    return false;
  }
  else
  {
    processed++;
    return true;
  }

  processed++;
  return true;
}

// This retreives the CRC, first by checking for an extended attribute
// then by looking for a hidden file.  Returns 0 if unsuccessful, otherwise
// return the checksum.*/
t_crc64 getCRC(const char *filename)
{
  //int     attribute_format;
  struct  s_crc64 crc; //crc, t

  //attribute_format = IsPresentCRC64(filename);

  if ( !IsPresentCRC64(filename) )
  {
    //printf("%s: No CRC stored\n", filename);
    return false;
  }

  if ( IsPresentCRC64(filename) )
  {
    if ((getxattr(filename, "user.crc64", &crc, sizeof(crc)) == -1))
    {
      printStatus(filename, "ERROR", RED);
      perror("getCRC:getxattr");
      return false;
    }
    else
    {
      return crc.crc;
    }
  }
  return false;
}

////////////////////////////////////////////////////////////
// Open file and calculate CRC
////////////////////////////////////////////////////////////
t_crc64 CalculateCRC64(const char *filename)
{
  unsigned char buf[MAX_BUF_LEN];
  size_t bufread = MAX_BUF_LEN;
  int cont = 1;
  int fd;
  uint64_t tot = 0;
  uint64_t temp = 0;

  if ((fd = open(filename,O_RDONLY)) == -1)
  {
    printStatus(filename, "ERROR", RED);
    perror("CalculateCRC64:open");
    return 0;
  }

  while (cont)
  {
    bufread = read(fd, buf, bufread);
    if (bufread == -1)
    {
      printStatus(filename, "ERROR", RED);
      perror("CalculateCRC64:read");
      close(fd);
      return 0;
    }

    temp =  (t_crc64) crc64(temp, buf, (unsigned int)bufread);
    tot = tot + bufread;

    if (bufread < MAX_BUF_LEN)
    {
      cont = 0;
    }
  }

  close(fd);
  return temp;
}


////////////////////////////////////////////////////////////
// Return:
//  0:   Continue next in tree walk on nftw()
//  !=0: Terminate tree walk
////////////////////////////////////////////////////////////
int nftw_callback(const char *fpath, const struct stat *sb, int tflag, __attribute__((unused)) struct FTW *ftwbuf)
{
  // FTW_F also includes PIPES; not sure about SOCKETS etc
  // Hence, use stat information to check whether it is a regular file
  //  if ( tflag == FTW_F )
  if S_ISREG(sb->st_mode)
  {
    //Process File
    // TODO return value
    processFile(fpath);
    return 0;
  }
  else
  {
    if (flags & VERBOSE)
    {
      switch (sb->st_mode & S_IFMT)
      {
        case S_IFBLK:  printStatus(fpath, "Block Devioce", YELLOW); break;
        case S_IFCHR:  printStatus(fpath, "Char Deviced", YELLOW);  break;
        case S_IFDIR:  printStatus(fpath, "Directory", RED);        break;
        case S_IFIFO:  printStatus(fpath, "FIFO/pipe", YELLOW);     break;
        case S_IFLNK:  printStatus(fpath, "Symlink", YELLOW);       break;
        case S_IFSOCK: printStatus(fpath, "Socket", YELLOW);        break;
        case S_ISVTX:  printStatus(fpath, "S_ISVTX", YELLOW);       break;
        default:       printStatus(fpath, "Unknown?", RED); printf("%04X", (sb->st_mode & S_IFMT)); break;
      }
    }
  }

  // continue tree walk
  return 0;
}


////////////////////////////////////////////////////////////
// First entry after option flag processing for cmdline arguments procession
// Separate files from directories
////////////////////////////////////////////////////////////
int processCmdLine(const char *filename)
{
  struct stat statbuf;

  if ( lstat(filename, &statbuf) == -1 )
  {
    printStatus(filename, "ERROR", RED);
    perror("processCmdLine:stat");
    return false;
  }

  // check if we are a directory AND need to process recursively
  if ( S_ISDIR(statbuf.st_mode) && (flags & RECURSE) )
  {
    // Process Directory
    int typeflag = FTW_PHYS | FTW_MOUNT;  //Symbolic links are not followed; stay in same FS
    const int maxDirs = 100;

    if ( nftw( filename, nftw_callback, maxDirs, typeflag) == -1 )
    {
      printStatus(filename, "ERROR", RED);
      perror("processCmdLine:nftw");
      return false;
    }

    return true;
  }

  // Check if we are a Regular file (symbolic links are excluded)
  if ( S_ISREG(statbuf.st_mode) && !S_ISLNK(statbuf.st_mode) )
  {
    // Process File
    processFile(filename);
    return true;
  }
  else
  {
    if (flags & VERBOSE)
    {
      switch (statbuf.st_mode & S_IFMT)
      {
        case S_IFBLK:  printStatus(filename, "Block Devioce", YELLOW); break;
        case S_IFCHR:  printStatus(filename, "Char Deviced", YELLOW);  break;
        case S_IFDIR:  printStatus(filename, "Directory", RED);        break;
        case S_IFIFO:  printStatus(filename, "FIFO/pipe", YELLOW);     break;
        case S_IFLNK:  printStatus(filename, "Symlink", YELLOW);       break;
        case S_IFSOCK: printStatus(filename, "Socket", YELLOW);        break;
        case S_ISVTX:  printStatus(filename, "S_ISVTX", YELLOW);       break;
        default:       printStatus(filename, "Unknown?", RED); printf("%04X", (statbuf.st_mode & S_IFMT)); break;
      }
    }
  }

  return false;
}

////////////////////////////////////////////////////////////
// Expects a Regular File
////////////////////////////////////////////////////////////
int processFile(const char *filename)
{
  struct stat statbuf;

  if ( lstat(filename, &statbuf) == -1 )
  {
    printStatus(filename, "ERROR", RED);
    perror("processFile:lstat");
    return false;
  }

  // Print CRC64
  // Red:   CRC = stale
  // Green: CRC = fresh
  if (flags & PRINT)
  {
    t_crc64 crc;

    // print entries with fresh CRC in green text
    if ( (crc = getCRC(filename)) )
    {
      if ( IsFreshCRC(filename) )
      {
        char* szCRC;
        asprintf(&szCRC, "%016llX", crc);
        printStatus(filename, szCRC, GREEN);
        free(szCRC); szCRC = NULL;
        return true;
      }
      else if (flags & DEBUG)
      {
        char* szCRC;
        asprintf(&szCRC, "%016llX", crc);
        printStatus(filename, szCRC, RED);
        free(szCRC); szCRC = NULL;
        return true;
      }
    }

    // print entries without CRC in white text
    else if (flags & DEBUG)
    {
      if (statbuf.st_size == 0)
      {
        printStatus(filename," Zero File Size ", YELLOW);
      }
      else
      {
        printStatus(filename,"No CRC available", YELLOW);
      }
      return false;
    }
    else
    {
      return false;
    }
  }

  // STORE CRC but do not OVERWRITE or UPDATE existing CRC
  if ( (flags & STORE) && (!(flags & OVERWRITE)) && (!(flags & UPDATE)) )
  {
    if(IsPresentCRC64(filename))
    {
      if (flags & DEBUG)
      {
        printStatus(filename,"CRC already stored", YELLOW);
      }
      return false;
    }
    else if ( !bPutCRC(filename, NULL, true) )
    {
      fprintf(stderr, "%s: Failed to store CRC.\n", filename);
      return false;
    }
    else if (flags & VERBOSE)
    {
      printStatus(filename, "    CRC is saved  ", GREEN);
    }
    return true;
  }

  // OVERWRITE CRC
  if (flags & OVERWRITE)
  {
    if ( !bPutCRC(filename, NULL, true) )
    {
      //fprintf(stderr, "%s: Failed to overwrite CRC.\n", filename);
      return false;
    }
    else if (flags  & VERBOSE)
    {
      printStatus(filename, "    CRC is overwritten  ", GREEN);
    }
    return true;
  }

  // UPDATE CRC when stale
  if (flags & UPDATE)
  {
    if ( !IsFreshCRC(filename) )
    {
      // CRC is stale
      if ( !bPutCRC(filename, NULL, true) )
      {
        //printStatus(filename, "FAILED to update CRC", RED);
	    return false;
      }
      else if (flags  & VERBOSE)
      {
        printStatus(filename, " Updated CRC ", GREEN);
        return true;
      }
    }
    else if (flags & DEBUG)
    {
      printStatus(filename, "No CRC Update", YELLOW);
    }
    return true;
  }

  // Check CRC only if CRC is not stale
  if (flags & CHECK)
  {
    if ( IsFreshCRC(filename) )
    {
      // check if extension to be ommitted
      if (flags & OMIT)
      {
        // copy filename
        char *buffer;
        buffer = malloc(strlen(filename) + 1);
        strcpy(buffer, filename);

        char* p = NULL;
        char* prev_p = NULL;
        p = strtok(buffer, ".");
        while (p != NULL)
        {
          if (prev_p != NULL)
            free(prev_p);
            prev_p = NULL;

          prev_p = malloc(strlen(p) + 1);
          strcpy(prev_p, p);
          p = strtok (NULL, ".");
        }
        free(buffer);
        buffer = NULL;

        for (int i = 0; i < ext_counter; i++)
        {
          if (strcmp(arr_ext[i], prev_p) == 0)
          {
            if (flags & VERBOSE)
            {
              printStatus(filename, " OMIT ", GREEN);
            }
            processed++;
            free(prev_p);
            prev_p = NULL;
            return true;
          }
        }
        free(prev_p);
        prev_p = NULL;
      }
      if (CalculateCRC64(filename) == getCRC(filename))
      {
        if (flags & VERBOSE)
        {
          printStatus(filename, "  OK  ", GREEN);
        }
      }
      else
      {
        printStatus(filename, "FAILED", RED);
        failed++;
      }
      processed++;
    }
  } // End of Check CRC routine

  // Remove CRC
  if (flags & REMOVE)
  {
    if ( removeCRC(filename) )
    {
	  return true;
    }
    else
    {
      char path[PATH_MAX];
      getcwd(path, PATH_MAX);
      printf("%s/%s: Remove CRC failed\n", path, filename);
      return false;
    }
  } // End of Remove CRC routine
  return true;
}


// Print status message
void printStatus(const char* filename, const char* status, int fg)
{
  printf("[ ");
  textcolor(BRIGHT,fg,BLACK);
  printf("%s", status);
  RESET_TEXT();
  printf(" ] ");
  printf(" %-20s\n", filename);
}

// Change text colour
void textcolor(int attr, int fg, int bg)
{
  char command[13];

  // Command is the control command to the terminal
  sprintf(command, "%c[%d;%d;%dm", 0x1B, attr, fg + 30, bg + 40);
  printf("%s", command);
}

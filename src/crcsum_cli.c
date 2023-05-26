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

#include "config.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
// Provides:
//   extern char *optarg;
//   extern int   optind, opterr, optopt;

#include <sysexits.h>
#include "crcsum.h"

void print_help(void);

extern int flags;
extern int failed;
extern int processed;
extern int ext_counter;
extern char arr_ext[MAXEXT][5];


void print_help(void)
{
  printf("CRCSUM: A file checksum utility.\tVersion : %s\n",VERSION);
  puts("(C) Hans IJntema (2023)");
  puts("");
  puts("CRCSUM stores a CRC checksum and file mtime as an extended attribute.");
  puts("CRCSUM checks stored CRC checksum against actual CRC checksum");
  puts("CRCSUM allows for CRC updates, only when mtime has changed");
  puts("ZERO size files will not be checksum-ed.");
  puts("Directory & File  SYMLINKS will not be followed.");
  puts("NOTE: Some programs do change file content (even w/o saving),");
  puts("while mtime is not update. Example, some Microsoft Excel versions");
  puts("This will result in false positves when checking file against CRC.");
  puts("");
  puts("Options :");
  puts(" -a  Calculate and Add checksum (no overwrite)");
  puts(" -u  As -a, and Update stale checksum");
  puts(" -f  As -a, Force overwrite existing checksum");
  puts(" -c  Check file against stored checksum; stale CRC's are omitted");
  puts(" -e  Assumes -c; omit extension; one extension per option; eg -c -e xls -e xlt");
  puts(" -p  Print CRC64 checksum; stale CRC's are omitted; Add -d to print stale and missing CRC's");
  puts(" -v  Verbose.  Print more information");
  puts(" -x  Remove stored CRC64 checksum");
  puts(" -r  Recurse through directories");
  puts(" -d  Print Debug info. Implies -v");
  puts("");
  printf("Report errors and requests at %s\n\n", PACKAGE_BUGREPORT);
}


int main(int argc, char *argv[])
{
  int optch = 0;
  extern char *optarg;

  // get option characters (returns -1 if all are retrieved) 
  while (( optch = getopt(argc, argv,"ahe:ufcpvxrd") ) != -1)
  {
    switch (optch)
    {
      case 'h' :
        print_help();
        return false;
        break;

      case 'a' :
	    flags |= STORE;
	    break;

      case 'c' :
	    flags |= CHECK;
	    break;

      case 'e' :
            flags |= OMIT;
            flags |= CHECK;
            if (ext_counter < MAXEXT)
              {
                strcpy(arr_ext[ext_counter++], optarg);
              }
            break;

      case 'v' :
	    flags |= VERBOSE;
	    break;

      case 'x' :
	    flags |= REMOVE;
	    break;

      case 'u' :
	    flags |= UPDATE;
	    break;

      case 'f' :
	    flags |= OVERWRITE;
	    break;

      case 'r' :
	    flags |= RECURSE;
	    break;

      case 'p' :
	    flags |= PRINT;
	    break;

      case 'd' :
        flags |= VERBOSE;
	    flags |= DEBUG;
	    break;

      case '?' :
	    puts("Unknown option.");
	    puts("");
	    print_help();
	    break;
    } //switch
  } //while


/*  for (int i = 0; i < ext_counter; i++)
  {
    printf("arg=%s\n", arr_ext[i]);
  }*/


  // no arguments are passed
  if (argc <=1)
  {
    print_help();
    return(0);
  }
    /* Check for conflicting options */
    if (((flags & CHECK) && (flags & STORE)) || ((flags & CHECK) && (flags & OVERWRITE)) || ((flags & CHECK) && (flags & UPDATE)) )
    {
      puts("Cannot store and check CRC at same time.");
      return EX_USAGE;
    }

    if ((flags & STORE) && (flags & OVERWRITE)  && (flags & UPDATE))
    {
      puts("Cannot store/update and refresh at same time.");
      return EX_USAGE;
    }

   if ((flags & STORE) && (flags & REMOVE))
    {
      puts("Cannot remove and store CRC at same time.");
      return EX_USAGE;
    }
    if ((flags & CHECK) && (flags & REMOVE))
    {
      puts("Cannot remove and check CRC at same time.");
      return EX_USAGE;
    }

  //optind is the index of the next element to be processed in argv
  optch = optind;

  if (optch < argc)
  {
    do
    {
//      printf("ARGUMENT = %s\n", argv[optch]);
//      processFile(argv[optch]);
       processCmdLine(argv[optch]);
    }
    while ( ++optch < argc);
  }

  printf("%d file(s) processed.\n", processed);
  if (failed && processed)
  {
    printf("%d file(s) failed.\n", failed);
    return EXIT_FAILURE;
  } /* Return the number of failed checks if any errors. */
  else if ( processed && (flags & CHECK) )
  {
    printf("All file(s) OK.\n");
  }
  return EXIT_SUCCESS;
}



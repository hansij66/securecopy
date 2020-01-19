# SecureCopy

crccp and crcmv are based on GNU coreutils and add crc checksum functionality to cp and mv commands. 
Checksums can optionally be stored in the file's xattr. 
The command crcsum can be used for further checksum analysis/verification, or for storing a checksum in the xattr of all files in a directory or filesystem.


## Getting Started

This package is tested on Debian buster

### Prerequisites

* Test if [GNU Coreutils 8.31](https://ftp.gnu.org/gnu/coreutils/coreutils-8.31.tar.xz) can be build 
* automake-1.15

### Installing

* Download source
* Adapt "TARGETDIR="/opt" in "build-secure-copy", point to location where crccp, crcmv and crcsum should be located
* sudo ./build-secure-copy: this will download coreutils package, apply patch files, build and copy target files to "TARGETDIR"
* The other coreutils tools are not installed!

## Deployment

Add to ~/.bashrc:"
* alias cp='TARGETDIR/crccp -cx'
* alias mv='TARGETDIR/crcmv -cx'
* alias crcsum='TARGETDIR/crcsum'
* (replace TARGETDIR with actual location)


## Versioning
* V2.0: based on core utils v8.31 
* [Previous versions](https://sourceforge.net/projects/crcsum/https://sourceforge.net/projects/crcsum/)

## Authors

Hans IJntema

CRC64 routine:
The checksum routine is the crc-64-jones created by Salvatore Sanfilippo.

Coreutils authors: see coreutils documentation.

## License

This project is licensed under the GNU General Public License

## Details

crcsum -h

CRCSUM stores a CRC checksum and file mtime as an extended attribute.
CRCSUM checks stored CRC checksum against actual CRC checksum
CRCSUM allows for CRC updates, only when mtime has changed
ZERO size files will not be checksum-ed.
Directory & File  SYMLINKS will not be followed.
NOTE: Some programs do change file content (even w/o saving),
while mtime is not update. Example, some Microsoft Excel versions
This will result in false positves when checking file against CRC.

Options :
 -a  Calculate and Add checksum (no overwrite)
 -u  As -a, and Update stale checksum
 -f  As -a, Force overwrite existing checksum
 -c  Check file against stored checksum; stale CRC's are omitted
 -p  Print CRC64 checksum; stale CRC's are omitted; Add -d to print stale and missing CRC's
 -v  Verbose.  Print more information
 -x  Remove stored CRC64 checksum
 -r  Recurse through directories
 -d  Print Debug info. Implies -v


Additional or relevant flags for crccp and crcmv (compared to cp, mv)
* -c,  --crc    Check whether copy was successfull with checksum comparison
* -cx, --crc=x  As -c and checksum is stored in xattr of src and dest file (if file is writeable and if filesystem supports xattr); stored crc can be used in a next cp/mv or integrity check with crcsum...bitrot check); implies --preserve=all
* -v, --verbose Explain what is being done and display crc's created

After a copy or move, integrity of file can be checked (again) with crcsum -c -r <dir>

If a file with crc storedd in xattr has been changed afterwards, crc is flagged as stale, and ignored.

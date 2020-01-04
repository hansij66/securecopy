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
alias cp='TARGETDIR/crccp -cx'
alias mv='$TARGETDIR/crcmv -cx'
alias crcsum='$TARGETDIR/crcsum'
(replace TARGETDIR with actual location)

## Contributing

Please read [CONTRIBUTING.md](https://gist.github.com/PurpleBooth/b24679402957c63ec426) for details on our code of conduct, and the process for submitting pull requests to us.

## Versioning

* V2.0: based on core utils v8.31 
*[Previous versions](https://sourceforge.net/projects/crcsum/https://sourceforge.net/projects/crcsum/)

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


Additional flags for crccp and crcmv (compared to cp, mv)
  -c --crc[=x]                 check whether copy was successfull with checksum comparison
                               x: use (fresh) checksum stored in xattr or if not present,
                               store checksum in xattr
                               See also tool <crcsum> in same secure copy package
                               implies --preserve=all


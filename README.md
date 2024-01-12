# SecureCopy
crccp and crcmv are based on GNU coreutils and add crc checksum functionality to cp and mv commands. 
Checksums can optionally be stored in the file's xattr. 
Checksum is generated on source file and verified agains destination file

The command crcsum can be used for further checksum analysis/verification, or for storing a checksum in the xattr of all files in a directory or filesystem.
I mount my filesystems with `mount -t ext4 -o noatime -o user_xattr <device> <dir>` or specify user_xattr in `/etc/fstab` under options.


A use case:
- I run `crcsum -u -r .` on my home directory before daily backup; I have xattr enabled on my filesystems, checksums are embedded in all files and stored in the backup.
- I run once a month (cronjob) `crcsum -c -r <backupdestination>` to verify if backup is healthy
- And yes, I do run once or twice a year into a corrupt file (bit rot?); ofcourse your luck varies with quality of HD's and size of your backups 


## Getting Started
This package is tested on Debian buster & bullseye

### Prerequisites
* Test if [GNU Coreutils 9.3](https://ftp.gnu.org/gnu/coreutils/coreutils-9.3.tar.xz) can be build and fix any build issues 
* automake-1.15

### Installing
* Download securecopy sources
* Adapt `TARGETDIR="/opt"` in `build-secure-copy`, point to location where crccp, crcmv and crcsum should be located
* `sudo ./build-secure-copy`: this will download coreutils package, apply patch files, build and copy target files to "TARGETDIR"
* The other coreutils tools are not installed!
* "build-secure-copy" works for Debian OS-ses; adapt for other distributions (specifically, install required packages)

## Deployment
Add to `~/.bashrc:`
* alias cp='TARGETDIR/crccp -cx'
* alias mv='TARGETDIR/crcmv -cx'
* alias crcsum='TARGETDIR/crcsum'
* (replace TARGETDIR with actual location)

## Usage
* `cp -c sourcefile destination` # will re-use crc stored with sourcefile (if any)
* `cp -cx sourcefile destination` # will re-created crc of sourcefile befie copy is started

## Versioning
* V9.3.0: based on coreutils v9.3; Added simple test
* V9.0.0: based on coreutils v9.0; First 2 decimals of version number refer to coreutils version
* V2.1: based on coreutils v8.32
* V2.0: based on coreutils v8.31 
* [Previous versions](https://sourceforge.net/projects/crcsum/https://sourceforge.net/projects/crcsum/)

## Authors
Hans IJntema

CRC64 routine:
The checksum routine is the crc-64-jones created by Salvatore Sanfilippo.
Coreutils authors: see coreutils documentation.

## License
This project is licensed under the GNU General Public License

## Details
```
crcsum -h
CRCSUM stores a CRC checksum and file mtime as an extended attribute.
CRCSUM checks stored CRC checksum against actual CRC checksum
CRCSUM allows for CRC updates, only when mtime has changed
ZERO size files will not be checksum-ed.
Directory & File  SYMLINKS will not be followed.
NOTE: Some programs do change file content (even w/o saving),
while mtime is not update. Example, some Microsoft Excel versions
This will result in false positves when checking file against CRC.
```

```
Options :
-a  Calculate and Add checksum (no overwrite)
-u  As -a, and Update stale checksum
-f  As -a, Force overwrite existing checksum
-c  Check file against stored checksum; stale CRC's are omitted
-e  Assumes -c; omit files with specified extension; one extension per option; eg -e xls -e xlt
-p  Print CRC64 checksum; stale CRC's are omitted; Add -d to print stale and missing CRC's
-v  Verbose.  Print more information
-x  Remove stored CRC64 checksum
-r  Recurse through directories
-d  Print Debug info. Implies -v
```

```
Relevant additional or extended flags for crccp and crcmv (compared to cp, mv)
-c,  --crc	Check whether copy was successfull with checksum comparison
-cx, --crc=x	As -c and checksum is stored in xattr of src and dest file (if file is writeable and if filesystem supports xattr); 
		Stored crc can be used in a next cp/mv or integrity check with crcsum...bitrot check); 
		Implies --preserve=all
-v, --verbose	Explain what is being done and display crc's created
```
After a copy or move, integrity of file can be checked (again) with `crcsum -c -r \<directory\>`
If a file with crc stored in its xattr has been changed afterwards, the crc is flagged as stale, and ignored (based on timestamps)
Note: Microsoft Excel files (on Windows) are known to change the file content but not the timestamp if you just open the excel file for reading and close without saving. You can use `crcsum -e xls` to ignore Excel files.

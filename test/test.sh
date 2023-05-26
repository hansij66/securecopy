#!/bin/bash

CRCPATH="../bin"
CHECKSUM="BE2C65AC2D090191"
#CHECKSUM="BE2C65AC2D090192"
TESTFILE="fileundertest.txt"
CPFILE="copyfile.txt"


echo ""
echo ""
echo "## Basic test for crcsum and crccp ##"
echo ""

rm -f ${TESTFILE} ${CPFILE}

echo "TEST: Add CRC to file and compare with expected pre-calculated CRC"
echo "Create test file without checksum..."
dd if=<(yes foo) of=${TESTFILE} bs=1024 count=200 > /dev/null 2>&1

echo "Add checksum..."
${CRCPATH}/crcsum -a ${TESTFILE}  > /dev/null 2>&1
echo "Display checksum..."
OUTPUT=$(${CRCPATH}/crcsum -p ${TESTFILE}) > /dev/null 2>&1

# https://www.regextester.com/107384
# Extract calculated checksum
[[ ${OUTPUT} =~ (\[ )(.*)(.* \]) ]]
CHECKSUM_CALCULATED=(${BASH_REMATCH[2]})

echo "Test..."
if [[ "$OUTPUT" =~ .*"$CHECKSUM".* ]]; then
  echo "SUCCESS: Checksum calculated = ${CHECKSUM_CALCULATED}; Checksum expected = ${CHECKSUM}"
else
  echo "FAILURE: Checksum calculated = ${CHECKSUM_CALCULATED}; Checksum expected = ${CHECKSUM}"
  exit 1
fi

##################################################################################################
##################################################################################################
rm ${TESTFILE}
echo ""
echo "TEST: Copy file (crccp -cx) without source CRC and compare with expected pre-calculated CRC"
echo "Create test file without checksum..."
dd if=<(yes foo) of=${TESTFILE} bs=1024 count=200 > /dev/null 2>&1

echo "Copy file and test checksum of destination..."
${CRCPATH}/crccp -cx ${TESTFILE} ${CPFILE} > /dev/null 2>&1

echo "Display checksum..."
OUTPUT=$(${CRCPATH}/crcsum -p ${CPFILE})

# Extract calculated checksum
[[ ${OUTPUT} =~ (\[ )(.*)(.* \]) ]]
CHECKSUM_CALCULATED=(${BASH_REMATCH[2]})

echo "Test..."
if [[ "$OUTPUT" =~ .*"$CHECKSUM".* ]]; then
  echo "SUCCESS: Checksum calculated = ${CHECKSUM_CALCULATED}; Checksum expected = ${CHECKSUM}"
else
  echo "FAILURE: Checksum calculated = ${CHECKSUM_CALCULATED}; Checksum expected = ${CHECKSUM}"
  exit 1
fi

##################################################################################################
##################################################################################################
echo ""
echo "TEST: File corruption test..."
# Add checksum to testfile
${CRCPATH}/crcsum -a ${TESTFILE} > /dev/null 2>&1
OUTPUT=$(${CRCPATH}/crcsum -p ${TESTFILE})

echo "Simulate that ${TESTFILE} got corrupted..."
# Change content of file but restore timestamp
touch -r ${TESTFILE} time.stamp  > /dev/null 2>&1

# add 1 character
echo "a" >> ${TESTFILE}
touch -r time.stamp  ${TESTFILE}  > /dev/null 2>&1
rm time.stamp  > /dev/null 2>&1

OUTPUT=$(${CRCPATH}/crcsum -c -v ${TESTFILE})

# Extract calculated checksum
#regexp="\[[[:space:]]+(.*)[[:space:]]*\].*"
regexp=".*(FAILED|OK).*"
[[ ${OUTPUT} =~ ${regexp} ]]
REMATCH=(${BASH_REMATCH[1]})

if [[ "$REMATCH" =~ .*"FAILED".* ]]; then
  echo "Success: corrupted file detected"
else
  echo "Failure: corupted file not detected"
  exit 1
fi

rm -f ${TESTFILE} ${CPFILE}

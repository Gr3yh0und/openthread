#!/bin/bash
# by Michael Morscher, 2017

# Variables
BINARYPATH="output/cc2538/bin"
COMPILERPREFIX="ot"
BINARYNAMEFTD="$COMPILERPREFIX-cli-ftd"
BINARYNAMEMTD="$COMPILERPREFIX-cli-mtd"
BINARYNAMENCPFTD="$COMPILERPREFIX-ncp-ftd"
BINARYNAMENCPMTD="$COMPILERPREFIX-ncp-mtd"

# Check platform arguments
if [[  ($1 != "cc2538" && $1 != "cc2650") ]]; then 
	echo "Error: Please supply cc2538 or cc2650 as an argument!"
	exit
fi
PLATFORM=$1

# Make project and check outcome
echo "Entering build phase for target $PLATFORM ..."
make -f examples/Makefile-$PLATFORM
if [ $? -ne 0 ]; 
then
    echo "Build phase failed! Exiting..."
    exit
else
	echo "Successfully built target $PLATFORM!"
	echo ""
fi

# Delete old bin files
echo "Deleting old binary files ..."
if [ -f "$BINARYPATH/$BINARYNAMEFTD.bin" ]; then
echo "$BINARYNAMEFTD.bin ...";
	rm $BINARYPATH/$BINARYNAMEFTD.bin
fi
if [ -f "$BINARYPATH/$BINARYNAMEMTD.bin" ]; then
echo "$BINARYNAMEMTD.bin ...";
	rm $BINARYPATH/$BINARYNAMEMTD.bin
fi
if [ -f "$BINARYPATH/$BINARYNAMENCPFTD.bin" ]; then
echo "$BINARYNAMENCPFTD.bin ...";
	rm $BINARYPATH/$BINARYNAMENCPFTD.bin
fi
if [ -f "$BINARYPATH/$BINARYNAMENCPMTD.bin" ]; then
echo "$BINARYNAMENCPMTD.bin ...";
	rm $BINARYPATH/$BINARYNAMENCPMTD.bin
fi
echo "Done!"
echo ""

# Print target size of current project binaries
echo "ELF file sizes:"
arm-none-eabi-size $BINARYPATH/$COMPILERPREFIX*

# Create new bin files for flashing
arm-none-eabi-objcopy -O binary $BINARYPATH/$BINARYNAMEFTD $BINARYPATH/$BINARYNAMEFTD.bin
arm-none-eabi-objcopy -O binary $BINARYPATH/$BINARYNAMEMTD $BINARYPATH/$BINARYNAMEMTD.bin
arm-none-eabi-objcopy -O binary $BINARYPATH/$BINARYNAMENCPFTD $BINARYPATH/$BINARYNAMENCPFTD.bin
arm-none-eabi-objcopy -O binary $BINARYPATH/$BINARYNAMENCPMTD $BINARYPATH/$BINARYNAMENCPMTD.bin

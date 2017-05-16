#!/bin/bash
# by Michael Morscher, 2017

# Variables
BINARYPATH="output/cc2538/bin"
COMPILERPREFIX="ot"
PLATFORM="cc2538"
BINARYNAMEFTD="$COMPILERPREFIX-cli-ftd"
BINARYNAMEMTD="$COMPILERPREFIX-cli-mtd"
BINARYNAMENCPFTD="$COMPILERPREFIX-ncp-ftd"
BINARYNAMENCPMTD="$COMPILERPREFIX-ncp-mtd"
BINARYNAMECOAPSFTD="$COMPILERPREFIX-coaps-ftd"
BINARYNAMECOAPSMTD="$COMPILERPREFIX-coaps-mtd"

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
if [ -f "$BINARYPATH/$BINARYNAMECOAPSFTD.bin" ]; then
echo "$BINARYNAMECOAPSFTD.bin ...";
	rm $BINARYPATH/$BINARYNAMECOAPSFTD.bin
fi
if [ -f "$BINARYPATH/$BINARYNAMECOAPSMTD.bin" ]; then
echo "$BINARYNAMECOAPSMTD.bin ...";
	rm $BINARYPATH/$BINARYNAMECOAPSMTD.bin
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
arm-none-eabi-objcopy -O binary $BINARYPATH/$BINARYNAMECOAPSFTD $BINARYPATH/$BINARYNAMECOAPSFTD.bin
arm-none-eabi-objcopy -O binary $BINARYPATH/$BINARYNAMECOAPSMTD $BINARYPATH/$BINARYNAMECOAPSMTD.bin
echo ""

# Get size of new build
OUTPUT_NEW="$(arm-none-eabi-size $BINARYPATH/$BINARYNAMECOAPSFTD)"
RE_NEW="([0-9]{3,6})\s*([0-9]{3,6})\s*([0-9]{3,6})\s*([0-9]{3,6})"
[[ $OUTPUT_NEW =~ $RE_NEW ]]
	TEXT_NEW=${BASH_REMATCH[1]}
	BSS_NEW=${BASH_REMATCH[2]}
	DATA_NEW=${BASH_REMATCH[3]}
	DEC_NEW=${BASH_REMATCH[4]}
	
# Get size of old build
OUTPUT_OLD="$(cat build.old)"
RE_OLD="([0-9]{3,6}),([0-9]{3,6}),([0-9]{3,6}),([0-9]{3,6})"
[[ $OUTPUT_OLD =~ $RE_OLD ]]
	TEXT_OLD=${BASH_REMATCH[1]}
	BSS_OLD=${BASH_REMATCH[2]}
	DATA_OLD=${BASH_REMATCH[3]}
	DEC_OLD=${BASH_REMATCH[4]}

# Calculate differences
TEXT_DIF=`expr $TEXT_NEW - $TEXT_OLD`
BSS_DIF=`expr $BSS_NEW - $BSS_OLD`
DATA_DIF=`expr $DATA_NEW - $DATA_OLD`
DEC_DIF=`expr $DEC_NEW - $DEC_OLD`

# Print results
echo "Old: TEXT($TEXT_OLD), BSS($BSS_OLD), DATA($DATA_OLD), DEC($DEC_OLD)"
echo "New: TEXT($TEXT_NEW), BSS($BSS_NEW), DATA($DATA_NEW), DEC($DEC_NEW)"
echo "Dif: TEXT($TEXT_DIF), BSS($BSS_DIF), DATA($DATA_DIF), DEC($DEC_DIF)"
echo "$TEXT_NEW,$BSS_NEW,$DATA_NEW,$DEC_NEW" > build.old
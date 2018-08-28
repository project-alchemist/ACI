#!/bin/bash

source ./config.sh

export ACI_EXE=$ACI_PATH/target/aci

echo $ACI_EXE

CURR_DIR=$PWD

echo " "
cd $ACI_PATH
echo "Building Alchemist Client Interface (ACI) for $SYSTEM"
LINE="=============================================="
for i in `seq 1 ${#SYSTEM}`;
do
	LINE="$LINE="
done
echo $LINE
echo " "
echo "Creating ACI executable:"
echo " "
cd ./build/$SYSTEM/
make
cd ../..
echo " "
echo $LINE
echo " "
echo "Building process for ACI has completed"
echo " "
echo "If no issues occurred during build:"
echo "  ACI executable located at:    $ACI_EXE"
echo " "
echo "  Run './start.sh' to start ACI"
echo " "
cd $CURR_DIR

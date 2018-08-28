#!/bin/bash

source ./config.sh

export ACI_EXE=$ACI_PATH/target/aci

echo $ACI_EXE

CURR_DIR=$PWD

if [ "$SYSTEM" == "MacOS" ]
then
	mpiexec -n 3 $ACI_EXE
	
elif [ "$SYSTEM" == "Cori" ]
then
	srun -n 5 $ACI_EXE
	
elif [ "$SYSTEM" == "<your system here>" ]
then
	srun -n 5 $ACI_EXE
fi
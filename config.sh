#!/bin/bash

# Configuration file for building Alchemist Client Interface (ACI)

export SYSTEM="MacOS"                # Options: MacOS, Cori, <add your own>

if [ "$SYSTEM" == "MacOS" ]
then
	export ACI_PATH=$HOME/Projects/ACI
	
	export ELEMENTAL_PATH=$HOME/Software/Elemental

elif [ "$SYSTEM" == "Cori" ]
then
	export ACI_PATH=$SCRATCH/Projects/ACI
	
	export ELEMENTAL_PATH=$SCRATCH/Software/Elemental
	export SPDLOG_PATH=$SCRATCH/Software/SPDLog	
	
else
	export ACI_PATH=$SCRATCH/Projects/ACI
	
	export ELEMENTAL_PATH=$SCRATCH/Software/Elemental
	export SPDLOG_PATH=$SCRATCH/Software/SPDLog	
fi

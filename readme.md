This is a C++ testing module for Alchemist (github.com/project-alchemist/Alchemist), provisionally called Alchemist-Client Interface (ACI).

# Dependencies

The ACI has the same prerequisites and dependencies as Alchemist.

# Installation instructions

It's assumed that an MPI implementation, Elemental, and SPDLog have been installed.

## MacOS

### Clone the ACI repo
```
export ALCHEMIST_PATH=(/desired/path/to/ACI/directory)
cd $ACI_PATH
git clone https://github.com/project-alchemist/ACI.git
```

### Update configuration file

In the config.sh file:
* change SYSTEM to the system you are working on;
* set ACI_PATH, ELEMENTAL_PATH, and SPDLOG_PATH to the appropriate paths.

It may also be a good idea to add the above paths to the bash profile.

### Building ACI

Assuming the configuration file is updated, ACI can be built using
```
./build.sh
```

## Cori

TO BE ADDED

# Running ACI

## Starting ACI

ACI is started using 
```
./start.sh
```

## Connecting to Alchemist

Alchemist needs to already be running. Put the IP address and port in the 'connection.info' file, and put the 'connection.info' file in the same folder as the executable. In the ACI menu, select 'Connect to Alchemist'.

## Testing Alchemist

Follow the prompts on screen to test if Alchemist is running properly.

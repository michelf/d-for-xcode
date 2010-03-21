#!/bin/sh

DX_BIN="/usr/local/bin"
DX_MAN="/usr/local/man"

# Check if version exists
if [ "$1" != "" ] && [ -e "$DX_BIN/dmd$1" ]
then
	DMD_COMMAND="dmd$1"
elif [ "$1" != "" ]
then
	# Error
	echo "$0: DMD version $1 is not installed, cannot set as default"
	exit -1
else 
	# Help page
	echo "usage: $0 version-number"
	echo "Set version of the Digital Mars D compiler invoked by the dmd command."
	exit -1
fi

# Check for root
if [ "$(id -u)" != "0" ]
then
	echo "$0: must be run as root"
	exit -1
fi

# Change link to compiler binary
rm -f "$DX_BIN/dmd"
ln -s "$DX_BIN/$DMD_COMMAND" "$DX_BIN/dmd"

# Change link to man page
rm -f "$DX_MAN/man1/dmd.1"
ln -s "$DX_MAN/man1/$DMD_COMMAND.1" "$DX_MAN/man1/dmd.1"

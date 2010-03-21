#!/bin/sh

DMD_COMMAND="dmd1"
DX_BIN="/usr/local/bin"

# Setup link to default compiler (if not already set)
if [ ! -e "$DX_BIN/dmd" ]
then
    ln -s "$DX_BIN/$DMD_COMMAND" "$DX_BIN/dmd"
fi

# Man page
if [ ! -e "$DX_MAN/man1/dmd.1" ]
then ln -s "$DX_MAN/man1/$DMD_COMMAND.1" "$DX_MAN/man1/dmd.1"
fi

exit 0
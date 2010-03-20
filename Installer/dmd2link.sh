#!/bin/sh

DMD_COMMAND="dmd2"
DMD_BIN="/Library/Compilers/dmd2/osx/bin"
DMD_MAN="/Library/Compilers/dmd2/osx/man"
DX_BIN="/usr/local/bin"

# Setup permissions
chmod +x "$DMD_BIN/"{dmd,dumpobj,obj2asm,rdmd}

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
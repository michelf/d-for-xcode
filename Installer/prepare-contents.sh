#!/bin/sh

COMPILERS_DIR="/Library/Compilers"
DX_CONTENTS_DIR="$SRCROOT/Installer/Prepared Contents"
mkdir -p "$DX_CONTENTS_DIR"
rm -rf "$DX_CONTENTS_DIR/"* # clear previous content

# Copy D for Xcode plugin and Xcode file and project templates
cp -r "$TARGET_BUILD_DIR/D for Xcode.xcplugin" "$DX_CONTENTS_DIR/"
cp -r "$SRCROOT/Installer/Templates" "$DX_CONTENTS_DIR/"

# Copy redirect script and create symlinks for dmd2
#   Note: this could be better: links to dumpobj, obj2asm and man pages 
#   are overwriten by DMD 1.x install when done after DMD 2.x.
DX_INSTALL_DIR="$DX_CONTENTS_DIR/dmd2-install"
DMD_DIR="$COMPILERS_DIR/dmd2"
mkdir -p "$DX_INSTALL_DIR/bin"
mkdir -p "$DX_INSTALL_DIR/lib"
mkdir -p "$DX_INSTALL_DIR/man/man1"
cp "$TARGET_BUILD_DIR/dmd2-redirect"  "$DX_INSTALL_DIR/bin/dmd2"
ln -s "$DMD_DIR/osx/bin/dumpobj"  "$DX_INSTALL_DIR/bin/dumpobj"
ln -s "$DMD_DIR/osx/bin/obj2asm"  "$DX_INSTALL_DIR/bin/obj2asm"
ln -s "$DMD_DIR/osx/bin/rdmd"  "$DX_INSTALL_DIR/bin/rdmd"
ln -s "$DMD_DIR/osx/lib/libphobos2.a"  "$DX_INSTALL_DIR/lib/libphobos2.a"
ln -s "$DMD_DIR/osx/lib/libdruntime.a"  "$DX_INSTALL_DIR/lib/libdruntime.a"
ln -s "$DMD_DIR/man/man1/dmd.1"  "$DX_INSTALL_DIR/man/man1/dmd2.1"
ln -s "$DMD_DIR/man/man1/dmd.conf.5"  "$DX_INSTALL_DIR/man/man1/dmd.conf.5"
ln -s "$DMD_DIR/man/man1/dumpobj.1"  "$DX_INSTALL_DIR/man/man1/dumpobj.1"
ln -s "$DMD_DIR/man/man1/obj2asm.1"  "$DX_INSTALL_DIR/man/man1/obj2asm.1"
ln -s "$DMD_DIR/man/man1/rdmd.1"  "$DX_INSTALL_DIR/man/man1/rdmd.1"
cp "$SRCROOT/Installer/setdmd.sh"  "$DX_INSTALL_DIR/bin/setdmd"

# Copy redirect script and create symlinks for dmd1
DX_INSTALL_DIR="$DX_CONTENTS_DIR/dmd1-install"
DMD_DIR="$COMPILERS_DIR/dmd"
mkdir -p "$DX_INSTALL_DIR/bin"
mkdir -p "$DX_INSTALL_DIR/lib"
mkdir -p "$DX_INSTALL_DIR/man/man1"
cp "$TARGET_BUILD_DIR/dmd1-redirect"  "$DX_INSTALL_DIR/bin/dmd1"
ln -s "$DMD_DIR/osx/bin/dumpobj"  "$DX_INSTALL_DIR/bin/dumpobj"
ln -s "$DMD_DIR/osx/bin/obj2asm"  "$DX_INSTALL_DIR/bin/obj2asm"
ln -s "$DMD_DIR/osx/lib/libphobos.a"  "$DX_INSTALL_DIR/lib/libphobos.a"
ln -s "$DMD_DIR/man/man1/dmd.1"  "$DX_INSTALL_DIR/man/man1/dmd1.1"
ln -s "$DMD_DIR/man/man1/dmd.conf.5"  "$DX_INSTALL_DIR/man/man1/dmd.conf.5"
ln -s "$DMD_DIR/man/man1/dumpobj.1"  "$DX_INSTALL_DIR/man/man1/dumpobj.1"
ln -s "$DMD_DIR/man/man1/obj2asm.1"  "$DX_INSTALL_DIR/man/man1/obj2asm.1"
cp "$SRCROOT/Installer/setdmd.sh"  "$DX_INSTALL_DIR/bin/setdmd"

#$DEVELOPER_BIN_DIR/packagemaker \
#	--doc "$SRCROOT/Installer/D for Xcode.pmdoc" \
#	--out "$TARGET_BUILD_DIR/$TARGET_NAME"
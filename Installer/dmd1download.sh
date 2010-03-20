#!/bin/sh

DMD_DIR_NAME="dmd"
DMD_LATEST_URL="http://michelf.com/docs/dmd-info/$DMD_DIR_NAME-latest"
DMD_ARCHIVE_NAME=$(curl --fail "$DMD_LATEST_URL" -s)
if [ $? != 0 ] || [ "$DMD_ARCHIVE_NAME" == "" ]
then echo "Error retrieving current DMD version." ; exit -1
fi

# Source URL / Destination Dir
DMD_URL="http://ftp.digitalmars.com/$DMD_ARCHIVE_NAME"
DX_INSTALL_DIR="/Library/Compilers"

# Directories
DX_TMP="/tmp/D for Xcode Install/$DMD_DIR_NAME"
DX_DOWNLOAD_DIR="$DX_TMP/Download"
DX_EXTRACT_DIR="$DX_TMP/Content"
DX_ERROR_PATH="$DX_TMP/Download Error Log.txt"

# Subject Paths
DX_DOWNLOAD_PATH="$DX_DOWNLOAD_DIR/$DMD_ARCHIVE_NAME"
DX_EXTRACT_PATH="$DX_EXTRACT_DIR/$DMD_DIR_NAME"
DX_INSTALL_PATH="$DX_INSTALL_DIR/$DMD_DIR_NAME"
DX_OLD_SAVE_PATH="$DX_TMP/Old $DMD_DIR_NAME"

# Download
echo "Downloading $DMD_URL"
mkdir -p "$DX_DOWNLOAD_DIR"
curl --fail "$DMD_URL" -o "$DX_DOWNLOAD_PATH" -s --stderr "$DX_ERROR_PATH" --continue-at -
if [[ $? != 0 && $? != 18 ]]
then exit $?
fi

# Extraction
echo "Extracting..."
mkdir -p "$DX_EXTRACT_DIR"
rm -rf "$DX_EXTRACT_DIR"/*
unzip -qq "$DX_DOWNLOAD_PATH" -d "$DX_EXTRACT_DIR"
if [ $? != 0 ]
then exit $?
fi

# Install
echo "Installing at $DX_INSTALL_PATH/"
if [ -e "$DX_INSTALL_PATH" ]
then rm -rf "$DX_OLD_SAVE_PATH"; mv -f "$DX_INSTALL_PATH" "$DX_OLD_SAVE_PATH"
else mkdir -p "$DX_INSTALL_DIR"
fi
mv -f "$DX_EXTRACT_PATH" "$DX_INSTALL_DIR"
if [ $? != 0 ]
then exit $?
fi

# Cleanup
echo "Cleanup..."
rm -rf "$DX_TMP"
exit 0
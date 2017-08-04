#!/bin/bash

set -e

SELF_PATH="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"


# setup
if [ ! -z "$QT_BIN" ]; then export PATH="$QT_BIN:$PATH";
elif [ -d "/opt/qt/bin" ]; then export PATH="/opt/qt/bin:$PATH";
elif [ -d "/usr/local/opt/qt/bin" ]; then export PATH="/usr/local/opt/qt/bin:$PATH";
fi
ROOT="$SELF_PATH/../.."
VERSION=`cat "$ROOT/version"`


# build
lrelease "$ROOT/multidir.pro"
qmake "$ROOT/"
make -j`nproc`


# test
mkdir -p tests
cd tests
qmake "$ROOT/tests/"
make -j`nproc`
cd ..
./tests/tests


# pack
strip multidir
make INSTALL_ROOT=appdir install
linuxdeployqt --appimage-extract
unset QTDIR
unset QT_PLUGIN_PATH
unset LD_LIBRARY_PATH
./squashfs-root/AppRun ./appdir/usr/share/applications/*.desktop -bundle-non-qt-libs
./squashfs-root/AppRun ./appdir/usr/share/applications/*.desktop -appimage
mv MultiDir-x86_64.AppImage multidir-$VERSION-x64.appimage 


echo "Finished"

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
echo `g++ --version`
echo `qmake --version`


# build
lrelease "$ROOT/src/multidir.pro"
qmake "$ROOT/"
make -j`nproc`


# test
./tests/tests


# pack
strip src/multidir
make INSTALL_ROOT=appdir install
cd src
linuxdeployqt --appimage-extract
unset QTDIR
unset QT_PLUGIN_PATH
unset LD_LIBRARY_PATH
./squashfs-root/AppRun ./appdir/usr/share/applications/*.desktop -bundle-non-qt-libs
./squashfs-root/AppRun ./appdir/usr/share/applications/*.desktop -appimage
mv MultiDir-x86_64.AppImage ../multidir-$VERSION-x64.appimage 
cd ..

ls -l `pwd`/multidir-$VERSION-x64.appimage

echo "Finished"

#!/bin/bash

set -e

SELF_PATH="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SELF_PATH"/make_ubuntu.sh

strip multidir
make INSTALL_ROOT=appdir install
MAKER="linuxdeployqt"
if [ -z `which $MAKER` ]; then
  wget -c "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage" -O $MAKER
  export PATH="$PWD:$PATH"
  chmod a+x "$MAKER"
fi
$MAKER --appimage-extract
MAKER="./squashfs-root/AppRun"
unset QTDIR
unset QT_PLUGIN_PATH
unset LD_LIBRARY_PATH
$MAKER ./appdir/usr/share/applications/*.desktop -bundle-non-qt-libs
$MAKER ./appdir/usr/share/applications/*.desktop -appimage

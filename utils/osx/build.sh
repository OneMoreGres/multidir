#!/bin/bash

set -e

SELF_PATH="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"


# setup
if [ ! -z "$QT_BIN" ]; then export PATH="$QT_BIN:$PATH";
elif [ -d "/usr/local/opt/qt/bin" ]; then export PATH="/usr/local/opt/qt/bin:$PATH";
elif [ -d "/opt/qt/bin" ]; then export PATH="/opt/qt/bin:$PATH";
fi
ROOT="$SELF_PATH/../.."
VERSION=`cat "$ROOT/version"`


# build
lrelease "$ROOT/multidir.pro"
qmake "$ROOT/"
make -j2


# test
mkdir -p tests
cd tests
qmake "$ROOT/tests/"
make -j2
cd ..
./tests/tests.app/Contents/MacOS/tests


# pack
macdeployqt multidir.app -dmg
mv multidir.dmg multidir-$VERSION.dmg
ls -l `pwd`/multidir-$VERSION.dmg


echo "Finished"

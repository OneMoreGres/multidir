#!/bin/bash

set -e

SELF_PATH="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SELF_PATH"/ubuntu_env.sh

lrelease "$SELF_PATH/../multidir.pro"
qmake "$SELF_PATH/../"
make -j4

mkdir tests
cd tests
qmake "$SELF_PATH/../tests/"
make -j4
cd ..
./tests/tests

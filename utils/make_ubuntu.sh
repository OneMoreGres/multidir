#!/bin/bash

set -e

SELF_PATH="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SELF_PATH"/ubuntu_env.sh

qmake "$SELF_PATH/../"
make -j4
lrelease "$SELF_PATH/../multidir.pro"

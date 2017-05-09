#!/bin/bash

find -type f -iregex '.*\(png\|jpg\)' \
    -exec cp '{}' '{}.bak' \; -exec leanify '{}' \;

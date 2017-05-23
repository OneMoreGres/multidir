#!/bin/bash

docker run -it --rm -v $PWD/..:/multidir --privileged --device /dev/fuse --network host multidir/precise /bin/bash

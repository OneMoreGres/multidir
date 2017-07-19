#!/bin/bash

docker run -it --rm -v $PWD/../..:/multidir --network host gres/multidir:precise /bin/bash

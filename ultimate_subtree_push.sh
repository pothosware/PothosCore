#!/bin/bash

BRANCH=$1

COMPONENTS="
pothos-opencl
pothos-widgets
pothos-gui
pothos-library
pothos-python
pothos-util
pothos-java
pothos-blocks
pothos-serialization
pothos-sdr
pothos-audio
"

for PREFIX in $COMPONENTS; do
    git subtree push --prefix=${PREFIX} git@github.com:pothosware/${PREFIX}.git ${BRANCH}
done

#!/bin/bash

BRANCH=$1

COMPONENTS="
pothos-widgets
pothos-plotters
pothos-gui
pothos-library
pothos-python
pothos-java
pothos-util
pothos-opencl
pothos-blocks
pothos-comms
pothos-serialization
pothos-sdr
pothos-audio
"

for PREFIX in $COMPONENTS; do
    git subtree push --prefix=${PREFIX} git@github.com:pothosware/${PREFIX}.git ${BRANCH}
done

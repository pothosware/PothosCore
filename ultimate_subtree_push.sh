#!/bin/bash

BRANCH=$1

COMPONENTS="
widgets
plotters
gui
library
python
blocks
comms
sdr
audio
"

for PREFIX in $COMPONENTS; do
    git subtree push --prefix=${PREFIX} git@github.com:pothosware/pothos-${PREFIX}.git ${BRANCH}
done

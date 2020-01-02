#!/bin/bash

set -e

mkdir build
cd build

export PATH=/usr/bin:$PATH

export PKG_CONFIG_PATH="/usr/local/opt/libarchive/lib/pkgconfig" # for macos
cmake --version
cmake ..
make -j`nproc`

export RELEASE_DIR="tasarch-${AGENT_OS}-${BUILD_BUILDNUMBER}"
mkdir $RELEASE_DIR
mkdir $RELEASE_DIR/cores

echo "Making Release Directory in $RELEASE_DIR"
cp tasarch $RELEASE_DIR/
cp ../cores/* $RELEASE_DIR/cores/
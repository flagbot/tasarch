#!/bin/bash

set -e

mkdir build
cd build

export PATH=/usr/bin:$PATH

export PKG_CONFIG_PATH="/usr/local/opt/libarchive/lib/pkgconfig" # for macos
cmake --version
cmake ..
make

export RELEASE_DIR="tasarch-${AGENT_OS}-${BUILD_ID}"
mkdir $RELEASE_DIR
mkdir $RELEASE_DIR/cores

cp tasarch $RELEASE_DIR/
cp ../cores/* $RELEASE_DIR/cores/
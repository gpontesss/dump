#!/bin/bash

set -o errexit

BIN_DIR="bin"
file="$1"
bin="$BIN_DIR/${file%.*}"
make "$bin" && $bin

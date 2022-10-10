#!/bin/bash

export PREFIX="$HOME/opt/cross" # path to target agnostic cross compiler
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"

make clean
make all
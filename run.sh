#!/bin/sh

echo cleaning...
make clean

echo compiling...
make

echo executing with input file $1...
./bin/maze 1 1 1 1 1 < $1


#!/bin/sh

cp *.c *.h ../upstream/busybox-0.60.5-dban/ 
cd ../upstream/busybox-0.60.5-dban/
#make clean
make install PREFIX=../../in.root || exit
cd ../..
./build.sh

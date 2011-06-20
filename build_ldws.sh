#!/bin/sh
#This build script is for SQ project ADR8612

# make LDWS
cd ./lib
make clean
make ldws
cd ..

# copy LDWS
cp ./lib/src/ldws/libldws.a ./src

# make ADR8612
cp ./lib/libsq8612.a ./src
#cd ./src
#make clean
#make
#cd ..
#cp ./src/adr.8612 ./bin

# make ADR for LDWS
cd ./src/test
make clean
make
cd ../..
cp ./src/test/adr.ldws ./bin

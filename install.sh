#!/bin/bash

#Install Selene in /usr/local/bin
#and its plugin in /usr/local/lib/Selene

mkdir -p /usr/local/lib/Selene || { echo "can't create target directory, please check if your user has enough rights" ; exit 0; }

cp src/SeleneLibrary/libSelene.h /usr/local/include
cp Sel*.so /usr/local/lib/Selene
cp libSelene.so /usr/local/lib
cp Selene /usr/local/bin

echo "please run ldconfig as root"

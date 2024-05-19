#!/bin/bash

#Install Selene in /usr/local/bin
#and its plugin in /usr/local/lib/Selene

mkdir -p /usr/local/lib/Selene || { echo "can't create target directory, please check if your user has enough rights" ; exit 0; }
mkdir -p /usr/local/include/Selene || { echo "can't create target directory, please check if your user has enough rights" ; exit 0; }

cp -r src/include/Selene/* /usr/local/include/Selene

cp lib/*.so.* /usr/local/lib
ln -s /usr/local/lib/libSelene.so.2 /usr/local/lib/libSelene.so
cp lib/Selene/*.so /usr/local/lib/Selene
cp Selene /usr/local/bin

echo "please run ldconfig as root"

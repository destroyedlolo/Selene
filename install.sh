#!/bin/bash

#Install Selene in /usr/local/bin
#and its plugin in /usr/local/lib/Selene

cp Sel*.so /usr/local/lib/Selene
cp libSelene.so /usr/local/lib
cp Selene /usr/local/bin

echo "please run ldconfig as root"

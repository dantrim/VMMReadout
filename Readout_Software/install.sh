#!/bin bash

cd include
rootcint -f aDict.cxx -c a.h LinkDef.h

mkdir ../build/objects
g++ -o ../build/objects/libMylib.so aDict.cxx `root-config --cflags --libs` -shared -fPIC

cd ../build
ln -s objects/libMylib.so .
qmake -o Makefile executables.pro


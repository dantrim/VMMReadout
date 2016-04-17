#!/bin bash

cd include
git clone -b master https://github.com/dantrim/boost_includes.git boost

rootcint -f aDict.cxx -c a.h LinkDef.h

mkdir ../build/objects
g++ -o ../build/objects/libMylib.so aDict.cxx `root-config --cflags --libs` -shared -fPIC

cd ../build
ln -s objects/libMylib.so .
qmake -o Makefile vmmall.pro

#!/bin/sh

echo "Compiling AIT QKD Module-1 example"

# we construct here the compiler line
set -x
$CXX \
    -std=c++11 \
    -I/usr/include/qt4  \
    -I/usr/include/qt4/Qt  \
    -o module-1 \
    main.cpp \
    -L/usr/lib -L/usr/lib/x86_64-linux-gnu -L/usr/lib/qt -L/usr/lib/qt4 \
    -lqkd -lQtDBus -lQtNetwork -lQtCore -lz -lboost_filesystem -lboost_program_options -lboost_system -lssl -lcrypto -ldbus-1 -lzmq -lgmp
 

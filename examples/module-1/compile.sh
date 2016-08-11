#!/bin/sh

echo "Compiling AIT QKD Module-1 example"
CXX=$(which g++)

ARCH=$(uname -i)
ARCH_INCLUDE="/usr/include/i386-linux-gnu"
if [ "${ARCH}" = "x86_64" ]; then
	ARCH_INCLUDE="/usr/include/x86_64-linux-gnu"
fi

# we construct here the compiler line
set -x
$CXX \
    -std=c++11 \
	-fPIC \
    -isystem ${ARCH_INCLUDE}/qt5  \
    -o module-1 \
    main.cpp \
    -L/usr/lib -L/usr/lib/x86_64-linux-gnu \
    -lqkd -lQt5DBus -lQt5Network -lQt5Core -lz -lboost_filesystem -lboost_program_options -lboost_system -lssl -lcrypto -ldbus-1 -lzmq -lgmp
 
 # -isystem /usr/include/x86_64-linux-gnu/qt5 -isystem /usr/include/x86_64-linux-gnu/qt5/QtCore 

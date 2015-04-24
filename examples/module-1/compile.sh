#!/bin/sh

echo "Compiling AIT QKD Module-1 example"

# we construct here the compiler line
set -x
g++ \
    -std=gnu++0x \
    -D_GLIBCXX_USE_NANOSLEEP -D_GLIBCXX_USE_SCHED_YIELD \
    -I/usr/include/qt4  \
    -L/usr/lib -L/usr/lib/x86_64-linux-gnu -L/usr/lib/qt -L/usr/lib/qt4 \
    -lqkd -lQtDBus -lQtNetwork -lQtCore -lz -lboost_filesystem-mt -lboost_program_options-mt -lboost_system-mt -lssl -lcrypto -ldbus-1 -lzmq -lgmp \
    -o module-1 \
    main.cpp

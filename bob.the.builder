#!/bin/bash

# ------------------------------------------------------------
# Bob the Builder script
#
# Do
#
# * prepare system
# * build
# * test
# * package
#
# Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
# 
# Copyright (C) 2012-2016 AIT Austrian Institute of Technology
# AIT Austrian Institute of Technology GmbH
# Donau-City-Strasse 1 | 1220 Vienna | Austria
# http://www.ait.ac.at
#
# This file is part of the AIT QKD Software Suite.
#
# The AIT QKD Software Suite is free software: you can redistribute 
# it and/or modify it under the terms of the GNU General Public License 
# as published by the Free Software Foundation, either version 3 of 
# the License, or (at your option) any later version.
# 
# The AIT QKD Software Suite is distributed in the hope that it will 
# be useful, but WITHOUT ANY WARRANTY; without even the implied warranty 
# of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with the AIT QKD Software Suite. 
# If not, see <http://www.gnu.org/licenses/>.
# ------------------------------------------------------------


function setup_debian() {
    echo 'Setting up Debian construction site...'
    sudo apt-get update
    sudo apt-get --quiet --yes install \
        build-essential \
        g++ \
        gcc \
        libboost-all-dev \
        libssl-dev \
        uuid-dev \
        cmake \
        libssl-dev \
        uuid-dev \
        libgmp3-dev \
        libzmq3-dev \
        libdbus-1-dev \
        libqt4-dev \
        libqwt-dev \
        doxygen \
        texlive-latex-base \
        texlive-latex-extra \
        texlive-font-utils \
        dbus-x11 \
        libcap2-bin \
        python3
}


# ------------------------------------------------------------


OS_ID=$(grep ^ID_LIKE= /etc/*-release* | cut -d = -f 2)
if [ -z $OS_ID ]; then
    OS_ID=$(grep ^ID= /etc/*-release* | cut -d = -f 2)
fi
case ${OS_ID} in
debian)
    echo 'Detected Debian like system...'
    setup_debian
    ;;

*)
    echo "Don't know how to deal with ${OS_ID} systems... =("
    exit 1
    ;;
esac


# ------------------------------------------------------------

echo 'Setting up build folder...'
mkdir build &> /dev/null
(
    cd build || exit 1
    echo 'wipe build folder...'
    rm -rf * 
    echo 'cmake...'
    cmake ..
    echo 'make...'
    make || exit 1
    echo 'ctest -V...'
    ctest -V || exit 1
    echo 'make package...'
    make package || exit 1
)


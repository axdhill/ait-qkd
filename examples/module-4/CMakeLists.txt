# ------------------------------------------------------------
# CMakeLists.txt for a AIT QKD Module
#
# Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
# 
# Copyright (C) 2013-2015 AIT Austrian Institute of Technology
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

# project data
project(qkd-module-4 C CXX)
cmake_minimum_required(VERSION 2.6)

# load necessary basic cmake modules
include(CheckIncludeFile)
include(CheckIncludeFileCXX)
include(CheckIncludeFiles)
include(CheckLibraryExists)
include(FindPkgConfig)


# ------------------------------------------------------------
# tweak the compiler to get C++11

# set compile flags
if (CMAKE_COMPILER_IS_GNUCC)

    # this is needed to have
    #   std::_this_thread::sleep(...)
    # at hand - at least for gcc 4.6.3 and glibc 2.15
    add_definitions(-D_GLIBCXX_USE_NANOSLEEP)

    # this is needed to have
    #   std::_this_thread::yield()
    # at hand - at least for gcc 4.6.3 and glibc 2.15
    add_definitions(-D_GLIBCXX_USE_SCHED_YIELD)
    
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    
endif (CMAKE_COMPILER_IS_GNUCC)

# set compile flags: clang
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")


# ------------------------------------------------------------
# ensure some packages/libs are installed on the system

# get the GMP lib
check_include_file(gmp.h HAVE_GMP_H)
check_library_exists(gmp __gmp_version "/lib;/usr/lib;/usr/local/lib" HAVE_GMP_LIB)
if (HAVE_GMP_LIB)
    set(CMAKE_REQUIRED_LIBRARIES "gmp;${CMAKE_REQUIRED_LIBRARIES}")
else (HAVE_GMP_LIB)
    message(FATAL_ERROR "Could not find gmp implementation (see http://gmplib.org/).")
endif (HAVE_GMP_LIB)


# get the 0MQ (zeromq: http://www.zeromq.org)
check_include_file(zmq.h HAVE_ZMQ_H)
check_library_exists(zmq zmq_init "/lib;/usr/lib;/usr/local/lib" HAVE_ZMQ_LIB)
if (HAVE_ZMQ_LIB)
    set(CMAKE_REQUIRED_LIBRARIES "zmq;${CMAKE_REQUIRED_LIBRARIES}")
else (HAVE_ZMQ_LIB)
    message(FATAL_ERROR "Could not find zeromq (0MQ) implementation (see http://www.zeromq.org/).")
endif (HAVE_ZMQ_LIB)


# DBus
pkg_check_modules(DBUS_1 REQUIRED dbus-1)
if (DBUS_1_FOUND)
    set(CMAKE_REQUIRED_LIBRARIES "${DBUS_1_LIBRARIES};${CMAKE_REQUIRED_LIBRARIES}")
    include_directories(${DBUS_1_STATIC_INCLUDE_DIRS})
else (DBUS_1_FOUND)
    message(FATAL_ERROR "Couldn't find dbus-1 installation.")
endif (DBUS_1_FOUND)


# get OpenSSL 
include(FindOpenSSL)
if (OPENSSL_FOUND)
    set(CMAKE_REQUIRED_LIBRARIES "${OPENSSL_LIBRARIES};${CMAKE_REQUIRED_LIBRARIES}")
else (OPENSSL_FOUND)
    message(FATAL "OpenSSL not found on the system.")
endif (OPENSSL_FOUND)


# get BOOST!
find_package(Boost 1.46.0 COMPONENTS filesystem program_options system)
if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    set(CMAKE_REQUIRED_LIBRARIES "${Boost_LIBRARIES};${CMAKE_REQUIRED_LIBRARIES}")
else (Boost_FOUND)
    message(FATAL_ERROR "Boost >= 1.46.0 library not found.")
endif (Boost_FOUND)


# get ZLIB
include(FindZLIB)
if (ZLIB_FOUND)
    set(CMAKE_REQUIRED_LIBRARIES "${ZLIB_LIBRARIES};${CMAKE_REQUIRED_LIBRARIES}")
    include_directories(${ZLIB_INCLUDE_DIRS})
else (ZLIB_FOUND)
    message(FATAL_ERROR "ZLIB library not found.")
endif (ZLIB_FOUND)


# Qt4
find_package(Qt4 COMPONENTS QtCore QtGui QtNetwork QtXml QtDbus)
if (NOT QT4_FOUND)
    message(FATAL_ERROR "Qt4 not found! Is QTDIR environment variable set?")
else (NOT QT4_FOUND)
    include(${QT_USE_FILE})
    set(CMAKE_REQUIRED_INCLUDES "${QT_INCLUDE_DIR};${CMAKE_REQUIRED_INCLUDES}")
    set(CMAKE_REQUIRED_LIBRARIES "${QT_LIBRARIES};${CMAKE_REQUIRED_LIBRARIES}")
endif (NOT QT4_FOUND)


# ------------------------------------------------------------
# The AIT QKD Package
#
# This is also a good starting candidate for a FindAITQKD.cmake

# get the AIT QKD Include folder
message(STATUS "Looking for AIT QKD includes")
find_path(AIT_QKD_INCLUDE_DIR "qkd/qkd.h" DOC "Path to the AIT QKD includes")
if (IS_DIRECTORY  "${AIT_QKD_INCLUDE_DIR}")
    message(STATUS "Looking for AIT QKD includes - ${AIT_QKD_INCLUDE_DIR}")
    set(CMAKE_REQUIRED_INCLUDES "${AIT_QKD_INCLUDE_DIR};${CMAKE_REQUIRED_INCLUDES}")
else (IS_DIRECTORY  "${AIT_QKD_INCLUDE_DIR}")
    message(FATAL_ERROR "AIT QKD includes not found - could not find AIT QKD implementation (see http://sqt.ait.ac.at/software/).")
endif (IS_DIRECTORY  "${AIT_QKD_INCLUDE_DIR}")

# get the AIT QKD Library
message(STATUS "Looking for AIT QKD library")
find_library(AIT_QKD_LIB qkd DOC "Path to the AIT QKD library" PATHS "/usr/local/lib;/usr/lib")
if (EXISTS  "${AIT_QKD_LIB}")
    message(STATUS "Looking for AIT QKD library - ${AIT_QKD_LIB}")
    set(CMAKE_REQUIRED_LIBRARIES "${AIT_QKD_LIB};${CMAKE_REQUIRED_LIBRARIES}")
else (EXISTS  "${AIT_QKD_LIB}")
    message(FATAL_ERROR "AIT QKD library not found - could not find AIT QKD implementation (see http://sqt.ait.ac.at/software/).")
endif (EXISTS  "${AIT_QKD_LIB}")

mark_as_advanced(AIT_QKD_INCLUDE_DIR AIT_QKD_LIB)


# ------------------------------------------------------------
# go through the subs

add_subdirectory(bin)


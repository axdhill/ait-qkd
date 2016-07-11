# ------------------------------------------------------------
# CMakeLists.txt the AIT QKD RX
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

# project data
project(qkd C CXX)
cmake_minimum_required(VERSION 2.6)

# load necessary basic cmake modules
include(CheckIncludeFile)
include(CheckIncludeFileCXX)
include(CheckIncludeFiles)
include(CheckLibraryExists)
include(FindPkgConfig)
include(FindGit)

# enable tests
ENABLE_TESTING()


# ------------------------------------------------------------
# set global compiler flags

set(VERSION "9.9999.8")
add_definitions(-DVERSION=\"${VERSION}\")

# grab current git branch details
if (GIT_EXECUTABLE) 

    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_BRANCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    execute_process(
      COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      OUTPUT_VARIABLE GIT_COMMIT_HASH
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    
    execute_process(
      COMMAND ${GIT_EXECUTABLE} diff --quiet
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      RESULT_VARIABLE GIT_CHANGE
    )
    
    add_definitions("-DGIT_BRANCH=\"${GIT_BRANCH}\"")
    add_definitions("-DGIT_COMMIT_HASH=\"${GIT_COMMIT_HASH}\"")
    add_definitions("-DGIT_CHANGE=${GIT_CHANGE}")
    
endif (GIT_EXECUTABLE)

add_definitions(-D_GNU_SOURCE)
add_definitions(-D_DEFAULT_SOURCE)

# define __FILENAME__ as macro like __FILE__ but relativ to CMAKE_SOURCE_DIR
# note: $(subst ....) and $(abspath ....) are GNU Make commands (!) passed on 1:1 by CMake
set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -D__FILENAME__='\"$(subst ${CMAKE_SOURCE_DIR}/,,$(abspath $<))\"'")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__FILENAME__='\"$(subst ${CMAKE_SOURCE_DIR}/,,$(abspath $<))\"'")

# enforce hard core compile settings
option(RIGID_COMPILE_MODE "enable rigid paranoid compile mode" on)
if (RIGID_COMPILE_MODE)
    if (CMAKE_COMPILER_IS_GNUCC)
        message(STATUS "rigid paranoid compile mode enabled")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror -pedantic")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -pedantic")
    endif (CMAKE_COMPILER_IS_GNUCC)
    if (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror -pedantic")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -pedantic")
    endif (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
endif (RIGID_COMPILE_MODE)

# set compile flags: gcc
if (CMAKE_COMPILER_IS_GNUCC)

    # tweak capabilities of gcc versions prior to 4.8
    if (${CMAKE_C_COMPILER_VERSION} LESS 4.8)
    
        message(WARNING "\nyour gcc compiler has version number less than 4.8\nwith some recent 4.7 versions this *might* work\nwe strongly recommend you to switch to a gcc version of 4.8 or higher")
        message(STATUS "gcc compiler < 4.8 detected - tweaking flags")

        # make this clear: we use std::thread
        # so enforce pthread bindings
        # this may not be needed for gcc >= 4.8
        add_definitions(-pthread)
        
        # this is needed to have
        #   std::_this_thread::sleep(...)
        # at hand - at least for gcc 4.6.3 and glibc 2.15
        add_definitions(-D_GLIBCXX_USE_NANOSLEEP)

        # this is needed to have
        #   std::_this_thread::yield()
        # at hand - at least for gcc 4.6.3 and glibc 2.15
        add_definitions(-D_GLIBCXX_USE_SCHED_YIELD)
        
        # this is needed to have
        #   std::map::emplace available
        # (this is bult-in available in gcc-4.8)
        add_definitions(-DFIX_STD_MAP_EMPLACE)
        
        # this is needed to have
        #   std::chrono::time_point be able to add different durations
        # (this is builtin in gcc-4.8)
        add_definitions(-DFIX_STD_CHRONO_DURATION_ADD)

    endif (${CMAKE_C_COMPILER_VERSION} LESS 4.8)
    
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -Wall -Wextra -pedantic -g -ggdb3 -rdynamic")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -pedantic -g -ggdb3 -rdynamic")
    
    # performance
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-builtin")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-builtin")
    
endif (CMAKE_COMPILER_IS_GNUCC)

# set compile flags: clang
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -Wall -Wextra -g -ggdb3 ")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -g -ggdb3")
endif (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")

# additional debug and profiling options
option(DEBUG_MODE_ENABLED "enable debug mode" off)
if (CMAKE_COMPILER_IS_GNUCC)
    if (DEBUG_MODE_ENABLED)
        message(STATUS "debug and profiling mode enabled")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -pg --coverage")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -pg --coverage")
    else(DEBUG_MODE_ENABLED)
        message(STATUS "debug and profiling mode disabled: go for optimizations")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
    endif(DEBUG_MODE_ENABLED)
endif (CMAKE_COMPILER_IS_GNUCC)


# build the handbook
option(DOC_HANDBOOK "create AIT QKD Software Handbook PDF" on)
if (DOC_HANDBOOK)
    message(STATUS "building handbook")
else (DOC_HANDBOOK)
    message(STATUS "not building handbook (turn DOC_HANDBOOK option on, if you want to build the handbook)")
endif (DOC_HANDBOOK)


# build the simulator docu
option(DOC_SIMULATOR "create AIT QKD Simulator Documentation PDF" on)
if (DOC_SIMULATOR)
    message(STATUS "building simulator documentation")
else (DOC_SIMULATOR)
    message(STATUS "not building simulator documentation (turn DOC_SIMULATOR option on, if you want to build the simulator documentation)")
endif (DOC_SIMULATOR)



# ------------------------------------------------------------
# check libs and packages (headers + lib)

# standard C files
check_include_file(stdio.h HAVE_STDIO_H)
check_include_file(stddef.h HAVE_STDDEF_H)
check_include_file(stdlib.h HAVE_STDLIB_H)
check_include_file(inttypes.h HAVE_INTTYPES_H)
check_include_file(memory.h HAVE_MEMORY_H)
check_include_file(string.h HAVE_STRING_H)
check_include_file(unistd.h HAVE_UNISTD_H)

# time
check_include_file(sys/time.h HAVE_SYS_TIME_H)
check_include_file(time.h HAVE_TIME_H)

# file system stuff
check_include_file(fcntl.h HAVE_FCNTL_H)
check_include_file(sys/stat.h HAVE_SYS_STAT_H)

# math
check_include_file(math.h HAVE_MATH_H)

# stdbool
check_include_file(stdbool.h HAVE_STDBOOL_H)

# endian
check_include_file(endian.h HAVE_ENDIAN_H)

# math.h
check_include_file(math.h HAVE_MATH_H)

# networking
check_include_file(netdb.h HAVE_NETDB_H)
check_include_file(ifaddrs.h HAVE_IFADDRS_H)
check_include_file(netinet/in.h HAVE_NETINET_IN_H)
check_include_file(arpa/inet.h HAVE_ARPA_INET_H)
check_include_file(sys/socket.h HAVE_SYS_SOCKET_H)
check_include_file(sys/un.h HAVE_SYS_UN_H)

# assert
check_include_file(assert.h HAVE_ASSERT_H)

# signal
check_include_file(signal.h HAVE_SIGNAL_H)

# sys/uio
check_include_file(sys/uio.h HAVE_SYS_UIO_H)

# syslog
check_include_file(syslog.h HAVE_SYSLOG_H)

# errno
check_include_file(errno.h HAVE_ERRNO_H)

# limits
check_include_file(limits.h HAVE_LIMITS_H)

# sys/mman.h
check_include_file(sys/mman.h HAVE_SYS_MMAN_H)


# enforce pthread linkage (may be obsolete with gcc >= 4.8)
check_library_exists(pthread pthread_kill "/usr/local/lib;/usr/lib;/lib" HAVE_PTHREAD_LIB)
if (HAVE_PTHREAD_LIB)
    set(CMAKE_REQUIRED_LIBRARIES "pthread;${CMAKE_REQUIRED_LIBRARIES}")
endif (HAVE_PTHREAD_LIB)


# get message queues
check_include_file(mqueue.h HAVE_MQUEUE_H)
check_library_exists(rt mq_open "/usr/local/lib;/usr/lib;/lib" HAVE_RT_LIB)
if (HAVE_RT_LIB)
    set(CMAKE_REQUIRED_LIBRARIES "rt;${CMAKE_REQUIRED_LIBRARIES}")
else (HAVE_RT_LIB)
    message(FATAL_ERROR "Could not find rt library, needed for message queue implementation.")
endif (HAVE_RT_LIB)


# get UUID
check_include_file(uuid/uuid.h HAVE_UUID_UUID_H)
check_library_exists(uuid uuid_generate "/usr/local/lib;/usr/lib;/lib" HAVE_UUID_LIB)
if (HAVE_UUID_LIB)
    set(CMAKE_REQUIRED_LIBRARIES "uuid;${CMAKE_REQUIRED_LIBRARIES}")
else (HAVE_UUID_LIB)
    message(FATAL_ERROR "Could not find uuid implementation.")
endif (HAVE_UUID_LIB)


# get the GMP lib
check_include_file(gmp.h HAVE_GMP_H)
check_library_exists(gmp __gmp_version "/usr/local/lib;/usr/lib;/lib" HAVE_GMP_LIB)
if (HAVE_GMP_LIB)
    set(CMAKE_REQUIRED_LIBRARIES "gmp;${CMAKE_REQUIRED_LIBRARIES}")
else (HAVE_GMP_LIB)
    message(FATAL_ERROR "Could not find gmp implementation (see http://gmplib.org/).")
endif (HAVE_GMP_LIB)


# get the 0MQ (zeromq: http://www.zeromq.org)
check_include_file(zmq.h HAVE_ZMQ_H)
check_library_exists(zmq zmq_init "/usr/local/lib;/usr/lib;/lib" HAVE_ZMQ_LIB)
if (HAVE_ZMQ_LIB)
    set(CMAKE_REQUIRED_LIBRARIES "zmq;${CMAKE_REQUIRED_LIBRARIES}")
else (HAVE_ZMQ_LIB)
    message(FATAL_ERROR "Could not find zeromq (0MQ) implementation (see http://www.zeromq.org/).")
endif (HAVE_ZMQ_LIB)


# DBus
pkg_check_modules(DBUS_1 REQUIRED dbus-1)
if (DBUS_1_FOUND)
    set(HAVE_DBUS_1_PKG 1)
    set(CMAKE_REQUIRED_LIBRARIES "${DBUS_1_LIBRARIES};${CMAKE_REQUIRED_LIBRARIES}")
    include_directories(${DBUS_1_STATIC_INCLUDE_DIRS})
else (DBUS_1_FOUND)
    message(FATAL_ERROR "Couldn't find dbus-1 installation.")
endif (DBUS_1_FOUND)


# get OpenSSL 
include(FindOpenSSL)
if (OPENSSL_FOUND)
    set(CMAKE_REQUIRED_LIBRARIES "${OPENSSL_LIBRARIES};${CMAKE_REQUIRED_LIBRARIES}")
    set(HAVE_OPENSSL_LIB 1)
else (OPENSSL_FOUND)
    message(FATAL_ERROR "OpenSSL not found on the system.")
endif (OPENSSL_FOUND)


# get BOOST!
find_package(Boost 1.46.0 COMPONENTS filesystem program_options system)
if (Boost_FOUND)
    set(CMAKE_REQUIRED_LIBRARIES "${Boost_LIBRARIES};${CMAKE_REQUIRED_LIBRARIES}")
    include_directories(${Boost_INCLUDE_DIRS})
    set(HAVE_BOOST_LIB 1)
else (Boost_FOUND)
    message(FATAL_ERROR "Boost >= 1.46.0 library not found.")
endif (Boost_FOUND)


# get ZLIB
include(FindZLIB)
if (ZLIB_FOUND)
    set(CMAKE_REQUIRED_LIBRARIES "${ZLIB_LIBRARIES};${CMAKE_REQUIRED_LIBRARIES}")
    include_directories(${ZLIB_INCLUDE_DIRS})
    set(HAVE_ZLIB_LIB 1)
else (ZLIB_FOUND)
    message(FATAL_ERROR "ZLIB library not found.")
endif (ZLIB_FOUND)


# Qt4
find_package(Qt4 COMPONENTS QtCore QtGui QtNetwork QtXml QtDbus)
if (NOT QT4_FOUND)
    message(FATAL_ERROR "Qt4 not found! Is QTDIR environment variable set?")
else (NOT QT4_FOUND)
    include(${QT_USE_FILE})
    set(CMAKE_REQUIRED_LIBRARIES "${QT_LIBRARIES};${CMAKE_REQUIRED_LIBRARIES}")
    set(HAVE_QT4_PKG 1)
endif (NOT QT4_FOUND)

# Qwt
include(${CMAKE_SOURCE_DIR}/cmake/FindQwt.cmake)
if (NOT QWT_FOUND)
    message(FATAL_ERROR "QWT not found!")
else (NOT QWT_FOUND)
    include_directories(${QWT_INCLUDE_DIRS})
    set(CMAKE_REQUIRED_LIBRARIES "${QWT_LIBRARIES};${CMAKE_REQUIRED_LIBRARIES}")
    set(HAVE_QWT_PKG 1)
endif (NOT QWT_FOUND)


# Doxygen
find_package(Doxygen)
if (DOXYGEN_FOUND STREQUAL "NO")
    message(FATAL_ERROR "Doxygen not found. Please get a copy http://www.doxygen.org")
endif (DOXYGEN_FOUND STREQUAL "NO")


# xxd
find_program(XXD xxd)
if (XXD-NOTFOUND)
    message(FATAL_ERROR "xxd tool not found. Please install vim-common.")
endif (XXD-NOTFOUND)


# search for latex if needed
if (DOC_HANDBOOK OR DOC_SIMULATOR)
    include(FindLATEX)
    message(STATUS "latex compiler: ${LATEX_COMPILER}")
    message(STATUS "pdflatex compiler: ${PDFLATEX_COMPILER}")
endif(DOC_HANDBOOK OR DOC_SIMULATOR)
    

# prepare doxygen configuration file
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

# add doxygen as target
add_custom_target(doxygen ALL
    ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    COMMENT "Doxygen running ...")

# cleanup $build/api-doc on "make clean"
set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES api-doc)

# add doxygen as dependency to doc-target
add_custom_target(doc)
add_dependencies(doc doxygen)

# install HTML API documentation and manual pages
set(DOC_PATH "share/doc/${CPACK_PACKAGE_NAME}")
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html DESTINATION ${DOC_PATH}/qkd COMPONENT doc)


# ------------------------------------------------------------
# dump the config file

# create the config.h and baseinc.h
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})


# ------------------------------------------------------------
# go through the subs

add_subdirectory(lib)
add_subdirectory(bin)
add_subdirectory(etc)
add_subdirectory(doc)
add_subdirectory(examples)
add_subdirectory(test)


# ------------------------------------------------------------
# additional stuff for installation

install(DIRECTORY etc/q3p etc/qkd DESTINATION /etc)


# ------------------------------------------------------------
# packaging
#
# at least now (cmake-3.2.2) we cannot build multiple packages
# inside a CMakeList.txt.
#
# Also calling cpack from the command line bears not the
# configuration options needed (e.g. not 'include', 'message', etc...)
#
# Therefore we can only build stuff for one dedicated platform 
# at a time ... =(
# 

option(PACKAGE_DEBIAN8      "Build Debian8 package"         ON)
option(PACKAGE_UBUNTU15     "Build Ubuntu15 package"        OFF)
option(PACKAGE_UBUNTU16     "Build Ubuntu16 package"        OFF)
set(PACKAGES_TO_BUILD 0)

# only one package at a time allowed
if (PACKAGE_DEBIAN8)
    message(STATUS "Building debian8 packages")
    MATH(EXPR PACKAGES_TO_BUILD "${PACKAGES_TO_BUILD} + 1")
    set(PACKAGE_TARGET_SYSTEM "debian8")
endif (PACKAGE_DEBIAN8)

if (PACKAGE_UBUNTU15)
    message(STATUS "Building ubuntu15 packages")
    MATH(EXPR PACKAGES_TO_BUILD "${PACKAGES_TO_BUILD} + 1")
    set(PACKAGE_TARGET_SYSTEM "ubuntu15")
endif (PACKAGE_UBUNTU15)

if (PACKAGE_UBUNTU16)
    message(STATUS "Building ubuntu16 packages")
    MATH(EXPR PACKAGES_TO_BUILD "${PACKAGES_TO_BUILD} + 1")
    set(PACKAGE_TARGET_SYSTEM "ubuntu16")
endif (PACKAGE_UBUNTU16)

if (PACKAGES_TO_BUILD GREATER 1) 
    message(WARNING "Can't build more package at the time. The last one counts: ${PACKAGE_TARGET_SYSTEM}")
endif (PACKAGES_TO_BUILD GREATER 1) 

set(CPACK_PACKAGE_NAME "qkd")
set(CPACK_PACKAGE_DESCRIPTION "QKD next generation")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "This is the AIT SQT QKD Software - R10")
set(CPACK_PACKAGE_CONTACT "Oliver Maurhart <oliver.maurhart@ait.ac.at>")
set(CPACK_PACKAGE_VENDOR "AIT")
set(CPACK_PACKAGE_VERSION_MAJOR "9")
set(CPACK_PACKAGE_VERSION_MINOR "9999")
set(CPACK_PACKAGE_VERSION_PATCH "8")
set(CPACK_PROJECT_VERSION_STRING "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}" )

# ---------- debian 8 (jessie) ----------

if (${PACKAGE_TARGET_SYSTEM} STREQUAL "debian8")

    message(STATUS "Package creation set to 'debian8'")

    set(CPACK_GENERATOR "DEB")

    set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}-${CMAKE_SYSTEM_PROCESSOR}")
    set(CPACK_SOURCE_IGNORE_FILES "/build/*;/.git/")

    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.13), libgcc1 (>= 1:4.4), libgmp10, libboost-filesystem1.55.0, libboost-program-options1.55.0, libboost-system1.55.0, libzmq3, libqtgui4, libqtdbus4, libqt4-network, libqwt6, libcap2-bin, python3")
    set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cpack/deb/control/postinst;${CMAKE_CURRENT_SOURCE_DIR}/cmake/cpack/deb/control/postrm;${CMAKE_CURRENT_SOURCE_DIR}/cmake/cpack/deb/control/prerm;")

    # debianization
    string(TOLOWER "${CPACK_PACKAGE_NAME}" CPACK_PACKAGE_NAME_LOWERCASE)
    find_program(DPKG_PROGRAM dpkg DOC "dpkg program of Debian-based systems")
    if (DPKG_PROGRAM)
        # use dpkg to fix the package file name
        execute_process(
            COMMAND ${DPKG_PROGRAM} --print-architecture
            OUTPUT_VARIABLE CPACK_DEBIAN_PACKAGE_ARCHITECTURE
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME_LOWERCASE}_${CPACK_PROJECT_VERSION_STRING}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")
        set(PACKAGE_ARCH ${CPACK_DEBIAN_PACKAGE_ARCHITECTURE})
    else (DPKG_PROGRAM)
        set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME_LOWERCASE}_${CPACK_PROJECT_VERSION_STRING}_${CMAKE_SYSTEM_NAME}")
        set(PACKAGE_ARCH ${CMAKE_HOST_SYSTEM_PROCESSOR})
    endif (DPKG_PROGRAM)

    include(CPack)

endif (${PACKAGE_TARGET_SYSTEM} STREQUAL "debian8")


# ---------- ubuntu 15 (vivid and wily) ----------

if (${PACKAGE_TARGET_SYSTEM} STREQUAL "ubuntu15")

    message(STATUS "Package creation set to 'ubuntu15'")
    
    set(CPACK_GENERATOR "DEB")

    set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}-0ubuntu15_${CMAKE_SYSTEM_PROCESSOR}")
    set(CPACK_SOURCE_IGNORE_FILES "/build/*;/.git/")

    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.13), libgcc1 (>= 1:4.4), libgmp10, libboost-filesystem1.58.0, libboost-program-options1.58.0, libboost-system1.58.0, libzmq3, libqtgui4, libqtdbus4, libqt4-network, libqwt6, libcap2-bin, python3")
    set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cpack/deb/control/postinst;${CMAKE_CURRENT_SOURCE_DIR}/cmake/cpack/deb/control/postrm;${CMAKE_CURRENT_SOURCE_DIR}/cmake/cpack/deb/control/prerm;")

    # debianization
    string(TOLOWER "${CPACK_PACKAGE_NAME}" CPACK_PACKAGE_NAME_LOWERCASE)
    find_program(DPKG_PROGRAM dpkg DOC "dpkg program of Debian-based systems")
    if (DPKG_PROGRAM)
        # use dpkg to fix the package file name
        execute_process(
            COMMAND ${DPKG_PROGRAM} --print-architecture
            OUTPUT_VARIABLE CPACK_DEBIAN_PACKAGE_ARCHITECTURE
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME_LOWERCASE}_${CPACK_PROJECT_VERSION_STRING}-0ubuntu15_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")
        set(PACKAGE_ARCH ${CPACK_DEBIAN_PACKAGE_ARCHITECTURE})
    else (DPKG_PROGRAM)
        set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME_LOWERCASE}_${CPACK_PROJECT_VERSION_STRING}-0ubuntu15_${CMAKE_SYSTEM_NAME}")
        set(PACKAGE_ARCH ${CMAKE_HOST_SYSTEM_PROCESSOR})
    endif (DPKG_PROGRAM)

    include(CPack)

endif (${PACKAGE_TARGET_SYSTEM} STREQUAL "ubuntu15")


# ---------- ubuntu 16 (xenial) ----------

if (${PACKAGE_TARGET_SYSTEM} STREQUAL "ubuntu16")

    message(STATUS "Package creation set to 'ubuntu16'")
    
    set(CPACK_GENERATOR "DEB")

    set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}-0ubuntu16_${CMAKE_SYSTEM_PROCESSOR}")
    set(CPACK_SOURCE_IGNORE_FILES "/build/*;/.git/")

    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.13), libgcc1 (>= 1:4.4), libgmp10, libboost-filesystem1.58.0, libboost-program-options1.58.0, libboost-system1.58.0, libzmq5, libqtgui4, libqtdbus4, libqt4-network, libqwt6abi1, libcap2-bin, python3")
    set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cpack/deb/control/postinst;${CMAKE_CURRENT_SOURCE_DIR}/cmake/cpack/deb/control/postrm;${CMAKE_CURRENT_SOURCE_DIR}/cmake/cpack/deb/control/prerm;")

    # debianization
    string(TOLOWER "${CPACK_PACKAGE_NAME}" CPACK_PACKAGE_NAME_LOWERCASE)
    find_program(DPKG_PROGRAM dpkg DOC "dpkg program of Debian-based systems")
    if (DPKG_PROGRAM)
        # use dpkg to fix the package file name
        execute_process(
            COMMAND ${DPKG_PROGRAM} --print-architecture
            OUTPUT_VARIABLE CPACK_DEBIAN_PACKAGE_ARCHITECTURE
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME_LOWERCASE}_${CPACK_PROJECT_VERSION_STRING}-0ubuntu16_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")
        set(PACKAGE_ARCH ${CPACK_DEBIAN_PACKAGE_ARCHITECTURE})
    else (DPKG_PROGRAM)
        set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME_LOWERCASE}_${CPACK_PROJECT_VERSION_STRING}-0ubuntu16_${CMAKE_SYSTEM_NAME}")
        set(PACKAGE_ARCH ${CMAKE_HOST_SYSTEM_PROCESSOR})
    endif (DPKG_PROGRAM)

    include(CPack)

endif (${PACKAGE_TARGET_SYSTEM} STREQUAL "ubuntu16")


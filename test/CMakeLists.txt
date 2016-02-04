# ------------------------------------------------------------
# CMakeLists.txt 
# 
# make: tests
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

# ------------------------------------------------------------
# test-data preparations


# copy the test data
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/test-data DESTINATION ${CMAKE_CURRENT_BINARY_DIR} USE_SOURCE_PERMISSIONS)

# copy test files to build folder
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/bin/test-functions ${CMAKE_CURRENT_BINARY_DIR}/bin/test-functions @ONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test-data/bb84-cascade/pipeline-alice.xml ${CMAKE_CURRENT_BINARY_DIR}/test-data/bb84-cascade/pipeline-alice.xml @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test-data/bb84-cascade/pipeline-bob.xml ${CMAKE_CURRENT_BINARY_DIR}/test-data/bb84-cascade/pipeline-bob.xml @ONLY)

# copy the test configs
configure_file(q3p/alice.conf ${CMAKE_CURRENT_BINARY_DIR}/q3p/alice.conf @ONLY)
configure_file(q3p/bob.conf ${CMAKE_CURRENT_BINARY_DIR}/q3p/bob.conf @ONLY)

# create a large shared-secret file: 1MB
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/test-data/shared-secret-very-long
    COMMAND dd if=/dev/urandom of=${CMAKE_CURRENT_BINARY_DIR}/test-data/shared-secret-very-long count=1000000 bs=1
    COMMENT "Creating a very large random shared secret (FOR TESTING ONLY)")
add_custom_target(shared_secret ALL
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/test-data/shared-secret-very-long)


# ------------------------------------------------------------
# go through the subs

add_subdirectory(lib)
add_subdirectory(bin)


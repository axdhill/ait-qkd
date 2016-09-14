# ------------------------------------------------------------
# CMakeLists.txt 
# 
# make: tests
#
# Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
#
# Copyright (C) 2013-2016 AIT Austrian Institute of Technology
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


# additional includes
include_directories(${CMAKE_SOURCE_DIR}/include ${CMAKE_BINARY_DIR})

# libs
set(CMAKE_REQUIRED_LIBRARIES "qkd;${CMAKE_REQUIRED_LIBRARIES}")

# test lib coverage
set(TEST_TABULAR_WRITER_SRC                     
    tabular_writer.cpp
    ../../../bin/tools/qkd-view/tabular_writer.cpp)
    
add_executable(test-tabular-writer              ${TEST_TABULAR_WRITER_SRC})

target_link_libraries(test-tabular-writer       ${CMAKE_REQUIRED_LIBRARIES})


# ------------------------------------------------------------
# test scripts

# tools test
configure_file(test-qkd-key-compare             ${CMAKE_CURRENT_BINARY_DIR}/test-qkd-key-compare            @ONLY)
configure_file(test-qkd-key-gen                 ${CMAKE_CURRENT_BINARY_DIR}/test-qkd-key-gen                @ONLY)
configure_file(test-qkd-key-slice               ${CMAKE_CURRENT_BINARY_DIR}/test-qkd-key-slice              @ONLY)
configure_file(test-qkd-pipeline                ${CMAKE_CURRENT_BINARY_DIR}/test-qkd-pipeline               @ONLY)
configure_file(test-qkd-pipeline-autoconnect    ${CMAKE_CURRENT_BINARY_DIR}/test-qkd-pipeline-autoconnect   @ONLY)
configure_file(test-qkd-statistics-average      ${CMAKE_CURRENT_BINARY_DIR}/test-qkd-statistics-average     @ONLY)
configure_file(test-pipeline-sifting            ${CMAKE_CURRENT_BINARY_DIR}/test-pipeline-sifting           @ONLY)


# ------------------------------------------------------------
# tests

# module pipeline test
add_test(qkd-pipeline                           ${CMAKE_CURRENT_BINARY_DIR}/test-qkd-pipeline)
add_test(qkd-pipeline-autoconnect               ${CMAKE_CURRENT_BINARY_DIR}/test-qkd-pipeline-autoconnect)
add_test(qkd-qkd-statistics-average             ${CMAKE_CURRENT_BINARY_DIR}/test-qkd-statistics-average)
add_test(qkd-key-gen                            ${CMAKE_CURRENT_BINARY_DIR}/test-qkd-key-gen)
add_test(qkd-key-compare                        ${CMAKE_CURRENT_BINARY_DIR}/test-qkd-key-compare)

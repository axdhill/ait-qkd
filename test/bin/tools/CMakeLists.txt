# ------------------------------------------------------------
# CMakeLists.txt 
# 
# make: tests
#
# Autor: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
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


# ------------------------------------------------------------
# test scripts

# tools test
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


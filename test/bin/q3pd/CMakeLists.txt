# ------------------------------------------------------------
# CMakeLists.txt 
# 
# make: tests
#
# Autor: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
#
# Copyright (C) 2012-2015 AIT Austrian Institute of Technology
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
# test

# q3pd test scripts
configure_file(test-q3pd-start-stop             ${CMAKE_CURRENT_BINARY_DIR}/test-q3pd-start-stop            @ONLY)
configure_file(test-q3pd-inject                 ${CMAKE_CURRENT_BINARY_DIR}/test-q3pd-inject                @ONLY)
configure_file(test-q3pd-connect                ${CMAKE_CURRENT_BINARY_DIR}/test-q3pd-connect               @ONLY)
configure_file(test-q3pd-reconnect              ${CMAKE_CURRENT_BINARY_DIR}/test-q3pd-reconnect             @ONLY)
configure_file(test-q3pd-config                 ${CMAKE_CURRENT_BINARY_DIR}/test-q3pd-config                @ONLY)
configure_file(test-q3pd-mq                     ${CMAKE_CURRENT_BINARY_DIR}/test-q3pd-mq                    @ONLY)
configure_file(test-q3pd-nic                    ${CMAKE_CURRENT_BINARY_DIR}/test-q3pd-nic                   @ONLY)
configure_file(test-q3pd-modules                ${CMAKE_CURRENT_BINARY_DIR}/test-q3pd-modules               @ONLY)
configure_file(test-q3pd-as-a-module            ${CMAKE_CURRENT_BINARY_DIR}/test-q3pd-as-a-module           @ONLY)

# tests
add_test(q3pd-start-stop                        ${CMAKE_CURRENT_BINARY_DIR}/test-q3pd-start-stop)
add_test(q3pd-inject                            ${CMAKE_CURRENT_BINARY_DIR}/test-q3pd-inject)
add_test(q3pd-connect                           ${CMAKE_CURRENT_BINARY_DIR}/test-q3pd-connect)
add_test(q3pd-reconnect                         ${CMAKE_CURRENT_BINARY_DIR}/test-q3pd-reconnect)
add_test(q3pd-config                            ${CMAKE_CURRENT_BINARY_DIR}/test-q3pd-config)
add_test(q3pd-mq                                ${CMAKE_CURRENT_BINARY_DIR}/test-q3pd-mq)
#add_test(q3pd-nic                               ${CMAKE_CURRENT_BINARY_DIR}/test-q3pd-nic)
add_test(q3pd-modules                           ${CMAKE_CURRENT_BINARY_DIR}/test-q3pd-modules)
add_test(q3pd-as-a-module                       ${CMAKE_CURRENT_BINARY_DIR}/test-q3pd-as-a-module)

# convinient test environment
configure_file(test-q3pd-environment            ${CMAKE_CURRENT_BINARY_DIR}/test-q3pd-environment           @ONLY)

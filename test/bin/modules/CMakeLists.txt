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
# test scripts

# module interoperation test
configure_file(test-stdin-stdout                ${CMAKE_CURRENT_BINARY_DIR}/test-stdin-stdout               @ONLY)
configure_file(test-ipc-cat-tee                 ${CMAKE_CURRENT_BINARY_DIR}/test-ipc-cat-tee                @ONLY)
configure_file(test-tcp-cat-tee                 ${CMAKE_CURRENT_BINARY_DIR}/test-tcp-cat-tee                @ONLY)
configure_file(test-throttle                    ${CMAKE_CURRENT_BINARY_DIR}/test-throttle                   @ONLY)
configure_file(test-killer                      ${CMAKE_CURRENT_BINARY_DIR}/test-killer                     @ONLY)
configure_file(test-ping                        ${CMAKE_CURRENT_BINARY_DIR}/test-ping                       @ONLY)
configure_file(test-interrupt                   ${CMAKE_CURRENT_BINARY_DIR}/test-interrupt                  @ONLY)

# module pipeline test
configure_file(test-mod-bb84                    ${CMAKE_CURRENT_BINARY_DIR}/test-mod-bb84                   @ONLY)
configure_file(test-mod-error-estimation        ${CMAKE_CURRENT_BINARY_DIR}/test-mod-error-estimation       @ONLY)
configure_file(test-mod-cascade                 ${CMAKE_CURRENT_BINARY_DIR}/test-mod-cascade                @ONLY)
configure_file(test-mod-confirmation            ${CMAKE_CURRENT_BINARY_DIR}/test-mod-confirmation           @ONLY)
configure_file(test-mod-buffer                  ${CMAKE_CURRENT_BINARY_DIR}/test-mod-buffer                 @ONLY)
configure_file(test-mod-drop                    ${CMAKE_CURRENT_BINARY_DIR}/test-mod-drop                   @ONLY)
configure_file(test-mod-reorder                 ${CMAKE_CURRENT_BINARY_DIR}/test-mod-reorder                @ONLY)
configure_file(test-mod-privacy-amplification   ${CMAKE_CURRENT_BINARY_DIR}/test-mod-privacy-amplification  @ONLY)
configure_file(test-mod-auth                    ${CMAKE_CURRENT_BINARY_DIR}/test-mod-auth                   @ONLY)
configure_file(test-mod-enkey                   ${CMAKE_CURRENT_BINARY_DIR}/test-mod-enkey                  @ONLY)
configure_file(test-mod-dekey                   ${CMAKE_CURRENT_BINARY_DIR}/test-mod-dekey                  @ONLY)
configure_file(test-full-pipeline               ${CMAKE_CURRENT_BINARY_DIR}/test-full-pipeline              @ONLY)
configure_file(test-full-long-running           ${CMAKE_CURRENT_BINARY_DIR}/test-full-long-running          @ONLY)
configure_file(test-disruptive-drop             ${CMAKE_CURRENT_BINARY_DIR}/test-disruptive-drop            @ONLY)
configure_file(test-disruptive-reorder          ${CMAKE_CURRENT_BINARY_DIR}/test-disruptive-reorder         @ONLY)
configure_file(test-full-disruptive             ${CMAKE_CURRENT_BINARY_DIR}/test-full-disruptive            @ONLY)
configure_file(test-parallel-cat-tee            ${CMAKE_CURRENT_BINARY_DIR}/test-parallel-cat-tee           @ONLY)
configure_file(test-simulator-pipeline          ${CMAKE_CURRENT_BINARY_DIR}/test-simulator-pipeline         @ONLY)


# ------------------------------------------------------------
# tests

# module interoperation test
add_test(stdin-stdout                           ${CMAKE_CURRENT_BINARY_DIR}/test-stdin-stdout)
add_test(ipc-cat-tee                            ${CMAKE_CURRENT_BINARY_DIR}/test-ipc-cat-tee)
add_test(tcp-cat-tee                            ${CMAKE_CURRENT_BINARY_DIR}/test-tcp-cat-tee)
add_test(throttle                               ${CMAKE_CURRENT_BINARY_DIR}/test-throttle)
add_test(killer                                 ${CMAKE_CURRENT_BINARY_DIR}/test-killer)
add_test(ping                                   ${CMAKE_CURRENT_BINARY_DIR}/test-ping)
add_test(interrupt                              ${CMAKE_CURRENT_BINARY_DIR}/test-interrupt)

# module pipeline test
add_test(mod-bb84                               ${CMAKE_CURRENT_BINARY_DIR}/test-mod-bb84)
add_test(mod-error-estimation                   ${CMAKE_CURRENT_BINARY_DIR}/test-mod-error-estimation)
add_test(mod-cascade                            ${CMAKE_CURRENT_BINARY_DIR}/test-mod-cascade)
add_test(mod-confirmation                       ${CMAKE_CURRENT_BINARY_DIR}/test-mod-confirmation)
add_test(mod-buffer                             ${CMAKE_CURRENT_BINARY_DIR}/test-mod-buffer)
add_test(mod-drop                               ${CMAKE_CURRENT_BINARY_DIR}/test-mod-drop)
add_test(mod-reorder                            ${CMAKE_CURRENT_BINARY_DIR}/test-mod-reorder)
add_test(mod-privacy-amplification              ${CMAKE_CURRENT_BINARY_DIR}/test-mod-privacy-amplification)
add_test(mod-auth                               ${CMAKE_CURRENT_BINARY_DIR}/test-mod-auth)
add_test(mod-enkey                              ${CMAKE_CURRENT_BINARY_DIR}/test-mod-enkey)
add_test(mod-dekey                              ${CMAKE_CURRENT_BINARY_DIR}/test-mod-dekey)
add_test(full-pipeline                          ${CMAKE_CURRENT_BINARY_DIR}/test-full-pipeline)
add_test(disruptive-drop                        ${CMAKE_CURRENT_BINARY_DIR}/test-disruptive-drop)
add_test(disruptive-reorder                     ${CMAKE_CURRENT_BINARY_DIR}/test-disruptive-reorder)
add_test(full-disruptive                        ${CMAKE_CURRENT_BINARY_DIR}/test-full-disruptive)
add_test(parallel-cat-tee                       ${CMAKE_CURRENT_BINARY_DIR}/test-parallel-cat-tee)


# ------------------------------------------------------------
# convenient scripts

configure_file(test-testing-pipeline            ${CMAKE_CURRENT_BINARY_DIR}/test-testing-pipeline   @ONLY)


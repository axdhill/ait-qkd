# ------------------------------------------------------------
# CMakeLists.txt 
# 
# make: tests
#
# Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
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
# tests

# additional includes
include_directories(${CMAKE_SOURCE_DIR}/include ${CMAKE_BINARY_DIR})

# libs
set(CMAKE_REQUIRED_LIBRARIES "qkd;${CMAKE_REQUIRED_LIBRARIES}")

# test lib coverage
set(TEST_LIB_SRC                            lib.cpp)


# --------------------------------------------------
# test lib source units
#
set(TEST_ATOF_SRC                           utility/atof.cpp)
set(TEST_AVERAGE_SRC                        utility/average.cpp)
set(TEST_BACKTRACE_SRC                      utility/backtrace.cpp)
set(TEST_BIGINT_SRC                         utility/bigint.cpp)
set(TEST_BUFFER_SRC                         utility/buffer.cpp)
set(TEST_CHECKSUM_SRC                       utility/checksum.cpp)
set(TEST_DBUS_SRC                           utility/dbus.cpp)
set(TEST_DEBUG_SRC                          utility/debug.cpp)
set(TEST_ENVIRONMENT_SRC                    utility/environment.cpp)
set(TEST_INVESTIGATION_SRC                  utility/investigation.cpp)
set(TEST_MEMORY_SRC                         utility/memory.cpp)
set(TEST_QUEUE_SRC                          utility/queue.cpp)
set(TEST_RANDOM_SRC                         utility/random.cpp)
set(TEST_SHANNON_SRC                        utility/shannon.cpp)
set(TEST_SI_UNITS_SRC                       utility/si_units.cpp)
set(TEST_SYSLOG_SRC                         utility/syslog.cpp)
set(TEST_ZIP_SRC                            utility/zip.cpp)

set(TEST_CRYPTO_SRC                         crypto/crypto.cpp)
set(TEST_EVHASH_SRC                         crypto/evhash.cpp)
set(TEST_ASSOCIATION_SRC                    crypto/association.cpp)
set(TEST_KEY_SRC                            key/key.cpp)
set(TEST_KEY_RING_SRC                       key/key_ring.cpp)

set(TEST_Q3P_MESSAGE_SRC                    q3p/message.cpp)

set(TEST_NULL_MODULE_SRC                    module/null_module.cpp)
set(TEST_CONFIG_MODULE_SRC                  module/config_module.cpp)
set(TEST_TERMINATE_MODULE_SRC               module/terminate_module.cpp)


# --------------------------------------------------
# test programs
#
add_executable(test-lib                     ${TEST_LIB_SRC})
add_executable(test-atof                    ${TEST_ATOF_SRC})
add_executable(test-average                 ${TEST_AVERAGE_SRC})
add_executable(test-backtrace               ${TEST_BACKTRACE_SRC})
add_executable(test-bigint                  ${TEST_BIGINT_SRC})
add_executable(test-buffer                  ${TEST_BUFFER_SRC})
add_executable(test-checksum                ${TEST_CHECKSUM_SRC})
add_executable(test-dbus                    ${TEST_DBUS_SRC})
add_executable(test-debug                   ${TEST_DEBUG_SRC})
add_executable(test-environment             ${TEST_ENVIRONMENT_SRC})
add_executable(test-investigation           ${TEST_INVESTIGATION_SRC})
add_executable(test-memory                  ${TEST_MEMORY_SRC})
add_executable(test-queue                   ${TEST_QUEUE_SRC})
add_executable(test-random                  ${TEST_RANDOM_SRC})
add_executable(test-shannon                 ${TEST_SHANNON_SRC})
add_executable(test-si-units                ${TEST_SI_UNITS_SRC})
add_executable(test-syslog                  ${TEST_SYSLOG_SRC})
add_executable(test-zip                     ${TEST_ZIP_SRC})

add_executable(test-crypto                  ${TEST_CRYPTO_SRC})
add_executable(test-evhash                  ${TEST_EVHASH_SRC})
add_executable(test-association             ${TEST_ASSOCIATION_SRC})
add_executable(test-key                     ${TEST_KEY_SRC})
add_executable(test-key_ring                ${TEST_KEY_RING_SRC})

add_executable(test-q3p-message             ${TEST_Q3P_MESSAGE_SRC})

add_executable(test-null-module             ${TEST_NULL_MODULE_SRC})
add_executable(test-config-module           ${TEST_CONFIG_MODULE_SRC})
add_executable(test-terminate-module        ${TEST_TERMINATE_MODULE_SRC})


# --------------------------------------------------
# linkage

target_link_libraries(test-lib                  ${CMAKE_REQUIRED_LIBRARIES})
target_link_libraries(test-atof                 ${CMAKE_REQUIRED_LIBRARIES})
target_link_libraries(test-average              ${CMAKE_REQUIRED_LIBRARIES})
target_link_libraries(test-backtrace            ${CMAKE_REQUIRED_LIBRARIES})
target_link_libraries(test-bigint               ${CMAKE_REQUIRED_LIBRARIES})
target_link_libraries(test-buffer               ${CMAKE_REQUIRED_LIBRARIES})
target_link_libraries(test-checksum             ${CMAKE_REQUIRED_LIBRARIES})
target_link_libraries(test-dbus                 ${CMAKE_REQUIRED_LIBRARIES})
target_link_libraries(test-debug                ${CMAKE_REQUIRED_LIBRARIES})
target_link_libraries(test-environment          ${CMAKE_REQUIRED_LIBRARIES})
target_link_libraries(test-investigation        ${CMAKE_REQUIRED_LIBRARIES})
target_link_libraries(test-memory               ${CMAKE_REQUIRED_LIBRARIES})
target_link_libraries(test-queue                ${CMAKE_REQUIRED_LIBRARIES})
target_link_libraries(test-random               ${CMAKE_REQUIRED_LIBRARIES})
target_link_libraries(test-shannon              ${CMAKE_REQUIRED_LIBRARIES})
target_link_libraries(test-si-units             ${CMAKE_REQUIRED_LIBRARIES})
target_link_libraries(test-syslog               ${CMAKE_REQUIRED_LIBRARIES})
target_link_libraries(test-zip                  ${CMAKE_REQUIRED_LIBRARIES})

target_link_libraries(test-crypto               ${CMAKE_REQUIRED_LIBRARIES})
target_link_libraries(test-evhash               ${CMAKE_REQUIRED_LIBRARIES})
target_link_libraries(test-association          ${CMAKE_REQUIRED_LIBRARIES})
target_link_libraries(test-key                  ${CMAKE_REQUIRED_LIBRARIES})
target_link_libraries(test-key_ring             ${CMAKE_REQUIRED_LIBRARIES})

target_link_libraries(test-q3p-message          ${CMAKE_REQUIRED_LIBRARIES})

target_link_libraries(test-null-module          ${CMAKE_REQUIRED_LIBRARIES})
target_link_libraries(test-config-module        ${CMAKE_REQUIRED_LIBRARIES})
target_link_libraries(test-terminate-module     ${CMAKE_REQUIRED_LIBRARIES})


# --------------------------------------------------
# test scripts

# module tests
configure_file(module/test-module-null          ${CMAKE_CURRENT_BINARY_DIR}/test-module-null            @ONLY)
configure_file(module/test-module-config        ${CMAKE_CURRENT_BINARY_DIR}/test-module-config          @ONLY)
configure_file(module/test-module-terminate     ${CMAKE_CURRENT_BINARY_DIR}/test-module-terminate       @ONLY)


# --------------------------------------------------
# add tests 
# (backtrace is not tested since it needs a SIGSEV to be successful 
# ... how to test this?)

add_test(lib                                test-lib)
add_test(debug                              test-debug)
add_test(environment                        test-environment)
add_test(si-units                           test-si-units)
add_test(atof                               test-atof)
add_test(investigation                      test-investigation)
add_test(memory                             test-memory)
add_test(buffer                             test-buffer)
add_test(checksum                           test-checksum)
add_test(bigint                             test-bigint)
add_test(random                             test-random)
add_test(average                            test-average)
add_test(shannon                            test-shannon)
add_test(syslog                             test-syslog)
add_test(queue                              test-queue)
add_test(dbus                               test-dbus)
add_test(zip                                test-zip)

add_test(crypto                             test-crypto)
add_test(evhash                             test-evhash)
add_test(association                        test-association)
add_test(key                                test-key)
add_test(key_ring                           test-key_ring)

add_test(q3p_message                        test-q3p-message)

add_test(module-null                        ${CMAKE_CURRENT_BINARY_DIR}/test-module-null)
add_test(module-config                      ${CMAKE_CURRENT_BINARY_DIR}/test-module-config)
add_test(module-terminate                   ${CMAKE_CURRENT_BINARY_DIR}/test-module-terminate)


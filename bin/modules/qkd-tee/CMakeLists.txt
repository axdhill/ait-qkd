# ------------------------------------------------------------
# CMakeLists.txt 
# 
# make: qkd/bin/modules/qkd-tee
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
# modules/qkd-tee

# additional includes
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# sources
set(QKD_TEE_SRC
    main.cpp
    qkd-tee.cpp
)

# Qt MOC
QT4_GENERATE_MOC(qkd-tee.h ${CMAKE_CURRENT_BINARY_DIR}/qkd-tee.moc.cpp) 
set(QKD_TEE_MOC
    qkd-tee.moc.cpp
)

# DBus Integration
QT4_GENERATE_DBUS_INTERFACE(qkd-tee.h ${CMAKE_CURRENT_BINARY_DIR}/qkd-tee.xml)
QT4_ADD_DBUS_ADAPTOR(QKD_TEE_DBUS_SRC
    ${CMAKE_CURRENT_BINARY_DIR}/qkd-tee.xml 
    ${CMAKE_CURRENT_SOURCE_DIR}/qkd-tee.h 
    qkd_tee
    qkd_tee_dbus)

# bin definition
add_executable(qkd-tee ${QKD_TEE_SRC} ${QKD_TEE_MOC} ${QKD_TEE_DBUS_SRC})

# linkage
target_link_libraries(qkd-tee ${CMAKE_REQUIRED_LIBRARIES})

# install
install(TARGETS qkd-tee RUNTIME DESTINATION bin COMPONENT modules)


# ------------------------------------------------------------
# CMakeLists.txt 
# 
# make: qkd/bin/q3pd
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
# q3pd

# additional includes
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# sources
set(Q3P_DAEMON_SRC
    main.cpp
    node.cpp
)

# Qt MOC
QT5_GENERATE_MOC(node.h ${CMAKE_CURRENT_BINARY_DIR}/node.moc.cpp) 
set(Q3P_DAEMON_MOC
    node.moc.cpp
)

# DBus Integration
QT5_GENERATE_DBUS_INTERFACE(node.h ${CMAKE_CURRENT_BINARY_DIR}/node.xml)
QT5_ADD_DBUS_ADAPTOR(Q3P_DAEMON_DBUS_SRC
    ${CMAKE_CURRENT_BINARY_DIR}/node.xml 
    ${CMAKE_CURRENT_SOURCE_DIR}/node.h 
    qkd::q3p::node
    node_dbus)

# bin definition
add_executable(q3pd ${Q3P_DAEMON_SRC} ${Q3P_DAEMON_MOC} ${Q3P_DAEMON_DBUS_SRC})

# just a nice text to show warning about capabilities for TUN/TAP
add_custom_command(TARGET q3pd 
    POST_BUILD
    COMMAND echo "   Hint: to use TUN/TAP features you should set the CAP_NET_ADMIN capability on the process like"
    COMMAND echo "EXECUTE: $$ sudo setcap cap_net_admin=ep ${CMAKE_CURRENT_BINARY_DIR}/q3pd" )

# linkage
target_link_libraries(q3pd ${CMAKE_REQUIRED_LIBRARIES})

# install
install(TARGETS q3pd RUNTIME DESTINATION bin COMPONENT node)


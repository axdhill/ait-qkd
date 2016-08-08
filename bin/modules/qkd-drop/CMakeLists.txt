# ------------------------------------------------------------
# CMakeLists.txt 
# 
# make: qkd/bin/modules/qkd-drop
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
# modules/qkd-drop

# additional includes
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# sources
set(QKD_DROP_SRC
    main.cpp
    qkd-drop.cpp
)

# Qt MOC
QT5_GENERATE_MOC(qkd-drop.h ${CMAKE_CURRENT_BINARY_DIR}/qkd-drop.moc.cpp) 
set(QKD_DROP_MOC
    qkd-drop.moc.cpp
)

# DBus Integration
QT5_GENERATE_DBUS_INTERFACE(qkd-drop.h ${CMAKE_CURRENT_BINARY_DIR}/qkd-drop.xml)
QT5_ADD_DBUS_ADAPTOR(QKD_DROP_DBUS_SRC
    ${CMAKE_CURRENT_BINARY_DIR}/qkd-drop.xml 
    ${CMAKE_CURRENT_SOURCE_DIR}/qkd-drop.h 
    qkd_drop
    qkd_drop_dbus)

# bin definition
add_executable(qkd-drop ${QKD_DROP_SRC} ${QKD_DROP_MOC} ${QKD_DROP_DBUS_SRC})
add_dependencies(qkd-drop qkd)

# linkage
target_link_libraries(qkd-drop qkd ${CMAKE_REQUIRED_LIBRARIES})

# install
install(TARGETS qkd-drop RUNTIME DESTINATION bin COMPONENT modules)


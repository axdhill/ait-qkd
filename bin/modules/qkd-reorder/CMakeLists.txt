# ------------------------------------------------------------
# CMakeLists.txt 
# 
# make: qkd/bin/modules/qkd-reorder
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
# modules/qkd-reorder

# additional includes
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# sources
set(QKD_REORDER_SRC
    main.cpp
    qkd-reorder.cpp
)

# Qt MOC
QT5_GENERATE_MOC(qkd-reorder.h ${CMAKE_CURRENT_BINARY_DIR}/qkd-reorder.moc.cpp) 
set(QKD_REORDER_MOC
    qkd-reorder.moc.cpp
)

# DBus Integration
QT5_GENERATE_DBUS_INTERFACE(qkd-reorder.h ${CMAKE_CURRENT_BINARY_DIR}/qkd-reorder.xml)
QT5_ADD_DBUS_ADAPTOR(QKD_REORDER_DBUS_SRC
    ${CMAKE_CURRENT_BINARY_DIR}/qkd-reorder.xml 
    ${CMAKE_CURRENT_SOURCE_DIR}/qkd-reorder.h 
    qkd_reorder
    qkd_reorder_dbus)

# bin definition
add_executable(qkd-reorder ${QKD_REORDER_SRC} ${QKD_REORDER_MOC} ${QKD_REORDER_DBUS_SRC})
add_dependencies(qkd-reorder qkd)

# linkage
target_link_libraries(qkd-reorder qkd ${CMAKE_REQUIRED_LIBRARIES})

# install
install(TARGETS qkd-reorder RUNTIME DESTINATION bin COMPONENT modules)


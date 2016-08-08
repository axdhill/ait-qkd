# ------------------------------------------------------------
# CMakeLists.txt 
# 
# make: qkd/bin/modules/qkd-throttle
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
# modules/qkd-throttle

# additional includes
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# sources
set(QKD_THROTTLE_SRC
    main.cpp
    qkd-throttle.cpp
)

# Qt MOC
QT5_GENERATE_MOC(qkd-throttle.h ${CMAKE_CURRENT_BINARY_DIR}/qkd-throttle.moc.cpp) 
set(QKD_THROTTLE_MOC
    qkd-throttle.moc.cpp
)

# DBus Integration
QT5_GENERATE_DBUS_INTERFACE(qkd-throttle.h ${CMAKE_CURRENT_BINARY_DIR}/qkd-throttle.xml)
QT5_ADD_DBUS_ADAPTOR(QKD_THROTTLE_DBUS_SRC
    ${CMAKE_CURRENT_BINARY_DIR}/qkd-throttle.xml 
    ${CMAKE_CURRENT_SOURCE_DIR}/qkd-throttle.h 
    qkd_throttle
    qkd_throttle_dbus)

# bin definition
add_executable(qkd-throttle ${QKD_THROTTLE_SRC} ${QKD_THROTTLE_MOC} ${QKD_THROTTLE_DBUS_SRC})
add_dependencies(qkd-throttle qkd)

# linkage
target_link_libraries(qkd-throttle qkd ${CMAKE_REQUIRED_LIBRARIES})

# install
install(TARGETS qkd-throttle RUNTIME DESTINATION bin COMPONENT modules)


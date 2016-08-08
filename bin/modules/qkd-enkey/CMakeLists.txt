# ------------------------------------------------------------
# CMakeLists.txt 
# 
# make: qkd/bin/modules/qkd-enkey
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
# modules/qkd-enkey

# additional includes
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# sources
set(QKD_ENKEY_SRC
    main.cpp
    qkd-enkey.cpp
)

# Qt MOC
QT5_GENERATE_MOC(qkd-enkey.h ${CMAKE_CURRENT_BINARY_DIR}/qkd-enkey.moc.cpp) 
set(QKD_ENKEY_MOC
    qkd-enkey.moc.cpp
)

# DBus Integration
QT5_GENERATE_DBUS_INTERFACE(qkd-enkey.h ${CMAKE_CURRENT_BINARY_DIR}/qkd-enkey.xml)
QT5_ADD_DBUS_ADAPTOR(QKD_ENKEY_DBUS_SRC
    ${CMAKE_CURRENT_BINARY_DIR}/qkd-enkey.xml 
    ${CMAKE_CURRENT_SOURCE_DIR}/qkd-enkey.h 
    qkd_enkey
    qkd_enkey_dbus)

# bin definition
add_executable(qkd-enkey ${QKD_ENKEY_SRC} ${QKD_ENKEY_MOC} ${QKD_ENKEY_DBUS_SRC})
add_dependencies(qkd-enkey qkd)

# linkage
target_link_libraries(qkd-enkey qkd ${CMAKE_REQUIRED_LIBRARIES})

# install
install(TARGETS qkd-enkey RUNTIME DESTINATION bin COMPONENT modules)


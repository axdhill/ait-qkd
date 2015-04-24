# ------------------------------------------------------------
# CMakeLists.txt 
# 
# make: qkd/bin/modules/qkd-auth
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
# qkd-auth

# additional includes
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# sources
set(QKD_AUTH_SRC
    main.cpp
    qkd-auth.cpp
)

# Qt MOC
QT4_GENERATE_MOC(qkd-auth.h ${CMAKE_CURRENT_BINARY_DIR}/qkd-auth.moc.cpp) 
set(QKD_AUTH_MOC
    qkd-auth.moc.cpp
)

# DBus Integration
QT4_GENERATE_DBUS_INTERFACE(qkd-auth.h ${CMAKE_CURRENT_BINARY_DIR}/qkd-auth.xml)
QT4_ADD_DBUS_ADAPTOR(QKD_AUTH_DBUS_SRC
    ${CMAKE_CURRENT_BINARY_DIR}/qkd-auth.xml 
    ${CMAKE_CURRENT_SOURCE_DIR}/qkd-auth.h 
    qkd_auth
    qkd_auth_dbus)

# bin definition
add_executable(qkd-auth ${QKD_AUTH_SRC} ${QKD_AUTH_MOC} ${QKD_AUTH_DBUS_SRC})

# linkage
target_link_libraries(qkd-auth ${CMAKE_REQUIRED_LIBRARIES})

# install
install(TARGETS qkd-auth RUNTIME DESTINATION bin)


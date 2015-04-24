# ------------------------------------------------------------
# CMakeLists.txt 
# 
# make: qkd/bin/modules/qkd-dekey
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
# modules/qkd-dekey

# additional includes
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# sources
set(QKD_DEKEY_SRC
    main.cpp
    qkd-dekey.cpp
)

# Qt MOC
QT4_GENERATE_MOC(qkd-dekey.h ${CMAKE_CURRENT_BINARY_DIR}/qkd-dekey.moc.cpp) 
set(QKD_DEKEY_MOC
    qkd-dekey.moc.cpp
)

# DBus Integration
QT4_GENERATE_DBUS_INTERFACE(qkd-dekey.h ${CMAKE_CURRENT_BINARY_DIR}/qkd-dekey.xml)
QT4_ADD_DBUS_ADAPTOR(QKD_DEKEY_DBUS_SRC
    ${CMAKE_CURRENT_BINARY_DIR}/qkd-dekey.xml 
    ${CMAKE_CURRENT_SOURCE_DIR}/qkd-dekey.h 
    qkd_dekey
    qkd_dekey_dbus)

# bin definition
add_executable(qkd-dekey ${QKD_DEKEY_SRC} ${QKD_DEKEY_MOC} ${QKD_DEKEY_DBUS_SRC})

# linkage
target_link_libraries(qkd-dekey ${CMAKE_REQUIRED_LIBRARIES})

# install
install(TARGETS qkd-dekey RUNTIME DESTINATION bin)



# ------------------------------------------------------------
# CMakeLists.txt 
# 
# make: qkd/bin/modules/qkd-sifting-bb84
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
# qkd-sifting-bb84

# additional includes
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# sources
set(QKD_SIFTING_BB84_SRC
    bb84_base.cpp
    main.cpp
    qauth.cpp
    qkd-sifting-bb84.cpp
)

# Qt MOC
QT4_GENERATE_MOC(qkd-sifting-bb84.h ${CMAKE_CURRENT_BINARY_DIR}/qkd-sifting-bb84.moc.cpp) 
set(QKD_SIFTING_BB84_MOC
    qkd-sifting-bb84.moc.cpp
)

# DBus Integration
QT4_GENERATE_DBUS_INTERFACE(qkd-sifting-bb84.h ${CMAKE_CURRENT_BINARY_DIR}/qkd-sifting-bb84.xml)
QT4_ADD_DBUS_ADAPTOR(QKD_SIFTING_BB84_DBUS_SRC
    ${CMAKE_CURRENT_BINARY_DIR}/qkd-sifting-bb84.xml 
    ${CMAKE_CURRENT_SOURCE_DIR}/qkd-sifting-bb84.h 
    qkd_sifting_bb84
    qkd_sifting_bb84_dbus)

# bin definition
add_executable(qkd-sifting-bb84 ${QKD_SIFTING_BB84_SRC} ${QKD_SIFTING_BB84_MOC} ${QKD_SIFTING_BB84_DBUS_SRC})

# linkage
target_link_libraries(qkd-sifting-bb84 ${CMAKE_REQUIRED_LIBRARIES})

# install
install(TARGETS qkd-sifting-bb84 RUNTIME DESTINATION bin COMPONENT modules)


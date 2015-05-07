# ------------------------------------------------------------
# CMakeLists.txt 
# 
# make: qkd/bin/modules/qkd-cat
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
# modules/qkd-cat

# additional includes
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# sources
set(QKD_CAT_SRC
    main.cpp
    qkd-cat.cpp
)

# Qt MOC
QT4_GENERATE_MOC(qkd-cat.h ${CMAKE_CURRENT_BINARY_DIR}/qkd-cat.moc.cpp) 
set(QKD_CAT_MOC
    qkd-cat.moc.cpp
)

# DBus Integration
QT4_GENERATE_DBUS_INTERFACE(qkd-cat.h ${CMAKE_CURRENT_BINARY_DIR}/qkd-cat.xml)
QT4_ADD_DBUS_ADAPTOR(QKD_CAT_DBUS_SRC
    ${CMAKE_CURRENT_BINARY_DIR}/qkd-cat.xml 
    ${CMAKE_CURRENT_SOURCE_DIR}/qkd-cat.h 
    qkd_cat
    qkd_cat_dbus)

# bin definition
add_executable(qkd-cat ${QKD_CAT_SRC} ${QKD_CAT_MOC} ${QKD_CAT_DBUS_SRC})

# linkage
target_link_libraries(qkd-cat ${CMAKE_REQUIRED_LIBRARIES})

# install
install(TARGETS qkd-cat RUNTIME DESTINATION bin COMPONENT modules)


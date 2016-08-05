# ------------------------------------------------------------
# CMakeLists.txt 
# 
# make: qkd/bin/modules/qkd-statistics
#
# Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
#
# Copyright (C) 2015-2016 AIT Austrian Institute of Technology
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
# modules/qkd-statistics

# additional includes
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# sources
set(QKD_STATISTICS_SRC
    main.cpp
    qkd-statistics.cpp
)

# Qt MOC
QT5_GENERATE_MOC(qkd-statistics.h ${CMAKE_CURRENT_BINARY_DIR}/qkd-statistics.moc.cpp) 
set(QKD_STATISTICS_MOC
    qkd-statistics.moc.cpp
)

# DBus Integration
QT5_GENERATE_DBUS_INTERFACE(qkd-statistics.h ${CMAKE_CURRENT_BINARY_DIR}/qkd-statistics.xml)
QT5_ADD_DBUS_ADAPTOR(QKD_STATISTICS_DBUS_SRC
    ${CMAKE_CURRENT_BINARY_DIR}/qkd-statistics.xml 
    ${CMAKE_CURRENT_SOURCE_DIR}/qkd-statistics.h 
    qkd_statistics
    qkd_statistics_dbus)

# bin definition
add_executable(qkd-statistics ${QKD_STATISTICS_SRC} ${QKD_STATISTICS_MOC} ${QKD_STATISTICS_DBUS_SRC})

# linkage
target_link_libraries(qkd-statistics ${CMAKE_REQUIRED_LIBRARIES})

# install
install(TARGETS qkd-statistics RUNTIME DESTINATION bin COMPONENT modules)


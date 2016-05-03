# ------------------------------------------------------------
# CMakeLists.txt 
# 
# make: qkd/bin/modules/qkd-sync
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
# qkd-sync

# additional includes
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# sources
set(QKD_RESIZE_SRC
    main.cpp
    qkd-sync.cpp
)

# Qt MOC
QT4_GENERATE_MOC(qkd-sync.h ${CMAKE_CURRENT_BINARY_DIR}/qkd-sync.moc.cpp) 
set(QKD_RESIZE_MOC
    qkd-sync.moc.cpp
)

# bin definition
add_executable(qkd-sync ${QKD_RESIZE_SRC} ${QKD_RESIZE_MOC} ${QKD_RESIZE_DBUS_SRC})

# linkage
target_link_libraries(qkd-sync ${CMAKE_REQUIRED_LIBRARIES})

# install
install(TARGETS qkd-sync RUNTIME DESTINATION bin COMPONENT modules)


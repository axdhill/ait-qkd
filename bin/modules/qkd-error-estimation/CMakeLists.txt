# ------------------------------------------------------------
# CMakeLists.txt 
# 
# make: qkd/bin/modules/qkd-error-estimation
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
# modules/qkd-error-estimation

# additional includes
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# sources
set(QKD_ERROR_ESTIMATION_SRC
    main.cpp
    qkd-error-estimation.cpp
)

# Qt MOC
QT4_GENERATE_MOC(qkd-error-estimation.h ${CMAKE_CURRENT_BINARY_DIR}/qkd-error-estimation.moc.cpp) 
set(QKD_ERROR_ESTIMATION_MOC
    qkd-error-estimation.moc.cpp
)

# DBus Integration
QT4_GENERATE_DBUS_INTERFACE(qkd-error-estimation.h ${CMAKE_CURRENT_BINARY_DIR}/qkd-error-estimation.xml)
QT4_ADD_DBUS_ADAPTOR(QKD_ERROR_ESTIMATION_DBUS_SRC
    ${CMAKE_CURRENT_BINARY_DIR}/qkd-error-estimation.xml 
    ${CMAKE_CURRENT_SOURCE_DIR}/qkd-error-estimation.h 
    qkd_error_estimation
    qkd_error_estimation_dbus)

# bin definition
add_executable(qkd-error-estimation ${QKD_ERROR_ESTIMATION_SRC} ${QKD_ERROR_ESTIMATION_MOC} ${QKD_ERROR_ESTIMATION_DBUS_SRC})

# linkage
target_link_libraries(qkd-error-estimation ${CMAKE_REQUIRED_LIBRARIES})

# install
install(TARGETS qkd-error-estimation RUNTIME DESTINATION bin COMPONENT modules)


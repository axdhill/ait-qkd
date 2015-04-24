# ------------------------------------------------------------
# CMakeLists.txt 
# 
# make: qkd/bin/modules/qkd-privacy-amplification
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
# modules/qkd-privacy-amplification

# additional includes
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# sources
set(QKD_PRIVACY_AMPLIFICATION_SRC
    main.cpp
    ntt.cpp
    qkd-privacy-amplification.cpp
)

# Qt MOC
QT4_GENERATE_MOC(qkd-privacy-amplification.h ${CMAKE_CURRENT_BINARY_DIR}/qkd-privacy-amplification.moc.cpp) 
set(QKD_PRIVACY_AMPLIFICATION_MOC
    qkd-privacy-amplification.moc.cpp
)

# DBus Integration
QT4_GENERATE_DBUS_INTERFACE(qkd-privacy-amplification.h ${CMAKE_CURRENT_BINARY_DIR}/qkd-privacy-amplification.xml)
QT4_ADD_DBUS_ADAPTOR(QKD_PRIVACY_AMPLIFICATION_DBUS_SRC
    ${CMAKE_CURRENT_BINARY_DIR}/qkd-privacy-amplification.xml 
    ${CMAKE_CURRENT_SOURCE_DIR}/qkd-privacy-amplification.h 
    qkd_privacy_amplification
    qkd_privacy_amplification_dbus)

# bin definition
add_executable(qkd-privacy-amplification ${QKD_PRIVACY_AMPLIFICATION_SRC} ${QKD_PRIVACY_AMPLIFICATION_MOC} ${QKD_PRIVACY_AMPLIFICATION_DBUS_SRC})

# linkage
target_link_libraries(qkd-privacy-amplification ${CMAKE_REQUIRED_LIBRARIES})

# install
install(TARGETS qkd-privacy-amplification RUNTIME DESTINATION bin)

# ------------------------------------------------------------
# CMakeLists.txt 
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
#
#       -----------------------------------------------------
#       Please subsitute the MY_MODULE_* placements as needed
#       -----------------------------------------------------
#
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


# additional includes
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

# Qt MOC
QT5_GENERATE_MOC(my_module.h ${CMAKE_CURRENT_BINARY_DIR}/my_module.moc.cpp) 
set(MY_MODULE_MOC
    my_module.moc.cpp
)

# DBus Integration
QT5_GENERATE_DBUS_INTERFACE(my_module.h ${CMAKE_CURRENT_BINARY_DIR}/my_module.xml)
QT5_ADD_DBUS_ADAPTOR(MY_MODULE_DBUS_SRC
    ${CMAKE_CURRENT_BINARY_DIR}/my_module.xml 
    ${CMAKE_CURRENT_SOURCE_DIR}/my_module.h 
    my_module
    my_module_dbus)


# sources
set(MY_MODULE_SRC
    main.cpp
    my_module.cpp
)

# bin definition
add_executable(my_module_exe ${MY_MODULE_SRC} ${MY_MODULE_MOC} ${MY_MODULE_DBUS_SRC})

# linkage
target_link_libraries(my_module_exe qkd Qt5::DBus Qt5::Network Qt5::Widgets Qt5::Gui Qt5::Core ${CMAKE_REQUIRED_LIBRARIES})

# install
install(TARGETS my_module_exe RUNTIME DESTINATION bin)

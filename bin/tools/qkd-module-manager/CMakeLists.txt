# ------------------------------------------------------------
# CMakeLists.txt 
# 
# make: qkd-module-manager
#
# Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
#
# Copyright (C) 2013-2015 AIT Austrian Institute of Technology
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
# qkd-module-manager

# additional includes
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# sources
set(QKD_MODULE_MANAGER_SRC

    main.cpp
    main_widget.cpp
    main_window.cpp
)


# Qt UIC
QT4_WRAP_UI(${CMAKE_CURRENT_BINARY_DIR}/ui_main_widget.h main_widget.ui)
set(QKD_MODULE_MANAGER_UIC
    ui_main_widget.h
)

# Qt MOC
QT4_GENERATE_MOC(main_widget.h              ${CMAKE_CURRENT_BINARY_DIR}/module_manager_main_widget.moc.cpp) 
QT4_GENERATE_MOC(main_window.h              ${CMAKE_CURRENT_BINARY_DIR}/module_manager_main_window.moc.cpp)
set(QKD_MODULE_MANAGER_MOC
    module_manager_main_widget.moc.cpp
    module_manager_main_window.moc.cpp
)

# bin definition
add_executable(qkd-module-manager ${QKD_MODULE_MANAGER_SRC} ${QKD_MODULE_MANAGER_MOC} ${QKD_MODULE_MANAGER_UIC})

# linkage
target_link_libraries(qkd-module-manager ${CMAKE_REQUIRED_LIBRARIES})

# copy share/qkd-module-manager stuff for convenient testing
configure_file(${CMAKE_SOURCE_DIR}/share/qkd-module-manager/ait_logo.png                  ${CMAKE_BINARY_DIR}/bin/tools/share/qkd-module-manager/ait_logo.png               COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/share/qkd-module-manager/ait_logo.gif                  ${CMAKE_BINARY_DIR}/bin/tools/share/qkd-module-manager/ait_logo.gif               COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/share/qkd-module-manager/ait_logo_no_claim.jpg         ${CMAKE_BINARY_DIR}/bin/tools/share/qkd-module-manager/ait_logo_no_claim.jpg      COPYONLY)

# install
install(TARGETS qkd-module-manager RUNTIME DESTINATION bin COMPONENT tools)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/share/qkd-module-manager DESTINATION share COMPONENT tools)


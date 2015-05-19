# ------------------------------------------------------------
# CMakeLists.txt 
# 
# make: examples
#
# Autor: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
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


# install the examples
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/module-1 DESTINATION share/doc/qkd/examples COMPONENT examples)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/module-2 DESTINATION share/doc/qkd/examples COMPONENT examples)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/module-3 DESTINATION share/doc/qkd/examples COMPONENT examples)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/module-4 DESTINATION share/doc/qkd/examples COMPONENT examples)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/module-5 DESTINATION share/doc/qkd/examples COMPONENT examples)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/README DESTINATION share/doc/qkd/examples COMPONENT examples)


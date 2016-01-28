# ------------------------------------------------------------
# CMakeLists.txt 
# 
# Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
#
# Copyright (C) 2013-2016 AIT Austrian Institute of Technology
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
# basic

# add libqkd to the libs
set(CMAKE_REQUIRED_LIBRARIES "qkd;${CMAKE_REQUIRED_LIBRARIES}")


# ------------------------------------------------------------

# sources
set(MY_MODULE_SRC
    main.cpp
    my_module.cpp
)

# bin definition
add_executable(module-4 ${MY_MODULE_SRC})

# linkage
target_link_libraries(module-4 ${CMAKE_REQUIRED_LIBRARIES})

# install
install(TARGETS module-4 RUNTIME DESTINATION bin)

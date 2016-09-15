# ------------------------------------------------------------
# CMakeLists.txt 
# 
# make: qkd/bin/tools/qkd-key-gen-cv
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
# tools/qkd-key-gen

# sources
set(QKD_KEY_GEN_CV_SRC
    entangled.cpp
    main.cpp
    mode.cpp
    prepare_measure_heterodyne.cpp
    prepare_measure_homodyne.cpp
)

# bin definition
add_executable(qkd-key-gen-cv ${QKD_KEY_GEN_CV_SRC})
add_dependencies(qkd-key-gen-cv qkd)

# linkage
target_link_libraries(qkd-key-gen-cv qkd ${CMAKE_REQUIRED_LIBRARIES})

# install
install(TARGETS qkd-key-gen-cv RUNTIME DESTINATION bin COMPONENT tools)

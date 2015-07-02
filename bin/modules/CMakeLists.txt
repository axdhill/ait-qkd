# ------------------------------------------------------------
# CMakeLists.txt 
# 
# make: qkd/bin/modules
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
# go through the subs

add_subdirectory(qkd-auth)
add_subdirectory(qkd-buffer)
add_subdirectory(qkd-cascade)
add_subdirectory(qkd-cat)
add_subdirectory(qkd-confirmation)
add_subdirectory(qkd-debug)
add_subdirectory(qkd-dekey)
add_subdirectory(qkd-drop)
add_subdirectory(qkd-enkey)
add_subdirectory(qkd-error-estimation)
add_subdirectory(qkd-ping)
add_subdirectory(qkd-privacy-amplification)
add_subdirectory(qkd-reorder)
add_subdirectory(qkd-sifting-bb84)
add_subdirectory(qkd-statistics)
add_subdirectory(qkd-tee)
add_subdirectory(qkd-throttle)

# ------------------------------------------------------------
# CMakeLists.txt 
# 
# make: qkd/bin/tools
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

add_subdirectory(q3p-keystore-dump)
add_subdirectory(q3p-mq-reader)
add_subdirectory(qkd-blob-keystream)
add_subdirectory(qkd-key-dump)
add_subdirectory(qkd-key-gen)
add_subdirectory(qkd-pipeline)
add_subdirectory(qkd-module-manager)
add_subdirectory(qkd-statistics-average)
add_subdirectory(qkd-simulate)
add_subdirectory(qkd-view)

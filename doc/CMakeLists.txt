# ------------------------------------------------------------
# CMakeLists.txt 
# 
# make: qkd/doc
#
# Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
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


# handbook PDF creation is an option
if (DOC_HANDBOOK)
    message(STATUS "enabled AIT QKD Software Handbook PDF creation")
    add_subdirectory(handbook)
else(DOC_HANDBOOK)
    message(STATUS "disabled AIT QKD Software Handbook PDF creation")
endif(DOC_HANDBOOK)

# simulator PDF creation is an option
if (DOC_SIMULATOR)
    message(STATUS "enabled AIT QKD Simulator Documentation PDF creation")
    add_subdirectory(simulator)
else(DOC_SIMULATOR)
    message(STATUS "disabled AIT QKD Simulator Documentation PDF creation")
endif(DOC_SIMULATOR)

# ------------------------------------------------------------
# CMakeLists.txt 
# 
# make: qkd/etc
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
# etc - system configuration files for QKD

# install

install(FILES dbus-1/qkd.conf DESTINATION /etc/dbus-1 COMPONENT configuration)
install(FILES default/qkd-dbus DESTINATION /etc/default COMPONENT configuration)
install(FILES init.d/qkd-dbus DESTINATION /etc/init.d COMPONENT configuration 
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                GROUP_READ GROUP_EXECUTE
                WORLD_READ WORLD_EXECUTE
)
install(FILES profile.d/qkd_dbus_address.sh DESTINATION /etc/profile.d COMPONENT configuration)
install(FILES q3p/sample-q3pd.conf DESTINATION /etc/q3p COMPONENT configuration)
install(FILES qkd/sample-pipeline-alice.xml DESTINATION /etc/qkd COMPONENT configuration)
install(FILES qkd/sample-pipeline-auth-final.conf DESTINATION /etc/qkd COMPONENT configuration)
install(FILES qkd/sample-pipeline-auth-starting.conf DESTINATION /etc/qkd COMPONENT configuration)
install(FILES qkd/sample-pipeline-bob.xml DESTINATION /etc/qkd COMPONENT configuration)
install(FILES qkd/sample-pipeline-default.conf DESTINATION /etc/qkd COMPONENT configuration)
install(FILES systemd/system/qkd-dbus.service DESTINATION /etc/systemd COMPONENT configuration)
install(FILES systemd/system/qkd-dbus.socket DESTINATION /etc/systemd COMPONENT configuration)


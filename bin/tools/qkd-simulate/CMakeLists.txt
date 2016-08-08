# ------------------------------------------------------------
# CMakeLists.txt 
# 
# make: qkd-simulate
#
# Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
#         Philipp Grabenweger
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
# qkd-simulate

# additional includes
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# sources
set(QKD_SIMULATE_SRC

    about_dialog.cpp
    default_values.cpp
    main.cpp
    main_widget.cpp
    main_window.cpp
    
    channel/channel.cpp
    channel/channel_bb84.cpp
    channel/channel_event_handler.cpp
    channel/channel_event_manager.cpp
    channel/detector.cpp
    channel/event.cpp
    channel/fiber.cpp
    channel/measurement.cpp
    channel/measurement_bb84.cpp
    channel/photon_pair_manager.cpp
    channel/random_functions.cpp
    channel/source.cpp
    channel/ttm.cpp
    
    channel/detector/detection_element.cpp
    channel/detector/detector_optics.cpp
    channel/detector/event_buffer.cpp
    channel/detector/sync_pulse_generator.cpp
    channel/detector/sync_pulse_receiver.cpp
    channel/detector/window_generator.cpp
    
    channel/fiber/delay_line.cpp
    channel/fiber/fiber_quantum.cpp
    channel/fiber/fiber_sync.cpp
    channel/fiber/noise_photon_source.cpp
)


# Qt UIC
QT5_WRAP_UI(${CMAKE_CURRENT_BINARY_DIR}/ui_main_widget.h main_widget.ui)
set(QKD_SIMULATE_UIC
    ui_main_widget.h
)

# Qt MOC
QT5_GENERATE_MOC(about_dialog.h             ${CMAKE_CURRENT_BINARY_DIR}/simulate_about_dialog.moc.cpp) 
QT5_GENERATE_MOC(main_widget.h              ${CMAKE_CURRENT_BINARY_DIR}/simulate_main_widget.moc.cpp) 
QT5_GENERATE_MOC(main_window.h              ${CMAKE_CURRENT_BINARY_DIR}/simulate_main_window.moc.cpp)
set(QKD_SIMULATE_MOC
    simulate_about_dialog.moc.cpp
    simulate_main_widget.moc.cpp
    simulate_main_window.moc.cpp
)

# bin definition
add_executable(qkd-simulate ${QKD_SIMULATE_SRC} ${QKD_SIMULATE_MOC} ${QKD_SIMULATE_UIC})
add_dependencies(qkd-simulate qkd)

# linkage
target_link_libraries(qkd-simulate Qt5::Xml Qt5::Widgets qkd ${CMAKE_REQUIRED_LIBRARIES})

# copy share/qkd-simulate stuff for convenient testing
configure_file(${CMAKE_SOURCE_DIR}/share/qkd-simulate/ait_logo.png                      ${CMAKE_BINARY_DIR}/bin/tools/share/qkd-simulate/ait_logo.png               COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/share/qkd-simulate/ait_logo.gif                      ${CMAKE_BINARY_DIR}/bin/tools/share/qkd-simulate/ait_logo.gif               COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/share/qkd-simulate/ait_logo_no_claim.jpg             ${CMAKE_BINARY_DIR}/bin/tools/share/qkd-simulate/ait_logo_no_claim.jpg      COPYONLY)

# install
install(TARGETS qkd-simulate RUNTIME DESTINATION bin COMPONENT tools)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/share/qkd-simulate DESTINATION share COMPONENT tools)


/*
 * default_values.cpp
 * 
 * implement the dafault values
 * 
 * Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *         Philipp Grabenweger
 *
 * Copyright (C) 2013-2015 AIT Austrian Institute of Technology
 * AIT Austrian Institute of Technology GmbH
 * Donau-City-Strasse 1 | 1220 Vienna | Austria
 * http://www.ait.ac.at
 *
 * This file is part of the AIT QKD Software Suite.
 *
 * The AIT QKD Software Suite is free software: you can redistribute 
 * it and/or modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation, either version 3 of 
 * the License, or (at your option) any later version.
 * 
 * The AIT QKD Software Suite is distributed in the hope that it will 
 * be useful, but WITHOUT ANY WARRANTY; without even the implied warranty 
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with the AIT QKD Software Suite. 
 * If not, see <http://www.gnu.org/licenses/>.
 */



// ------------------------------------------------------------
// incs

// ait
#include "default_values.h"


using namespace qkd::simulate;


// ------------------------------------------------------------
// vars


/**
 * the about text
 */
const QString qkd::simulate::g_sDefaultValues = "\
<qkd-simulate>\
    <source>\
        <source_photon_rate value=\"1000000.00\" />\
        <fiber_length value=\"1.00\" />\
        <fiber_absorption_coeff value=\"1.00\" />\
        <source_signal_error_probability value=\"5.00\" />\
        <sync_det_time_stnd_deviation value=\"1.0\" />\
        <multi_photon_rate value=\"0.0\" />\
        <noise_photon_rate value=\"0.0\" />\
        <simulation_end_time value=\"10000.0\" />\
    </source>\
    <alice>\
        <detection_efficiency value=\"50.00\" />\
        <dark_count_rate value=\"100.00\" />\
        <time_slot_width value=\"30.00\" />\
        <time_slot_delay value=\"0.00\" />\
        <distance_indep_loss value=\"0.00\" />\
        <det_time_delay value=\"5.00\" />\
        <det_time_stnd_deviation value=\"1.00\" />\
        <det_down_time value=\"10.00\" />\
        <table_size value=\"4096\" />\
    </alice>\
    <bob>\
        <detection_efficiency value=\"50.00\" />\
        <dark_count_rate value=\"100.00\" />\
        <time_slot_width value=\"30.00\" />\
        <time_slot_delay value=\"0.00\" />\
        <distance_indep_loss value=\"0.00\" />\
        <det_time_delay value=\"5.00\" />\
        <det_time_stnd_deviation value=\"1.00\" />\
        <det_down_time value=\"10.00\" />\
        <table_size value=\"4096\" />\
    </bob>\
    <general>\
        <multi_photon_simulation value=\"false\" />\
        <sync_pulse_simulation value=\"false\" />\
        <transmission_loss_simulation value=\"false\" />\
        <inifinte_loop_simulation value=\"false\" />\
    </general>\
    <output>\
        <free value=\"udp\">\
        <free_udp_alice value=\"127.0.0.1:3001\" />\
        <free_udp_bob value=\"127.0.0.1:3001\" />\
        <free_file_alice value=\"ttm.alice\" />\
        <free_file_bob value=\"ttm.bob\" />\
        <event value=\"pipe\">\
        <event_pipe_alice value=\"ipc:///bb84.alice.in\" />\
        <event_pipe_bob value=\"ipc:///bb84.bob.in\" />\
        <event_file_alice value=\"event.alice\" />\
        <event_file_bob value=\"event.bob\" />\
    </output>\
</qkd-simulate>\
";

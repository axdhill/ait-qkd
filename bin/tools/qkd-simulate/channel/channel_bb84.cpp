/*
 * channel_bb84.cpp
 *
 * definition of the bb84 quantum channel
 *
 * Author: Mario Kahn
 *         Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *         Philipp Grabenweger
 *
 * Copyright (C) 2013-2016 AIT Austrian Institute of Technology
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

#include <math.h>

// ait
#include "channel_bb84.h"
#include "measurement_bb84.h"
#include "detector/detection_modes.h"
#include "ttm.h"


using namespace qkd::simulate;


// -----------------------------------------------------------------
// code


/**
 * perform a measurement
 * 
 * @return  the measurement
 */
measurement channel_bb84::measure_internal() {
    
    // Creation of time tags / key pair according to following conventions:
    //
    //             Model of Quantum Cryptography system
    //
    //                          +--------------+
    //                          |  EPR Source  |
    //                          +--------------+
    //                               #   #
    //                              #   # 
    //                             #   # 
    //   +-------------------+    #   #               +-------------------+
    //   |  Alice            | ###   #   / /          |  Bob              |
    //   |                   |         ###\ \#########|                   | 
    //   | Base I            |            / /         | Base I            | 
    //   |   Det1: |H>  (0)  |                        |   Det1: |H>  (0)  |
    //   |   Det2: |V> (90)  |                        |   Det2: |V> (90)  | 
    //   |                   |                        |                   | 
    //   | Base II           |                        | Base II           | 
    //   |   Det3: |P>  (45) |                        |   Det3: |P>  (45) | 
    //   |   Det4: |M> (135) |                        |   Det4: |M> (135) |
    //
    
    // create the measurement
    measurement cMeasurement = measurement(new qkd::simulate::measurement_bb84());
    measurement_bb84 * cMeasurementBB84 = dynamic_cast<qkd::simulate::measurement_bb84 *>(cMeasurement.get());
    
    // perform the simulation
    m_cPhotonPairManager.init_simulation();
    m_cManager.init_simulation();
    m_cManager.dispatch(this);
    
    cMeasurementBB84->set_acquisition_duration(m_cManager.get_time() * (1e9 * ttm::RESOLUTION));

    if (alice()->get_detection_mode() != detection_mode::FREE_RUNNING) {   

        cMeasurementBB84->set_free_running(false);
        
        // get next key id
        m_nKeyId = qkd::key::key::counter().inc();
        
        // setup final key pair
        cMeasurement->key_alice() = qkd::key::key(m_nKeyId, qkd::utility::memory(alice()->event_table_size()));
        cMeasurement->key_alice().meta().eKeyState = qkd::key::key_state::KEY_STATE_RAW;
        cMeasurement->key_bob() = qkd::key::key(m_nKeyId, qkd::utility::memory(bob()->event_table_size()));
        cMeasurement->key_bob().meta().eKeyState = qkd::key::key_state::KEY_STATE_RAW;

        unsigned char * buffer_alice = alice()->get_buffer();
        unsigned char * buffer_bob = bob()->get_buffer();
        
        for (uint64_t i = 0; i < alice()->event_table_size(); i++) {
            cMeasurement->key_alice().data()[i] = buffer_alice[i];
        }
        for (uint64_t i = 0; i < bob()->event_table_size(); i++) {
            cMeasurement->key_bob().data()[i] = buffer_bob[i];
        }
    }
    else {
        cMeasurementBB84->set_free_running(true);
    }
    
    return cMeasurement;
}


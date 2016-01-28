/*
 * sync_pulse_generator.cpp
 * 
 * Implementation of a sync pulse generator to be used inside detector at Alice side
 *
 * Author: Philipp Grabenweger
 *         Oliver Maurhart, <oliver.maurhart@ait.ac.at>
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

#include <fstream>

// ait
#include "sync_pulse_generator.h"
#include "../channel_event_manager.h"
#include "../photon_pair.h"


using namespace qkd::simulate;


// -----------------------------------------------------------------
// code


/**
 * handle a channel event. 
 * 
 * @param   cEvent      the channel event to be handled
 */
void sync_pulse_generator::handle(event const & cEvent) {

    switch (cEvent.eType) {
        
    case event::type::DETECTOR_PULSE: 
    
        { 
            // detector pulse received from some detection element
            unsigned int det_ind = 0;
            event cEventNew;
            
            switch (cEvent.cData.m_ePhotonState) {
                
            case photon_state::HORIZONTAL:
                det_ind = 0;
                break;
                
            case photon_state::VERTICAL:
                det_ind = 1;
                break;
                
            case photon_state::PLUS:
                det_ind = 2;
                break;
                
            case photon_state::MINUS:
                det_ind = 3;
                break;
                
            default:
                break;
            }
            
            if (m_bDetReady && m_bWindowGeneratorReady) {
                
                cEventNew.ePriority = event::priority::HIGH;
                cEventNew.eType = event::type::SYNC_PULSE;
                cEventNew.cDestination = parent();
                cEventNew.cSource = this;
                cEventNew.nTime = manager()->get_time();
                
                // generate sync pulse event
                manager()->add_event(cEventNew); 
                
                m_bWindowGeneratorReady = false;
                
                // if detection element causing the sync pulse is now in down state and sync_initiator_ready detection mode is set
                if (m_eDetectionMode == detection_mode::SYNC_INITIATOR_READY && cEvent.cData.m_bDown) { 
                    m_nSyncInitiator = det_ind;
                    m_bDetReady = false;
                }
            }
            
            if (cEvent.cData.m_bDown) {
                m_bDown[det_ind] = true;
            }
            
            // if detection element causing the event is now in down state and sync_all_ready detection mode is set
            if (m_eDetectionMode == detection_mode::SYNC_ALL_READY && cEvent.cData.m_bDown) {
                m_bDetReady = false;
            }
        }
        break;
        
    case event::type::DOWN_END: 
    
        { 
            // end of some detection element's down time
            unsigned int det_ind = 0;
            
            switch (cEvent.cData.m_ePhotonState) {
                
            case photon_state::HORIZONTAL:
                det_ind = 0;
                break;
            
            case photon_state::VERTICAL:
                det_ind = 1;
                break;
            
            case photon_state::PLUS:
                det_ind = 2;
                break;
            
            case photon_state::MINUS:
                det_ind = 3;
                break;
            
            default:
                break;
            }
            
            m_bDown[det_ind] = false;
            
            if (m_eDetectionMode == detection_mode::SYNC_INITIATOR_READY && (!m_bDetReady) && (det_ind == m_nSyncInitiator)) {
                m_bDetReady = true;
            }
            
            if (m_eDetectionMode == detection_mode::SYNC_ALL_READY) {
                m_bDetReady = (!m_bDown[0]) && (!m_bDown[1]) && (!m_bDown[2]) && (!m_bDown[3]);
            }
        }
        break;
    
    case event::type::INIT:
        
        // simulation initialization
        m_bDetReady = true;
        m_bDown[0] = false;
        m_bDown[1] = false;
        m_bDown[2] = false;
        m_bDown[3] = false;
        m_bWindowGeneratorReady = true;
    
        break;
        
    case event::type::WINDOW_END: 
        // end of window generated by window generator
        m_bWindowGeneratorReady = true;
        break;
    
    default:
        break;
    }
}


/**
 * write out all parameters of this event handler and its child event handlers
 * 
 * @param   cStream     the stream to write to
 */
void sync_pulse_generator::write_parameters(std::ofstream & cStream) {

    cStream << "NAME: " << get_name() << std::endl;
    cStream << "m_eDetectionMode: " << static_cast<int>(m_eDetectionMode) << std::endl;
    cStream << "m_bDetReady: " << m_bDetReady << std::endl;
    cStream << "m_down: " << m_bDown[0] << m_bDown[1] << m_bDown[2] << m_bDown[3] << std::endl;
    cStream << "m_nSyncInitiator: " << m_nSyncInitiator << std::endl;
    cStream << "m_bWindowGeneratorReady: " << m_bWindowGeneratorReady << std::endl;
    cStream << std::endl;
}

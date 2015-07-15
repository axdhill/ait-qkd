/*
 * sync_pulse_receiver.cpp
 * 
 * Implementation of a sync pulse receiver to be used inside detector at Bob side
 *
 * Author: Philipp Grabenweger
 *         Oliver Maurhart, <oliver.maurhart@ait.ac.at>
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

#include <fstream>

// ait
#include "sync_pulse_receiver.h"
#include "../channel_event_manager.h"
#include "../random_functions.h"
#include "../ttm.h"
#include "../photon_pair.h"


using namespace qkd::simulate;


// -----------------------------------------------------------------
// code


/**
 * handle a channel event. 
 * 
 * @param   cEvent      the channel event to be handled
 */
void sync_pulse_receiver::handle(event const & cEvent) {

    switch (cEvent.eType) {
        
    case event::type::DETECTOR_PULSE: 
        
        // detector pulse coming from some detection element
        if (cEvent.cData.m_bDown) {
            
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
            
            m_bDown[det_ind] = true;
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
        }
        break;
        
    case event::type::INIT: 
        
        // simulation initialization
        m_bDown[0] = false;
        m_bDown[1] = false;
        m_bDown[2] = false;
        m_bDown[3] = false;
    
        break;
        
    case event::type::SYNC_PULSE:
        
        if (cEvent.cSource == parent()) { // sync pulse received from sync transmission fiber
            
            event cEventNew;
            double td;
            
            do {
                // disallow acausal sync detection times 
                // (allowing them would probably spoil the logic of simulation and cause serious inconsistency problems)
                td = m_nDelay + random_functions::random_gaussian(0.0, m_nJitter);
            } while (td < 0.0); 
            
            cEventNew.ePriority = event::priority::NORMAL;
            cEventNew.eType = event::type::SYNC_PULSE;
            cEventNew.cDestination = this;
            cEventNew.cSource = this;
            cEventNew.nTime = manager()->get_time() + static_cast<int64_t>(td * 1e-9 / ttm::RESOLUTION);
            
            // add sync detection event after delay and jitter time
            manager()->add_event(cEventNew); 
        }
        else 
        if (cEvent.cSource == this) { 
            
            // sync pulse detection event received from this component
            event cEventNew;
            cEventNew.ePriority = event::priority::NORMAL;
            if (m_eDetectionMode == detection_mode::SYNC_ALL_READY && (m_bDown[0] || m_bDown[1] || m_bDown[2] || m_bDown[3])) {
                // in sync_all_ready detection mode, a sync pulse coming while some detection elements are down is a bad sync pulse
                cEventNew.eType = event::type::SYNC_PULSE_BAD; 
            }
            else {
                cEventNew.eType = event::type::SYNC_PULSE;
            }
            cEventNew.cDestination = parent();
            cEventNew.cSource = this;
            cEventNew.nTime = manager()->get_time();
            
            // forward event to detector
            manager()->add_event(cEventNew); 
        }
        
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
void sync_pulse_receiver::write_parameters(std::ofstream & cStream) {

    cStream << "NAME: " << get_name() << std::endl;
    cStream << "m_nDelay: " << m_nDelay << std::endl;
    cStream << "m_eDetectionMode: " << static_cast<int>(m_eDetectionMode) << std::endl;
    cStream << "m_bDown: " << m_bDown[0] << m_bDown[1] << m_bDown[2] << m_bDown[3] << std::endl;
    cStream << "m_nJitter: " << m_nJitter << std::endl;
    cStream << std::endl;
}

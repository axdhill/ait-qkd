/*
 * noise_photon_source.cpp
 * 
 * Implementation of a noise photon source
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
#include <iostream>

// ait
#include "noise_photon_source.h"
#include "../random_functions.h"
#include "../ttm.h"
#include "../channel_event_manager.h"
#include "../photon_pair_manager.h"


using namespace qkd::simulate;


// -----------------------------------------------------------------
// code


/**
 * add next noise photon generation event to event queue
 */
void noise_photon_source::add_next_source_event() {
    
    if (m_nNoisePhotonRate > 0.0) { 
        
        // only add noise photon event if noise photon rate is > 0
        event ev;
        
        ev.ePriority = event::priority::NORMAL;
        ev.eType = event::type::PHOTON;
        ev.cDestination = this;
        ev.cSource = this;
        
        // assume an exponential distributed time period between noise photon events
        ev.nTime = manager()->get_time() + static_cast<int64_t>(random_functions::random_exponential(1.0 / (ttm::RESOLUTION * m_nNoisePhotonRate)));
        
        manager()->add_event(ev);
    }
}


/**
 * handle a channel event. 
 * 
 * @param   cEvent      the channel event to be handled
 */
void noise_photon_source::handle(event const & cEvent) {

    switch (cEvent.eType) {
        
    case event::type::INIT:
        
        // simulation initialization
        add_next_source_event();
        break;
        
    case event::type::PHOTON: 
    
        { 
            // photon generation event
            event cEventNew;
            photon_pair php;
            uint64_t php_id;
            
            php.eStateA = photon_state::ABSORBED;
            php.eStateB = photon_state::NONPOLARIZED;
            
            try { 
                
                // try block because photon pair insertion could fail
                php_id = pp_manager()->insert(php);
                
                cEventNew.ePriority = event::priority::NORMAL;
                cEventNew.eType = event::type::PHOTON;
                cEventNew.cDestination = parent();
                cEventNew.cSource = this;
                cEventNew.nTime = manager()->get_time();
                cEventNew.cData.m_nPhotonPairId = php_id;
                
                manager()->add_event(cEventNew);
            }
            catch(std::runtime_error & e) {
                std::cerr << "exception caught: std::runtime_error in noise_photon_source::handle_event: " << e.what() << std::endl;
            }
      
            add_next_source_event();
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
void noise_photon_source::write_parameters(std::ofstream & cStream) {
    
    cStream << "NAME: " << get_name() << std::endl;
    cStream << "m_nNoisePhotonRate: " << m_nNoisePhotonRate << std::endl;
    cStream << std::endl;
}

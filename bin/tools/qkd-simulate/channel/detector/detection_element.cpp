/*
 * detection_element.cpp
 * 
 * Implementation of a detection_element describing the single photon detection based on avalanche effect
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
#include "../random_functions.h"
#include "../ttm.h"
#include "../channel_event_manager.h"
#include "detection_element.h"


using namespace qkd::simulate;


// -----------------------------------------------------------------
// code


/**
 * add the next dark count event to the event manager's event queue
 */
void detection_element::add_next_dark_count_event() {
    
    if (m_nDarkCountRate > 0.0) { 
        
        // only add dark count if dark count rate > 0
        event ev;
        
        ev.ePriority = event::priority::NORMAL;
        ev.eType = event::type::DARK_COUNT;
        ev.cDestination = this;
        ev.cSource = this;
        
        // assume an exponential distribution of time period between generated dark counts
        ev.nTime = manager()->get_time() + static_cast<int64_t>(random_functions::random_exponential(1.0 / (ttm::RESOLUTION * m_nDarkCountRate)));
        
        manager()->add_event(ev);
    }
}


/**
 * handle a channel event. 
 * 
 * @param   cEvent      the channel event to be handled
 */
void detection_element::handle(event const & cEvent) {

    switch (cEvent.eType) {
        
    case event::type::DARK_COUNT:  
        
        // dark count event
        add_next_dark_count_event();
        // !continue with next case
        
    case event::type::DETECT: 
        
        // photon detection event
        if (m_bEnabled && !m_bDown) {

            event cEventNew;
            
            cEventNew.ePriority = event::priority::NORMAL;
            cEventNew.eType = event::type::PULSE;
            cEventNew.cDestination = parent();
            cEventNew.cSource = this;
            cEventNew.nTime = manager()->get_time();
            cEventNew.cData.m_nDetectTime = manager()->get_time();
            cEventNew.cData.m_bDown = (m_nDownTime > 0.0);
            
            // generate electrical pulse event
            manager()->add_event(cEventNew); 
            
            if (m_nDownTime > 0.0) { 
                
                // detection element should go in down state only if m_down_time > 0
                set_down(true);
                
                cEventNew.ePriority = event::priority::SUPERHIGH;
                cEventNew.eType = event::type::DOWN_END;
                cEventNew.cDestination = this;
                cEventNew.cSource = this;
                cEventNew.nTime = manager()->get_time() + static_cast<int64_t>(m_nDownTime * 1e-9 / ttm::RESOLUTION);
                
                // generate down time end event
                manager()->add_event(cEventNew); 
            }
        }
        break;
    
    case event::type::DISABLE: 
        
        // event for disabling this detection element
        set_enabled(false);
        break;
        
    case event::type::DOWN_END: 
        
        { 
            // down time end event
            event cEventNew;
            
            set_down(false);
            
            cEventNew.ePriority = event::priority::SUPERHIGH;
            cEventNew.eType = event::type::DOWN_END;
            cEventNew.cDestination = parent();
            cEventNew.cSource = this;
            cEventNew.nTime = manager()->get_time();
            
            // forward event to detector          
            manager()->add_event(cEventNew); 
        }
        break;
    
    case event::type::ENABLE: 
        
        // event for enabling this detection element
        set_enabled(true);
        break;
        
    case event::type::INIT:  
        
        // simulation initialization
        m_bDown = false;
        m_bEnabled = m_bInitEnabled;
        add_next_dark_count_event();
        break;
        
    case event::type::PHOTON:  
        
        // incoming photon event
        if (m_bEnabled && !m_bDown) {
            
            event cEventNew;
            double td;
            
            do {
                // disallow acausal detection times 
                // (allowing them would probably spoil the logic of simulation and cause serious inconsistency problems)
                td = m_nDelay + random_functions::random_gaussian(0.0, m_nJitter);
            } while (td < 0.0); 
            
            cEventNew.ePriority = event::priority::NORMAL;
            cEventNew.eType = event::type::DETECT;
            cEventNew.cDestination = this;
            cEventNew.cSource = this;
            cEventNew.nTime = manager()->get_time() + static_cast<int64_t>(td * 1e-9 / ttm::RESOLUTION);
            
            // create photon detection event after detection delay + jitter time
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
void detection_element::write_parameters(std::ofstream & cStream) {

    cStream << "NAME: " << get_name() << std::endl;
    cStream << "m_dark_count_rate: " << m_nDarkCountRate << std::endl;
    cStream << "m_delay: " << m_nDelay << std::endl;
    cStream << "m_down: " << m_bDown << std::endl;
    cStream << "m_down_time: " << m_nDownTime << std::endl;
    cStream << "m_enabled: " << m_bEnabled << std::endl;
    cStream << "m_init_enabled: " << m_bInitEnabled << std::endl;
    cStream << "m_jitter: " << m_nJitter << std::endl;
    cStream << std::endl;
}

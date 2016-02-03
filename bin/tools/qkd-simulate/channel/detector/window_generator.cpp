/*
 * window_generator.cpp
 * 
 * Implementation of a window generator to be used inside detectors at Alice/Bob sides
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
#include "window_generator.h"
#include "../ttm.h"
#include "../channel_event_manager.h"


using namespace qkd::simulate;


// -----------------------------------------------------------------
// code


/**
 * handle a channel event. 
 * 
 * @param   cEvent      the channel event to be handled
 */
void window_generator::handle(event const & cEvent) {

    switch (cEvent.eType) {
        
    case event::type::INIT: 
        
        // simulation initialization
        m_bWindowActive = false;
        break;
        
    case event::type::SYNC_PULSE: 
    
        { 
            // sync pulse coming from sync pulse generator or receiver
            event cEventNew;
            
            if (m_bWindowActive) {
                // in case a window is already open, the window end event set in the future must be removed
                manager()->remove_event(m_nWindowEndEventId); 
            }
            
            // open window
            m_bWindowActive = true; 
            
            cEventNew.nId = m_nWindowEndEventId;
            cEventNew.ePriority = event::priority::SUPERHIGH;
            cEventNew.eType = event::type::WINDOW_END;
            cEventNew.cDestination = this;
            cEventNew.cSource = this;
            cEventNew.nTime = manager()->get_time() + static_cast<int64_t>(m_nWindowWidth / (1e9 * ttm::RESOLUTION)) + 1LL;
            
            // set window end event
            m_nWindowEndEventId = manager()->add_event(cEventNew); 
            
            cEventNew.ePriority = event::priority::HIGH;
            cEventNew.eType = event::type::WINDOW_START;
            cEventNew.cDestination = parent();
            cEventNew.cSource = this;
            cEventNew.nTime = manager()->get_time();
            
            // set window start event
            manager()->add_event(cEventNew); 
        }
        break;
        
    case event::type::SYNC_PULSE_BAD: 
    
        { 
            // sync pulse received by sync pulse receiver while not all detection elements were ready
            event cEventNew;
            
            if (m_bWindowActive) { 
                
                // window is currently open:
                // remove window end event that has been set
                manager()->remove_event(m_nWindowEndEventId); 
                
                // close window
                m_bWindowActive = false; 
                
                cEventNew.ePriority = event::priority::SUPERHIGH;
                cEventNew.eType = event::type::WINDOW_END_BAD;
                cEventNew.cDestination = parent();
                cEventNew.cSource = this;
                cEventNew.nTime = manager()->get_time();
                
                // forward bad window end event
                manager()->add_event(cEventNew); 
            }
            else { 
                
                // window is currently closed
                cEventNew.ePriority = event::priority::NORMAL;
                cEventNew.eType = event::type::SYNC_PULSE_BAD;
                cEventNew.cDestination = parent();
                cEventNew.cSource = this;
                cEventNew.nTime = manager()->get_time();
                
                // forward bad sync pulse event
                manager()->add_event(cEventNew); 
            }
        }
        break;
        
    case event::type::WINDOW_END: 
    
        { 
            // window end event
            event cEventNew;
            
            // close window
            m_bWindowActive = false; 
            
            cEventNew.ePriority = event::priority::SUPERHIGH;
            cEventNew.eType = event::type::WINDOW_END;
            cEventNew.cDestination = parent();
            cEventNew.cSource = this;
            cEventNew.nTime = manager()->get_time();
            
            // forward window end event
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
void window_generator::write_parameters(std::ofstream & cStream) {

    cStream << "NAME: " << get_name() << std::endl;
    cStream << "m_bWindowActive: " << m_bWindowActive << std::endl;
    cStream << "m_nWindowEndEventId: " << m_nWindowEndEventId << std::endl;
    cStream << "m_nWindowWidth: " << m_nWindowWidth << std::endl;
    cStream << std::endl;
}

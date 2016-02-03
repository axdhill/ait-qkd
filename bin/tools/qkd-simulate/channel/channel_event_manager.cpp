/*
 * channel_event_manager.cpp
 * 
 * Implementation of a channel event manager
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

// ait
#include <qkd/utility/debug.h>

#include "channel.h"
#include "channel_event_manager.h"
#include "channel_event_handler.h"
#include "event.h"
#include "ttm.h"
#include "detector/detection_modes.h"


using namespace qkd::simulate;


// -----------------------------------------------------------------
// code


/**
 * ctor
 */
channel_event_manager::channel_event_manager() : m_nSimulationEndTime(0LL) {
}


/** 
 * add channel event
 * 
 * @param   cEvent      the event to add
 * @return  id of new event inserted
 */
uint64_t channel_event_manager::add_event(event & cEvent) throw (std::runtime_error) {
    
    ++m_nNextId;
    cEvent.nId = m_nNextId;
    if (qkd::utility::debug::enabled()) qkd::utility::debug() << "\"add_event\": " << cEvent.str();    
    m_cEvents.push(cEvent);
    if (qkd::utility::debug::enabled()) qkd::utility::debug() << m_cEvents.dump();

    return cEvent.nId;
}


/** 
 * add channel event handler
 * 
 * @param   cHandler        the handler to add
 */
void channel_event_manager::add_event_handler(channel_event_handler * cHandler) {
    assert(cHandler != nullptr);
    m_cHandlers.push_back(cHandler);
}


/** 
 * dispatch all events in the event queue
 *
 * this is run until the event queue is empty or the simulation is stopped
 * 
 * @param   cChannel        the channel to pass events to
 */
void channel_event_manager::dispatch(channel * cChannel) {   
    
    if (qkd::utility::debug::enabled()) qkd::utility::debug() << "started dispatching events";
    
    // run down all events
    while (!m_cEvents.empty()) {
        
        if (!cChannel->is_simulation_running()) break;
        
        if (cChannel->alice()->get_detection_mode() == detection_mode::FREE_RUNNING) {
            if (m_nTime >= m_nSimulationEndTime) break;
        }
        else {
            if (cChannel->alice()->is_buffer_full() && cChannel->bob()->is_buffer_full()) break;
        }
        
        /// pick next event
        event cEvent = m_cEvents.top();
        m_cEvents.pop();
        
        // safety net: ensure monotonic event time and target existence
        assert(m_nTime <= cEvent.nTime);
        assert(cEvent.cDestination != nullptr);
        
        m_nTime = cEvent.nTime;
        if (qkd::utility::debug::enabled()) {
            qkd::utility::debug() << "\"dispatch_event\": " << cEvent.str() << "\n" << m_cEvents.dump();            
        }
        
        // ... and handle it
        cEvent.cDestination->handle(cEvent);
    }
    
    if (qkd::utility::debug::enabled()) qkd::utility::debug() << "end of event dispatch loop - winding down";

    // send simulation stop events to all event handlers
    event cEvent;
    cEvent.ePriority = event::priority::SYSTEM;
    cEvent.eType = event::type::STOP;
    cEvent.cSource = nullptr;
    cEvent.nTime = m_nTime;
    
    // give all handler the terminate "event"
    for (channel_event_handler * cHandler : m_cHandlers) {
        
        cEvent.cDestination = cHandler;
        if (qkd::utility::debug::enabled()) qkd::utility::debug() << "\"dispatch_event\": " << cEvent.str();
        cHandler->handle(cEvent);
    }
}


/** 
 * initialize simulation
 */
void channel_event_manager::init_simulation() {
    
    m_cEvents = event_queue();
    m_nTime = 0LL;
    m_nNextId = 0ULL;
    
    // send simulation initialization events to all event handlers
    event cEvent;
    cEvent.ePriority = event::priority::SYSTEM;
    cEvent.eType = event::type::INIT;
    cEvent.cSource = nullptr;
    cEvent.nTime = m_nTime;
    
    for (channel_event_handler * cHandler : m_cHandlers) {
        
        cEvent.cDestination = cHandler;
        if (qkd::utility::debug::enabled()) qkd::utility::debug() << "\"init_simulation\": " << cEvent.str();
        assert(cHandler != nullptr);
        cHandler->handle(cEvent);
    }
}


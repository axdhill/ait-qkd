/*
 * channel_event_manager.h
 * 
 * Declaration of a channel event manager
 * 
 * Autor: Philipp Grabenweger
 * Autor: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
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


#ifndef __QKD_QKD_SIMULATE_CHANNEL_EVENT_MANAGER_H_
#define __QKD_QKD_SIMULATE_CHANNEL_EVENT_MANAGER_H_


// ------------------------------------------------------------
// include

#include <atomic>
#include <stdexcept>

// ait
#include "event.h"


// -----------------------------------------------------------------
// decl


namespace qkd {
    
namespace simulate {
    
    
// fwd. decl.
class channel;
class channel_event_handler;


/**
 * channel event manager
 * 
 * the channel event manager runs a series of events on 
 * registered event handlers. this is done on a per
 * measurment basis
 */
class channel_event_manager {

    
public:
    
    
    /** 
     * ctor
     */
    channel_event_manager();
    
    
    /** 
     * dtor
     */
    virtual ~channel_event_manager() {};
    
    
    /** 
     * add channel event
     * 
     * @param   cEvent      the event to add
     * @return  id of new event inserted
     */
    uint64_t add_event(event & cEvent) throw (std::runtime_error);
    

    /** 
     * add channel event handler
     * 
     * @param   cHandler        the handler to add
     */
    void add_event_handler(channel_event_handler * cHandler);
    
    
    /** 
     * get simulation end time
     * 
     * @return  the simulation end time in units of ttm::RESOLUTION
     */
    int64_t get_sim_end_time() { return m_nSimulationEndTime; }
    
    
    /** 
     * get current simulation time
     * 
     * @return  the current simulation time in units of ttm::RESOLUTION
     */
    int64_t get_time() const { return m_nTime; }
    
    
    /** 
     * dispatch all events in the event queue
     *
     * this is run until the event queue is empty or the simulation is stopped
     * 
     * @param   cChannel        the channel to pass events to
     */
    void dispatch(channel * cChannel);
    
    
    /** 
     * initialize simulation
     */
    void init_simulation();
    
    
    /** 
     * remove a channel event from the event queue
     * 
     * @param   nId     the channel event identifier of the event to be removed
     */
    void remove_event(uint64_t nId) { m_cEvents.remove(nId); };
    
    
    /**
     * set simulation end time
     * 
     * @param   nSimulationEndTime      new simulation end time in units of ttm::RESOLUTION
     */
    void set_sim_end_time(int64_t nSimulationEndTime) { m_nSimulationEndTime = nSimulationEndTime; }
    
    
private:
    
    
    /**
     * priority queue of channel event links. 
     * 
     * This queue contains all events to be dispatched, but may also 
     * contain channel event links whose channel event identifiers
     */
    event_queue m_cEvents;
    
    
    /**
     * registered channel event handlers 
     */
    std::vector<channel_event_handler *> m_cHandlers; 
    
    
    /**
     * next event id 
     */
    uint64_t m_nNextId;         
    
    
    /**
     * simulation end time in units of ttm::RESOLUTION 
     */
    std::atomic<int64_t> m_nSimulationEndTime;           
    
    
    /**
     * current simulation time in units of ttm::RESOLUTION 
     */
    std::atomic<int64_t> m_nTime;                        
    
};

}
}

#endif

/*
 * event.h
 * 
 * Declaration of a channel event and associated structures, functions and constant definitions
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


#ifndef __QKD_QKD_SIMULATE_EVENT_H_
#define __QKD_QKD_SIMULATE_EVENT_H_


// ------------------------------------------------------------
// include

#include <queue>
#include <string>
#include <sstream>

#include <string.h>

// ait
#include "photon_pair.h"


// -----------------------------------------------------------------
// declarations and constant definitions


namespace qkd {
    
namespace simulate {
    
    
// fwd. decl.
class channel_event_handler;


/**
 * constant strings naming the channel event types defined in event::type
 */
static std::string const event_type_str[] = {
    
    "DARK_COUNT",
    "DETECT",
    "DETECTOR_PULSE",
    "DISABLE",
    "DOWN_END",
    "ENABLE",
    "INIT",
    "PHOTON",
    "PULSE",
    "STOP",
    "SYNC_PULSE",
    "SYNC_PULSE_BAD",
    "WINDOW_END",
    "WINDOW_END_BAD",
    "WINDOW_START"
};


/**
 * constant strings naming the channel event priorities defined in event::priority
 */
static std::string const event_priority_str[] = {
    
    "SYSTEM",
    "SUPERHIGH",
    "HIGH",
    "NORMAL",
    "SUBNORMAL",
    "LOW"
};


/**
 * a single event (photon generated, ...)
 */
class event {
    

public:    
    
    
    /**
     * channel event types
     */
    enum type : uint8_t {
        
        DARK_COUNT,             /**< detector dark count */
        DETECT,                 /**< photon detection */
        DETECTOR_PULSE,         /**< detector electrical pulse */
        DISABLE,                /**< disable detection element */
        DOWN_END,               /**< end of detector down period */
        ENABLE,                 /**< enable detection element */
        INIT,                   /**< simulation initialization */
        PHOTON,                 /**< incoming or outgoing photon */
        PULSE,                  /**< electrical pulse */
        STOP,                   /**< simulation stop */
        SYNC_PULSE,             /**< synchronization pulse */
        SYNC_PULSE_BAD,         /**< synchronization pulse coming while some detection elements are down */
        WINDOW_END,             /**< end of window */
        WINDOW_END_BAD,         /**< end of window due to bad sync pulse */
        WINDOW_START            /**< start of window */
    };
    

    /**
     * channel event priorities
     */
    enum priority : uint8_t {
        
        SYSTEM,
        SUPERHIGH,
        HIGH,
        NORMAL,
        SUBNORMAL,
        LOW
    };

    
    uint64_t nId;                               /**< channel event identifier */
    priority ePriority;                         /**< channel event priority */
    type eType;                                 /**< channel event type */
    channel_event_handler * cDestination;       /**< destination channel event handler */
    channel_event_handler * cSource;            /**< source channel event handler */
    int64_t nTime;                              /**< event time */
    
    struct {
        
        uint64_t m_nPhotonPairId;           /**< photon pair identifier */
        photon_state m_ePhotonState;        /**< photon state */
        int64_t m_nDetectTime;              /**< photon detection time */
        bool m_bAlice;                      /**< alice state */
        bool m_bDown;                       /**< states if detector is going in down state now */
        
    } cData;                                    /**< data associated with the event */
    
    
    /**
     * ctor
     */
    event() : 
        nId(0), 
        ePriority(event::priority::NORMAL), 
        eType(event::type::PHOTON), 
        cDestination(nullptr),
        cSource(nullptr),
        nTime(0) { memset(&cData, 0, sizeof(cData)); }
        

        
    /**
     * create a priority value by shifting the time value and binary OR of the priority
     * 
     * @return  a priority value for comparison of events
     */
    int64_t priority_value() const { return ((nTime << 3) | (ePriority & 0x07)); };
    
        
    /**
     * > operator
     * 
     * if true than this event has to be handled prior
     *
     * @param   rhs     right hand side
     * @return  true if this event is earlier or at the same time but with higher priority
     */
    bool operator>(event const & rhs) const { return (priority_value() < rhs.priority_value()); };
    
    
    /**
     * < operator
     * 
     * if true then this event has to be handled after rhs
     *
     * @param   rhs     right hand side
     * @return  true if this event is earlier or at the same time but with higher priority
     */
    bool operator<(event const & rhs) const { return (priority_value() > rhs.priority_value()); };
    
    
    /**
     * string representation of this event in JSON syntax
     * 
     * @return  a string representation of this event
     */
    std::string str() const;

};


/**
 * event less comparison
 */
class event_less {
public:    
    bool operator()(event const & lhs, event const & rhs) const { return lhs < rhs; };
};


/**
 * a queue of prioritized events
 */
class event_queue : public std::priority_queue<event, std::vector<event>, event_less> {
    
public:
    
    
    /**
     * dump the current queue
     */
    std::string dump() const {
        std::stringstream ss;
        ss << "\"event_queue\": {\n";
        bool f = true;
        for (event const & e: c) {
            ss << "\t" << e.str() << (f ? "," : "") << "\n";
            f = false;
        }
        ss << "}";
        return ss.str();
    };
    
    
    /**
     * remove a certain event
     * 
     * @param   nId     the Id of the event to remove
     */
    void remove(uint64_t nId) {
        container_type::iterator i = c.begin();
        for (; i != c.end(); ++i) if ((*i).nId == nId) break;
        if (i != c.end()) c.erase(i);
    };
    
};


}
}


#endif

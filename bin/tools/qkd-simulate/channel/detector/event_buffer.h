/*
 * event_buffer.h
 * 
 * Declaration of an event buffer to be used inside detector at Alice/Bob sides
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


#ifndef __QKD_QKD_EVENT_BUFFER_H_
#define __QKD_QKD_EVENT_BUFFER_H_


// ------------------------------------------------------------
// include

// ait
#include "../channel_event_handler.h"


// -----------------------------------------------------------------
// decl

namespace qkd {
    
namespace simulate {
    
    
class event_buffer : public channel_event_handler {
    
    
public:
    
    
    /**
     * ctor
     */
    event_buffer() : m_cBuffer(nullptr), m_nBufferSize(0) {}

    
    /**
     * dtor
     */
    ~event_buffer() { if (m_cBuffer) { delete[] m_cBuffer; m_cBuffer = nullptr; } }
    
    
    /**
     * get the event buffer
     * 
     * @return  the event buffer
     */
    unsigned char * get_buffer() { return m_cBuffer; }
    
    /**
     * get event buffer size
     * 
     * @return  the event buffer size in bytes
     */
    uint64_t get_buffer_size() { return m_nBufferSize; }
    
    
    /**
     * handle a channel event. 
     * 
     * @param   cEvent      the channel event to be handled
     */
    void handle(event const & cEvent);
    
    
    /**
     * test if event buffer is full
     * 
     * @return  true if event buffer is full, false otherwise
     */
    bool is_buffer_full() { return (m_nNextIndex >= m_nBufferSize); }
    
    
    /**
     * set event buffer size
     * 
     * @param   nBufferSize     the event buffer size in bytes
     */
    void set_buffer_size(uint64_t nBufferSize);


    /**
     * write out all parameters of this event handler and its child event handlers
     * 
     * @param   cStream     the stream to write to
     */
    void write_parameters(std::ofstream & cStream);
    
    
private:
    
    
    /**
     * write events currently in detector latch to event buffer (if not full already)
     */
    void write_event();
    
    unsigned char * m_cBuffer;          /**< event buffer */
    uint64_t        m_nBufferSize;      /**< size of event buffer in bytes */
    bool            m_det_latch[4];     /**< latch for detector events (0 = H, 1 = V, 2 = P, 3 = M) */
    bool            m_bNextHigh;        /**< states whether the next event entry should go into the high half-byte */
    uint64_t        m_nNextIndex;       /**< index of next event entry in buffer */
    bool            m_bWindowOpen;      /**< states whether a sync window is currently open */
};
    
}
}

#endif

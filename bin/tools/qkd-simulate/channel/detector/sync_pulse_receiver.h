/*
 * sync_pulse_receiver.h
 * 
 * Declaration of a sync pulse receiver to be used inside detector at Bob side
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


#ifndef __QKD_QKD_SYNC_PULSE_RECEIVER_H_
#define __QKD_QKD_SYNC_PULSE_RECEIVER_H_


// ------------------------------------------------------------
// include

// ait
#include "detection_modes.h"
#include "../channel_event_handler.h"


// -----------------------------------------------------------------
// decl


namespace qkd {
    
namespace simulate {


class sync_pulse_receiver : public channel_event_handler {
    
public:
    
    
    /**
     * ctor
     */
    sync_pulse_receiver() : m_eDetectionMode(detection_mode::FREE_RUNNING) {}
    
    
    /**
     * get the delay time
     * 
     * @return  the delay time in ns
     */
    double get_delay() const { return m_nDelay; }
    
    
    /**
     * get the detection mode
     * 
     * @return  the detection mode
     */
    detection_mode get_detection_mode() const { return m_eDetectionMode; }
    
    
    /**
     * get the jitter standard deviation
     * 
     * @return  the jitter standard deviation in ns
     */
    double get_jitter() const { return m_nJitter; }
    
    
    /**
     * handle a channel event. 
     * 
     * @param   cEvent      the channel event to be handled
     */
    void handle(event const & cEvent);
    
    
    /**
     * set the delay time
     * 
     * @param   nDelay      the delay time in ns 
     */
    void set_delay(double nDelay) { m_nDelay = nDelay; };
    
    
    /**
     * set the detection mode
     * 
     * @param   eDetectionMode      the detection mode
     */
    void set_detection_mode(detection_mode eDetectionMode) { m_eDetectionMode = eDetectionMode; };
    
    
    /**
     * set the jitter standard deviation
     * 
     * @param   nJitter     the jitter standard deviation in ns
     */
    void set_jitter(double nJitter) { m_nJitter = nJitter; }

    
    /**
     * write out all parameters of this event handler and its child event handlers
     * 
     * @param   cStream     the stream to write to
     */
    void write_parameters(std::ofstream & cStream);
    
    
private:
    
    
    /**
     * delay time in ns 
     */
    double m_nDelay;                  
    
    /**
     * the detection mode in which the detector at Bob side is running 
     */
    detection_mode m_eDetectionMode; 

    
    /**
     * stores the down states of the four detection elements (0 = H, 1 = V, 2 = P, 3 = M) 
     */
    bool m_bDown[4];                  
    
    
    /**
     * jitter standard deviation in ns 
     */
    double m_nJitter;                 
};
    
}
}

#endif

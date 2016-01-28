/*
 * detection_element.h
 * 
 * Declaration of a detection_element describing the single photon detection based on avalanche effect
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


#ifndef __QKD_QKD_SIMULATE_DETECTION_ELEMENT_H_
#define __QKD_QKD_SIMULATE_DETECTION_ELEMENT_H_


// ------------------------------------------------------------
// include

// ait
#include "../event.h"
#include "../channel_event_handler.h"


// -----------------------------------------------------------------
// decl


namespace qkd {
    
namespace simulate {

    
/**
 * the detection element of a single photon detector based on avalanche effect
 */
class detection_element : public channel_event_handler {
    
public:
    
    
    /** 
     * ctor
     */
    detection_element() : m_bDown(false), m_bEnabled(true), m_bInitEnabled(true) {}
    
    
    /**
     * get the dark count rate
     *
     * @return  the dark count rate in 1/s
     */
    double get_dark_count_rate() const { return m_nDarkCountRate; }
    
    
    /**
     * get the delay time
     *
     * @return  the delay time in ns
     */
    double get_delay() const { return m_nDelay; }
    
    
    /**
     * get the down state variable
     *
     * @return  the down state variable
     */
    bool get_down() const { return m_bDown; }
    
    
    /**
     * get the down time
     *
     * @return  the down time in ns
     */
    double get_down_time() const { return m_nDownTime; }
    
    
    /**
     * get the enabled state
     *
     * @return  the enabled state
     */
    bool get_enabled() const { return m_bEnabled; }
    
    
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
     * set the dark count rate
     * 
     * @param   nDarkCountRate      dark count rate in 1/s
     */
    void set_dark_count_rate(double nDarkCountRate) { m_nDarkCountRate = nDarkCountRate; }    
    
    
    /**
     * set the delay time
     * 
     * @param   nDelay          delay time in ns
     */
    void set_delay(double nDelay ) { m_nDelay = nDelay; }
    
    
    /**
     * set the down state variable
     * 
     * @param   bDown           new down state
     */
    void set_down(bool bDown)  { m_bDown = bDown; }
    
    
    /**
     * set the down time
     * 
     * @param   nDownTime           down time in ns
     */
    void set_down_time(double nDownTime) { m_nDownTime = nDownTime; }
    
    
    /**
     * set the enabled state
     * 
     * @param   bEnabled            new enabled state
     */
    void set_enabled(bool bEnabled) { m_bEnabled = bEnabled; }
    
    
    /**
     * set the initial enabled state
     * 
     * @param   bInitEnabled        new initial enabled state
     */
    void set_init_enabled(bool bInitEnabled) { m_bInitEnabled = bInitEnabled; }
    
    
    /**
     * set the jitter standard deviation
     * 
     * @param   nJitter             jitter standard deviation in ns
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
     * add the next dark count event to the event manager's event queue
     */
    void add_next_dark_count_event();
    
    
    double m_nDarkCountRate;            /**< dark count rate in 1/s */
    double m_nDelay;                    /**< delay in ns */
    bool m_bDown;                       /**< states whether this detection element is down */
    double m_nDownTime;                 /**< down time in ns */
    bool m_bEnabled;                    /**< states whether this detection element is enabled */
    bool m_bInitEnabled;                /**< states whether this detection element is enabled at initialization */
    double m_nJitter;                   /**< jitter in ns */
};

}
}

#endif

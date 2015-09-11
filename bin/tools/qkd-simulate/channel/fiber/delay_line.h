/*
 * delay_line.h
 * 
 * Declaration of an ideal photon delay line
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


#ifndef __QKD_QKD_SIMULATE_DELAY_LINE_H_
#define __QKD_QKD_SIMULATE_DELAY_LINE_H_


// ------------------------------------------------------------
// include

// ait
#include "../channel_event_handler.h"


// -----------------------------------------------------------------
// decl


namespace qkd {
    
namespace simulate {

    
/**
 * ideal photon delay line
 */ 
class delay_line : public channel_event_handler {
    
    
public:
    
    
    /** 
     * ctor
     */
    delay_line() : m_nDelayTime(0.0) {}
    
    
    /**
     * get the delay time
     *
     * @return  the delay time in ns
     */
    double get_delay_time() const { return m_nDelayTime; }
    

    /**
     * handle a channel event. 
     * 
     * @param   cEvent      the channel event to be handled
     */
    void handle(event const & cEvent);
    
    
    /**
     * set the delay time
     * 
     * @param   nDelayTime      the new delay time in ns
     */
    void set_delay_time(double nDelayTime ) { m_nDelayTime = nDelayTime; }

    
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
    double m_nDelayTime; 
    
};

}
}

#endif

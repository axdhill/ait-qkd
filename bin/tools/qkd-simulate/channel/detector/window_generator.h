/*
 * window_generator.h
 * 
 * Implementation of a window generator to be used inside detectors at Alice/Bob sides
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


#ifndef __QKD_QKD_WINDOW_GENERATOR_H_
#define __QKD_QKD_WINDOW_GENERATOR_H_


// ------------------------------------------------------------
// include

// ait
#include "../event.h"
#include "../channel_event_handler.h"


// -----------------------------------------------------------------
// decl


namespace qkd {
    
namespace simulate {

    
class window_generator : public channel_event_handler {
    
    
public:
    
    
    /**
     * get window with
     * 
     * @return  the window width in ns
     */
    double get_window_width() const { return m_nWindowWidth; }
    
    
    /**
     * handle a channel event. 
     * 
     * @param   cEvent      the channel event to be handled
     */
    void handle(event const & cEvent);
    
    
    /**
     * set window width
     * 
     * @param   nWindowWidth            new window width in ns
     */
    void set_window_width(double nWindowWidth) { m_nWindowWidth = nWindowWidth; }
    

    /**
     * write out all parameters of this event handler and its child event handlers
     * 
     * @param   cStream     the stream to write to
     */
    void write_parameters(std::ofstream & cStream);

    
private:
    
    
    /**
     * states whether the window generator is currently outputting a window 
     */
    bool m_bWindowActive;                          
    
    
    /**
     * channel_ev_id of window_end event in case it has been set 
     */
    uint64_t m_nWindowEndEventId;        
    
    
    /**
     * window width in ns 
     */
    double m_nWindowWidth;                      
};
    
}
}

#endif

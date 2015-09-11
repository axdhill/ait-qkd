/*
 * fiber_sync.h
 * 
 * Declaration of a synchronization impulse transmission fiber
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


#ifndef __QKD_QKD_SIMULATE_FIBER_SYNC_H_
#define __QKD_QKD_SIMULATE_FIBER_SYNC_H_


// ------------------------------------------------------------
// include

// ait
#include "../channel_event_handler.h"


// -----------------------------------------------------------------
// decl


namespace qkd {
    
namespace simulate {

    
class fiber_sync : public channel_event_handler {

    
public:
    

    
    /**
     * handle a channel event. 
     * 
     * @param   cEvent      the channel event to be handled
     */
    void handle(event const & cEvent);

    
    /**
     * write out all parameters of this event handler and its child event handlers
     * 
     * @param   cStream     the stream to write to
     */
    void write_parameters(std::ofstream & cStream);
    
};

}
}

#endif

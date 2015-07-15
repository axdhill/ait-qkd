/*
 * sync_pulse_generator.h
 * 
 * Declaration of a sync pulse generator to be used inside detector at Alice side
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


#ifndef __QKD_QKD_SYNC_PULSE_GENERATOR_H_
#define __QKD_QKD_SYNC_PULSE_GENERATOR_H_


// ------------------------------------------------------------
// include

// ait
#include "../channel_event_handler.h"
#include "detection_modes.h"


// -----------------------------------------------------------------
// decl

namespace qkd {
    
namespace simulate {

    
class sync_pulse_generator : public channel_event_handler {
    
    
public:
    
    
    /**
     * ctor
     */
    sync_pulse_generator() : m_eDetectionMode(detection_mode::FREE_RUNNING) {}
    
    
    /**
     * get the detection mode
     * 
     * @return  the detection mode
     */
    detection_mode get_detection_mode() const { return m_eDetectionMode; }
    
    
    /**
     * handle a channel event. 
     * 
     * @param   cEvent      the channel event to be handled
     */
    void handle(event const & cEvent);

    
    /**
     * set the detection mode
     * 
     * @param   eDetectionMode      the new detection mode
     */
    void set_detection_mode(detection_mode eDetectionMode) { m_eDetectionMode = eDetectionMode; }

    
    /**
     * write out all parameters of this event handler and its child event handlers
     * 
     * @param   cStream     the stream to write to
     */
    void write_parameters(std::ofstream & cStream);

    
private:
    
    detection_mode m_eDetectionMode;        /**< the detection mode in which the detector at Alice side is running */
    bool m_bDetReady;                       /**< states whether the detection elements are ready so that the next sync pulse is allowed to be generated */
    bool m_bDown[4];                        /**< stores the down states of the four detection elements (0 = H, 1 = V, 2 = P, 3 = M) */
    unsigned int m_nSyncInitiator;          /**< stores the index of the detection element that initiated the last sync pulse */
    bool m_bWindowGeneratorReady;           /**< states whether the window generator is ready so that the next sync pulse is allowed to be generated */
};

}
}

#endif

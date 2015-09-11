/*
 * noise_photon_source.h
 * 
 * Declaration of a noise photon source
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


#ifndef __QKD_QKD_SIMULATE_NOISE_PHOTON_SOURCE_H_
#define __QKD_QKD_SIMULATE_NOISE_PHOTON_SOURCE_H_


// ------------------------------------------------------------
// include

// ait
#include "../channel_event_handler.h"


// -----------------------------------------------------------------
// decl


namespace qkd {
    
namespace simulate {

    
/**
 * a noise photon source describing the noise photons interspersed into the quantum fiber
 */ 
class noise_photon_source : public channel_event_handler {
    
    
public:
    
    
    /** 
     * ctor
     */
    noise_photon_source() : m_nNoisePhotonRate(0.0) {}
    
    
    /**
     * get the noise photon rate
     *
     * @return  noise photon rate in 1/s
     */
    double get_noise_photon_rate() const { return m_nNoisePhotonRate; }
    
    
    /**
     * handle a channel event. 
     * 
     * @param   cEvent      the channel event to be handled
     */
    void handle(event const & cEvent);
    
    
    /**
     * set the noise photon rate
     * 
     * @param   nNoisePhotonRate            noise photon rate in 1/s
     */
    void set_noise_photon_rate(double nNoisePhotonRate) { m_nNoisePhotonRate = nNoisePhotonRate; }
    

    /**
     * write out all parameters of this event handler and its child event handlers
     * 
     * @param   cStream     the stream to write to
     */
    void write_parameters(std::ofstream & cStream);
    

private:
    
    
    /**
     * add next noise photon generation event to event queue
     */
    void add_next_source_event();
    

    /**
     * noise photon rate in 1/s 
     */
    double m_nNoisePhotonRate; 
};

}
}

#endif

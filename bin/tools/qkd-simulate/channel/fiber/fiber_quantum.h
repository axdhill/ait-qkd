/*
 * fiber_quantum.h
 * 
 * Declaration of a quantum fiber describing photon transport
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


#ifndef __QKD_QKD_SIMULATE_FIBER_QUANTUM_H_
#define __QKD_QKD_SIMULATE_FIBER_QUANTUM_H_


// ------------------------------------------------------------
// include

// ait
#include "../channel_event_handler.h"


// -----------------------------------------------------------------
// decl


namespace qkd {
    
namespace simulate {

    
/**
 * quantum fiber describing photon transport
 */
class fiber_quantum : public channel_event_handler {

    
public:
    
    
    /**
     * get the absorption coefficient
     *
     * @return  absorption coefficient in dB/km
     */
    double get_absorption_coefficient() const { return m_nAbsorptionCoefficient; }
    
    
    /**
     * get the fiber length
     *
     * @return  the fiber length in km
     */
    double get_length() const { return m_nLength; }
    
    
    /**
     * handle a channel event. 
     * 
     * @param   cEvent      the channel event to be handled
     */
    void handle(event const & cEvent);
    
    
    /**
     * set the absorption coefficient
     * 
     * @param   nAbsorptionCoefficient      new absorption coefficient in dB/km
     */
    void set_absorption_coefficient(double nAbsorptionCoefficient) { 
        m_nAbsorptionCoefficient = nAbsorptionCoefficient; update_transmission_probability();
    }
    
    
    /**
     * set the fiber length
     * 
     * @param   nLength          new fiber length in km
     */
    void set_length(double nLength) { m_nLength = nLength; update_transmission_probability(); }

    
    /**
     * write out all parameters of this event handler and its child event handlers
     * 
     * @param   cStream     the stream to write to
     */
    void write_parameters(std::ofstream & cStream);

    
private:

    
    /**
     * update the m_nTransmissionProbability member
     */
    void update_transmission_probability() { m_nTransmissionProbability = pow(10.0, -m_nLength * m_nAbsorptionCoefficient / 10.0); }
    
    
    /**
     * absorption coefficient in dB/km 
     */
    double m_nAbsorptionCoefficient;   
    
    
    /**
     * fiber length in km 
     */
    double m_nLength;                   
    
    
    /**
     * probability of photon not getting absorbed during transmission [0 - 1] 
     */
    double m_nTransmissionProbability; 
};

}
}

#endif

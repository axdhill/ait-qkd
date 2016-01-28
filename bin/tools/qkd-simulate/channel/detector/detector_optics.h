/*
 * detector_optics.h
 * 
 * Declaration of detector_optics describing the optical pathway of photon detection in the BB84 protocol
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


#ifndef __QKD_QKD_SIMULATE_DETECTOR_OPTICS_H_
#define __QKD_QKD_SIMULATE_DETECTOR_OPTICS_H_


// ------------------------------------------------------------
// include

#include <math.h>

// ait
#include "../channel_event_handler.h"


// -----------------------------------------------------------------
// decl


namespace qkd {
    
namespace simulate {


/**
 * the detector optics along the optical pathway of photon detection in the BB84 protocol
 */
class detector_optics : public channel_event_handler {
    
public:
    
    
    /**
     * get the alice state
     *
     * @return  the alice state
     */
    bool get_alice() const { return m_bAlice; }
    
    
    /**
     * get the detection efficiency
     *
     * @return  the detection efficiency [0 - 1]
     */
    double get_efficiency() const { return m_nEfficiency; }
    
    
    /**
     * get the loss
     *
     * @return  the loss in dB
     */
    double get_loss() const { return m_nLoss; }
    
    
    /**
     * handle a channel event. 
     * 
     * @param   cEvent      the channel event to be handled
     */
    void handle(event const & cEvent);
    
    
    /**
     * set the alice state
     * 
     * @param   bAlice          the new alice state
     */
    void set_alice(bool bAlice) { m_bAlice = bAlice; }

        
    /**
     * set the detection efficiency
     * 
     * @param   nEfficiency         the detector efficiency [0 - 1]
     */
    void set_efficiency(double nEfficiency) { m_nEfficiency = nEfficiency; update_detect_probability(); }
    
    
    /**
     * set the loss
     * 
     * @param   nLoss           the loss in dB
     */
    void set_loss(double nLoss ) { m_nLoss = nLoss; update_detect_probability(); }

    
    /**
     * write out all parameters of this event handler and its child event handlers
     * 
     * @param   cStream     the stream to write to
     */
    void write_parameters(std::ofstream & cStream);
    
    
private:
    
    
    /**
     * update the m_detect_probability member
     */
    void update_detect_probability() { m_nDetectProbability = m_nEfficiency * pow(10.0, -m_nLoss / 10.0); }
    
    
    /**
     * associates a detector index with that 
     * of the detector for the orthogonal state (0 = H, 1 = V, 2 = P, 3 = M) 
     */
    const static int det_num_ortho[4]; 
    
    
    /**
     * specifies the detector index (0 = H, 1 = V, 2 = P, 3 = M) 
     * to choose at alice side for a specific photon state and 
     * based on a random number.
     * 
     * The first index determines the alice photon state: 
     * 0 = nonpolarized / entangled, 1 = horizontal, 2 = vertical, 3 = plus, 4 = minus.
     * 
     * The second index should be chosen based on a random number equally 
     * distributed among the integer numbers from 0 to 3. 
     */    
    const static int det_num[5][4]; 
    
    
    /**
     * states whether this detector_optics object is at Alice side 
     */
    bool m_bAlice;               
    
    
    /**
     * combined probability for photon detection [0 - 1] 
     */
    double m_nDetectProbability;  
    
    
    /**
     * detection efficiency [0 - 1] 
     */
    double m_nEfficiency;          
    
    
    /**
     * loss in dB 
     */
    double m_nLoss;
    
};

}
}

#endif

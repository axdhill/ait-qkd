/*
 * source.h
 * 
 * definition of a quantum source
 *
 * Author: Mario Kahn
 *         Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *         Philipp Grabenweger
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

 
#ifndef __QKD_QKD_SIMULATE_SOURCE_H_
#define __QKD_QKD_SIMULATE_SOURCE_H_


// ------------------------------------------------------------
// incs

// ait
#include "channel_event_handler.h"


// -----------------------------------------------------------------
// decl


namespace qkd {
    
namespace simulate {


/**
 * an quantum source, it procuces photons
 */
class source : public channel_event_handler {

public:
    

    /** 
     * ctor
     */
    source();

    
    /** 
     * dtor
     */
    virtual ~source() {};
    
    
    /**
     * handle a channel event. 
     * 
     * @param   cEvent      the channel event to be handled
     */
    void handle(event const & cEvent);
    
    
    /**
     * get the multi photons enabled flag
     * 
     * @return  multi photons flag
     */
    bool multi_photons() const { return m_bMultiPhotons; };
    
    
    /**
     * get multi photon rate in [0 - 1 0000 Hz]
     * 
     * @return  the multi photon rate 
     */
    double multi_photon_rate() const { return m_nMultiPhotonRate; }
        
        
    /**
     * get source photon rate in [0 - 10 000 000 Hz]
     * 
     * @return  source photon rate 
     */
    double photon_rate() const { return m_nPhotonRate; };
    

    /**
     * set the multi photons enabled flag
     * 
     * @param   bMultiPhotons           the new multi photons flags
     */
    void set_multi_photons(bool bMultiPhotons) { m_bMultiPhotons = bMultiPhotons; };
    
    
    /**
     * set multi photon rate in [0 - 1 0000 Hz]
     * 
     * @param   nMultiPhotonRate        the new multphoton rate
     */
    void set_multi_photon_rate(double nMultiPhotonRate) throw(std::out_of_range);
    
        
    /**
     * set source photon rate in [0 - 10 000 000 Hz]
     * 
     * @param   nPhotonRate     the new source photon rate 
     */
    void set_photon_rate(double nPhotonRate) throw(std::out_of_range);
    

    /**
     * sets source signal error probability in [0 - 100 %]
     * 
     * @param   nSignalErrorProbablity      the new signal/error probability
     */
    void set_signal_error_probability(double nSignalErrorProbablity) throw(std::out_of_range);

    
    /**
     * get source signal error probability in [0 - 100 %]
     *
     * @return  signal error probability in [%]
     */
    double signal_error_probability() const { return m_nSignalErrorProbablity * 100.0; };

    
    /**
     * write out all parameters of this event handler and its child event handlers
     * 
     * @param   cStream     the stream to write to
     */
    void write_parameters(std::ofstream & cStream);

    
private:
    
    
    /**
     * add the next source event to event queue
     */
    void add_next_source_event();
    
    
    bool m_bMultiPhotons;                       /**< multi photons enabled flag */
    double m_nMultiPhotonRate;                  /**< multi photon rate in 1/s */
    double m_nPhotonRate;                       /**< photon rate in 1/s */
    double m_nSignalErrorProbablity;            /**< signal/error probability [0-1] */
};


}
}

#endif

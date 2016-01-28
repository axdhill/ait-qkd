/*
 * fiber.h
 * 
 * definition of a quantum fiber system
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

 
#ifndef __QKD_QKD_SIMULATE_FIBER_H_
#define __QKD_QKD_SIMULATE_FIBER_H_


// -----------------------------------------------------------------
// include

#include <exception>

// ait
#include "channel_event_handler.h"
#include "fiber/delay_line.h"
#include "fiber/fiber_quantum.h"
#include "fiber/fiber_sync.h"
#include "fiber/noise_photon_source.h"


// -----------------------------------------------------------------
// decl


namespace qkd {
    
namespace simulate {


/**
 * the quantum fiber system: the path medium a photon travels from Alice to Bob
 */
class fiber : public channel_event_handler {

public:
    

    /** 
     * ctor
     */
    fiber();

    
    /** 
     * dtor
     */
    virtual ~fiber() {};
    
    
    /**
     * get fiber absorption coefficient in [0 - 10 dB/km]
     *
     * @return  the fiber absorbtion coefficient
     */
    double absorption_coefficient() const { return m_nAbsorptionCoefficient; };
    
    
    /**
     * handle a channel event. 
     * 
     * @param   cEvent      the channel event to be handled
     */
    void handle(event const & cEvent);
    
    
    /**
     * function that initializes the channel event handler. 
     * 
     * @param   cParent                 the parent channel event handler
     * @param   cManager              the event manager
     * @param   cPhotonPairManager      the photon pair manager
     */
    void init(channel_event_handler * cParent, channel_event_manager * cManager, photon_pair_manager * cPhotonPairManager);  
    
    
    /**
     * get fiber length in [0 - 500 km]
     *
     * @return  the fiber length
     */
    double length() const { return m_nLength; };


    /**
     * check if loss is enabled
     * 
     * @return  loss flag
     */
    bool loss() const { return m_bLoss; };
     
    
    /**
     * get fiber noise photon rate in 1/s
     *
     * @return  the fiber noise photon rate
     */
    double noise_photon_rate() const { return m_noise_photon_source.get_noise_photon_rate(); };
     
    
    /**
     * get the photon delay time
     * 
     * @return the photon delay time in ns
     */
    double photon_delay() { return m_delay_line.get_delay_time(); }
    
    
    /**
     * set fiber absorption coefficient in [0-10 dB/km]
     *
     * @param  nAbsorptionCoefficient       the new fiber absorption coefficient in [db/km]
     */
    void set_absorption_coefficient(double nAbsorptionCoefficient) throw(std::out_of_range);
    
    
    /**
     * set fiber length in [0 - 500 km]
     *
     * @param  nLength          the new fiber length
     */
    void set_length(double nLength) throw(std::out_of_range);

    
    /**
     * set loss flag  
     * 
     * @param   bLoss     new loss flag
     */
    void set_loss(bool bLoss) { m_bLoss = bLoss; update_absorption_coefficient(); };
    
    
    /**
     * set fiber noise photon rate in 1/s
     *
     * @param  nNoisePhotonRate         the new fiber noise_photon_rate
     */
    void set_noise_photon_rate(double nNoisePhotonRate) throw(std::out_of_range);
    
    
    /**
     * set the photon delay time
     * 
     * @param   nDelay      photon delay time in ns
     */
    void set_photon_delay(double nDelay ) throw(std::out_of_range);
    
    
    /**
     * set the sync pulse delay time
     * 
     * @param   nDelay      sync pulse delay time in ns 
     */
    void set_sync_delay(double nDelay) throw(std::out_of_range);


    /**
     * get the sync pulse delay time
     * 
     * @return the sync pulse delay time in ns
     */
    double sync_delay() { return m_delay_line_sync.get_delay_time(); }

    
    /**
     * write out all parameters of this event handler and its child event handlers
     * 
     * @param   cStream     the stream to write to
     */
    void write_parameters(std::ofstream & cStream);

    
private:
    
    
    // TODO
    void update_absorption_coefficient() {
        if (m_bLoss) m_fiber_quantum.set_absorption_coefficient(m_nAbsorptionCoefficient);
        else m_fiber_quantum.set_absorption_coefficient(0.0);
    }
    
    
    double m_nAbsorptionCoefficient;        /**< absorption coefficient in dB/km */
    double m_nLength;                       /**< length of the fiber in km */
    bool m_bLoss;                           /**< transmission loss */
    
    delay_line m_delay_line;                     /**< the photon delay line */
    delay_line m_delay_line_sync;                /**< the sync pulse delay line */
    fiber_quantum m_fiber_quantum;               /**< the quantum optical fiber */
    fiber_sync m_fiber_sync;                     /**< the sync transmission fiber */
    noise_photon_source m_noise_photon_source;   /**< the noise photon source */
};


}
}

#endif

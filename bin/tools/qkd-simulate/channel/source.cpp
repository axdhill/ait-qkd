/*
 * source.cpp
 *
 * implementation of a quantum source
 *
 * Author: Mario Kahn
 *         Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *         Philipp Grabenweger
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


// ------------------------------------------------------------
// incs

#include <fstream>
#include <iostream>

// ait
#include "channel_event_manager.h"
#include "random_functions.h"
#include "photon_pair_manager.h"
#include "ttm.h"
#include "source.h"


using namespace qkd::simulate;


// -----------------------------------------------------------------
// code


/** 
 * ctor
 */
source::source() : m_bMultiPhotons(false), m_nMultiPhotonRate(0.0), m_nPhotonRate(1000000.0), m_nSignalErrorProbablity(0.05) {
}


/**
 * add the next source event to event queue
 */
void source::add_next_source_event() {
    
    if (m_nPhotonRate > 0.0) { 
        
        // only generate source photons if photon rate > 0
        event ev;
        
        ev.ePriority = event::priority::NORMAL;
        ev.eType = event::type::PHOTON;
        ev.cDestination = this;
        ev.cSource = this;
        
        // assume an exponential distribution of the time 
        // between source photon generation events
        ev.nTime = manager()->get_time() + static_cast<int64_t>(random_functions::random_exponential(1.0 / (ttm::RESOLUTION * m_nPhotonRate)));

        manager()->add_event(ev);
    }
}


/**
 * handle a channel event. 
 * 
 * @param   cEvent      the channel event to be handled
 */
void source::handle(event const & cEvent) {

    switch (cEvent.eType) {
        
    case event::type::INIT: 
        
        // simulation initialization
        add_next_source_event();
        break;
        
    case event::type::PHOTON: 
    
        { 
            // photon generation event
            event cEventNew;
            photon_pair php;
            uint64_t php_id;
            
            php.eStateA = photon_state::ENTANGLED;
            php.eStateB = photon_state::ENTANGLED;
            php.nEntanglementError = m_nSignalErrorProbablity;
            
            try { 
                
                // try block because photon pair insertion could fail due to key collision
                php_id = pp_manager()->insert(php);
                
                cEventNew.ePriority = event::priority::NORMAL;
                cEventNew.eType = event::type::PHOTON;
                cEventNew.cDestination = parent();
                cEventNew.cSource = this;
                cEventNew.nTime = manager()->get_time();
                cEventNew.cData.m_nPhotonPairId = php_id;

                manager()->add_event(cEventNew);
            }
            catch(std::runtime_error & e) {
                std::cerr << "exception caught: std::runtime_error in source::handle_event: " << e.what() << std::endl;
            }

            add_next_source_event();
        }
        break;
    
    default:
        break;
    }
}

    
/**
 * set multi photon rate in [0 - 1 0000 Hz]
 * 
 * @param   nMultiPhotonRate        the new multphoton rate
 */
void source::set_multi_photon_rate(double nMultiPhotonRate) throw(std::out_of_range) { 
    if (nMultiPhotonRate < 0.0 || nMultiPhotonRate > 10000.0) throw std::out_of_range("source::set_multi_photon_rate: nMultiPhotonRate"); 
    m_nMultiPhotonRate = nMultiPhotonRate;
}


/**
 * set source photon rate in [0 - 10 000 000 Hz]
 * 
 * @param   nPhotonRate     the new source photon rate 
 */
void source::set_photon_rate(double nPhotonRate) throw(std::out_of_range) { 
    if (nPhotonRate < 0.0 || nPhotonRate > 10000000.0) throw std::out_of_range("source::set_photon_rate: nPhotonRate"); 
    m_nPhotonRate = nPhotonRate;
}


/**
 * sets source signal error probability in [0 - 100 %]
 * 
 * @param   nSignalErrorProbablity      the new signal/error probability
 */
void source::set_signal_error_probability(double nSignalErrorProbablity) throw(std::out_of_range) {
    if (nSignalErrorProbablity < 0.0 || nSignalErrorProbablity > 100.0) throw std::out_of_range("source::set_signal_error_probability: nSignalErrorProbablity");
    m_nSignalErrorProbablity = nSignalErrorProbablity / 100.0; // P.G.: added / 100.0
}


/**
 * write out all parameters of this event handler and its child event handlers
 * 
 * @param   cStream     the stream to write to
 */
void source::write_parameters(std::ofstream & cStream) {

    cStream << "NAME: " << get_name() << std::endl;
    cStream << "m_bMultiPhotons: " << m_bMultiPhotons << std::endl;
    cStream << "m_nMultiPhotonRate: " << m_nMultiPhotonRate << std::endl;
    cStream << "m_nPhotonRate: " << m_nPhotonRate << std::endl;
    cStream << "m_nSignalErrorProbablity: " << m_nSignalErrorProbablity << std::endl;
    cStream << std::endl;
}


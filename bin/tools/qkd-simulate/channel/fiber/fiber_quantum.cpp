/*
 * fiber_quantum.cpp
 * 
 * Implementation of a quantum fiber describing photon transport
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


// ------------------------------------------------------------
// incs

#include <fstream>

#include <math.h>

// ait
#include "fiber_quantum.h"
#include "../random_functions.h"
#include "../channel_event_manager.h"
#include "../photon_pair_manager.h"


using namespace qkd::simulate;


// -----------------------------------------------------------------
// code


/**
 * handle a channel event. 
 * 
 * @param   cEvent      the channel event to be handled
 */
void fiber_quantum::handle(event const & cEvent) {

    switch (cEvent.eType) {
        
    case event::type::PHOTON: 
        
        // incoming photon
        if (random_functions::random_uniform() < m_nTransmissionProbability || m_nTransmissionProbability == 1.0) {
            
            // photon has not been absorbed by fiber
            event cEventNew;
            
            cEventNew.ePriority = event::priority::NORMAL;
            cEventNew.eType = event::type::PHOTON;
            cEventNew.cDestination = parent();
            cEventNew.cSource = this;
            cEventNew.nTime = manager()->get_time();
            cEventNew.cData.m_nPhotonPairId = cEvent.cData.m_nPhotonPairId;
            
            // forward event to fiber
            manager()->add_event(cEventNew); 
        }
        else { 
            
            // photon is absorbed by fiber
            photon_pair & php = pp_manager()->get(cEvent.cData.m_nPhotonPairId);
            php.eStateB = photon_state::ABSORBED;
            if (php.eStateA == photon_state::ABSORBED) {
                
                // if both photons belonging to the photon pair are in absorbed state now, 
                // the photon pair must be removed
                pp_manager()->remove(cEvent.cData.m_nPhotonPairId);
            }
        }
        break;
    
    default:
        break;
    }
}


/**
 * write out all parameters of this event handler and its child event handlers
 * 
 * @param   cStream     the stream to write to
 */
void fiber_quantum::write_parameters(std::ofstream & cStream) {

    cStream << "NAME: " << get_name() << std::endl;
    cStream << "m_nAbsorptionCoefficient: " << m_nAbsorptionCoefficient << std::endl;
    cStream << "m_nLength: " << m_nLength << std::endl;
    cStream << "m_nTransmissionProbability: " << m_nTransmissionProbability << std::endl;
    cStream << std::endl;
}

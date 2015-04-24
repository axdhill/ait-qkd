/*
 * detector_optics.cpp
 * 
 * Implementation of detector_optics describing the optical pathway of photon detection in the BB84 protocol
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


// ------------------------------------------------------------
// incs

#include <fstream>

// ait
#include "../channel_event_manager.h"
#include "../photon_pair_manager.h"
#include "../random_functions.h"
#include "detector_optics.h"


using namespace qkd::simulate;


// ------------------------------------------------------------
// initializations


const int detector_optics::det_num_ortho[4] = {1, 0, 3, 2};
const int detector_optics::det_num[5][4] = { 
    {0, 1, 2, 3},
    {0, 0, 2, 3},
    {1, 1, 2, 3},
    {0, 1, 2, 2},
    {0, 1, 3, 3} 
};
                                      

// ------------------------------------------------------------
// code


/**
 * handle a channel event. 
 * 
 * @param   cEvent      the channel event to be handled
 */
void detector_optics::handle(event const & cEvent) {

    switch (cEvent.eType) {
        
    case event::type::PHOTON: 
    
        { 
            // incoming photon event
            photon_state * phs_here;
            photon_state * phs_there;
            
            photon_pair & php = pp_manager()->get(cEvent.cData.m_nPhotonPairId);
            if (m_bAlice) {
                phs_here = &(php.eStateA);
                phs_there = &(php.eStateB);
            }
            else {
                phs_here = &(php.eStateB);
                phs_there = &(php.eStateA);
            }
            
            if ((random_functions::random_uniform() < m_nDetectProbability || m_nDetectProbability == 1.0) && (*phs_here != photon_state::ABSORBED)) { 
                
                // if photon has not already been absorbed and photon is detected now here
                event cEventNew;
                int det_num_index = 0;
                int det_num_here;
                int det_num_there;
                int nrand = static_cast<int>(random_functions::random_uniform_int(4));
                
                if (*phs_here == photon_state::NONPOLARIZED || *phs_here == photon_state::ENTANGLED) {
                    det_num_index = 0;
                }
                else 
                if (*phs_here == photon_state::HORIZONTAL) {
                    det_num_index = 1;
                }
                else 
                if (*phs_here == photon_state::VERTICAL) {
                    det_num_index = 2;
                }
                else 
                if (*phs_here == photon_state::PLUS) {
                    det_num_index = 3;
                }
                else 
                if (*phs_here == photon_state::MINUS) {
                    det_num_index = 4;
                }
                det_num_here = det_num[det_num_index][nrand];
                
                switch (det_num_here) {
                    
                case 0:
                    cEventNew.cData.m_ePhotonState = photon_state::HORIZONTAL;
                    break;
                    
                case 1:
                    cEventNew.cData.m_ePhotonState = photon_state::VERTICAL;
                    break;
                
                case 2:
                    cEventNew.cData.m_ePhotonState = photon_state::PLUS;
                    break;
                
                case 3:
                    cEventNew.cData.m_ePhotonState = photon_state::MINUS;
                    break;
                }
                
                cEventNew.ePriority = event::priority::NORMAL;
                cEventNew.eType = event::type::PHOTON;
                cEventNew.cDestination = parent();
                cEventNew.cSource = this;
                cEventNew.nTime = manager()->get_time();
                
                // add photon event to detector containing measured photon state information
                manager()->add_event(cEventNew); 
                
                if (*phs_here == photon_state::ENTANGLED) { 
                    
                    // for an entangled state, the photon state of the second photon must also be 
                    // set accordingly after detection of the first photon here
                    
                    if (random_functions::random_uniform() < php.nEntanglementError || php.nEntanglementError == 1.0) {
                        // set wrong (=same) photon polarization
                        det_num_there = det_num_here; 
                    }
                    else {
                        // set correct (=orthogonal) photon polarization
                        det_num_there = det_num_ortho[det_num_here]; 
                    }
                    
                    switch (det_num_there) {
                        
                    case 0:
                        *phs_there = photon_state::HORIZONTAL;
                        break;
                        
                    case 1:
                        *phs_there = photon_state::VERTICAL;
                        break;
                        
                    case 2:
                        *phs_there = photon_state::PLUS;
                        break;
                        
                    case 3:
                        *phs_there = photon_state::MINUS;
                        break;
                    }
                }
            }
            
            // first photon has been absorbed now
            *phs_here = photon_state::ABSORBED; 
            
            if (*phs_there == photon_state::ENTANGLED) { 
                // an entangled state is broken after absorption of first photon
                // assume that second photon is unpolarized now, which means that all directions of
                // polarization will be equally probable when the second photon is detected
                *phs_there = photon_state::NONPOLARIZED; 
            }
            else 
            if (*phs_there == photon_state::ABSORBED) {
                // if now both photons belonging to this photon pair are 
                // in absorbed state, the photon pair must be removed
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
void detector_optics::write_parameters(std::ofstream & cStream) {

    cStream << "NAME: " << get_name() << std::endl;
    cStream << "m_bAlice: " << m_bAlice << std::endl;
    cStream << "m_nDetectProbability: " << m_nDetectProbability << std::endl;
    cStream << "m_nEfficiency: " << m_nEfficiency << std::endl;
    cStream << "m_nLoss: " << m_nLoss << std::endl;
    cStream << std::endl;
}


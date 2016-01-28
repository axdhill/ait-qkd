/*
 * photon_pair_manager.cpp
 * 
 * Implementation of a photon pair manager
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

#include <assert.h>

// ait
#include "channel_event_manager.h"
#include "photon_pair_manager.h"


using namespace qkd::simulate;


// -----------------------------------------------------------------
// code


/**
 * get photon pair with given identifier from photon pair map
 * 
 * If the identifier does not match any photon pair in 
 * the photon pair map, a std::out_of_range exception is thrown.
 *
 * @param   nId         identifier of the photon pair to get 
 * @return  a reference to the photon pair with the given identifier.
 */
photon_pair& photon_pair_manager::get(uint64_t nId) throw(std::out_of_range) {
    return m_cPhotonPairs.at(nId);
}


/**
 * initialize simulation
 */
void photon_pair_manager::init_simulation() {
    m_cPhotonPairs.clear();
    m_nNextIdLow = 0;
}


/**
 * insert photon pair into photon pair map
 * 
 * In case of an error due to a key collision
 * a std::runtime_error exception is thrown
 * 
 * @param   cPhotonPair     photon pair to insert
 * @return  identifier of the photon pair in the map if the insertion was successful.
 */
uint64_t photon_pair_manager::insert(photon_pair const & cPhotonPair) throw(std::runtime_error) {
    
    assert(m_cManager != nullptr);
    
    uint64_t id = 0ULL;
    
    id = static_cast<uint64_t>(m_cManager->get_time());
    id &= 0x00000000FFFFFFFF;
    id <<= 32;
    id |= static_cast<uint64_t>(m_nNextIdLow);
    
    m_nNextIdLow++;
    
    if (m_cPhotonPairs.count(id) > 0) {
        throw std::runtime_error("photon_pair_manager::insert: photon pair insertion failed due to key collision");
    }
    m_cPhotonPairs[id] = cPhotonPair;
    
    return id;
}


/**
 * remove photon pair from photon pair map
 * 
 * @param   nId     identifier of the photon pair to remove
 */
void photon_pair_manager::remove(uint64_t id) {
    m_cPhotonPairs.erase(id);
}

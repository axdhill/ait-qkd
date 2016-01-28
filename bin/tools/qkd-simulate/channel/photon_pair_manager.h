/*
 * photon_pair_manager.h
 * 
 * Declaration of a photon pair manager
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

#ifndef __QKD_QKD_SIMULATE_PHOTON_PAIR_MANAGER_H_
#define __QKD_QKD_SIMULATE_PHOTON_PAIR_MANAGER_H_


// ------------------------------------------------------------
// include

#include <unordered_map>

// ait
#include "photon_pair.h"


// -----------------------------------------------------------------
// decl

namespace qkd {
    
namespace simulate {

    
// fwd. decl.
class channel_event_manager;


/**
 * photon pair manager
 */
class photon_pair_manager
{
    
public:
    
    
    /**
     * get photon pair with given identifier from photon pair map
     * 
     * If the identifier does not match any photon pair in 
     * the photon pair map, a std::out_of_range exception is thrown.
     *
     * @param   nId         identifier of the photon pair to get 
     * @return  a reference to the photon pair with the given identifier.
     */
    photon_pair & get(uint64_t nId) throw(std::out_of_range);
    
    
    /**
     * initialize simulation
     */
    void init_simulation();
    
    
    /**
     * insert photon pair into photon pair map
     * 
     * In case of an error due to a key collision
     * a std::runtime_error exception is thrown
     * 
     * @param   cPhotonPair     photon pair to insert
     * @return  identifier of the photon pair in the map if the insertion was successful.
     */
    uint64_t insert(photon_pair const & cPhotonPair) throw(std::runtime_error);

    
    /**
     * get the manager of this
     * 
     * @return  the event manager of this one
     */
    channel_event_manager * manager() { return m_cManager; };
    
    
    /**
     * get the manager of this
     * 
     * @return  the event manager handler of this one
     */
    channel_event_manager const * manager() const { return m_cManager; };
    
    
    /**
     * remove photon pair from photon pair map
     * 
     * @param   nId     identifier of the photon pair to remove
     */
    void remove(uint64_t nId);
    
    
    /**
     * set the channel event manager
     * 
     * @param   cManager            the new channel event manager
     */
    void set_manager(channel_event_manager * cManager) { m_cManager = cManager; }
    
    
private:
    
    
    /**
     * pointer to the channel event manager 
     */
    channel_event_manager * m_cManager;     
    
    
    
    /**
     * low part of the next photon pair identifier to be assigned to a newly generated photon pair 
     */
    uint32_t m_nNextIdLow; 

    
    /**
     * unordered photon pair map 
     */
    std::unordered_map<uint64_t, photon_pair> m_cPhotonPairs; 
};

}
}

#endif

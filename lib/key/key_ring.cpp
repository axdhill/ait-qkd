/*
 * key_ring.cpp
 * 
 * Implements the ring of QKD keys
 *
 * Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *
 * Copyright (C) 2012-2016 AIT Austrian Institute of Technology
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

// ait
#include <qkd/key/key_ring.h>

using namespace qkd::key;


// ------------------------------------------------------------
// code



/**
 * add a key to the key ring
 * 
 * depending on the key_ring's key size the given
 * key will be split into enough keys to
 * match the key_ring's specs.
 * 
 * Any key added will contain a new key id.
 * 
 * @param   cKey        the key to add
 */
void key_ring::push_back(qkd::key::key const & cKey) {
    
    // do not push keys into an NULL-key ring
    if (key_size() == 0) return;
    
    unsigned char const * cNewKeyData = cKey.data().get();
    uint64_t nSizeOfNewKey = cKey.size();
    
    // ensure that at least one last key is present in the ring
    if (cKey.size() && !size()) {
        std::vector<qkd::key::key>::push_back(qkd::key::key(id(), qkd::utility::memory(0)));
        qkd::key::key cLastKey = back();
        m_nId++;
    }

    // add slices of the new key
    while (nSizeOfNewKey) {
    
        // get the last key
        qkd::key::key & cLastKey = back();
        uint64_t nBytesToAdd = std::min((uint64_t)(key_size() - cLastKey.size()), nSizeOfNewKey);
        
        // last key full: add a new, empty one and continue
        if (!nBytesToAdd) {
            std::vector<qkd::key::key>::push_back(qkd::key::key(id(), qkd::utility::memory(0)));
            m_nId++;
            continue;
        }
        
        // add bytes to the last key
        cLastKey.data().resize(cLastKey.size() + nBytesToAdd);
        memcpy(cLastKey.data().get() + cLastKey.data().size() - nBytesToAdd, cNewKeyData, nBytesToAdd);
        nSizeOfNewKey -= nBytesToAdd;
        cNewKeyData += nBytesToAdd;
    }
}

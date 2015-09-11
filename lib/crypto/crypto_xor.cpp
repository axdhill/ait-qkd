/*
 * crypto_xor.cpp
 * 
 * implement the XOR encryption
 *
 * Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *
 * Copyright (C) 2012-2015 AIT Austrian Institute of Technology
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
#include <qkd/common_macros.h>
#include <qkd/crypto/context.h>

#include "crypto_xor.h"

using namespace qkd::crypto;
using namespace qkd::key;
using namespace qkd::utility;


// ------------------------------------------------------------
// code


/**
 * ctor
 * 
 * @param   cKey        the initial key
 * @throws  context_init
 * @throws  context_wrong_key
 */
crypto_xor::crypto_xor(qkd::key::key const & cKey) : context(cKey) {
}


/**
 * add another crypto context
 *
 * @param   cContext        the crypto context to add
 * @throws  context_final, if the algorithm has finished and does not allow another addition
 */
void crypto_xor::add_internal(UNUSED qkd::crypto::crypto_context const & cContext) {
    throw std::logic_error("xor crypto context cannot be added");
}


/**
 * add a memory BLOB to the algorithm
 *
 * @param   cMemory         memory block to be added
 * @throws  context_final, if the algorithm has finished and does not allow another addition
 */
void crypto_xor::add_internal(qkd::utility::memory const & cMemory) {
    
    // add the BLOB to the end of the existing data
    uint64_t nOldSize = m_cData.size();
    m_cData.resize(m_cData.size() + cMemory.size());
    memcpy(m_cData.get() + nOldSize, cMemory.get(), cMemory.size());
}


/**
 * finalize the algorithm and get the tag
 * 
 * @param   cKey        the key used to finalize the algorithm
 * @return  a memory BLOB representing the tag
 * @throws  context_wrong_key
 */
qkd::utility::memory crypto_xor::finalize_internal(qkd::key::key const & cKey) {
    
    // check if the key suits our needs
    if (!is_valid_final_key(cKey)) throw qkd::crypto::context::context_wrong_key();

    // prepare the result
    qkd::utility::memory cFinalTag(m_cData.size());
    
    // walk over the memory blob so far and xor
    uint64_t * pKey = (uint64_t *)cKey.data().get();
    uint64_t * pSource = (uint64_t *)m_cData.get();
    uint64_t * pDestination = (uint64_t *)cFinalTag.get();
    uint64_t nCount = m_cData.size();

    // this will automatically pick the proper word size of the current CPU
    while (nCount > sizeof(uint64_t)) {
        *pDestination = *pSource ^ *pKey;
        pDestination++;
        pSource++;
        pKey++;
        nCount -= sizeof(uint64_t);
    }
    
    // treat the last bytes
    unsigned char * pKey_Last_Bytes = (unsigned char *)pKey;
    unsigned char * pSource_Last_Bytes = (unsigned char *)pSource;
    unsigned char * pDestination_Last_Bytes = (unsigned char *)pDestination;
    for (uint64_t i = 0; i < nCount; i++) {
        *pDestination_Last_Bytes = *pSource_Last_Bytes ^ *pKey_Last_Bytes;
        pDestination_Last_Bytes++;
        pSource_Last_Bytes++;
        pKey_Last_Bytes++;
    }
    
    return cFinalTag;
}


/**
 * check if the given key is suitable as final key
 * 
 * @param   cKey        the key candidate
 * @return  true, if this cKey can be used in the finalize call
 */
bool crypto_xor::is_valid_final_key(qkd::key::key const & cKey) const {
    return (cKey.size() >= m_cData.size());
}


/**
 * return the scheme string (at the current state) of this context
 * 
 * @return  a string holding the current scheme string
 */
qkd::crypto::scheme crypto_xor::scheme_internal() const {
    return qkd::crypto::scheme("xor");
}


/**
 * sets the state as specified in the memory block
 * 
 * @param   cMemory         the BLOB holding the state data
 * @throws  context_init
 */
void crypto_xor::set_state_internal(UNUSED qkd::utility::memory const & cMemory) {
}


/**
 * return the current state of the crypto context
 * 
 * @return  a memory BLOB defining the current state
 */
qkd::utility::memory crypto_xor::state_internal() const {
    
    // no internal state for XOR
    static qkd::utility::memory cNullState;
    return cNullState;
}


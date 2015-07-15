/*
 * crypto_null.cpp
 * 
 * implement the NULL encryption/authentication
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

#include "crypto_null.h"

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
crypto_null::crypto_null(qkd::key::key const & cKey) : context(cKey) {
}


/**
 * add another crypto context
 *
 * @param   cContext        the crypto context to add
 * @throws  context_final, if the algorithm has finished and does not allow another addition
 */
void crypto_null::add_internal(UNUSED qkd::crypto::crypto_context const & cContext) {
}


/**
 * add a memory BLOB to the algorithm
 *
 * The NULL context does not add
 *
 * @param   cMemory         memory block to be added
 * @throws  context_final, if the algorithm has finished and does not allow another addition
 */
void crypto_null::add_internal(UNUSED qkd::utility::memory const & cMemory) {
}


/**
 * finalize the algorithm and get the tag
 * 
 * @param   cKey        the key used to finalize the algorithm
 * @return  a memory BLOB representing the tag
 * @throws  context_wrong_key
 */
qkd::utility::memory crypto_null::finalize_internal(UNUSED qkd::key::key const & cKey) {
    return m_cData;
}


/**
 * check if the given key is suitable as final key
 * 
 * @param   cKey        the key candidate
 * @return  true, if this cKey can be used in the finalize call
 */
bool crypto_null::is_valid_final_key(UNUSED qkd::key::key const & cKey) const {
    return true;
}


/**
 * return the scheme string (at the current state) of this context
 * 
 * @return  a string holding the current scheme string
 */
qkd::crypto::scheme crypto_null::scheme_internal() const {
    return qkd::crypto::scheme("null");
}


/**
 * sets the state as specified in the memory block
 * 
 * @param   cMemory         the BLOB holding the state data
 */
void crypto_null::set_state_internal(UNUSED qkd::utility::memory const & cMemory) {
}


/**
 * return the current state of the crypto context
 * 
 * @return  a memory BLOB defining the current state
 * @throws  context_init
 */
qkd::utility::memory crypto_null::state_internal() const {
    static qkd::utility::memory cNullState;
    return cNullState;
}


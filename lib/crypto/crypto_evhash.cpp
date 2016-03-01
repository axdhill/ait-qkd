/*
 * crypto_evhash.cpp
 * 
 * implement the evaluation hash authentication
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

#include <map>
#include <sstream>

// ait
#include <qkd/common_macros.h>
#include <qkd/crypto/context.h>
#include <qkd/crypto/engine.h>
#include <qkd/utility/buffer.h>

#include "evhash.h"

#include "crypto_evhash.h"

using namespace qkd::crypto;


// ------------------------------------------------------------
// decl


/**
 * this class neatly switches GF2 implementations
 */
class qkd::crypto::crypto_evhash::evhash_data {


public:


    /**
     * ctor
     *
     * @param   cKey        key to work on (--> alpha)
     */
    explicit evhash_data(qkd::key::key const & cKey) : m_cKey(cKey) {
        m_cEvhash = evhash_abstract::create(cKey);
    }


    /** 
     * the evhash instance
     */
    evhash m_cEvhash;


    /**
     * the key used
     */
    qkd::key::key m_cKey;

};


// ------------------------------------------------------------
// code


/**
 * ctor
 * 
 * @param   cKey        the initial key
 */
crypto_evhash::crypto_evhash(qkd::key::key const & cKey) : context(cKey) {
    if (!is_valid_input_key(cKey)) throw std::invalid_argument("invalid init key for evhash");
    d = std::shared_ptr<qkd::crypto::crypto_evhash::evhash_data>(new qkd::crypto::crypto_evhash::evhash_data(cKey));
}


/**
 * add another crypto context
 *
 * @param   cContext        the crypto context to add
 * @throws  context_final, if the algorithm has finished and does not allow another addition
 */
void crypto_evhash::add_internal(qkd::crypto::crypto_context const & cContext) {
    
    if (name() != cContext->name()) {
        throw std::invalid_argument("can't add different crypto contexts algorithms");
    }
    if (init_key().size() != cContext->init_key().size()) {
        throw std::invalid_argument("can't add evaluation hash of different tag size");
    }
    if (!init_key().is_equal(cContext->init_key())) {
        throw std::invalid_argument("can't add evaluation hash of different keys");
    }

    d->m_cEvhash->finalize();
    crypto_evhash * cEvHashContext = dynamic_cast<crypto_evhash *>(cContext.get());
    
    // t_n(this) = t_n-1(this) * k^(m)
    // t_n(this) = t_n-1(this) + t_m(other)
    qkd::utility::memory cTagOther = cEvHashContext->d->m_cEvhash->finalize();
    d->m_cEvhash->times(cEvHashContext->d->m_cEvhash->blocks());
    d->m_cEvhash->add(cTagOther);
}


/**
 * add a memory BLOB to the algorithm
 *
 * @param   cMemory         memory block to be added
 * @throws  context_final, if the algorithm has finished and does not allow another addition
 */
void crypto_evhash::add_internal(qkd::utility::memory const & cMemory) {
    d->m_cEvhash->update(cMemory);
}


/**
 * finalize the algorithm and get the tag
 * 
 * @param   cKey        the key used to finalize the algorithm
 * @return  a memory BLOB representing the tag
 * @throws  context_wrong_key
 */
qkd::utility::memory crypto_evhash::finalize_internal(qkd::key::key const & cKey) {
    
    // check if the key suits our needs
    if (!is_valid_final_key(cKey)) throw std::invalid_argument("invalid final key for evhash");

    // get the final tag
    qkd::utility::memory cHashTag;
    cHashTag = d->m_cEvhash->finalize();
    
    // encrypt it with the final key
    qkd::utility::memory cFinalTag;
    qkd::crypto::crypto_context cXOR = qkd::crypto::engine::create("xor", qkd::key::key());
    cXOR << cHashTag;
    cFinalTag = cXOR->finalize(cKey);
    
    return cFinalTag;
}


/**
 * get the size of the final key in bytes
 * 
 * @return  the size of the final key or 0 if inappropriate
 */
uint64_t crypto_evhash::final_key_size_internal() const {
    return d->m_cEvhash->block_size();
}


/**
 * get the size of the init key
 * 
 * @return  the size of the init key or 0 if inappropriate
 */
uint64_t crypto_evhash::init_key_size_internal() const {
    return d->m_cEvhash->block_size();
}


/**
 * check if the given key is suitable as final key
 * 
 * @param   cKey        the key candidate
 * @return  true, if this cKey can be used in the finalize call
 */
bool crypto_evhash::is_valid_final_key(qkd::key::key const & cKey) const {
    return cKey.size() == d->m_cEvhash->block_size();
}


/**
 * check if the given key is suitable for an init key
 * 
 * @return  true, if the key can be used as init key
 */
bool crypto_evhash::is_valid_input_key(qkd::key::key const & cKey) {
    return ((cKey.size() == 32/8) 
            || (cKey.size() == 64/8)  
            || (cKey.size() == 96/8)  
            || (cKey.size() == 128/8)  
            || (cKey.size() == 256/8));
}


/**
 * return the scheme string (at the current state) of this context
 * 
 * @return  a string holding the current scheme string
 */
qkd::crypto::scheme crypto_evhash::scheme_internal() const {
    
    std::stringstream ss;
    ss << "evhash-";
    ss << d->m_cEvhash->block_size() * 8;
    ss << ":";
    ss << d->m_cKey.data().as_hex();
    ss << ":";
    ss << d->m_cEvhash->state().as_hex();
    
    return qkd::crypto::scheme(ss.str());
}


/**
 * get the size of the result of a computation
 * 
 * if the context is an authentication context, this
 * is the size of the authentication-tag.
 * 
 * if the return is 0, the result may have any size
 * 
 * @return  expected size of the computation result (or 0 for any size)
 */
uint64_t crypto_evhash::result_size_internal() const { 
    return d->m_cEvhash->block_size();
}


/**
 * sets the state as specified in the memory block
 * 
 * @param   cMemory         the BLOB holding the state data
 */
void crypto_evhash::set_state_internal(qkd::utility::memory const & cMemory) {
    qkd::utility::buffer cStateBuffer(cMemory);
    d->m_cEvhash->set_state(cStateBuffer);
}


/**
 * return the current state of the crypto context
 * 
 * @return  a memory BLOB defining the current state
 * @throws  context_init
 */
qkd::utility::memory crypto_evhash::state_internal() const {
    return d->m_cEvhash->state();
}


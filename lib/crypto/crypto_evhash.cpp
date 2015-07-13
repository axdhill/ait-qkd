/*
 * crypto_evhash.cpp
 * 
 * implement the evaluation hash authentication
 *
 * Autor: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
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

#include <map>
#include <sstream>

// ait
#include <qkd/common_macros.h>
#include <qkd/crypto/context.h>
#include <qkd/crypto/engine.h>
#include <qkd/utility/buffer.h>

#include "crypto_evhash.h"
#include "evhash.h"

using namespace qkd::crypto;
using namespace qkd::key;
using namespace qkd::utility;



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
     * @param   nBits       size of GF2
     * @param   cKey        key to work on (--> alpha)
     */
    explicit evhash_data(unsigned int nBits, qkd::key::key const & cKey) : m_cKey(cKey) {

        switch (nBits) {

        case 32:
            
            m_cEvhash = new evhash<32>(cKey);
            break;

        case 64:

            m_cEvhash = new evhash<64>(cKey);
            break;

        case 96:
            
            m_cEvhash = new evhash<96>(cKey);
            break;

        case 128:
            
            m_cEvhash = new evhash<128>(cKey);
            break;

        case 256:
            
            m_cEvhash = new evhash<256>(cKey);
            break;

        default:

            throw std::invalid_argument("evaluation hash bit width not implemented.");

        }
    }


    /**
     * dtor
     */
    ~evhash_data() {
        delete m_cEvhash;
    }

    /**
     * add a memory BLOB to the algorithm
     *
     * This transforms the tag stored in this object
     *
     * @param   cMemory         memory block to be added
     */
    void add(qkd::utility::memory const & cMemory) { m_cEvhash->add(cMemory); }


    /**
     * bit width of GF2
     *
     * @return  bit width of GF2
     */
    unsigned int bits() const { return m_cEvhash->bits(); }


    /**
     * number of blocks added so far
     *
     * @return  number of blocks in the tag
     */
    uint64_t blocks() const { return m_cEvhash->blocks(); }


    /**
     * size of a single block
     *
     * @return  size of a single block in bytes
     */
    unsigned int block_size() const { return m_cEvhash->block_size(); }


    /**
     * get the key used for this evhash
     *
     * @return  the key for this evhash
     */
    qkd::key::key const & key() const { return m_cKey; };


    /**
     * get the final tag
     *
     * @return  the final tag
     */
    qkd::utility::memory finalize() { return m_cEvhash->finalize(); }


    /**
     * set the current state
     *
     * @param   cState      the state serialized
     */
    void set_state(qkd::utility::buffer & cState) { m_cEvhash->set_state(cState); }


    /**
     * get the current state
     *
     * @return  serialized current state
     */
    qkd::utility::buffer state() const { return m_cEvhash->state(); }


    /**
     * get the current tag
     *
     * @return  the current tag
     */
    qkd::utility::memory tag() const { return m_cEvhash->tag(); }


private:


    /** 
     * the evhash instance
     */
    evhash_abstract * m_cEvhash;


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
 * @throws  context_init
 * @throws  context_wrong_key
 */
crypto_evhash::crypto_evhash(qkd::key::key const & cKey) : context(cKey) {
    if (!is_valid_input_key(cKey)) throw qkd::crypto::context::context_wrong_key();
    d = boost::shared_ptr<qkd::crypto::crypto_evhash::evhash_data>(new qkd::crypto::crypto_evhash::evhash_data(cKey.size() * 8, cKey));
}


/**
 * add another crypto context
 *
 * @param   cContext        the crypto context to add
 * @throws  context_final, if the algorithm has finished and does not allow another addition
 */
void crypto_evhash::add_internal(UNUSED qkd::crypto::crypto_context const & cContext) {
    throw std::logic_error("evhash crypto context cannot be added");
}


/**
 * add a memory BLOB to the algorithm
 *
 * @param   cMemory         memory block to be added
 * @throws  context_final, if the algorithm has finished and does not allow another addition
 */
void crypto_evhash::add_internal(qkd::utility::memory const & cMemory) {
    d->add(cMemory);
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
    if (!is_valid_final_key(cKey)) throw qkd::crypto::context::context_wrong_key();

    // get the final tag
    qkd::utility::memory cHashTag;
    cHashTag = d->finalize();
    
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
 * @return  the size of the final key or 0 if inapprobiate
 */
uint64_t crypto_evhash::final_key_size_internal() const {
    return d->block_size();
}


/**
 * get the size of the init key
 * 
 * @return  the size of the init key or 0 if inapprobiate
 */
uint64_t crypto_evhash::init_key_size_internal() const {
    return d->block_size();
}


/**
 * check if the given key is suitable as final key
 * 
 * @param   cKey        the key candidate
 * @return  true, if this cKey can be used in the finalize call
 */
bool crypto_evhash::is_valid_final_key(qkd::key::key const & cKey) const {
    return cKey.size() == d->block_size();
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
    ss << d->block_size() * 8;
    ss << ":";
    ss << d->key().data().as_hex();
    ss << ":";
    ss << d->state().as_hex();
    
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
    return d->block_size();
}


/**
 * sets the state as specified in the memory block
 * 
 * @param   cMemory         the BLOB holding the state data
 */
void crypto_evhash::set_state_internal(qkd::utility::memory const & cMemory) {
    qkd::utility::buffer cStateBuffer(cMemory);
    d->set_state(cStateBuffer);
}


/**
 * return the current state of the crypto context
 * 
 * @return  a memory BLOB defining the current state
 * @throws  context_init
 */
qkd::utility::memory crypto_evhash::state_internal() const {
    return d->state();
}


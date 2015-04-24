/*
 * crypto_umac.cpp
 * 
 * implement the UMAC authentication
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

#include <sstream>

// ait
#include <qkd/common_macros.h>
#include <qkd/crypto/context.h>
#include <qkd/crypto/engine.h>
#include <qkd/utility/buffer.h>

#include "crypto_umac.h"

using namespace qkd::crypto;
using namespace qkd::key;
using namespace qkd::utility;

// old crypto implmentation
#include "old/algorithm.h"
#include "old/context.h"
#include "old/utility.h"

// this will be declared by all those umac_*.c files
extern ce_algorithm uhash_32;
extern ce_algorithm uhash_64;
extern ce_algorithm uhash_96;
extern ce_algorithm uhash_128;


// ------------------------------------------------------------
// decl


/**
 * the evhash pimpl
 */
class qkd::crypto::crypto_umac::umac_data {
    
    
public:
    
    
    /**
     * ctor
     * 
     * @param   nTagSize        size of the hash tag in bits
     * @throws  context_init
     */
    umac_data(uint64_t nTagSize) : m_cAlgorithm(nullptr), m_cContext(nullptr), m_cState(nullptr), m_nTagSize(nTagSize) { 
        
        // get the algorithm reference
        switch (nTagSize) {
            
        case 128:
            m_cAlgorithm = &uhash_128;
            break;
        }
        
        // sanity check
        if (!m_cAlgorithm) throw qkd::crypto::context::context_init();
    };
    

    /**
     * dtor
     */
    ~umac_data() { 
        
        // free resources
        
        if (m_cState)  {
            if (m_cState->output) free(m_cState->output);
            m_cState->output = nullptr;
            m_cState->destroy(m_cState);
            free(m_cState);
        }
        m_cState = nullptr;
        
        if (m_cContext) m_cContext->destroy(m_cContext);  
        m_cContext = nullptr;
        
        m_cAlgorithm = nullptr;
    };
    
    
    /**
     * finalize the hash algorithm
     * 
     * @param   cTag        will receive the final tag
     */
    void finalize(qkd::utility::memory & cTag) {
        
        // sanity check
        if (!m_cState) return;
        
        char * cTagBuffer = nullptr;
        size_t nSizeOfTag = 0;
        
        // get the tag
        cTagBuffer = ce_finalize(m_cState, &nSizeOfTag);
        
        // copy to memory area
        cTag.resize(nSizeOfTag);
        memcpy(cTag.get(), cTagBuffer, nSizeOfTag);
        
        // bad design of old crypto: ownership is transfered
        free(cTagBuffer);
    };
    
    
    /**
     * return the key used to init the UMAC
     * 
     * @return  the init UMAC key
     */
    qkd::key::key const & key() const { return m_cKey; };
    
    
    /**
     * setup the context and state
     * 
     * @param   cKey        the key used to init the context and state
     * @throws  context_init, if we've already setup the thing
     */
    void setup(qkd::key::key const & cKey) {

        // sanity check
        if (!m_cAlgorithm) throw qkd::crypto::context::context_init();
        if (!(cKey.size() * 8 == tag_size())) throw qkd::crypto::context::context_init();
        m_cKey = cKey;
        
        // get up context
        if (m_cContext) throw qkd::crypto::context::context_init();
        m_cContext = m_cAlgorithm->create_context((char *)cKey.data().get(), cKey.size());
        
        // santiy check
        if (!m_cContext) throw qkd::crypto::context::context_init();
        
        // get up state
        if (m_cState) throw qkd::crypto::context::context_init();
        m_cState = m_cContext->create_state(m_cContext);

        // sanity check
        if (!m_cState) throw qkd::crypto::context::context_init();
    };
    
    
    /**
     * the old ev-hash state
     * 
     * @return  the old ev-hash state
     */
    ce_state * state() { return m_cState; };
    
    
    /**
     * the old ev-hash state
     * 
     * @return  the old ev-hash state
     */
    ce_state const * state() const { return m_cState; };
    
    
    /**
     * return the number of bits for the tag
     */
    uint64_t tag_size() const { return m_nTagSize; };
    
    
    /**
     * add some memory BLOB to the algorithm
     * 
     * @param   cMemory     memory to add
     * @param   nSize       number of bytes of memory
     * @throws  context_init, if not proper initilized
     */
    void update(char const * cMemory, uint64_t nSize) {
        
        // sanity check
        if (!cMemory) return;
        if (!nSize) return;
        if (!m_cState) throw qkd::crypto::context::context_init();
            
        ce_update(m_cState, cMemory, nSize);
    };
    
    
private:
    
    
    /**
     * the old algorithm reference
     */
    ce_algorithm* m_cAlgorithm;
    
    
    /**
     * the old ev-hash context
     */
    ce_context* m_cContext;
    

    /**
     * the key used to init the UMAC
     */
    qkd::key::key m_cKey;
    
    
    /**
     * the old ev-hash state
     */
    ce_state* m_cState;
    
    
    /**
     * number of bits managed
     */
    uint64_t m_nTagSize;
    
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
crypto_umac::crypto_umac(qkd::key::key const & cKey) : context(cKey) {
    if (!is_valid_input_key(cKey)) throw qkd::crypto::context::context_wrong_key();
    d = boost::shared_ptr<qkd::crypto::crypto_umac::umac_data>(new qkd::crypto::crypto_umac::umac_data(cKey.size() * 8));
    d->setup(cKey);
}


/**
 * add a memory BLOB to the algorithm
 *
 * @param   cMemory         memory block to be added
 * @throws  context_final, if the algorithm has finished and does not allow another addition
 */
void crypto_umac::add_internal(qkd::utility::memory const & cMemory) {
    d->update((char const *)cMemory.get(), cMemory.size());
}


/**
 * finalize the algorithm and get the tag
 * 
 * @param   cKey        the key used to finalize the algorithm
 * @return  a memory BLOB representing the tag
 * @throws  context_wrong_key
 */
qkd::utility::memory crypto_umac::finalize_internal(UNUSED qkd::key::key const & cKey) {
    
    // get the final tag
    qkd::utility::memory cFinalTag;
    d->finalize(cFinalTag);
    
    return cFinalTag;
}


/**
 * get the size of the init key
 * 
 * @return  the size of the init key or 0 if inapprobiate
 */
uint64_t crypto_umac::init_key_size_internal() const {
    return d->tag_size() / 8;
}


/**
 * check if the given key is suitable as final key
 * 
 * @param   cKey        the key candidate
 * @return  true, if this cKey can be used in the finalize call
 */
bool crypto_umac::is_valid_final_key(UNUSED qkd::key::key const & cKey) const {
    return true;
}


/**
 * check if the given key is suitable for an init key
 * 
 * @return  true, if the key can be used as init key
 */
bool crypto_umac::is_valid_input_key(qkd::key::key const & cKey) {
    return (cKey.size() == 128/8);
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
uint64_t crypto_umac::result_size_internal() const { 
    return d->tag_size();
}


/**
 * return the scheme string (at the current state) of this context
 * 
 * @return  a string holding the current scheme string
 */
qkd::crypto::scheme crypto_umac::scheme_internal() const {
    
    std::stringstream ss;
    ss << "umac-";
    ss << d->tag_size();
    ss << ":";
    ss << d->key().data().as_hex();
    ss << ":";
    ss << state().as_hex();
    
    return qkd::crypto::scheme(ss.str());
}


/**
 * sets the state as specified in the memory block
 * 
 * @param   cMemory         the BLOB holding the state data
 * @throws  context_init
 */
void crypto_umac::set_state_internal(UNUSED qkd::utility::memory const & cMemory) {

    // current UMAC implementation is too broken to let us serialize the state
    throw qkd::crypto::context::context_init();
}


/**
 * return the current state of the crypto context
 * 
 * @return  a memory BLOB defining the current state
 */
qkd::utility::memory crypto_umac::state_internal() const {
    
    // current UMAC implementation is too broken to let us serialize the state
    throw qkd::crypto::context::context_init();
}


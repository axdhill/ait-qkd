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

using namespace qkd::crypto;
using namespace qkd::key;
using namespace qkd::utility;

// old crypto implmentation
#include "old/algorithm.h"
#include "old/context.h"
#include "old/utility.h"

// this will be declared by all those evhash_*.c files
extern ce_algorithm evhash_32;
extern ce_algorithm evhash_64;
extern ce_algorithm evhash_96;
extern ce_algorithm evhash_128;
extern ce_algorithm evhash_256;


    #include "evhash.h"



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
    d->finalize();
    
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



























#if 0


/**
 * The evhash_context enables the reuse of already instantiated
 * contexts per key. If an init key has been used twice for the
 * evhash context creation, we might use the very same underlaying
 * context in the old impementation. This should speed up.
 * 
 * A databse of these context objects is maintained. Whenever an
 * init key is presented at evaluation hash context creation, we
 * check if an object already has been created in the past.
 * 
 * A refrence count is managed to indicate instantion numbers.
 * When this reference count drops to 0, we remove it from the
 * database.
 */
class evhash_context {
    
    
public:
    
    
    /**
     * init the context
     * 
     * @param   cAlgorithm      the old evhash algorithm
     * @param   cKey            the key used to init the context and state
     * @throws  context_init
     */
    evhash_context(ce_algorithm * cAlgorithm, qkd::key::key const & cKey) : m_cKey(cKey), m_nReferenceCount(1) {
        
        // sanity check
        if (!cAlgorithm) throw qkd::crypto::context::context_init();
        
        m_cContext = cAlgorithm->create_context((char *)m_cKey.data().get(), m_cKey.size());
        
        // sanity check
        if (!m_cContext) throw qkd::crypto::context::context_init();
    };
    
    
    /**
     * dtor
     */
    ~evhash_context() { 
        
        // free resources
        if (m_cContext) m_cContext->destroy(m_cContext);  
        m_cContext = nullptr;
    };
    
    
    /**
     * increase reference count
     */
    void add_reference() { m_nReferenceCount++; };
    
    
    /**
     * return the old context
     * 
     * @return  the old evhash context
     */
    ce_context * context() { return m_cContext; };
    
    
    /**
     * return the old context
     * 
     * @return  the old evhash context
     */
    ce_context const * context() const { return m_cContext; };
    
    
    /**
     * decrement reference count
     */
    void dec_reference() { m_nReferenceCount--; };
    
    
    /**
     *  key used to create the context
     */
    qkd::key::key const & key() const { return m_cKey; };
    
    
    /**
     * get current reference count
     */
    uint64_t reference_count() const { return m_nReferenceCount; };
    
    
private:
    
    
    /**
     * the old ev-hash context
     */
    ce_context * m_cContext;
    
    
    /**
     * key used to create the context
     */
    qkd::key::key m_cKey;
    
    
    /**
     * simple reference counting
     */
    uint64_t m_nReferenceCount;
    
};


/**
 * smart pointer on a evhash_context
 */
typedef boost::shared_ptr<evhash_context> evhash_context_ptr;


}
}


/**
 * Database of already instantiated evhash_contexts
 */
typedef std::map<std::string, qkd::crypto::evhash_context_ptr> evhash_context_database;
evhash_context_database g_cEvHashContextDatabase;


/**
 * the evhash pimpl
 */
class qkd::crypto::crypto_evhash::evhash_data {
    
    
public:
    
    
    /**
     * ctor
     * 
     * @param   nTagSize        size of the hash tag in bits
     * @throws  context_init
     */
    evhash_data(uint64_t nTagSize) : m_cAlgorithm(nullptr), m_cState(nullptr), m_nTagSize(nTagSize) { 

        // get the algorithm reference
        switch (nTagSize) {
            
        case 32:
            m_cAlgorithm = &evhash_32;
            break;
                
        case 64:
            m_cAlgorithm = &evhash_64;
            break;
                
        case 96:
            m_cAlgorithm = &evhash_96;
            break;
                
        case 128:
            m_cAlgorithm = &evhash_128;
            break;
                
        case 256:
            m_cAlgorithm = &evhash_256;
            break;
        }
        
        // sanity check
        if (!m_cAlgorithm) throw qkd::crypto::context::context_init();
    };
    

    /**
     * dtor
     */
    ~evhash_data() { 
        
        // free resources
        if (m_cState) {
            if (m_cState->output) free(m_cState->output);
            m_cState->output = nullptr;
            m_cState->destroy(m_cState);
            free(m_cState);
        }
        m_cState = nullptr;
        
        // decrement the context reference count
        // and remove the context from the database when necessary
        m_cContext->dec_reference();
        if (m_cContext->reference_count() == 0) {
            g_cEvHashContextDatabase.erase(g_cEvHashContextDatabase.find(m_cContext->key().data().as_hex()));
        }
        
        m_cAlgorithm = nullptr;
    };
    
    
    /**
     * return the internal context pointer
     * 
     * @return  the internal evhash data pointer
     */
    evhash_context_ptr const & context() const { return m_cContext; };
    
    
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
     * setup the context and state
     * 
     * @param   cKey        the key used to init the context and state
     * @throws  context_init, if we've already setup the thing
     */
    void setup(qkd::key::key const & cKey) {
        
        // sanity check
        if (!m_cAlgorithm) throw qkd::crypto::context::context_init();
        if (!(cKey.size() * 8 == tag_size())) throw qkd::crypto::context::context_init();

        // get up context
        evhash_context_database::iterator iter = g_cEvHashContextDatabase.find(cKey.data().as_hex());
        if (iter == g_cEvHashContextDatabase.end()) {
            
            // create a new ev hash context
            m_cContext = evhash_context_ptr(new qkd::crypto::evhash_context(m_cAlgorithm, cKey));
            g_cEvHashContextDatabase.insert(std::pair<std::string, evhash_context_ptr>(cKey.data().as_hex(), m_cContext));
        }
        else {
            // found: add reference and reuse it
            m_cContext = (*iter).second;
            m_cContext->add_reference();
        }
        
        // sanity check
        if (!m_cContext) throw qkd::crypto::context::context_init();
        
        // get up state
        if (m_cState) throw qkd::crypto::context::context_init();
        m_cState = m_cContext->context()->create_state(m_cContext->context());

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
    ce_algorithm * m_cAlgorithm;
    
    
    /**
     * the evhash context
     */
    evhash_context_ptr m_cContext;
    

    /**
     * the old ev-hash state
     */
    ce_state * m_cState;
    
    
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
crypto_evhash::crypto_evhash(qkd::key::key const & cKey) : context(cKey) {
    if (!is_valid_input_key(cKey)) throw qkd::crypto::context::context_wrong_key();
    d = boost::shared_ptr<qkd::crypto::crypto_evhash::evhash_data>(new qkd::crypto::crypto_evhash::evhash_data(cKey.size() * 8));
    d->setup(cKey);
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
    d->update((char const *)cMemory.get(), cMemory.size());
}


/**
 * number of blocks done so far
 *
 * @return  number of encoded blocks with this algorithms
 */
uint64_t crypto_evhash::blocks() const {
    ce_state * cState = d->state();
    assert(cState != nullptr);
    return cState->nRound;
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
    d->finalize(cHashTag);
    
    // encrypt it with the final key
    qkd::utility::memory cFinalTag;
    qkd::crypto::crypto_context cXOR = qkd::crypto::engine::create("xor", qkd::key::key());
    cXOR << cHashTag;
    cFinalTag = cXOR->finalize(cKey);
    
    return cFinalTag;
}


/**
 * get the size of the final key in bits
 * 
 * @return  the size of the final key or 0 if inapprobiate
 */
uint64_t crypto_evhash::final_key_size_internal() const {
    return d->tag_size() / 8;
}


/**
 * get the size of the init key
 * 
 * @return  the size of the init key or 0 if inapprobiate
 */
uint64_t crypto_evhash::init_key_size_internal() const {
    return d->tag_size() / 8;
}


/**
 * check if the given key is suitable as final key
 * 
 * @param   cKey        the key candidate
 * @return  true, if this cKey can be used in the finalize call
 */
bool crypto_evhash::is_valid_final_key(qkd::key::key const & cKey) const {
    return ((cKey.size() * 8) == d->tag_size());
}


/**
 * check if the given key is suitable for an init key
 * 
 * @return  true, if the key can be used as init key
 */
bool crypto_evhash::is_valid_input_key(qkd::key::key const & cKey) {
    return ((cKey.size() == 32/8) || (cKey.size() == 64/8)  || (cKey.size() == 96/8)  || (cKey.size() == 128/8)  || (cKey.size() == 256/8));
}


/**
 * return the scheme string (at the current state) of this context
 * 
 * @return  a string holding the current scheme string
 */
qkd::crypto::scheme crypto_evhash::scheme_internal() const {
    
    std::stringstream ss;
    ss << "evhash-";
    ss << d->tag_size();
    ss << ":";
    ss << d->context()->key().data().as_hex();
    ss << ":";
    ss << state().as_hex();
    
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
    return d->tag_size();
}


/**
 * set the number of blocks calculated
 *
 * @param   nBlocks         the new number of blocks done
 */
void crypto_evhash::set_blocks(uint64_t nBlocks) {
    ce_state * cState = d->state();
    assert(cState != nullptr);
    cState->nRound = nBlocks;
}


/**
 * sets the state as specified in the memory block
 * 
 * @param   cMemory         the BLOB holding the state data
 */
void crypto_evhash::set_state_internal(qkd::utility::memory const & cMemory) {
    
    // sanity check
    if (cMemory.get() == nullptr) return;
    if (cMemory.size() == 0) return;
    
    ce_state * cState = d->state();
    assert(cState != nullptr);

    // stream out
    qkd::utility::buffer cBuffer(cMemory);
    qkd::utility::memory cReadMem;
    cBuffer.set_position(0);

    // read in ce_state.buf
    cBuffer >> cState->buf.capacity;
    cState->buf.capacity = be64toh(cState->buf.capacity);

    cBuffer >> cState->buf.fill;
    cState->buf.fill = be64toh(cState->buf.fill);

    cBuffer >> cReadMem;
    if (cState->buf.hold) free(cState->buf.hold);
    cState->buf.hold = (char *)malloc(cState->buf.capacity);
    memcpy(cState->buf.hold, cReadMem.get(), cState->buf.capacity);

    // read in output
    bool bOutputPresent;
    cBuffer >> bOutputPresent;
    if (bOutputPresent) {
        
        cBuffer >> cReadMem;
        if (cState->output) free(cState->output);
        cState->output = (char *)malloc(cReadMem.size());
        memcpy(cState->output, cReadMem.get(), cReadMem.size());
    }
    else {
        if (cState->output) free(cState->output);
        cState->output = nullptr;
    }
    
    // evhash state integrity check
    if (!cState->buf.capacity) throw qkd::crypto::context::context_init();
}


/**
 * return the current state of the crypto context
 * 
 * @return  a memory BLOB defining the current state
 * @throws  context_init
 */
qkd::utility::memory crypto_evhash::state_internal() const {
    
    qkd::utility::buffer cBuffer;
    qkd::utility::memory cMemory;
    ce_state const * cState = d->state();

    // dump ce_state.buf
    cBuffer << (uint64_t)htobe64(cState->buf.capacity);
    cBuffer << (uint64_t)htobe64(cState->buf.fill);
    cMemory = qkd::utility::memory(cState->buf.capacity);
    memcpy(cMemory.get(), cState->buf.hold, cState->buf.capacity);
    cBuffer << cMemory;
    
    // dump output
    if (!cState->output) cBuffer << false;
    else {
        cBuffer << true;
        cMemory = qkd::utility::memory(ce_output_size(cState->pctx));
        memcpy(cMemory.get(), cState->output, cMemory.size());
        cBuffer << cMemory;
    }

    return cBuffer;
}


#endif



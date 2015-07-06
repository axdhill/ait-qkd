/*
 * context.h
 * 
 * crypto context interface
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

 
#ifndef __QKD_CRYPTO_CONTEXT_H_
#define __QKD_CRYPTO_CONTEXT_H_


// ------------------------------------------------------------
// incs

#include <exception>
#include <string>

#include <inttypes.h>

#include <boost/shared_ptr.hpp>

// ait
#include <qkd/crypto/scheme.h>
#include <qkd/key/key.h>
#include <qkd/utility/memory.h>


// ------------------------------------------------------------
// decls


namespace qkd {
    
namespace crypto {    

    
// fwd
class context;
class engine;
typedef boost::shared_ptr<context> crypto_context;


/**
 * A crypto context is a single crypto algorithm instance.
 * 
 * This is an abstract class.
 * 
 * You may not create a crypto context directly but with the
 * help of the qkd::crypto::engine::create method.
 * 
 * See engine::create() in the qkd::crypto namespace for this.
 * 
 * A crypto context may have
 * 
 *  - a name
 *  - an optional variant of the algorithm used (eg. 96 bits, 128 bits, ...)
 * 
 *  - an initial key to setup the crypto context
 *  - a final key to render the final crypto result
 * 
 * Using scenario:
 * 
 *  1. create a context with the help of the qkd::crypto::engine class
 * 
 *  2. Apply an initial key if necessary during creation
 * 
 *  3. Add memory to the context: method add()
 * 
 *  4. Compute the result with an final key if needed: method finalize()
 * 
 * 
 *      Code sample:
 * 
 *          qkd::crypto::crypto_context ctx = qkd::crypto::engine::create("evhash-96:87103893a579");
 *      
 *          qkd::utility::buffer data;
 * 
 *          data << std::string("some text");
 *          data << 12.5;
 *          ctx << data;
 * 
 *          data << 12345;
 *          ctx << data;
 * 
 *          qkd::utility::memory secret = qkd::utility::memory::from_hex("f4b0d86ffd53"); 
 *          qkd::utility::memory auth_tag = ctx.finalize(qkd::key::key(1, secret));
 * 
 *          std::cout << "the auth tag is: " << auth_tag.as_hex() << std::endl;
 * 
 * 
 * Hint: look at the test cases in the test/lib/crypto/crypto.cpp on how
 *       to work with these classes
 */
class context {
    

    // friend
    friend class qkd::crypto::engine;
    

public:


    /**
     * exception type thrown if already finalized
     */
    struct context_final : virtual std::exception, virtual boost::exception { };
    

    /**
     * exception type thrown when something unexpected happened during init
     */
    struct context_init : virtual std::exception, virtual boost::exception { };
    

    /**
     * exception type when we want to clone a non-cloneable context
     */
    struct context_not_clonable : virtual std::exception, virtual boost::exception { };
    

    /**
     * exception type thrown when a wrong key is passed
     */
    struct context_wrong_key : virtual std::exception, virtual boost::exception { };
    

    /**
     * dtor
     */
    virtual ~context() {};


    /**
     * stream into
     * 
     * add a memory blob to the algorithm
     *
     * @param   cMemory         memory block to stream into algorithm
     * @return  the crypto context
     */
    inline context & operator<<(qkd::utility::memory const & cMemory) { 
        add(cMemory); 
        return *this; 
    };


    /**
     * add a memory BLOB to the algorithm
     *
     * @param   cMemory         memory block to be added
     * @throws  context_final
     */
    inline void add(qkd::utility::memory const & cMemory) { 
        if (is_finalized()) throw context_final(); 
        add_internal(cMemory); 
    };
    
    
    /**
     * number of blocks done so far
     *
     * @return  number of encoded blocks with this algorithms
     */
    virtual uint64_t blocks() const { return 0; };


    /**
     * clone the current context
     * 
     * @return  a new cloned context
     */
    crypto_context clone() const { 
        if (!is_cloneable()) throw context_not_clonable(); 
        return clone_internal(); 
    };


    /**
     * check if this context allows to reuse the final key
     * 
     * @return  true, if the final key can be reused
     */
    inline bool final_key_reusable() const { return final_key_reusable_internal(); };
    
    
    /**
     * get the size of the final key in bytes
     * 
     * @return  the size of the final key or 0 if inappropriate
     */
    inline uint64_t final_key_size() const { return final_key_size_internal(); };
    

    /**
     * finalize the algorithm and get the tag
     * 
     * @param   cKey        the key used to finalize the algorithm
     * @return  a memory BLOB representing the tag
     * @throws  context_wrong_key
     * @throws  context_final
     */
    inline qkd::utility::memory finalize(qkd::key::key const & cKey = qkd::key::key::null()) { 
        if (is_finalized()) throw context_final(); 
        m_bFinalized = true; 
        return finalize_internal(cKey); 
    };
    
    
    /**
     * returns the initial key of the crypto context
     * 
     * @return  the key used to create the context
     */
    inline qkd::key::key const & init_key() const { return m_cKey; };
    
    
    /**
     * check if this context allows to reuse the init key
     * 
     * @return  true, if the init key can be reused
     */
    inline bool init_key_reusable() const { return init_key_reusable_internal(); };
    

    /**
     * get the size of the init key in bytes
     * 
     * @return  the size of the init key or 0 if inappropriate
     */
    inline uint64_t init_key_size() const { return init_key_size_internal(); };
    

    /**
     * check if this context can be cloned
     * 
     * @return  true, if we can make a clone of a concrete instance
     */
    inline bool is_cloneable() const { return is_cloneable_internal(); };
    
    
    /**
     * check if this context has been already finalized
     * 
     * @return  true, if the finalize method has been called at least once
     */
    inline bool is_finalized() const { return m_bFinalized; };
    
    
    /**
     * check if the given key is suitable as final key
     * 
     * @param   cKey        the key candidate
     * @return  true, if this cKey can be used in the finalize call
     */
    virtual bool is_valid_final_key(qkd::key::key const & cKey) const = 0;
    
    
    /**
     * name of the crypto context (the algorithm)
     * 
     * @return  the name of the crypto context (the algorithm)
     */
    virtual std::string name() = 0;
    
    
    /**
     * check if this context needs a final key
     * 
     * @return  true, if a final key is needed
     */
    inline bool needs_final_key() const { return needs_final_key_internal(); };
    

    /**
     * check if this context needs an init key
     * 
     * @return  true, if an init key is needed
     */
    inline bool needs_init_key() const { return needs_init_key_internal(); };
    

    /**
     * checks if this is the NULL instance
     * 
     * @return  true, if it is
     */
    virtual bool null() const = 0;
    
    
    /**
     * get the size of the result of a computation in bits
     * 
     * if the context is an authentication context, this
     * is the size of the authentication-tag.
     * 
     * if the return is 0, the result may have any size
     * 
     * @return  expected size of the computation result (or 0 for any size)
     */
    inline uint64_t result_size() const { return result_size_internal(); };
    
    
    /**
     * return the scheme string (at the current state) of this context
     * 
     * @return  the scheme identifiying this context
     */
    inline qkd::crypto::scheme scheme() const { return scheme_internal(); };
    

    /**
     * set the number of blocks calculated
     *
     * @param   nBlocks         the new number of blocks done
     */
    void set_blocks(uint64_t nBlocks) { m_nBlocks = nBlocks; };


    /**
     * sets the state as specified in the memory block
     * 
     * @param   cMemory         the BLOB holding the state data
     * @throws  context_init
     */
    inline void set_state(qkd::utility::memory const & cMemory) { return set_state_internal(cMemory); };
    
    
    /**
     * return the current state of the crypto context
     * 
     * @return  a memory BLOB defining the current state
     */
    inline qkd::utility::memory state() const { return state_internal(); };
    
    
protected:
    
    
    /**
     * ctor
     * 
     * @param   cKey        the initial key
     * @param   nBlocks     the blocks done with this algorithm
     * @throws  context_wrong_key
     */
    explicit context(qkd::key::key const & cKey = qkd::key::key::null(), uint64_t nBlocks = 0) 
            : m_nBlocks(nBlocks), m_bFinalized(false), m_cKey(cKey) {};

    
private:
    
    
    /**
     * ctor
     * 
     * forbidden: only qkd::crypto::engine may create crypto contexts
     */
    context() {};
    
    
    /**
     * add a memory BLOB to the algorithm
     *
     * @param   cMemory         memory block to be added
     * @throws  context_final, if the algorithm has finished and does not allow another addition
     */
    virtual void add_internal(qkd::utility::memory const & cMemory) = 0;


    /**
     * default clone implementation
     * 
     * get a scheme string and call engine::create
     * with it
     * 
     * Overwrite this is you need a more sophisticated
     * cloning algorithm
     * 
     * @return  a new cloned context
     */
    virtual crypto_context clone_internal() const;


    /**
     * check if this context allows to reuse the final key
     * 
     * @return  true, if the final key can be reused
     */
    virtual bool final_key_reusable_internal() const = 0;
    

    /**
     * get the size of the final key
     * 
     * @return  the size of the final key or 0 if inappropriate
     */
    virtual uint64_t final_key_size_internal() const = 0;
    

    /**
     * finalize the algorithm and get the tag
     * 
     * @param   cKey        the key used to finalize the algorithm
     * @return  a memory BLOB representing the tag
     * @throws  context_wrong_key
     */
    virtual qkd::utility::memory finalize_internal(qkd::key::key const & cKey) = 0;
    
    
    /**
     * check if this context allows to reuse the init key
     * 
     * @return  true, if the init key can be reused
     */
    virtual bool init_key_reusable_internal() const = 0;
    

    /**
     * get the size of the init key
     * 
     * @return  the size of the init key or 0 if inappropriate
     */
    virtual uint64_t init_key_size_internal() const = 0;
    

    /**
     * check if this context can be cloned
     * 
     * @return  true, if we can make a clone of a concrete instance
     */
    virtual bool is_cloneable_internal() const = 0;
    
    
    /**
     * check if this context needs a final key
     * 
     * @return  true, if a final key is needed
     */
    virtual bool needs_final_key_internal() const = 0;
    

    /**
     * check if this context needs an init key
     * 
     * @return  true, if an init key is needed
     */
    virtual bool needs_init_key_internal() const = 0;
    

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
    virtual uint64_t result_size_internal() const = 0;
    
    
    /**
     * return the scheme string (at the current state) of this context
     * 
     * @return  the scheme identifying this context
     */
    virtual qkd::crypto::scheme scheme_internal() const = 0;
    
    
    /**
     * sets the state as specified in the memory block
     * 
     * @param   cMemory         the BLOB holding the state data
     * @throws  context_init
     */
    virtual void set_state_internal(qkd::utility::memory const & cMemory) = 0;
    
    
    /**
     * return the current state of the crypto context
     * 
     * @return  a memory BLOB defining the current state
     */
    virtual qkd::utility::memory state_internal() const = 0;
    
    
    /**
     * the blocks calculated so far
     */
    uint64_t m_nBlocks;


    /**
     * finalized flag
     */
    bool m_bFinalized;
    

    /**
     * the initial key
     */
    qkd::key::key m_cKey;

};
  

/**
 * stream into
 * 
 * Add some memory data to the crypto algorithm
 * 
 * @param   lhs     the crypto context object
 * @param   rhs     the memory to add
 * @return  the crypto context object
 */
inline crypto_context & operator<<(crypto_context & lhs, qkd::utility::memory const & rhs) { (*lhs) << rhs; return lhs; }


}

}



#endif


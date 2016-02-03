/*
 * crypto_null.h
 * 
 * interface for the NULL encryption/authentication
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

 
#ifndef __QKD_CRYPTO_CRYPTO_NULL_H_
#define __QKD_CRYPTO_CRYPTO_NULL_H_


// ------------------------------------------------------------
// incs

#include <inttypes.h>

// ait
#include <qkd/crypto/context.h>
#include <qkd/key/key.h>
#include <qkd/utility/memory.h>


// ------------------------------------------------------------
// decls


namespace qkd {

namespace crypto {


/**
 * this class holds the empty NULL encryption/authentication
 */
class crypto_null : public context {


public:


    /**
     * ctor
     * 
     * @param   cKey        the initial key
     * @throws  context_init
     * @throws  context_wrong_key
     */
    explicit crypto_null(qkd::key::key const & cKey);
    

    /**
     * check if the given key is suitable as final key
     * 
     * @param   cKey        the key candidate
     * @return  true, if this cKey can be used in the finalize call
     */
    bool is_valid_final_key(qkd::key::key const & cKey) const;
    
    
    /**
     * check if the given key is suitable for an init key
     * 
     * @return  true, if the key can be used as init key
     */
    static bool is_valid_input_key(UNUSED qkd::key::key const & cKey);

    
    /**
     * name of the crypto algorithm
     * 
     * @return  the name of the crypto algorithm
     */
    std::string name() { return std::string("null"); };
    
    
    /**
     * checks if this is the NULL instance
     * 
     * @return  true, if it is
     */
    bool null() const { return true; };
    
    
private:
    

    /**
     * add another crypto context
     *
     * @param   cContext        the crypto context to add
     * @throws  context_final, if the algorithm has finished and does not allow another addition
     */
    void add_internal(qkd::crypto::crypto_context const & cContext);


    /**
     * add a memory BLOB to the algorithm
     *
     * @param   cMemory         memory block to be added
     * @throws  context_final, if the algorithm has finished and does not allow another addition
     */
    void add_internal(qkd::utility::memory const & cMemory);


    /**
     * check if this context allows to reuse the final key
     * 
     * @return  true, if the final key can be reused
     */
    bool final_key_reusable_internal() const { return false; };
    

    /**
     * get the size of the final key in bytes
     * 
     * @return  the size of the final key or 0 if inappropriate
     */
    uint64_t final_key_size_internal() const { return 0; };
    

    /**
     * finalize the algorithm and get the tag
     * 
     * @param   cKey        the key used to finalize the algorithm
     * @return  a memory BLOB representing the tag
     * @throws  context_wrong_key
     */
    qkd::utility::memory finalize_internal(qkd::key::key const & cKey);
    
    
    /**
     * check if this context allows to reuse the init key
     * 
     * @return  true, if the init key can be reused
     */
    bool init_key_reusable_internal() const { return false; };
    

    /**
     * NULL is cloneable
     * 
     * @return  true
     */
    bool is_cloneable_internal() const { return true; };
    
    
    /**
     * check if this context needs a final key
     * 
     * @return  true, if a final key is needed
     */
    bool needs_final_key_internal() const { return false; };
    

    /**
     * check if this context needs an init key
     * 
     * @return  true, if an init key is needed
     */
    bool needs_init_key_internal() const { return false; };
    

    /**
     * get the size of the init key
     * 
     * @return  the size of the init key or 0 if inappropriate
     */
    uint64_t init_key_size_internal() const { return 0; };
    

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
    uint64_t result_size_internal() const { return 0; };
    
    
    /**
     * return the scheme string (at the current state) of this context
     * 
     * @return  the scheme identifying this context
     */
    qkd::crypto::scheme scheme_internal() const;
    
    
    /**
     * sets the state as specified in the memory block
     * 
     * @param   cMemory         the BLOB holding the state data
     * @throws  context_init
     */
    void set_state_internal(qkd::utility::memory const & cMemory);
    
    
    /**
     * return the current state of the crypto context
     * 
     * @return  a memory BLOB defining the current state
     */
    qkd::utility::memory state_internal() const;
    
    
    /**
     * data to encrypt so far
     */
    qkd::utility::memory m_cData;
    
};


}
    
    
}

#endif


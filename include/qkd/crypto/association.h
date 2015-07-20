/*
 * association.h
 * 
 * a crypto association holds the context for incoming and outgoing data traffic
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

 
#ifndef __QKD_CRYPTO_ASSOCIATION_H_
#define __QKD_CRYPTO_ASSOCIATION_H_


// ------------------------------------------------------------
// incs

// ait
#include <qkd/key/key_ring.h>
#include <qkd/crypto/context.h>
#include <qkd/crypto/engine.h>


// ------------------------------------------------------------
// decls


namespace qkd {
    
namespace crypto {    


/**
 * This class defines an association: all crypto contexts needed for incoming and outgoing communication
 * 
 * A crypto associated groups an encryption and 
 * an authentication context for both communication directions.
 * 
 * The grouping of one incoming aspect and one outgoing aspect
 * results in the association_io class. There should be an association_io
 * for authentication and one for encryption.
 */
class association {
    
    
public:
    
    
    /**
     * an association description is the stringified template for an association
     * 
     * These are 4 scheme strings which fully qualifiy a 
     * connection in incoiming, outgoing, encryption and
     * authentication.
     * 
     * The given scheme strings can be given to qkd::crypto::engine::create()
     * to instantiate a concrete crpyto context.
     */
    struct association_definition {
        
        std::string sAuthenticationIncoming;        /**< crypto scheme string for incoming authentication */
        std::string sAuthenticationOutgoing;        /**< crypto scheme string for outgoing authentication */
        std::string sEncryptionIncoming;            /**< crypto scheme string for incoming authentication */
        std::string sEncryptionOutgoing;            /**< crypto scheme string for outgoing authentication */
    };
    
    
    /**
     * an association_io is a pair of Incoming and Outgoing crypto contexts ... a crpyto "duplex"
     * 
     * This is the combination of a single
     */
    class association_io {
        
        
    public:
        
        
        /**
         * ctor
         */
        association_io() { cIncoming = qkd::crypto::engine::create("null"); cOutgoing = qkd::crypto::engine::create("null"); };
        

        /**
         * copy ctor
         * 
         * @param   rhs     right hand side
         */
        association_io(association_io const & rhs) : cIncoming(rhs.cIncoming), cOutgoing(rhs.cOutgoing) {};
        
        crypto_context cIncoming;               /**< crypto context used for incoming messages */
        crypto_context cOutgoing;               /**< crypto context used for outgoing messages */
    };
    
    
    /**
     * ctor
     */
    association() {}
    
    
    /**
     * ctor
     * 
     * @param   cDefinition         definition for creation
     * @throws  algorithm_unknown
     * @throws  scheme_invalid
     * @throws  context_init
     * @throws  context_wrong_key
     */
    association(association_definition const & cDefinition);
    
    
    /**
     * copy ctor
     * 
     * @param   rhs     right hand side
     */
    association(association const & rhs) : m_cAuthentication(rhs.m_cAuthentication), m_cEncryption(rhs.m_cEncryption) {}
    
    
    /**
     * get the authentication pair
     * 
     * @return  the authentication pair
     */
    inline association_io & authentication() { return m_cAuthentication; }
    
    
    /**
     * get the authentication pair
     * 
     * @return  the authentication pair
     */
    inline association_io const & authentication() const { return m_cAuthentication; }
    
    
    /**
     * get the encryption pair
     * 
     * @return  the encryption pair
     */
    inline association_io & encryption() { return m_cEncryption; }
    
    
    /**
     * get the encryption pair
     * 
     * @return  the encryption pair
     */
    inline association_io const & encryption() const { return m_cEncryption; }
    
    
    /**
     * calculate how many key material (in bytes) is consumed with one
     * round if given an association definition.
     * 
     * A "round" is: 1 x message send back and forth both authenticated and
     * encrypted.
     * 
     * @param   cDefinition     the current association definition
     * @return  number of bytes needed for 1 round
     */
    static uint64_t key_consumption(association_definition const & cDefinition);
    
    
private:
    
    
    /**
     * authentication
     */
    association_io m_cAuthentication;


    /**
     * encryption
     */
    association_io m_cEncryption;

};

    
}
    
}


#endif


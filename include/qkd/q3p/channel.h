/*
 * channel.h
 * 
 * a Q3P channel
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

 
#ifndef __QKD_Q3P_CHANNEL_H_
#define __QKD_Q3P_CHANNEL_H_


// ------------------------------------------------------------
// incs

#include <inttypes.h>

// ait
#include <qkd/crypto/association.h>
#include <qkd/q3p/db.h>
#include <qkd/q3p/message.h>


// ------------------------------------------------------------
// decls


namespace qkd {
    
namespace q3p {    


// fwd
class engine_instance;
    

/**
 * the error of a message handling on a channel
 */
enum class channel_error : uint8_t {
    
    CHANNEL_ERROR_NO_ERROR = 0,                 /**< not an error */
    CHANNEL_ERROR_MESSAGE,                      /**< the message object is malformed */
    CHANNEL_ERROR_KEYS,                         /**< not enough keys in the buffers to perform action or invalid */
    CHANNEL_ERROR_AUTH,                         /**< authentication failed NOTE: THIS IS CRITICAL! THIS IS AN ATTACK! */
    CHANNEL_ERROR_CONTEXT                       /**< crpyto operation failed internally */
};

   
/**
 * All messages on a Q3P channel have a crypto association.
 * 
 * A channel is responsible for authentication and encryption of
 * incoming and outgoing messages.
 * 
 * Channel ID 0 is invalid.
 */
class channel {
    
    
public:
    
    
    /**
     * ctor
     * 
     * @param   nId             channel id
     * @param   cEngine         the parent engine
     * @param   cAssociation    crypto association
     */
    channel(uint16_t nId = 0, qkd::q3p::engine_instance * cEngine = nullptr, qkd::crypto::association cAssociation = qkd::crypto::association()) : m_cAssociation(cAssociation), m_cEngine(cEngine), m_nId(nId), m_nMessageId(1) {};
    
    
    /**
     * return the crypto association
     * 
     * @return  the crypto association for this channel
     */
    inline qkd::crypto::association & association() { return m_cAssociation; };
    

    /**
     * return the crypto association
     * 
     * @return  the crypto association for this channel
     */
    inline qkd::crypto::association const & association() const { return m_cAssociation; };
    

    /**
     * give a human readable description of the error
     * 
     * @param   eError      the error questioned
     * @return  a string describing the error
     */
    static std::string channel_error_description(channel_error eError);
    
    
    /**
     * apply decryption and an authentication-check to a message
     * 
     * this is after a message has been received
     * 
     * @param   cMessage    message received
     * @return  a channel_error value
     */
    channel_error decode(qkd::q3p::message & cMessage);
    
    
    /**
     * apply encryption and authentication to a message
     * 
     * this is for preparing a message to be sent
     * 
     * @param   cMessage    message to prepare
     * @return  a channel_error value
     */
    channel_error encode(qkd::q3p::message & cMessage);
    
    
    /**
     * return the Q3P engine
     * 
     * @return  the Q3P engine associated
     */
    inline qkd::q3p::engine_instance * engine() { return m_cEngine; };
    
    
    /**
     * return the Q3P engine
     * 
     * @return  the Q3P engine associated
     */
    inline qkd::q3p::engine_instance const * engine() const { return m_cEngine; };
    
    
    /**
     * return the channel id
     * 
     * @return  the channel id
     */
    inline uint16_t id() const { return m_nId; };
    

    /**
     * the maximum size of an uncompressed message payload
     * 
     * a message holding anything bigger than that will be compressed
     * to save bandwitdth
     * 
     * encrypted messages are always compressed to save key material
     * 
     * @return  the upper bound for uncompressed payload
     */
    static uint64_t max_uncompressed_payload() { return 1 << 15; };
    
    
    /**
     * return the next message id (for sending) on this channel
     * 
     * @return  the next message id for sending on this channel
     */
    inline uint32_t message_id() const { return m_nMessageId; };
    
    
private:
    
    
    /**
     * perform authentication on a message
     * 
     * @param   cMessage        the message to authenticate
     */
    channel_error authenticate(qkd::q3p::message & cMessage);
    

    /**
     * check the authentication of a message
     * 
     * @param   cMessage        the message to check
     */
    channel_error authentication_verify(qkd::q3p::message & cMessage);
    

    /**
     * perform compression on a message
     * 
     * @param   cMessage        the message to compress
     */
    channel_error compress(qkd::q3p::message & cMessage);
    

    /**
     * perform decompression on a message
     * 
     * @param   cMessage        the message to decompress
     */
    channel_error decompress(qkd::q3p::message & cMessage);
    

    /**
     * perform decryption on a message
     * 
     * @param   cMessage        the message to decrypt
     */
    channel_error decrypt(qkd::q3p::message & cMessage);
    

    /**
     * perform encryption on a message
     * 
     * @param   cMessage        the message to encrypt
     */
    channel_error encrypt(qkd::q3p::message & cMessage);
    

    /**
     * the authentication context used
     */
    qkd::crypto::association m_cAssociation;

    
    /**
     * the Q3P engine
     */
    qkd::q3p::engine_instance * m_cEngine;
    
    
    /**
     * channel id
     */
    uint16_t m_nId;


    /**
     * message id
     */
    uint32_t m_nMessageId;
};


}
    
}



#endif


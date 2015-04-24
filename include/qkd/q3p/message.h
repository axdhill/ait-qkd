/*
 * message.h
 * 
 * a Q3P message
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

 
#ifndef __QKD_Q3P_MESSAGE_H_
#define __QKD_Q3P_MESSAGE_H_


// ------------------------------------------------------------
// incs

#include <chrono>

#include <inttypes.h>

// ait
#include <qkd/key/key.h>
#include <qkd/utility/buffer.h>
#include <qkd/utility/memory.h>


// ------------------------------------------------------------
// decls


namespace qkd {
    
namespace q3p {    


/**
 * This class represents a single Q3P Message
 * 
 * This is a buffer (== the data sent/received) plus some message stuff
 * 
 * It includes the total package from head to toe. This means:
 * 
 * It includes the header, the payload and the tag. In this order.
 * 
 * This the Q3P message layout:
 * 
 * 
 *      0                   1                   2                   3
 *      0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *   0  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *      |                            Length                             |
 *   4  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *      |                            Msg-Id                             |
 *   8  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *      |E A Z r r| Vers|    Command    |          Channel              |
 *  12  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *      |                       Encryption Key Id                       |
 *  16  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *      |                     Authentication Key Id                     |
 *  20  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *      |                             Data ...                           
 *      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *                                ... Data ...                          
 *      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *                                ... Data                              |
 *      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *      |                             A-Tag ...                          
 *      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *                               ... A-Tag                              |
 *      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * 
 * with:
 * 
 *      Length:         total size of packet, including the length field itself
 *      Msg-Id:         message number (inside a channel)
 *      E:              Encryption bit. If set the payload is encrypted
 *      A:              Authentication bit. If set the payload is authenticated
 *      Z:              Compressed bit. If set the payload is compressed
 *      r:              reserved for future use
 *      Vers:           Q3P version field: ALWAYS 2 for this implementation
 *      Command:        Protocol Command (HANDSHAKE, DATA, LOAD, LOAD-REQUEST, STORE, ...)
 *      Channel:        Q3P Channel number
 *      E-KeyId:        start offset for the encryption key within the buffers
 *      A-KeyId:        start offset for the authentication key within the buffers
 *      Data:           User Data
 *      A-Tag:          Authentication tag
 */
class message : public qkd::utility::buffer {

    
public:
    
    
    /**
     * the meassage header 
     * 
     * all data here is stored in big endian (network byte order)
     */
    struct header_t {
        
        uint32_t nLength;                   /**< message length field */
        uint32_t nMessageId;                /**< message id field */
        char nFlagsAndVersion;              /**< flags and version field */
        unsigned char nCommand;             /**< command and protocol field */
        uint16_t nChannel;                  /**< channel id */
        uint32_t nEncryptionKeyId;          /**< encryption key id */
        uint32_t nAuthenticationKeyId;      /**< authentication key id */
    };
    

    /**
     * ctor
     * 
     * @param   bAuthentic      authentic flag
     * @param   bEncrypted      encryption flag
     */
    message(bool bAuthentic = true, bool bEncrypted = true);
    
    
    /**
     * get the age of the message in nanoseconds
     * 
     * the message's age is defined as NOW - the last action on it (send/recv)
     * 
     * @return  the age of the message
     */
    inline std::chrono::high_resolution_clock::duration age() const { return std::chrono::high_resolution_clock::now() - m_cTimeStamp; };


    /**
     * check if the authentic flag is set
     * 
     * @return  the authentic flag
     */
    inline bool authentic() const { if (size() < header_size()) return false; return ((header().nFlagsAndVersion & 0x02) != 0); };
    
    
    /**
     * authentication key id
     * 
     * the authentication key id points into the
     * Q3P transmission buffers. it points the needed
     * init or final key or both inside the buffer.
     * 
     * @return  the authentication key id
     */
    inline qkd::key::key_id authentication_key() const { if (size() < header_size()) return 0; return be32toh(header().nAuthenticationKeyId); };
    
    
    /**
     * return the channel id
     * 
     * Hence, a channel id of 0 is invalid.
     * Use qkd::q3p::channel::encode() to prepare
     * the message for sending
     * 
     * @return  the channel id
     */
    inline uint16_t channel_id() const { if (size() < header_size()) return 0; return be16toh(header().nChannel); };
    
    
    /**
     * check if the encrypted flag is set
     * 
     * @return  the encrypted flag
     */
    inline bool encrypted() const { if (size() < header_size()) return false; return ((header().nFlagsAndVersion & 0x01) != 0); };
    
    
    /**
     * encryption key id
     * 
     * the encryption key id points into the
     * Q3P transmission buffers. it points the needed
     * init or final key or both inside the buffer.
     * 
     * @return  the encryption key id
     */
    inline qkd::key::key_id encryption_key() const { if (size() < header_size()) return 0; return be32toh(header().nEncryptionKeyId); };
    
    
    /**
     * get the header
     * 
     * @return  the header data
     */
    inline header_t & header() { return *(qkd::q3p::message::header_t *)get(); };
    
    
    /**
     * get the header
     * 
     * @return  the header data
     */
    inline header_t const & header() const { return *(qkd::q3p::message::header_t const *)get(); };
    
    
    /**
     * calculate the size of a Q3P header
     * 
     * @return  the size of Q3P header
     */
    static uint64_t header_size() { return sizeof(header_t); };
        

    /**
     * return the message id
     * 
     * @return  the message id
     */
    inline uint32_t id() const { if (size() < header_size()) return 0; return be32toh(header().nMessageId); };
    
    
    /**
     * return the message length as stored in the header
     * 
     * @return  the message length in the header
     */
    inline uint32_t length() const { if (size() < header_size()) return 0; return be32toh(header().nLength); };
    
    
    /**
     * return the payload
     * 
     * @return  the payload
     */
    qkd::utility::memory payload();
    
    
    /**
     * return the protocol id
     * 
     * @return  the protocol id
     */
    inline unsigned char protocol_id() const { if (size() < header_size()) return 0; return header().nCommand; };
    
    
    /**
     * record the current time
     */
    inline void record_timestamp() { m_cTimeStamp = std::chrono::high_resolution_clock::now(); };
    
    
    /**
     * set read/write buffer pointer to start of payload
     */
    inline void seek_payload() { ensure_header(); set_position(header_size()); };
    
    
    /**
     * set the authentic flag
     * 
     * @param   bAuthentic      the new authentic flag
     */
    void set_authentic(bool bAuthentic);
    
    
    /**
     * set authentication key id
     * 
     * @param   nKeyId          the new authentication key id
     */
    inline void set_authentication_key(qkd::key::key_id const & nKeyId) { ensure_header(); header().nAuthenticationKeyId = htobe32(nKeyId); };
    
    
    /**
     * set a new channel id
     * 
     * @param   nChannelId      the new channel id
     */
    inline void set_channel_id(uint16_t nChannelId) { ensure_header(); header().nChannel = htobe16(nChannelId); };
    
    
    /**
     * set the encrypted flag
     * 
     * @param   bEncrypted      the new encrypted flag
     */
    void set_encrypted(bool bEncrypted);
    
    
    /**
     * set encryption key id
     * 
     * @param   nKeyId          the new encryption key id
     */
    inline void set_encryption_key(qkd::key::key_id const & nKeyId) { ensure_header(); header().nEncryptionKeyId = htobe32(nKeyId); };
    
    
    /**
     * set the message id
     * 
     * @param   nId             the new message id
     */
    inline void set_id(uint32_t nId) { ensure_header(); header().nMessageId = htobe32(nId); };
    
    
    /**
     * set the message length stored in the header
     * 
     * @param   nLength         the message length in the header
     */
    inline void set_length(uint32_t nLength) { ensure_header(); header().nLength = htobe32(nLength); };
    
    
    /**
     * set the protocol id
     * 
     * @param   nProtocolId     the new protocol id
     */
    inline void set_protocol_id(unsigned char nProtocolId) { ensure_header(); header().nCommand = nProtocolId; };
    
    
    /**
     * set the authentication tag
     * 
     * @param   cTag            the authentication tag of the message
     */
    inline void set_tag(qkd::utility::memory const & cTag) { m_cTag = cTag; };
    
    
    /**
     * set the zipped flag
     * 
     * @param   bZipped         the new zipped flag
     */
    void set_zipped(bool bZipped);
    
    
    /**
     * return a small string describing the header, a bit payload and the tag
     * 
     * usefull for debuging
     * 
     * @return  a string describing the message
     */
    std::string str() const;
    
    
    /**
     * get the authentication tag
     * 
     * @return  the authentication tag of the message
     */
    inline qkd::utility::memory const & tag() const { return m_cTag; };
    
    
    /**
     * return timestamp of last send/recv
     * 
     * @return  timestamp of last action of message
     */
    inline std::chrono::high_resolution_clock::time_point const & timestamp() const { return m_cTimeStamp; };
    
    
    /**
     * get the Q3P version
     * 
     * @return  the Q3P version of this message
     */
    inline uint8_t version() const { if (size() < header_size()) return 0; return ((header().nFlagsAndVersion & 0xe0) >> 5); };
    

    /**
     * check the compressed flag
     * 
     * @return  if the message has been sent (or received) compressed
     */
    inline bool zipped() const { if (size() < header_size()) return false; return ((header().nFlagsAndVersion & 0x04) != 0); };
    
    
private:
    
    
    /**
     * ensure we have at least header size space available
     */
    inline void ensure_header() { if (size() < header_size()) resize(header_size()); };
    
    
    /**
     * set the Q3P version
     */
    void set_version();
    

    /**
     * authentication tag
     */
    qkd::utility::memory m_cTag;
    

    /**
     * timestamp
     */
    mutable std::chrono::high_resolution_clock::time_point m_cTimeStamp;
    
};


}
    
}



#endif


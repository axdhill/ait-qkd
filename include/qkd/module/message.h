/*
 * message.h
 * 
 * a QKD module message
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

 
#ifndef __QKD_MODULE_MESSAGE_H_
#define __QKD_MODULE_MESSAGE_H_


// ------------------------------------------------------------
// incs

#include <chrono>

#include <inttypes.h>

// ait
#include <qkd/crypto/context.h>
#include <qkd/utility/buffer.h>


// ------------------------------------------------------------
// decls


namespace qkd {
    
namespace module {    

    
/**
 * message type
 */
enum class message_type : uint32_t {
    
    MESSAGE_TYPE_DATA,              /**< plain user data */
    MESSAGE_TYPE_KEY_SYNC           /**< key sync message */
};
    

/**
 * This class represents a single QKD Module Message
 *
 * A QKD Module message has an id and associated data. Also it notes
 * the timestamp of the last send or receive action, so one can 
 * calculate its age().
 * 
 * messages may not be copied ==> they have to be created on the heap
 * 
 * Sending and receiving is done by the qkd::module::module object instance.
 * 
 * see qkd::module::module::send() and qkd::module::module::recv()
 */
class message {
    
    
    friend class connection;
    friend class module;
    
public:
    
    
    /**
     * ctor
     * 
     * @param   eType       type of message
     */
    message(qkd::module::message_type eType = qkd::module::message_type::MESSAGE_TYPE_DATA);
    

    /**
     * get the age of the message in nanoseconds
     * 
     * the message's age is defined as NOW - the last action on it (send/recv)
     * 
     * @return  the age of the message
     */
    inline std::chrono::high_resolution_clock::duration age() const { 
        return std::chrono::high_resolution_clock::now() - m_cTimeStamp; 
    }


    /**
     * return the data of the message
     * 
     * @return  memory block of the message
     */
    inline qkd::utility::buffer & data() { return m_cData; }
    
    
    /**
     * return the data of the message (const version)
     * 
     * @return  memory block of the message
     */
    inline qkd::utility::buffer const & data() const { return m_cData; }
    
    
    /**
     * return the message id
     * 
     * @return  the message id
     */
    inline uint64_t id() const { return be32toh(m_cHeader.nId); }
    
    
    /**
     * give a debug string
     * 
     * @param   sIndent     indent of message canonical dump
     * @return  a debug string describing the message
     */
    std::string string(std::string const & sIndent) const;
    
    
    /**
     * return timestamp of last send/recv
     * 
     * @return  timestamp of last action of message
     */
    inline std::chrono::high_resolution_clock::time_point const & timestamp() const { return m_cTimeStamp; }
    
    
    /**
     * message data type
     * 
     * @return  type of the message
     */
    inline qkd::module::message_type type() const { return m_cHeader.eType; }
    

private:
    
    
    /**
     * message id counter
     */
    static uint32_t m_nLastId;
    
    
    // push tight memory aligment
#if defined(__GNUC__) || defined(__GNUCPP__)
#   pragma pack(push, 1)
#else
#   error "pragma pack statement for your current compiler needed. please use GCC when confused"
#endif
    
    
    /**
     * message header
     */
    struct header {
        
        uint32_t nId;                                                         /**< message id */
        message_type eType = qkd::module::message_type::MESSAGE_TYPE_DATA;    /**< type of the message */
        
    } m_cHeader;

    
    // pop old memory aligment
#if defined(__GNUC__) || defined(__GNUCPP__)
#   pragma pack(pop)
#else
#   error "pragma pack statement for your current compiler needed. please use GCC when confused"
#endif
    

    /**
     * data block of message == payload
     */
    qkd::utility::buffer m_cData;
    

    /**
     * timestamp
     */
    mutable std::chrono::high_resolution_clock::time_point m_cTimeStamp;
    
};


    
}
    
}



#endif


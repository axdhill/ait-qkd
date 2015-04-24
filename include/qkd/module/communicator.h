/*
 * communicator.h
 * 
 * This is a neat light-weighted object to easily communicate with peer modules
 *
 * Autor: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *
 * Copyright (C) 2014-2015 AIT Austrian Institute of Technology
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

 
#ifndef __QKD_MODULE_COMMUNICATOR_H
#define __QKD_MODULE_COMMUNICATOR_H


// ------------------------------------------------------------
// incs

// ait
#include <qkd/crypto/context.h>
#include <qkd/module/message.h>
#include <qkd/utility/buffer.h>
#include <qkd/utility/memory.h>


// ------------------------------------------------------------
// decls


namespace qkd {
    
namespace module {    
    

// fwd
class module;


/**
 * communicator makes communication (send/recv) with the peer easy
 *
 * this is a facade object wrapping module's send/recv methods
 * one can instantiate it, with the module's communicator method
 *
 * the idea is to use send/recv module outside a module's scope like this:
 *
 *      void foo(qkd::module::communicator & comm) {
 *
 *          qkd::utility::buffer payload;
 *          payload << std::string('Hello peer!');
 *          
 *          // send to peer!
 *          comm << payload;
 *      
 *          // recv from peer
 *          qkd::utility::memory answer;
 *          comm >> answer;
 *
 *      }
 *
 *  
 * then somewhere in the module's process code
 *
 *      bool mymodule::process(....) {
 *
 *          ...
 *          // get communicator instance
 *          qkd::module::communicator comm = communicator(in_auth, out_auth);
 *
 *          // talk to peer
 *          foo(comm);
 *
 *      }
 *
 * this is as short and easy is can get by now
 *
 */
class communicator {


    // extended access
    friend class module;


private:


    /**
     * ctor
     *
     * @param   cModule             the owning module
     * @param   cIncomingContext    the incoming auth context
     * @param   cOutgoingContext    the outgoing auth context
     */
    communicator(module * cModule, qkd::crypto::crypto_context & cIncomingContext, qkd::crypto::crypto_context & cOutgoingContext) : m_cModule(cModule), m_cIncomingContext(cIncomingContext), m_cOutgoingContext(cOutgoingContext) {};


public:


    /**
     * >> operator
     *
     * recv a blob
     *
     * @param   cBuffer         blob to receive
     * @return  true for success
     */
    inline bool operator>>(qkd::utility::buffer & cBuffer) throw (std::runtime_error) { qkd::module::message cMessage; if (recv(cMessage)) { cMessage.data() >> cBuffer; return true; } return false; };


    /**
     * >> operator
     *
     * recv a blob
     *
     * @param   cMemory         blob to receive
     * @return  true for success
     */
    inline bool operator>>(qkd::utility::memory & cMemory) throw (std::runtime_error) { qkd::module::message cMessage; if (recv(cMessage)) { cMessage.data() >> cMemory; return true; } return false; };


    /**
     * >> operator
     *
     * recv a message
     *
     * @param   cMessage        message to receive
     * @return  true for success
     */
    inline bool operator>>(qkd::module::message & cMessage) throw (std::runtime_error) {  return recv(cMessage); };


    /**
     * << operator
     *
     * send a blob
     *
     * @param   cBuffer         blob to send
     * @return  true for success
     */
    inline void operator<<(qkd::utility::buffer const & cBuffer) throw (std::runtime_error) { qkd::module::message cMessage; cMessage.data() << cBuffer; send(cMessage); };


    /**
     * << operator
     *
     * send a blob
     *
     * @param   cMemory         blob to send
     * @return  true for success
     */
    inline void operator<<(qkd::utility::memory const & cMemory) throw (std::runtime_error) { qkd::module::message cMessage; cMessage.data() << cMemory; send(cMessage); };


    /**
     * << operator
     *
     * send a message
     *
     * @param   cMessage        message to send
     * @return  true for success
     */
    inline void operator<<(qkd::module::message & cMessage) throw (std::runtime_error) {  send(cMessage); };


    /**
     * get the internal module
     *
     * @return  the module used for sending/receiving
     */
    module * mod() { return m_cModule; };


    /**
     * read a message from the peer module
     *
     * this is a facade wrap to module's recv method
     * 
     * this call is blocking (with respect to timout)
     * 
     * The nTimeOut value is interpreted in these ways:
     * 
     *      n ...   wait n milliseconds for an reception of a message
     *      0 ...   do not wait: get the next message and return
     *     -1 ...   wait infinite (must be interrupted: see interrupt_worker())
     *     
     *      the value of std::numeric_limits< int >::min() means: no change to the
     *      current timeout setting
     * 
     * The given message object will be deleted with delet before assigning new values.
     * Therefore if message receive has been successful the message is not NULL
     * 
     * This call waits explcitly for the next message been of type eType. If this
     * is NOT the case a exception is thrown.
     * 
     * @param   cMessage            this will receive the message
     * @param   eType               message type to receive
     * @param   nTimeOut            timeout in ms
     * @return  true, if we have receuived a message
     */
    bool recv(qkd::module::message & cMessage, qkd::module::message_type eType = qkd::module::message_type::MESSAGE_TYPE_DATA, int nTimeOut = std::numeric_limits< int >::min()) throw (std::runtime_error); 


    /**
     * send a message to the peer module
     * 
     * this is a facade wrap to module's send method
     * 
     * this call is blocking (with respect to timout)
     * 
     * The nTimeOut value is interpreted in these ways:
     * 
     *      n ...   wait n milliseconds
     *      0 ...   do not wait
     *     -1 ...   wait infinite (must be interrupted: see interrupt_worker())
     *     
     *      the value of std::numeric_limits< int >::min() means: no change to the
     *      current timeout setting
     * 
     * this call is blocking
     * 
     * Note: this function takes ownership of the message's data sent! 
     * Afterwards the message's data will be void
     * 
     * @param   cMessage            the message to send
     * @param   nTimeOut            timeout in ms
     */
    void send(qkd::module::message & cMessage, int nTimeOut = std::numeric_limits< int >::min()) throw (std::runtime_error);


private:


    module * m_cModule;                                         /**< module wrapped */
    qkd::crypto::crypto_context & m_cIncomingContext;           /**< incoming auth context */
    qkd::crypto::crypto_context & m_cOutgoingContext;           /**< outgoing auth context */

};


}

}

#endif


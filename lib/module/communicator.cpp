/*
 * communicator.cpp
 * 
 * Implementation of the communicator object
 *
 * Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *
 * Copyright (C) 2014-2016 AIT Austrian Institute of Technology
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

// ait
#include <qkd/crypto/context.h>
#include <qkd/module/communicator.h>
#include <qkd/module/module.h>


using namespace qkd::module;


// ------------------------------------------------------------
// code


/**
 * read a message from the peer module
 *
 * this is a facade wrap to module's recv method
 * 
 * this call is blocking (with respect to timout)
 * 
 * The given message object will be deleted with delet before assigning new values.
 * Therefore if message receive has been successful the message is not NULL
 * 
 * This call waits explcitly for the next message been of type eType. If this
 * is NOT the case a exception is thrown.
 * 
 * @param   cMessage            this will receive the message
 * @param   eType               message type to receive
 * @return  true, if we have receuived a message
 */
bool communicator::recv(qkd::module::message & cMessage, qkd::module::message_type eType) { 
    return m_cModule->recv(cMessage, m_cIncomingContext, eType);
}


/**
 * send a message to the peer module
 * 
 * this is a facade wrap to module's send method
 * 
 * this call is blocking
 * 
 * Note: this function takes ownership of the message's data sent! 
 * Afterwards the message's data will be void
 * 
 * Sending might fail on interrupt.
 *
 * @param   cMessage            the message to send
 * @returns true, if the message has been sent
 */
bool communicator::send(qkd::module::message & cMessage) { 
    return m_cModule->send(cMessage, m_cOutgoingContext); 
}       



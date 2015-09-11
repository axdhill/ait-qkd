/*
 * protocol.cpp
 *
 * implement the abstract protocol class
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

 
// ------------------------------------------------------------
// incs

// ait
#include <qkd/q3p/engine.h>
#include <qkd/utility/debug.h>
#include <qkd/utility/syslog.h>

#include "protocol.h"


using namespace qkd::q3p::protocol;


// ------------------------------------------------------------
// code


/**
 * ctor
 * 
 * @param   cSocket     the socket we operate on
 * @param   cEngine     the parent engine
 * @throws  protocol_no_engine
 */
protocol::protocol(QAbstractSocket * cSocket, qkd::q3p::engine_instance * cEngine) : QObject(cSocket), m_cEngine(cEngine), m_cSocket(cSocket) {
    if (!m_cEngine) throw qkd::q3p::protocol::protocol::protocol_no_engine();
}


/**
 * maximum size of a packet
 * 
 * This is the maximum size a packet can have
 * minus max. trailer (auth-tag)
 * 
 * @return  maximum size of a packet in bytes
 */
uint64_t protocol::max_size() {
    const uint64_t MAX_AUTH_TAG = 256 / 8;
    return (PACKET_MAX_SIZE - MAX_AUTH_TAG);
}


/**
 * give a human readable description of the error
 * 
 * @param   eError      the error questioned
 * @return  a string describing the error
 */
QString protocol::protocol_error_description(protocol_error eError) {
    
    switch (eError) {
        
    case protocol_error::PROTOCOL_ERROR_NO_ERROR: return "no error";
    case protocol_error::PROTOCOL_ERROR_PENDING: return "data missing, more to come";
    case protocol_error::PROTOCOL_ERROR_ENGINE: return "no Q3P engine present to handle protocol data";
    case protocol_error::PROTOCOL_ERROR_SOCKET: return "socket error";
    case protocol_error::PROTOCOL_ERROR_CONNECTION_LOST: return "connection lost";
    case protocol_error::PROTOCOL_ERROR_TIMEOUT: return "timeout";
    case protocol_error::PROTOCOL_ERROR_ANSWER: return "invalid message from peer";
    case protocol_error::PROTOCOL_ERROR_PACKET_SIZE: return "packet size too big";
    case protocol_error::PROTOCOL_ERROR_CONFIG: return "local and peer configuration does not match";
    case protocol_error::PROTOCOL_ERROR_CHANNEL: return "q3p channel could not perform operation";
    case protocol_error::PROTOCOL_ERROR_ROLE: return "wrong role (master/slave) to handle data";
    case protocol_error::PROTOCOL_ERROR_NOT_IMPLEMENTED: return "!! not yet implemented !!";
        
    }
    
    return "unkown error";
}


/**
 * give a human readable string for a protocol_id
 * 
 * @param   nProtocolId     the protocol_id questioned
 * @return  a string version of a protocol_id
 */
std::string const & protocol::protocol_id_name(uint8_t nProtocolId) {
    
    // names we know
    static const std::string sProtocolIdNames[] = {
        "HANDSHAKE",
        "LOAD",
        "LOAD-REQUEST",
        "STORE",
        "DATA"
    };
    
    // the: "Me, Oliver, forgot to name a sub-protocol of Q3P accordingly - branch"
    static const std::string sProtocolIdUnknown = "UNKNOWN";
    
    // good id
    if (nProtocolId <= (uint8_t)protocol_type::PROTOCOL_DATA) return sProtocolIdNames[nProtocolId];

    // bad id --> tell Oliver, to fix that if you encounter this line!
    return sProtocolIdUnknown;
}


/**
 * parse data from the peer
 * 
 * the read buffer is examined if it containes
 * a Q3P message. If so, the message is removed and parsed
 * 
 * @param   cBuffer         read buffer which may contain a Q3P message
 * @param   cMessage        the message read
 * @param   eProtocol       the identified protocol the received message belongs
 * @return  an protocol error variable
 */
protocol_error protocol::recv(QByteArray & cBuffer, qkd::q3p::message & cMessage, protocol_type & eProtocol) {
    
    // we need the length of the Q3P message at a minimum
    if (cBuffer.size() < 4) return protocol_error::PROTOCOL_ERROR_PENDING;
    
    // check the first particle: length
    uint32_t nPacketSize = 0;
    nPacketSize = be32toh(*((uint32_t *)cBuffer.data()));

    // don't proceed if we ain't got enough to work on
    if ((uint32_t)cBuffer.size() < nPacketSize) return protocol_error::PROTOCOL_ERROR_PENDING;
    
    // cut off the first packet and leave the remaining piece intakt
    QByteArray cPacket = cBuffer.left(nPacketSize);
    cBuffer = cBuffer.right(cBuffer.size() - nPacketSize);
    
    return recv_packet(cPacket, cMessage, eProtocol);
}
    
    
/**
 * read data from the peer
 * 
 * if data is available a Q3P packet is taken from
 * the socket and dispatched accordingly
 * 
 * @param   cPacket         the packet read from the socket
 * @param   cMessage        the message read
 * @param   eProtocol       the identified protocol the received message belongs
 * @return  an protocol error variable
 */
protocol_error protocol::recv_packet(QByteArray & cPacket, qkd::q3p::message & cMessage, protocol_type & eProtocol) {
    
    // the cPacket IS the message
    cMessage = qkd::q3p::message();
    cMessage.resize(cPacket.size());
    memcpy(cMessage.get(), cPacket.data(), cPacket.size());

    // extract the Q3P version number
    if (cMessage.version() != 2) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "received malformed data from peer: Q3P version mismatch - dropping incoming bytes";
        return protocol_error::PROTOCOL_ERROR_ANSWER;
    }
    
    // fetch the command
    eProtocol = (protocol_type)(cMessage.protocol_id());

    // record timestamp
    cMessage.record_timestamp();

    return protocol_error::PROTOCOL_ERROR_NO_ERROR;
}


/**
 * run the protocol
 */
void protocol::run() { 
    
    // sanity check
    if (!socket()) { 
        emit failed((uint8_t)protocol_error::PROTOCOL_ERROR_SOCKET); 
        return; 
    } 
    
    // activate timer
    QTimer * cTimer = new QTimer(this);
    connect(cTimer, SIGNAL(timeout()), SLOT(timeout()));
    cTimer->setInterval(1000);
    cTimer->setSingleShot(false);
    cTimer->start();
    
    run_internal(); 
}


/**
 * send to the peer
 * 
 * @param   cMessage        message to send to the peer
 * @return  an protocol error variable
 */
protocol_error protocol::send(qkd::q3p::message & cMessage) {
    
    // fix protocol id in the message
    cMessage.set_protocol_id(protocol_id());
    
    // precheck size: single packet fragments may not exceed 32 bit size limits
    if (cMessage.size() >= max_size()) return protocol_error::PROTOCOL_ERROR_PACKET_SIZE;
    
    // get the current security facility
    qkd::q3p::channel & cChannel = engine()->channel();

    // encode the message: apply encryption and authentication
    qkd::q3p::channel_error eChannelError = cChannel.encode(cMessage);
    if (eChannelError != qkd::q3p::channel_error::CHANNEL_ERROR_NO_ERROR) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": "  
        << "Failed to encode message on channel #" << cChannel.id() 
        << " encoding message returned: " << (uint8_t)eChannelError 
        << " (" << qkd::q3p::channel::channel_error_description(eChannelError) << ")";
        return protocol_error::PROTOCOL_ERROR_CHANNEL;
    }
    
    // write message to socket
    uint64_t nWritten = 0;
    while (nWritten < cMessage.size()) {
        long nCurrentWrite = socket()->write((char *)(cMessage.get() + nWritten), cMessage.size() - nWritten);
        if (nCurrentWrite == -1) break;
        nWritten += nCurrentWrite;
    }
    
    // troubles?
    if (nWritten != cMessage.size()) return protocol_error::PROTOCOL_ERROR_SOCKET;

    // enforce sending
    socket()->flush();
    
    // record timestamp
    cMessage.record_timestamp();
    
    // debug?
    if (qkd::utility::debug::enabled()) {
        std::string sMessage = cMessage.str();
        qkd::utility::debug() << "<Q3P-SEND>" << sMessage;
    }

    return protocol_error::PROTOCOL_ERROR_NO_ERROR;
}


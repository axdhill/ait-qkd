/*
 * protocol.h
 * 
 * this is the base abstract protocol class
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

 
#ifndef __QKD_Q3P_PROTOCOL_PROTOCOL_H_
#define __QKD_Q3P_PROTOCOL_PROTOCOL_H_


// ------------------------------------------------------------
// incs

#include <inttypes.h>

#include <boost/shared_ptr.hpp>

// Qt
#include <QtCore/QObject>
#include <QtCore/QTimer>
#include <QtNetwork/QAbstractSocket>

// ait
#include <qkd/q3p/message.h>
#include <qkd/utility/buffer.h>
#include <qkd/utility/memory.h>


// ------------------------------------------------------------
// defs


/**
 * maximum packet size in bytes
 * 
 * This defines also the maximum size of a single Q3P message.
 * This size is INCLUDING all meta data (header, tags, ...)
 * 
 * Currently: 16 MB
 */
#define PACKET_MAX_SIZE         16 * 1024 * 1024



// ------------------------------------------------------------
// decls


namespace qkd {
    
namespace q3p {    

// fwd
class engine_instance;
    
namespace protocol {    
    

/**
 * the error conditions we know
 */
enum class protocol_error : uint8_t {
    
    PROTOCOL_ERROR_NO_ERROR = 0,                /**< not an error */
    PROTOCOL_ERROR_PENDING,                     /**< not an error: data pending to proceed */
    PROTOCOL_ERROR_ENGINE,                      /**< no Q3P engine present to handle protocol data */
    PROTOCOL_ERROR_SOCKET,                      /**< inappropriate socket instance */
    PROTOCOL_ERROR_CONNECTION_LOST,             /**< connection lost */
    PROTOCOL_ERROR_TIMEOUT,                     /**< opertion did not finish within time constraint */
    PROTOCOL_ERROR_ANSWER,                      /**< an unexpected protocol answer was received */
    PROTOCOL_ERROR_PACKET_SIZE,                 /**< packet too big */
    PROTOCOL_ERROR_CONFIG,                      /**< local and peer have inappropriate configs */
    PROTOCOL_ERROR_CHANNEL,                     /**< channel was unable to peform work */
    PROTOCOL_ERROR_ROLE,                        /**< wrong role (master/slave) to handle data */
    PROTOCOL_ERROR_NOT_IMPLEMENTED              /**< not implemented yet */
};


/**
 * the protocols we know
 */
enum class protocol_type : uint8_t {
    
    PROTOCOL_HANDSHAKE = 0,                     /**< handshake protocol */
    PROTOCOL_LOAD,                              /**< master->slave LOAD protocol */
    PROTOCOL_LOAD_REQUEST,                      /**< slave -> master LOAD-REQUEST protocol */
    PROTOCOL_STORE,                             /**< master->slave STORE protocol */
    PROTOCOL_DATA                               /**< DATA protocol */
};


/**
 * This is the Q3P KeyStore to KeyStore Handshake Protocol template.
 * 
 * All Q3P protocol instances are derived from this class.
 */
class protocol : public QObject {
    

    Q_OBJECT
    
    
public:
    
    
    /**
     * exception type thrown if we don't have an engine
     */
    struct protocol_no_engine : virtual std::exception, virtual boost::exception { };
    
    
    /**
     * ctor
     * 
     * @param   cSocket     the socket we operate on
     * @param   cEngine     the parent engine
     * @throws  protocol_no_engine
     */
    protocol(QAbstractSocket * cSocket, qkd::q3p::engine_instance * cEngine);
    
    
    /**
     * give a human readable description of the error
     * 
     * @param   eError      the error questioned
     * @return  a string describing the error
     */
    static QString protocol_error_description(protocol_error eError);
    
    
    /**
     * create the Q3P message header
     * 
     * this applies the header data to the message
     * 
     * @param   cMessage        message to send
     */
    void create_header(qkd::q3p::message & cMessage);
    
    
    /**
     * return the Q3P engine
     * 
     * @return  the Q3P engine associated
     */
    qkd::q3p::engine_instance * engine() { return m_cEngine; };
    
    
    /**
     * return the Q3P engine
     * 
     * @return  the Q3P engine associated
     */
    qkd::q3p::engine_instance const * engine() const { return m_cEngine; };
    
    
    /**
     * maximum size of a packet
     * 
     * This is the maximum size a packet can have
     * minus max. trailer (auth-tag)
     * 
     * @return  maximum size of a packet in bytes
     */
    static uint64_t max_size();
    
    
    /**
     * get the protocol type
     * 
     * @return  the protocol type
     */
    uint8_t protocol_id() const { return (uint8_t)protocol_id_internal(); }
    
    
    /**
     * give a human readable string for a protocol_id
     * 
     * @param   nProtocolId     the protocol_id questioned
     * @return  a string version of a protocol_id
     */
    static std::string const & protocol_id_name(uint8_t nProtocolId);
    
    
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
    static protocol_error recv(QByteArray & cBuffer, qkd::q3p::message & cMessage, protocol_type & eProtocol);
    
    
    /**
     * process a message received
     * 
     * @param   cMessage        the message read
     * @return  an protocol error variable
     */
    protocol_error recv(qkd::q3p::message & cMessage) { return recv_internal(cMessage); };
    
    
    /**
     * send to the peer
     * 
     * @param   cMessage        message to send to the peer
     * @return  an protocol error variable
     */
    protocol_error send(qkd::q3p::message & cMessage);
    
    
    /**
     * return the socket
     * 
     * @return  the socket we operate on
     */
    QAbstractSocket * socket() { return m_cSocket; };


    /**
     * return the socket
     * 
     * @return  the socket we operate on
     */
    QAbstractSocket const * socket() const { return m_cSocket; };
    
    
    /**
     * test if we have a living connection on the socket
     * 
     * @return  true, if we are well connected
     */
    bool valid_socket() const { if (!socket()) return false; if (!socket()->isValid()) return false; if (!socket()->state() == QAbstractSocket::ConnectedState) return false; return true; };
    
    
public slots:
    
    
    /**
     * run the protocol
     */
    void run();
    

private slots:
    
    
    /**
     * called every second to detect timeouts
     */
    void timeout() { timeout_internal(); };
    
    
signals:
    

    /**
     * handshake failed
     * 
     * the reason is a protocol::protocol_error value
     * 
     * @param   nReason     the reson why the protocol failed
     */
    void failed(uint8_t nReason);
    
    
    /**
     * handshake succeeded
     */
    void success();
    
    
private:
    
    
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
    static protocol_error recv_packet(QByteArray & cPacket, qkd::q3p::message & cMessage, protocol_type & eProtocol);
    
    
    /**
     * process a message received
     * 
     * @param   cMessage        the message read
     * @return  an protocol error variable
     */
    virtual protocol_error recv_internal(qkd::q3p::message & cMessage) = 0;
    
    
    /**
     * concrete run method
     * 
     * this is called when the protocol is to run
     */
    virtual void run_internal() = 0;
    
    
    /**
     * concrete timeout method
     * 
     * this is called every second to detect a timeout
     */
    virtual void timeout_internal() = 0;
    
    
    /**
     * get the protocol type
     * 
     * @return  the protocol type
     */
    virtual protocol_type protocol_id_internal() const = 0;
    
    
    /**
     * the Q3P engine
     */
    qkd::q3p::engine_instance * m_cEngine;
    
    
    /**
     * the socket we operate on
     */
    QAbstractSocket * m_cSocket;
    
};
  

}

}

}


#endif

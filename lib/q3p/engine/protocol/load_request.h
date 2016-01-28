/*
 * load_request.h
 * 
 * this is the Q3P KeyStore to KeyStore LOAD-REQUEST protocol
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

 
#ifndef __QKD_Q3P_PROTOCOL_LOAD_REQUEST_H_
#define __QKD_Q3P_PROTOCOL_LOAD_REQUEST_H_


// ------------------------------------------------------------
// incs

#include <boost/shared_ptr.hpp>

// Qt
#include <QtCore/QObject>
#include <QtNetwork/QAbstractSocket>

// ait
#include "key_move.h"


// ------------------------------------------------------------
// decls


namespace qkd {
    
namespace q3p {    

namespace protocol {    

    
/**
 * This is the Q3P KeyStore to KeyStore LOAD-REQUEST Protocol.
 * 
 * The LOAD-REQUEST protocol loads keys from the common store to the
 * master's outgoing buffer (== slave's incoming) and application buffer.
 * 
 * It is very similar to the LOAD protocol, but triggered by the slave.
 * 
 * It is:
 * 
 *  Master                                               Slave
 *    |                                                    |
 *    |                       MsgId-S-1, "LOAD-REQ", Bytes |
 *    |     <----------------------------------------------|
 *    |                                                    |
 *    | MsgId-M-1, "LOAD",                                 | 
 *    |    "OUTGOING", "C", CS-Key+, "O", Buffer-Key+      |
 *    |    "APPLICAT", "C", CS-Key+, "I", Buffer-Key+      |
 *    |    AUTH                                            |
 *    |----------------------------------------------->    |
 *    |                                                    |
 *    |                  MsgId-S-1, "LOAD-ACK", MsgId-M-1, |
 *    |                               "OUTGOING", CS-Key*, |
 *    |                               "APPLICAT", CS-Key*, |
 *    |                                               AUTH |
 *    |     <----------------------------------------------|
 *    |                                                    |
 *
 * Particles:
 * 
 *    MsgId-M-1     Message ID 1 of the Master
 *    MsgId-S-1     Message ID 1 of the Slave
 * 
 *    "LOAD"        a string stating "LOAD"
 *    "LOAD-ACK"    a string stating "LOAD"-Acknowledgment
 *    "INCOMING"    a string marking the beginning of keys to move to the incoming buffer
 *    "I"           a string "I"
 *    "C"           a string "C"
 *    "A"           a string "A"
 *    "APPLICAT"    a string marking the beginning of keys to move to the incoming buffer
 * 
 *    CS-Key        a key id of a real sync key in the common store
 *    Buffer-Key    a key id of a buffer (incoming, outgoing, application)
 *    AUTH          authentication tag
 * 
 * Steps (short and brief):
 * 
 *  A.  The slave checks how many bytes he wishes to transfer to his incoming buffer.
 *      If non 0 the slave tells the master a "LOAD-REQ" with this amount of bytes.
 * 
 *  B.  On reception, the master picks real-sync keys from the common store (CS-Key)
 *      and assigns a number of buffer keys to this. He then composes a "LOAD" message
 *      for the slave.
 * 
 *  C.  The slave moves the keys from the common store to the buffers and
 *      respondes with the list of successfull moved keys.
 * 
 *  D.  On reception of the master moves the keys from the common store
 *      to the buffers.
 * 
 * Hence, this is not a full disussion of the protocol. Look for accompanying
 * documents about Q3P.
 */
class load_request : public key_move {
    

    Q_OBJECT
    
    
public:


    /**
     * ctor
     * 
     * @param   cSocket     the socket we operate on
     * @param   cEngine     the parent engine
     * @throws  protocol_no_engine
     */
    load_request(QAbstractSocket * cSocket, qkd::q3p::engine_instance * cEngine);


private:
    
    
    /**
     * process a message received
     * 
     * @param   cMessage        the message read
     * @return  an protocol error variable
     */
    protocol_error recv_internal(qkd::q3p::message & cMessage);
    
    
    /**
     * process a message "LOAD" received
     * 
     * @param   cMessage        the message read
     */
    protocol_error recv_LOAD(qkd::q3p::message & cMessage);
    
    
    /**
     * process a message "LOAD-ACK" received
     * 
     * @param   cMessage        the message read
     */
    protocol_error recv_LOAD_ACK(qkd::q3p::message & cMessage);
    
    
    /**
     * process a message "LOAD-REQ" received
     * 
     * @param   cMessage        the message read
     */
    protocol_error recv_LOAD_REQ(qkd::q3p::message & cMessage);
    
    
    /**
     * protocol starts
     */
    void run_internal();
    
    
    /**
     * timer event: check for timeout
     */
    void timeout_internal();
    
    
    /**
     * get the protocol type
     * 
     * @return  the protocol type
     */
    protocol_type protocol_id_internal() const { return protocol_type::PROTOCOL_LOAD_REQUEST; };

    
    // pimpl
    class load_request_data;
    boost::shared_ptr<load_request_data> d;
};
  

}

}

}


#endif

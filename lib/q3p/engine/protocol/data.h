/*
 * data.h
 * 
 * this is the Q3P KeyStore to KeyStore DATA protocol
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

 
#ifndef __QKD_Q3P_PROTOCOL_DATA_H_
#define __QKD_Q3P_PROTOCOL_DATA_H_


// ------------------------------------------------------------
// incs

#include <boost/shared_ptr.hpp>

// Qt
#include <QtCore/QObject>
#include <QtNetwork/QAbstractSocket>

//ait
#include "protocol.h"


// ------------------------------------------------------------
// decls


namespace qkd {
    
namespace q3p {    

namespace protocol {    

    
/**
 * This is the Q3P KeyStore to KeyStore DATA Protocol.
 */
class data : public protocol {
    

    Q_OBJECT
    
    
public:


    /**
     * ctor
     * 
     * @param   cSocket     the socket we operate on
     * @param   cEngine     the parent engine
     * @throws  protocol_no_engine
     */
    data(QAbstractSocket * cSocket, qkd::q3p::engine_instance * cEngine);

    
private:
    
    
    /**
     * process a message received
     * 
     * @param   cMessage        the message read
     * @return  an protocol error variable
     */
    protocol_error recv_internal(qkd::q3p::message & cMessage);
    
    
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
    protocol_type protocol_id_internal() const { return protocol_type::PROTOCOL_DATA; };
    
};
  

}

}

}


#endif

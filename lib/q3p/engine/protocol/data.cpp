/*
 * data.cpp
 *
 * implement the Q3P KeyStore to Q3P KeyStore DATA protocol
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
#include <qkd/utility/syslog.h>

#include "data.h"


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
data::data(QAbstractSocket * cSocket, qkd::q3p::engine_instance * cEngine) : protocol(cSocket, cEngine) {
}


/**
 * process a message received
 * 
 * @param   cMessage        the message read
 * @return  an protocol error variable
 */
protocol_error data::recv_internal(qkd::q3p::message & cMessage) {
    
    // sanity check
    if (!engine()) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "DATA protocol without an engine! This is a bug.";
        return protocol_error::PROTOCOL_ERROR_ENGINE;
    }
    
    // get the payload
    qkd::utility::memory cPayload;
    cMessage >> cPayload;
    
    // write to the NIC
    engine()->recv_data(cPayload);

    return protocol_error::PROTOCOL_ERROR_NO_ERROR;
}


/**
 * protocol starts
 */
void data::run_internal() {
    
    // sanity check
    if (!engine()) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "DATA protocol without an engine! This is a bug.";
        return;
    }
    
    // the DATA protocol instance is not "run"
    // it is simply a send->recv call
    // that's all there to it and say about
    // over, roger and out
}


/**
 * timer event: check for timeout
 */
void data::timeout_internal() {
    
    // as the DATA protocol is not run
    // we do not expect any reply. So we
    // do not wait for anything and 
    // that's why this is an empty 
    // function ... no "timeout" possible
}


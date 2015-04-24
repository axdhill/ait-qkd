/*
 * store.cpp
 *
 * implement the Q3P KeyStore to Q3P KeyStore STORE protocol
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

 
// ------------------------------------------------------------
// incs

#include <iostream>


// ait
#include <qkd/q3p/engine.h>
#include <qkd/utility/syslog.h>

#include "store.h"


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
store::store(QAbstractSocket * cSocket, qkd::q3p::engine_instance * cEngine) : protocol(cSocket, cEngine) {
}


/**
 * process a message received
 * 
 * @param   cMessage        the message read
 */
protocol_error store::recv_internal(UNUSED qkd::q3p::message & cMessage) {
    std::cout << "STORE message received: implementation missing" << std::endl;
    return protocol_error::PROTOCOL_ERROR_NOT_IMPLEMENTED;
}


/**
 * protocol starts
 */
void store::run_internal() {
    
    // sanity check
    if (!engine()) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "STORE protocol without an engine! This is a bug.";
        return;
    }
    
    // this is a master only protocol
    if (!engine()->master()) return;
}


/**
 * timer event: check for timeout
 */
void store::timeout_internal() {
    // no timeout
}


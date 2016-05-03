/*
 * qkd-sync.cpp
 * 
 * This is the implementation of the QKD postprocessing sync facilities
 * 
 * Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *
 * Copyright (C) 2016 AIT Austrian Institute of Technology
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
#include <qkd/crypto/engine.h>
#include <qkd/utility/syslog.h>

#include "qkd-sync.h"


// ------------------------------------------------------------
// defs

#define MODULE_DESCRIPTION      "This is the qkd-sync QKD Module."
#define MODULE_ORGANISATION     "(C)opyright 2016 AIT Austrian Institute of Technology, http://www.ait.ac.at"


// ------------------------------------------------------------
// code


/**
 * ctor
 */
qkd_sync::qkd_sync() : qkd::module::module("sync", qkd::module::module_type::TYPE_OTHER, MODULE_DESCRIPTION, MODULE_ORGANISATION) {
}


/**
 * module work
 * 
 * @param   cKey                    the key just read from the input pipe
 * @param   cIncomingContext        incoming crypto context
 * @param   cOutgoingContext        outgoing crypto context
 * @return  true, if the key is to be pushed to the output pipe
 */
bool qkd_sync::process(UNUSED qkd::key::key & cKey, UNUSED qkd::crypto::crypto_context & cIncomingContext, UNUSED qkd::crypto::crypto_context & cOutgoingContext) {
    
    // ensure we are talking about the same stuff with the peer
    if (!is_synchronizing()) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " 
                << "you deliberately turned off key synchronizing in resizing - but this is essential for this module: dropping key";
        return false;
    }
    
    return true;
}

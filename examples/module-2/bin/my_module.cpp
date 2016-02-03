/*
 * my_module.cpp
 * 
 * This is the implementation file for a sample QKD module
 * 
 * This module just simply puts "Hello World!" as key to
 * stdout and quits. This is totally nonsense but gives a hint
 * to very, very low requirements in building a module.
 * 
 * Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *
 * Copyright (C) 2013-2016 AIT Austrian Institute of Technology
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

// the module
#include "my_module.h"


// ------------------------------------------------------------
// code


/**
 * constructor
 */
my_module::my_module() : 
    qkd::module::module(
        "my-module", 
        qkd::module::module_type::TYPE_OTHER, 
        "This is example module #2: same as module-1 ... but better (CMake support).", 
        "Place in here your organisation/company.") {
        
}


/**
 * module work
 */
bool my_module::process(qkd::key::key & cKey, qkd::crypto::crypto_context & cIncomingContext, qkd::crypto::crypto_context & cOutgoingContext) {

    if (keys_outgoing() > 0) { 
        
        // at least we had a key already: terminate module!
        std::cout.flush();
        terminate();
        return false;
    }
    
    // the new key is "Hello World!" ...
    qkd::utility::buffer buf;
    buf << std::string("Hello World!");
    cKey.data() = buf;

    return true;
}

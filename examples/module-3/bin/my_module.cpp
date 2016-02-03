/*
 * my_module.cpp
 * 
 * This is the implementation file for a sample QKD module
 * 
 * This module counts the bits set in the bypassing keys
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
        "This is example module #3: count the bits set of bypassing keys.", 
        "Place in here your organisation/company.") {
        
}


/**
 * module work
 */
bool my_module::process(qkd::key::key & cKey, qkd::crypto::crypto_context & cIncomingContext, qkd::crypto::crypto_context & cOutgoingContext) {
    
    // get the number of bits set in the key
    // by converting the key data to a bigint
    // which enables sophisticated bit operations
    
    qkd::utility::bigint bi = qkd::utility::bigint(cKey.data());
    
    // dump the data
    std::cerr 
            << "key id: " << cKey.id() 
            << " length of key (bytes): " << cKey.data().size() 
            << " bits set: " << bi.bits_set() 
            << " ratio: " << bi.bits_set() * 100.0 / (cKey.data().size() * 8) << "%" 
            << std::endl;

    return true;
}

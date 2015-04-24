/*
 * my_module.cpp
 * 
 * This is the implmentation file for a sample QKD module
 * 
 * This module counts the bits set in the bypassing keys
 * 
 * Autor: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *
 * Copyright (C) 2013-2015 AIT Austrian Institute of Technology
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

// get the DBus Adaptor
#include "my_module_dbus.h"


// ------------------------------------------------------------
// code


/**
 * constructor
 */
my_module::my_module() : 
    qkd::module::module(
        "my-module", 
        qkd::module::module_type::TYPE_OTHER, 
        "This is example module #5: give the last MD5 checksum on DBus.", 
        "Place in here your organisation/company.") {    

    // enforce DBus registration
    new My_moduleAdaptor(this);
}


/**
 * return the last known MD5 checksum
 */
QString my_module::last_md5() {
    return _last_md5;
}


/**
 * module work
 */
bool my_module::process(qkd::key::key & cKey, qkd::crypto::crypto_context & cIncomingContext, qkd::crypto::crypto_context & cOutgoingContext) {
    
    // get the number of bits set in the key
    // by converting the key data to a bigint
    // which enables sophisticated bit operations
    
    qkd::utility::bigint bi = qkd::utility::bigint(cKey.data());
    
    // create my MD5 hashsum
    qkd::utility::checksum md5_algorithm = qkd::utility::checksum_algorithm::create("md5");
    md5_algorithm << cKey.data();
    qkd::utility::memory md5_checksum;
    md5_algorithm >> md5_checksum;
    
    // space for the peer's checksum
    qkd::utility::memory md5_checksum_peer;
    
    // exchange information with peer
    
    // ... as alice
    if (is_alice()) {
        
        // send our md5 checksum to bob
        qkd::module::message msg;
        msg.data() << md5_checksum;
        send(msg, cOutgoingContext);
        
        // get bob's answer (right into the same message object)
        recv(msg, cIncomingContext);
        
        // extract bob's answer
        msg.data() >> md5_checksum_peer;

    }
    
    // ... as bob
    if (is_bob()) {
        
        // receive alice's md5 checksum
        qkd::module::message msg;
        recv(msg, cIncomingContext);
        msg.data() >> md5_checksum_peer;
        
        // reinit the message object
        msg = qkd::module::message();
        msg.data() << md5_checksum;
        send(msg, cOutgoingContext);
    }
    
    // remember the last MD5 checksum
    _last_md5 = QString::fromStdString(md5_checksum.as_hex());
    
    // for output: give a texted role name
    std::string role = (is_alice() ? "alice" : "bob");
    
    // dump the data
    std::cerr 
            << "I am " << role
            << " key id: " << cKey.id() 
            << " length of key (bytes): " << cKey.data().size() 
            << " bits set: " << bi.bits_set() 
            << " ratio: " << bi.bits_set() * 100.0 / (cKey.data().size() * 8) << "%" 
            << " my MD5 sum: " << md5_checksum.as_hex()
            << " peer's MD5 sum: " << md5_checksum_peer.as_hex()
            << std::endl;

    return true;
}

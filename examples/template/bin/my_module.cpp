/*
 * my_module.cpp
 * 
 * This is the implementation file for an arbitrary QKD module
 * 
 * Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 * 
 *
 *       -----------------------------------------------------
 *       Please substitute the MY_MODULE_* placements as needed
 *       -----------------------------------------------------
 *
 *
 *
 * Copyright (C) 2014-2016 AIT Austrian Institute of Technology
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

/* Uncomment the lines below if you *do* have some DBus Properties, Methods or Signals defined
// get the DBus Adaptor
#include "my_module_dbus.h"
*/

// ------------------------------------------------------------
// code


/**
 * constructor
 */
my_module::my_module() : 
    qkd::module::module(
        "my-module", 
        qkd::module::module_type::TYPE_OTHER, 
        "MY_MODULE_DESCRIPTION here", 
        "MY_MODULE_COMPANY here.") {    

/* Uncomment the lines below if you *do* have some DBus Properties, Methods or Signals defined
    // enforce DBus registration
    new My_moduleAdaptor(this);
*/    
}


/**
 * module work
 * 
 * @param   cKey                the current new key to work on
 * @param   cIncomingContext    incoming authentication context used for receiving data from peer module
 * @param   cOutgoingContext    outgoing authentication context used when sending data to peer module
 * @return  true, if key should be forwarded to next module
 */
bool my_module::process(qkd::key::key & cKey, qkd::crypto::crypto_context & cIncomingContext, qkd::crypto::crypto_context & cOutgoingContext) {

    /*
     *  -------------
     *  MAIN KEY LOOP
     *  -------------
     * 
     * Insert code/algorithm/protocol to work on key here 
     */

    return true;
}

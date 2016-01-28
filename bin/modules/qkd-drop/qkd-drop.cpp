/*
 * qkd-drop.cpp
 * 
 * This is the implementation of the QKD postprocessing
 * drop facilities
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

// ait
#include <qkd/utility/atof.h>
#include <qkd/utility/syslog.h>

#include "qkd-drop.h"
#include "qkd_drop_dbus.h"


// ------------------------------------------------------------
// defs

#define MODULE_DESCRIPTION      "This is the qkd-drop QKD Module."
#define MODULE_ORGANISATION     "(C)opyright 2012-2016 AIT Austrian Institute of Technology, http://www.ait.ac.at"


// ------------------------------------------------------------
// decl


/**
 * the qkd-drop pimpl
 */
class qkd_drop::qkd_drop_data {
    
public:

    
    /**
     * ctor
     */
    qkd_drop_data() : nDropRatio(0.1) {};
    
    std::recursive_mutex cPropertyMutex;            /**< property mutex */
    
    double nDropRatio;                              /**< drop ratio */
};


// ------------------------------------------------------------
// code


/**
 * ctor
 */
qkd_drop::qkd_drop() : qkd::module::module("drop", qkd::module::module_type::TYPE_OTHER, MODULE_DESCRIPTION, MODULE_ORGANISATION) {

    d = boost::shared_ptr<qkd_drop::qkd_drop_data>(new qkd_drop::qkd_drop_data());
    
    // apply default values
    set_drop_ratio(0.05);

    // enforce DBus registration
    new DropAdaptor(this);
}


/**
 * apply the loaded key value map to the module
 * 
 * @param   sURL            URL of config file loaded
 * @param   cConfig         map of key --> value
 */
void qkd_drop::apply_config(UNUSED std::string const & sURL, qkd::utility::properties const & cConfig) {
    
    // delve into the given config
    for (auto const & cEntry : cConfig) {
        
        // grab any key which is intended for us
        if (!is_config_key(cEntry.first)) continue;
        
        // ignore standard config keys: they should have been applied already
        if (is_standard_config_key(cEntry.first)) continue;
        
        std::string sKey = cEntry.first.substr(config_prefix().size());

        // module specific config here
        if (sKey == "drop_ratio") {
            set_drop_ratio(qkd::utility::atof(cEntry.second.c_str()));
        }
        else {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "found unknown key: \"" << cEntry.first << "\" - don't know how to handle this.";
        }
    }
}


/**
 * get the drop ration for incoming keys
 * 
 * @return  the drop ration for incoming keys
 */
double qkd_drop::drop_ratio() const {
    
    // get exclusive access to properties
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    return d->nDropRatio;
}


/**
 * module work
 * 
 * @param   cKey                    the key to drop
 * @param   cIncomingContext        incoming crypto context
 * @param   cOutgoingContext        outgoing crypto context
 * @return  always true, when forward key
 */
bool qkd_drop::process(qkd::key::key & cKey, UNUSED qkd::crypto::crypto_context & cIncomingContext, UNUSED qkd::crypto::crypto_context & cOutgoingContext) {

    // roll a dice ...
    double nRandom = 0.0;
    random() >> nRandom;
    if (nRandom <= drop_ratio()) {
        qkd::utility::debug() << "dropping key " << cKey.id();
        return false;
    }
    
    return true;
}


/**
 * set the new drop ration for incoming keys
 * 
 * @param   nRatio          the new drop ration for incoming keys
 */
void qkd_drop::set_drop_ratio(double nRatio) {
    
    // limit input ratio to reasonable values
    double nBoundedRatio = std::min<double>(std::max<double>(nRatio, 0.0), 1.0);
    if (nBoundedRatio != nRatio) qkd::utility::debug() << "input ratio " << nRatio << " set to " << nBoundedRatio;
    
    // get exclusive access to properties
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    d->nDropRatio = nRatio;
}



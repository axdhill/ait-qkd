/*
 * qkd-throttle.cpp
 * 
 * This is the startup code for the qkd-throttle module.
 * 
 * The qkd-throttle slows down the key stream bypassing (handy for development)
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

// ait
#include <qkd/utility/syslog.h>

#include "qkd-throttle.h"
#include "qkd_throttle_dbus.h"


// ------------------------------------------------------------
// defs

#define MODULE_DESCRIPTION      "This is the qkd-throttle QKD Module: it slows down the bypassing keystream."
#define MODULE_ORGANISATION     "(C)opyright 2012, 2013 AIT Austrian Institute of Technology, http://www.ait.ac.at"


// ------------------------------------------------------------
// decl


/**
 * the qkd-throttle pimpl
 */
class qkd_throttle::qkd_throttle_data {
    
public:

    
    /**
     * ctor
     */
    qkd_throttle_data() {
        nMaxBitsPerSecond = 0.0;
        nMaxKeysPerSecond = 0.0;
    };
    
    std::recursive_mutex cPropertyMutex;            /**< property mutex */
    
    double nMaxBitsPerSecond;                       /**< maximum bits per seond */
    double nMaxKeysPerSecond;                       /**< maximum keys per seond */
    
    qkd::utility::average m_cBitsPerSecond;         /**< current bits per second */
    qkd::utility::average m_cKeysPerSecond;         /**< current keys per second */
};


// ------------------------------------------------------------
// code


/**
 * ctor
 */
qkd_throttle::qkd_throttle() : qkd::module::module("throttle", qkd::module::module_type::TYPE_OTHER, MODULE_DESCRIPTION, MODULE_ORGANISATION) {

    d = boost::shared_ptr<qkd_throttle::qkd_throttle_data>(new qkd_throttle::qkd_throttle_data());
    
    d->m_cBitsPerSecond = qkd::utility::average_technique::create("time", 1000);
    d->m_cKeysPerSecond = qkd::utility::average_technique::create("time", 1000);
    
    // apply default values
    set_max_bits_per_second(8192);
    set_max_keys_per_second(10);
    
    // enforce DBus registration
    new ThrottleAdaptor(this);
}


/**
 * apply the loaded key value map to the module
 * 
 * @param   sURL            URL of config file loaded
 * @param   cConfig         map of key --> value
 */
void qkd_throttle::apply_config(UNUSED std::string const & sURL, qkd::utility::properties const & cConfig) {
    
    // delve into the given config
    for (auto const & cEntry : cConfig) {
        
        // grab any key which is intended for us
        if (!is_config_key(cEntry.first)) continue;
        
        // ignore standard config keys: they should have been applied already
        if (is_standard_config_key(cEntry.first)) continue;
        
        std::string sKey = cEntry.first.substr(config_prefix().size());
        
        // module specific config here
        if (sKey == "max_bits_per_second") {
            std::stringstream ss(cEntry.second);
            uint64_t nMaxBitsPerSecond = 0;
            ss >> nMaxBitsPerSecond;
            set_max_bits_per_second(nMaxBitsPerSecond);
        }
        else
        if (sKey == "max_keys_per_second") {
            std::stringstream ss(cEntry.second);
            uint64_t nMaxKeysPerSecond = 0;
            ss >> nMaxKeysPerSecond;
            set_max_keys_per_second(nMaxKeysPerSecond);
        }
        else {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "found unknown key: \"" << cEntry.first << "\" - don't know how to handle this.";
        }
    }
}


/**
 * return the current bits per second
 * 
 * @return  the current bits per second
 */
double qkd_throttle::bits_per_second() const {
    
    // get exclusive access to properties
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    return d->m_cBitsPerSecond->sum();
}


/**
 * return the current keys per second
 * 
 * @return  the current keys per second
 */
double qkd_throttle::keys_per_second() const {
    
    // get exclusive access to properties
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    return d->m_cKeysPerSecond->sum();
}


/**
 * return the maximum bits per second
 * 
 * @return  the maximum bits per second
 */
double qkd_throttle::max_bits_per_second() const {
    
    // get exclusive access to properties
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    return d->nMaxBitsPerSecond;
}


/**
 * return the maximum keys per second
 * 
 * @return  the maximum keys per second
 */
double qkd_throttle::max_keys_per_second() const {
    
    // get exclusive access to properties
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    return d->nMaxKeysPerSecond;
}


/**
 * module work
 * 
 * @param   cKey                    the key to forward
 * @param   cIncomingContext        incoming crypto context
 * @param   cOutgoingContext        outgoing crypto context
 * @return  always true
 */
bool qkd_throttle::process(UNUSED qkd::key::key & cKey, UNUSED qkd::crypto::crypto_context & cIncomingContext, UNUSED qkd::crypto::crypto_context & cOutgoingContext) {
    
    // get the current values
    double nMaxBitsPerSecond = 0.0;
    double nMaxKeysPerSecond = 0.0;
    {
        std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
        nMaxBitsPerSecond = d->nMaxBitsPerSecond;
        nMaxKeysPerSecond = d->nMaxKeysPerSecond;
    }
        
    // do nothing if no maximum has been set
    if ((nMaxBitsPerSecond == 0.0) && (nMaxKeysPerSecond == 0.0)) return true;
    
    // add to average
    d->m_cBitsPerSecond << cKey.data().size() * 8;
    d->m_cKeysPerSecond << 1.0;
    
    // figure out if we should delay based on the incoming values
    bool bDelay = false;
    do {
       
        // TODO: this does not yet correctly work --> killer test takes way too long

        bool bDelayCausedByBits = false;
        bool bDelayCausedByKeys = false;
        volatile double nBitsPerSecond = bits_per_second();
        volatile double nKeysPerSecond = keys_per_second();
        if (nMaxBitsPerSecond != 0.0) bDelayCausedByBits = (nBitsPerSecond >= nMaxBitsPerSecond);
        if (nMaxKeysPerSecond != 0.0) bDelayCausedByKeys = (nKeysPerSecond >= nMaxKeysPerSecond);
        bDelay = (bDelayCausedByBits || bDelayCausedByKeys);
        
        // debug to the user
        if (qkd::utility::debug::enabled()) {
            
            auto cAgeInMS = std::chrono::duration_cast<std::chrono::milliseconds>(age());
            
            std::stringstream ss;
            ss << "time: " << cAgeInMS.count() << "ms ";
            ss << "current bps: " << nBitsPerSecond << "/" << nMaxBitsPerSecond << " ";
            ss << "current kps: " << nKeysPerSecond << "/" << nMaxKeysPerSecond << " ";
            ss << "forwarding: " << (bDelay ? "no" : "yes");
            
            qkd::utility::debug() << ss.str();
        }
        
        // wait for a while, if necessary
        if (bDelay) rest();
            
    } while (bDelay);
    
    return true;
}


/**
 * sets the maximum bits per second
 * 
 * @param   nMaximum        the new maximum (0 == no maximum)
 */
void qkd_throttle::set_max_bits_per_second(double nMaximum) {
    
    // get exclusive access to properties
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    d->nMaxBitsPerSecond = nMaximum;
}


/**
 * sets the maximum keys per second
 * 
 * @param   nMaximum        the new maximum (0 == no maximum)
 */
void qkd_throttle::set_max_keys_per_second(double nMaximum) {
    
    // get exclusive access to properties
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    d->nMaxKeysPerSecond = nMaximum;
}

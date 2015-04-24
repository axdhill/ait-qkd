/*
 * qkd-buffer.cpp
 * 
 * This is the implementation of the QKD postprocessing
 * buffer facilities
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

#include "qkd-buffer.h"
#include "qkd_buffer_dbus.h"


// ------------------------------------------------------------
// defs

#define MODULE_DESCRIPTION      "This is the qkd-buffer QKD Module."
#define MODULE_ORGANISATION     "(C)opyright 2012, 2013 AIT Austrian Institute of Technology, http://www.ait.ac.at"


// ------------------------------------------------------------
// decl


/**
 * the qkd-buffer pimpl
 */
class qkd_buffer::qkd_buffer_data {
    
public:

    
    /**
     * ctor
     */
    qkd_buffer_data() : 
        nMinimumKeySize(10000),
        nErrorBits(0),
        nDisclosedBits(0),
        nKeyBits(0) {};
    
    std::recursive_mutex cPropertyMutex;            /**< property mutex */
    
    uint64_t nMinimumKeySize;                       /**< minimum key size (in bytes) for forwarding */
    
    uint64_t nErrorBits;                            /**< all the error bits so far for the current key */
    uint64_t nDisclosedBits;                        /**< all disclosed bits so far for the current key */
    uint64_t nKeyBits;                              /**< all key bits so far for the current key */

    qkd::key::key cKey;                             /**< current key we work on */
    qkd::crypto::crypto_context cIncomingContext;   /**< current incoming crypto context */
    qkd::crypto::crypto_context cOutgoingContext;   /**< current outgoing crypto context */

    
};


// ------------------------------------------------------------
// code


/**
 * ctor
 */
qkd_buffer::qkd_buffer() : qkd::module::module("buffer", qkd::module::module_type::TYPE_OTHER, MODULE_DESCRIPTION, MODULE_ORGANISATION) {

    d = boost::shared_ptr<qkd_buffer::qkd_buffer_data>(new qkd_buffer::qkd_buffer_data());
    
    // apply default values
    set_min_key_size(2048);

    // enforce DBus registration
    new BufferAdaptor(this);
}


/**
 * apply the loaded key value map to the module
 * 
 * @param   sURL            URL of config file loaded
 * @param   cConfig         map of key --> value
 */
void qkd_buffer::apply_config(UNUSED std::string const & sURL, qkd::utility::properties const & cConfig) {
    
    // delve into the given config
    for (auto const & cEntry : cConfig) {
        
        // grab any key which is intended for us
        if (!is_config_key(cEntry.first)) continue;
        
        // ignore standard config keys: they should have been applied already
        if (is_standard_config_key(cEntry.first)) continue;
        
        std::string sKey = cEntry.first.substr(config_prefix().size());

        // module specific config here
        if (sKey == "min_key_size") {
            set_min_key_size(atoll(cEntry.second.c_str()));
        }
        else {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "found unknown key: \"" << cEntry.first << "\" - don't know how to handle this.";
        }
    }
}


/**
 * get the current key size (in bytes) for forwarding
 * 
 * @return  the current key size (in bytes) for forwarding
 */
qulonglong qkd_buffer::cur_key_size() const {
    
    // get exclusive access to properties
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    return d->cKey.data().size();
}


/**
 * get the minimum key size for forwarding
 * 
 * @return  the minimum key size for forwarding
 */
qulonglong qkd_buffer::min_key_size() const {
    
    // get exclusive access to properties
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    return d->nMinimumKeySize;
}


/**
 * module work
 * 
 * @param   cKey                    the key to buffer
 * @param   cIncomingContext        incoming crypto context
 * @param   cOutgoingContext        outgoing crypto context
 * @return  always true
 */
bool qkd_buffer::process(qkd::key::key & cKey, qkd::crypto::crypto_context & cIncomingContext, qkd::crypto::crypto_context & cOutgoingContext) {
    
    // ensure we are talking about the same stuff with the peer
    if (!is_synchronizing()) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "you deliberately turned off key synchonrizing in buffering - but this is essential fot this module: dropping key";
        return false;
    }

    // get exclusive access to properties
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    
    // is this the first time?
    if (d->cKey == qkd::key::key::null()) {
        
        // "consume" key if it has not been disclosed
        if (cKey.meta().eKeyState != qkd::key::key_state::KEY_STATE_DISCLOSED) {
            d->cKey = cKey;
            d->cIncomingContext = cIncomingContext;
            d->cOutgoingContext = cOutgoingContext;
        }
        
        // add meta values (even if disclosed)
        d->nErrorBits += cKey.meta().nErrorBits;
        d->nDisclosedBits += cKey.meta().nDisclosedBits;
        d->nKeyBits += cKey.data().size() * 8;
        
    }
    else {
        
        // extend our local greater key
        if (cKey.meta().eKeyState != qkd::key::key_state::KEY_STATE_DISCLOSED) {
            d->cKey.data().add(cKey.data());
        }
        
        // always add meta key values
        d->nErrorBits += cKey.meta().nErrorBits;
        d->nDisclosedBits += cKey.meta().nDisclosedBits;
        d->nKeyBits += cKey.data().size() * 8;
        
        // add the crypto contexts as well
        // TODO: what if the algorithm/initkey has switched in the mean time?
        d->cIncomingContext << cIncomingContext->state();
        d->cOutgoingContext << cOutgoingContext->state();
    }
    
    // forward?
    if (d->cKey.data().size() < d->nMinimumKeySize) {
        qkd::utility::debug() << "buffered key " << cKey.id() << " buffered bytes: " << d->cKey.data().size() << "/" << d->nMinimumKeySize;
        return false;
    }

    // forward the new key
    cKey = d->cKey;
    cKey.meta().nErrorRate = (double)d->nErrorBits / (double)d->nKeyBits;
    cKey.meta().nDisclosedBits = d->nDisclosedBits;
    cKey.meta().nErrorBits = 0;
    
    cIncomingContext = d->cIncomingContext;
    cOutgoingContext = d->cOutgoingContext;
    
    // reset key gathering data
    d->cKey = qkd::key::key::null();
    d->nErrorBits = 0;
    d->nDisclosedBits = 0;
    d->nKeyBits = 0;
    
    qkd::utility::debug() << "forwarding key " << cKey.id() << " with size " << cKey.data().size();
    
    return true;
}


/**
 * set the new minimum key size for forwarding
 * 
 * @param   nSize       the new minimum key size for forwarding
 */
void qkd_buffer::set_min_key_size(qulonglong nSize) {
    
    // get exclusive access to properties
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    d->nMinimumKeySize = nSize;
}

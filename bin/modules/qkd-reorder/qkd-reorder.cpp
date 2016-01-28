/*
 * qkd-reorder.cpp
 * 
 * This is the implementation of the QKD postprocessing
 * reorder facilities
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
#include <qkd/utility/syslog.h>

#include "qkd-reorder.h"
#include "qkd_reorder_dbus.h"


// ------------------------------------------------------------
// defs

#define MODULE_DESCRIPTION      "This is the qkd-reorder QKD Module."
#define MODULE_ORGANISATION     "(C)opyright 2012-2016 AIT Austrian Institute of Technology, http://www.ait.ac.at"


// ------------------------------------------------------------
// decl


/**
 * the qkd-reorder pimpl
 */
class qkd_reorder::qkd_reorder_data {
    
public:

    
    /**
     * ctor
     */
    qkd_reorder_data() : nBufferSize(10) {};
    
    std::recursive_mutex cPropertyMutex;            /**< property mutex */
    
    uint64_t nBufferSize;                           /**< reorder ratio */
    
    /**
     * a buffered key consists of key data and the crypto contexts
     */
    typedef struct {
        
        qkd::key::key cKey;                             /**< the key buffered */
        qkd::crypto::crypto_context cIncomingContext;   /**< the incoming context associated with the key so far */
        qkd::crypto::crypto_context cOutgoingContext;   /**< the outgoing context associated with the key so far */
        
    } buffered_key;
    
    std::vector<buffered_key> cBuffer;              /**< our buffer */
};


// ------------------------------------------------------------
// code


/**
 * ctor
 */
qkd_reorder::qkd_reorder() : qkd::module::module("reorder", qkd::module::module_type::TYPE_OTHER, MODULE_DESCRIPTION, MODULE_ORGANISATION) {

    d = boost::shared_ptr<qkd_reorder::qkd_reorder_data>(new qkd_reorder::qkd_reorder_data());

    // apply default values
    set_buffer_size(5);
    
    // enforce DBus registration
    new ReorderAdaptor(this);
}


/**
 * apply the loaded key value map to the module
 * 
 * @param   sURL            URL of config file loaded
 * @param   cConfig         map of key --> value
 */
void qkd_reorder::apply_config(UNUSED std::string const & sURL, qkd::utility::properties const & cConfig) {
    
    // delve into the given config
    for (auto const & cEntry : cConfig) {
        
        // grab any key which is intended for us
        if (!is_config_key(cEntry.first)) continue;
        
        // ignore standard config keys: they should have been applied already
        if (is_standard_config_key(cEntry.first)) continue;
        
        std::string sKey = cEntry.first.substr(config_prefix().size());

        // module specific config here
        if (sKey == "buffer_size") {
            set_buffer_size(atoll(cEntry.second.c_str()));
        }
        else {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "found unknown key: \"" << cEntry.first << "\" - don't know how to handle this.";
        }
    }
}


/**
 * get the size of the reorder buffer
 * 
 * @return  the size of the reorder buffer
 */
qulonglong qkd_reorder::buffer_size() const {
    
    // get exclusive access to properties
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    return d->nBufferSize;
}


/**
 * module work
 * 
 * @param   cKey                    the key to reorder
 * @param   cIncomingContext        incoming crypto context
 * @param   cOutgoingContext        outgoing crypto context
 * @return  always true
 */
bool qkd_reorder::process(qkd::key::key & cKey, qkd::crypto::crypto_context & cIncomingContext, qkd::crypto::crypto_context & cOutgoingContext) {
    
    // get exclusive access to properties
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    
    // reordering turned off?
    if (d->nBufferSize == 0) return true;
    
    // roll a dice ... for all position in the buffer + the current key
    uint64_t nRandom;
    random() >> nRandom;
    nRandom %= d->nBufferSize + 1;
    
    // forward the current key?
    if (nRandom == d->nBufferSize) return true;
    
    // replace key to forward with one in the buffer
    qkd_reorder_data::buffered_key cBufferedKey;
    cBufferedKey.cKey = cKey;
    cBufferedKey.cIncomingContext = cIncomingContext;
    cBufferedKey.cOutgoingContext = cOutgoingContext;
    std::swap(cBufferedKey, d->cBuffer[nRandom]);
    cKey = cBufferedKey.cKey;
    cIncomingContext = cBufferedKey.cIncomingContext;
    cOutgoingContext = cBufferedKey.cOutgoingContext;
    
    // do not forward NULL keys
    if (cKey.is_null()) return false;
    
    return true;
}


/**
 * set the new size of the reorder buffer
 * 
 * @param   nSize           the size of the reorder buffer
 */
void qkd_reorder::set_buffer_size(qulonglong nSize) {
    
    // get exclusive access to properties
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    d->nBufferSize = nSize;
    
    // TODO: when the new buffer size is less then the old, some keys might be lost
    d->cBuffer.resize(d->nBufferSize);
}


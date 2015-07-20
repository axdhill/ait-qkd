/*
 * qkd-resize.cpp
 * 
 * This is the implementation of the QKD postprocessing
 * resize facilities
 * 
 * Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
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
#include <qkd/crypto/engine.h>
#include <qkd/utility/syslog.h>

#include "qkd-resize.h"
#include "qkd_resize_dbus.h"


// ------------------------------------------------------------
// defs

#define MODULE_DESCRIPTION      "This is the qkd-resize QKD Module."
#define MODULE_ORGANISATION     "(C)opyright 2012-2015 AIT Austrian Institute of Technology, http://www.ait.ac.at"


// ------------------------------------------------------------
// decl


/**
 * the qkd-resize pimpl
 */
class qkd_resize::qkd_resize_data {
    
public:

    
    /**
     * ctor
     */
    qkd_resize_data() : nCurrentSize(0), nExactKeySize(0), nMinimumKeySize(0) {};
    
    
    std::recursive_mutex cPropertyMutex;            /**< property mutex */
    
    qkd::module::workload cWorkReceived;            /**< the workload received so far */
    
    uint64_t nCurrentSize;                          /**< current size (in bytes) we have */
    uint64_t nExactKeySize;                         /**< exact key size (in bytes) */
    uint64_t nMinimumKeySize;                       /**< minimum key size (in bytes) for forwarding */
    
    qkd::key::key::key_id_counter cKeyIdCounter;    /**< new key id dispenser */
};


// ------------------------------------------------------------
// code


/**
 * ctor
 */
qkd_resize::qkd_resize() : qkd::module::module("resize", qkd::module::module_type::TYPE_OTHER, MODULE_DESCRIPTION, MODULE_ORGANISATION) {
    d = boost::shared_ptr<qkd_resize::qkd_resize_data>(new qkd_resize::qkd_resize_data());
    new ResizeAdaptor(this);
}


/**
 * apply the loaded key value map to the module
 * 
 * @param   sURL            URL of config file loaded
 * @param   cConfig         map of key --> value
 */
void qkd_resize::apply_config(UNUSED std::string const & sURL, qkd::utility::properties const & cConfig) {
    
    // delve into the given config
    for (auto const & cEntry : cConfig) {
        
        // grab any key which is intended for us
        if (!is_config_key(cEntry.first)) continue;
        
        // ignore standard config keys: they should have been applied already
        if (is_standard_config_key(cEntry.first)) continue;
        
        std::string sKey = cEntry.first.substr(config_prefix().size());

        // module specific config here
        if (sKey == "exact_key_size") {
            set_exact_key_size(atoll(cEntry.second.c_str()));
        }
        else
        if (sKey == "minimum_key_size") {
            set_minimum_key_size(atoll(cEntry.second.c_str()));
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
qulonglong qkd_resize::current_key_size() const {
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    return d->nCurrentSize;
}


/**
 * get the exact key size for forwarding
 * 
 * @return  the exact key size for forwarding
 */
qulonglong qkd_resize::exact_key_size() const {
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    return d->nExactKeySize;
}


/**
 * get the minimum key size for forwarding
 * 
 * @return  the minimum key size for forwarding
 */
qulonglong qkd_resize::minimum_key_size() const {
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    return d->nMinimumKeySize;
}


/**
 * picks of exact_key_size() keys from the internal buffer
 * 
 * @param   cWorkload               place where the keys are stuffed in
 */
void qkd_resize::pick_exact_keys(qkd::module::workload & cWorkload) {
    
    static qkd::crypto::crypto_context cNullContext = qkd::crypto::engine::create("null");

    uint64_t nExactKeySize = exact_key_size();
    if (nExactKeySize == 0) return;
    if (d->nCurrentSize < nExactKeySize) return;

    // two runs:
    // 1. run --> split keys in workload such that 
    //    summing up will yield a single key of exact size
    // 2. run --> collect all part keys and form new keys of 
    //    exact size
    
    // first run: split keys into proper sizes
    uint64_t nCurrentKeySize = 0;
    for (auto it = d->cWorkReceived.begin(); it != d->cWorkReceived.end(); ) {
        
        // for splitting we ignore disclosed keys here
        if ((*it).cKey.meta().eKeyState == qkd::key::key_state::KEY_STATE_DISCLOSED) {
            ++it;
            continue;
        }
        
        if ((nCurrentKeySize + (*it).cKey.data().size()) < nExactKeySize) {
            nCurrentKeySize += (*it).cKey.data().size();
            ++it;
            continue;
        }
        
        // split the current key
        while ((nCurrentKeySize + (*it).cKey.data().size()) >= nExactKeySize) {
            
            // we assume the disclosed bits and the error rate is
            // equaly distributed in the key
            
            uint64_t nCut = nExactKeySize - nCurrentKeySize;
            double nPart = (double)nCut / (double)(*it).cKey.data().size();

            // first half
            auto new_it = d->cWorkReceived.emplace(it);
            (*new_it).cKey.data() = qkd::utility::memory(nCut);
            memcpy((*new_it).cKey.data().get(), (*it).cKey.data().get(), nCut);
            (*new_it).cKey.meta().nErrorRate = (*it).cKey.meta().nErrorRate;
            (*new_it).cKey.meta().nDisclosedBits = (*it).cKey.meta().nDisclosedBits * nPart;
            (*new_it).cIncomingContext = (*it).cIncomingContext;
            (*new_it).cOutgoingContext = (*it).cOutgoingContext;

            // second half (left in workload list)
            new_it = d->cWorkReceived.emplace(it);
            (*new_it).cKey.data() = qkd::utility::memory((*it).cKey.data().size() - nCut);
            memcpy((*new_it).cKey.data().get(), (*it).cKey.data().get() + nCut, (*it).cKey.data().size() - nCut);
            (*new_it).cKey.meta().nErrorRate = (*it).cKey.meta().nErrorRate;
            (*new_it).cKey.meta().nDisclosedBits = (*it).cKey.meta().nDisclosedBits * (1.0 - nPart);
            (*new_it).cIncomingContext = (*it).cIncomingContext;
            (*new_it).cOutgoingContext = (*it).cOutgoingContext;
            
            // kick the old key
            d->cWorkReceived.erase(it);
            it = new_it;
            nCurrentKeySize = 0;
        }
        
        ++it;
    }

    // second run: concatenate keys into exact size
    double nErrorBits = 0;
    double nTotalBits = 0;
    uint64_t nDisclosedBits = 0;
    qkd::module::work cForwardWork;
    auto it_last = d->cWorkReceived.begin();
    for (auto it = d->cWorkReceived.begin(); it != d->cWorkReceived.end(); ++it) {
        
        (*it).bForward = false;
        nTotalBits += (*it).cKey.data().size() * 8;
        nErrorBits += (*it).cKey.meta().nErrorRate * ((*it).cKey.data().size() * 8);

        if ((*it).cKey.meta().eKeyState != qkd::key::key_state::KEY_STATE_DISCLOSED) {            
            
            if (cForwardWork.is_null()) {
                cForwardWork = (*it);
            }
            else {
                cForwardWork.cKey.data().add((*it).cKey.data());
                cForwardWork.cIncomingContext << (*it).cIncomingContext;
                cForwardWork.cOutgoingContext << (*it).cOutgoingContext;
            }
            
            nDisclosedBits += (*it).cKey.meta().nDisclosedBits;
            
            // new key --> place into forward and prepare next
            if (cForwardWork.cKey.data().size() == nExactKeySize) {
                
                d->cKeyIdCounter.inc();
                cForwardWork.cKey.set_id(d->cKeyIdCounter.count());
                cForwardWork.cKey.meta().nErrorRate = nErrorBits / nTotalBits;
                cForwardWork.cKey.meta().nDisclosedBits = nDisclosedBits;
                cForwardWork.bForward = true;
                cWorkload.push_back(cForwardWork);
                
                // mark all keys in between to be deleted
                while (it_last != it) {
                    (*it_last).bForward = true;
                    ++it_last;
                }
                (*it_last).bForward = true;
                
                nErrorBits = 0;
                nTotalBits = 0;
                nDisclosedBits = 0;
                d->nCurrentSize -= nExactKeySize;
                cForwardWork = qkd::module::work{ qkd::key::key(), cNullContext, cNullContext, false };
            }
        }
    }

    d->cWorkReceived.remove_if([](qkd::module::work & w) -> bool { return w.bForward; });
    
    // sanity: as we had nCurrentSize >= nExactKeySize before, we *must* have one key (at least)
    if (cWorkload.empty()) {
        throw std::logic_error("exact key resize: no keys extracted though current size is bigger or equal to exact size");
    }
    if (d->nCurrentSize >= nExactKeySize) {
        throw std::logic_error("exact key resize: still key bytes left to forward");
    }
}


/**
 * picks the key from the internal buffer if we have minimum size set
 * 
 * @param   cWorkload               place the key is stuffed in
 */
void qkd_resize::pick_minimum_key(qkd::module::workload & cWorkload) {
    
    uint64_t nMinimumKeySize = minimum_key_size();
    if (nMinimumKeySize == 0) return;
    if (d->nCurrentSize < nMinimumKeySize) return;

    double nErrorBits = 0;
    double nTotalBits = 0;
    uint64_t nDisclosedBits = 0;
    
    // concatenate the keys
    // we misuse the bForward in the buffered workload as delete flag
    qkd::module::work cForwardWork;
    for (auto & w : d->cWorkReceived) {
        
        nTotalBits += w.cKey.data().size() * 8;
        nErrorBits += w.cKey.meta().nErrorRate * (w.cKey.data().size() * 8);
        
        if (w.cKey.meta().eKeyState != qkd::key::key_state::KEY_STATE_DISCLOSED) {            
            
            if (cForwardWork.is_null()) {
                cForwardWork = w;
            }
            else {
                cForwardWork.cKey.data().add(w.cKey.data());
                cForwardWork.cIncomingContext << w.cIncomingContext;
                cForwardWork.cOutgoingContext << w.cOutgoingContext;
            }
            
            nDisclosedBits += w.cKey.meta().nDisclosedBits;
            d->nCurrentSize -= w.cKey.data().size();
        }
        
        w.bForward = true;
    }
    
    d->cWorkReceived.remove_if([](qkd::module::work & w) -> bool { return w.bForward; });
    
    // sanity: as we had nCurrentSize >= nMinimumKeySize before, we *must* have one key
    // and d->nCurrentSize *must* be 0 now
    // and for minimum we consume all keys
    if (cForwardWork.is_null()) {
        throw std::logic_error("minimum key: no key to foward as expected");
    }
    if (d->nCurrentSize != 0) {
        throw std::logic_error("current size of buffered keys is not 0 as expected");
    }
    if (d->cWorkReceived.size() != 0) {
        throw std::logic_error("still workload left to add to resized key");
    }
    
    // finalize key
    d->cKeyIdCounter.inc();
    cForwardWork.cKey.set_id(d->cKeyIdCounter.count());
    cForwardWork.cKey.meta().nErrorRate = nErrorBits / nTotalBits;
    cForwardWork.cKey.meta().nDisclosedBits = nDisclosedBits;
    cForwardWork.bForward = true;
    
    cWorkload.push_back(cForwardWork);
}


/**
 * work directly on the workload
 * 
 * as we are able to create more keys than on input we have to
 * overwrite the workload entry point
 * 
 * @param   cWorkload               the work to be done
 */
void qkd_resize::process(qkd::module::workload & cWorkload) {
    
    static qkd::crypto::crypto_context cNullContxt = qkd::crypto::engine::create("null");
    
    // ensure we are talking about the same stuff with the peer
    if (!is_synchronizing()) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " 
                << "you deliberately turned off key synchonrizing in resizing - but this is essential fot this module: dropping key";
        cWorkload = { qkd::module::work{ qkd::key::key::null(), cNullContxt, cNullContxt, false } };
        return;
    }
    
    // sanity checks
    if ((exact_key_size() ==0) && (minimum_key_size() == 0)) {
        qkd::utility::syslog::warning() << "qkd-resize: neither minimum nor exact size set --> don't know what to, shipping key as-is.";
        qkd::module::work & w = cWorkload.front();
        w.bForward = w.cKey.meta().eKeyState != qkd::key::key_state::KEY_STATE_DISCLOSED;
        return;
    }
    if ((exact_key_size() > 0) && (minimum_key_size() > 0)) {
        qkd::utility::syslog::warning() << "qkd-resize: both minimum and exact size set --> don't know what to, shipping key as-is.";
        qkd::module::work & w = cWorkload.front();
        w.bForward = w.cKey.meta().eKeyState != qkd::key::key_state::KEY_STATE_DISCLOSED;
        return;
    }
    
    // do work merging
    qkd::module::workload cForwardWorkload;
    for (auto & w : cWorkload) {
        
        d->cWorkReceived.push_back(w);
        if (w.cKey.meta().eKeyState != qkd::key::key_state::KEY_STATE_DISCLOSED) {
            d->nCurrentSize += w.cKey.data().size();
        }
        
        pick_exact_keys(cForwardWorkload);
        pick_minimum_key(cForwardWorkload);
    }
    
    cWorkload = cForwardWorkload;
}


/**
 * set the new exact key size for forwarding
 * 
 * @param   nSize       the new exact key size for forwarding
 */
void qkd_resize::set_exact_key_size(qulonglong nSize) {
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    d->nExactKeySize = nSize;
    d->nMinimumKeySize = 0;
}


/**
 * set the new minimum key size for forwarding
 * 
 * @param   nSize       the new minimum key size for forwarding
 */
void qkd_resize::set_minimum_key_size(qulonglong nSize) {
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    d->nExactKeySize = 0;
    d->nMinimumKeySize = nSize;
}

/*
 * qkd-error-estimation.cpp
 * 
 * This module discloses a portion of bits for error estimation
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
#include <qkd/utility/atof.h>
#include <qkd/utility/bigint.h>
#include <qkd/utility/syslog.h>

#include "qkd-error-estimation.h"
#include "qkd_error_estimation_dbus.h"


// ------------------------------------------------------------
// defs

#define MODULE_DESCRIPTION      "This is the qkd-error-estimation QKD Module: it discloses some bits for error estimation."
#define MODULE_ORGANISATION     "(C)opyright 2012-2015 AIT Austrian Institute of Technology, http://www.ait.ac.at"


// ------------------------------------------------------------
// decl


/**
 * the qkd-error-estimation pimpl
 */
class qkd_error_estimation::qkd_error_estimation_data {
    
public:

    
    /**
     * ctor
     */
    qkd_error_estimation_data() {
        
        cAvgError = qkd::utility::average_technique::create("value", 10);
        nDisclose = 0.1;
        nLastError = 0.0;
    };
    
    std::recursive_mutex cPropertyMutex;    /**< property mutex */
    
    qkd::utility::average cAvgError;        /**< the error rate averaged over the last samples */
    double nDisclose;                       /**< the ratio to disclose */
    double nLastError;                      /**< the last error */
};


// ------------------------------------------------------------
// code


/**
 * ctor
 */
qkd_error_estimation::qkd_error_estimation() : qkd::module::module("error-estimation", qkd::module::module_type::TYPE_ERROR_ESTIMATION, MODULE_DESCRIPTION, MODULE_ORGANISATION) {

    d = boost::shared_ptr<qkd_error_estimation::qkd_error_estimation_data>(new qkd_error_estimation::qkd_error_estimation_data());
    
    // apply default values
    set_disclose(0.1);
    
    // enforce DBus registration
    new ErrorestimationAdaptor(this);
}


/**
 * apply the loaded key value map to the module
 * 
 * @param   sURL            URL of config file loaded
 * @param   cConfig         map of key --> value
 */
void qkd_error_estimation::apply_config(UNUSED std::string const & sURL, qkd::utility::properties const & cConfig) {
    
    // delve into the given config
    for (auto const & cEntry : cConfig) {
        
        // grab any key which is intended for us
        if (!is_config_key(cEntry.first)) continue;
        
        // ignore standard config keys: they should have been applied already
        if (is_standard_config_key(cEntry.first)) continue;
        
        std::string sKey = cEntry.first.substr(config_prefix().size());
        
        // module specific config here
        if (sKey == "disclose") {
            set_disclose(qkd::utility::atof(cEntry.second.c_str()));
        }
        else {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "found unknown key: \"" << cEntry.first << "\" - don't know how to handle this.";
        }
    }
}


/**
 * return the average error over the last keys
 * 
 * @return  the average error over the last keys
 */
double qkd_error_estimation::average_error() const {
    
    // get exclusive access to properties
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    return d->cAvgError->avg();
}


/**
 * return the ratio of disclosed bits
 * 
 * @return  the ratio of disclosed bits
 */
double qkd_error_estimation::disclose() const{
    
    // get exclusive access to properties
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    return d->nDisclose;
}


/**
 * return the last error estimation
 * 
 * @return  the last error estimation
 */
double qkd_error_estimation::last_error() const {
    
    // get exclusive access to properties
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    return d->nLastError;
}


/**
 * module work
 * 
 * @param   cKey                    key with errors to estimate
 * @param   cIncomingContext        incoming crypto context
 * @param   cOutgoingContext        outgoing crypto context
 * @return  always true
 */
bool qkd_error_estimation::process(qkd::key::key & cKey, qkd::crypto::crypto_context & cIncomingContext, qkd::crypto::crypto_context & cOutgoingContext) {

    if (is_alice()) return process_alice(cKey, cIncomingContext, cOutgoingContext);
    if (is_bob()) return process_bob(cKey, cIncomingContext, cOutgoingContext);
    
    // should not happen to reach this line, but 
    // we return true: pass on the key to the next module
    return true;
}


/**
 * module work as alice
 * 
 * @param   cKey                    key with errors to estimate
 * @param   cIncomingContext        incoming crypto context
 * @param   cOutgoingContext        outgoing crypto context
 * @return  always true
 */
bool qkd_error_estimation::process_alice(qkd::key::key & cKey, qkd::crypto::crypto_context & cIncomingContext, qkd::crypto::crypto_context & cOutgoingContext) {
    
    // grab current disclose ratio
    double nDisclose = 0.0;
    {
        std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
        nDisclose = d->nDisclose;
    }
    
    // check limits
    if (nDisclose < 0.0) nDisclose = 0.0;
    if (nDisclose >= 1.0) {
        nDisclose = 1.0;
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "disclosing 100% of key for error estimation";
    }
    
    // pick positions
    qkd::utility::bigint cKeyBigint = qkd::utility::bigint(cKey.data());
    uint64_t nBits = cKeyBigint.bits();
    qkd::utility::bigint cMask(nBits);
    cMask.clear();
    
    // positions to disclose
    std::list<uint64_t> cPositionsDisclosed;
    for (uint64_t i = 0; i < nBits; i++) {
        double nRandom = 0.0;
        random() >> nRandom;
        if (nRandom <= nDisclose) {
            cPositionsDisclosed.push_back(i);
            cMask.set(i, true);
        }
    }
    
    // this is going public
    qkd::utility::bigint cPublicLocal = cKeyBigint & cMask;
    
    // create message for peer
    qkd::module::message cMessage;
    
    // send some header data to ensure we are talking about the same key
    cMessage.data() << cKey.id();
    cMessage.data() << nBits;
    
    // send the disclosed positions
    cMessage.data() << cPositionsDisclosed.size();
    for (auto i : cPositionsDisclosed) cMessage.data() << i;
    cMessage.data() << cPublicLocal.memory();
    try {
        send(cMessage, cOutgoingContext);
    }
    catch (std::runtime_error const & cRuntimeError) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to send message: " << cRuntimeError.what();
        return false;
    }
    
    // read from peer
    cMessage = qkd::module::message();
    try {
        if (!recv(cMessage, cIncomingContext)) return false;
    }
    catch (std::runtime_error const & cRuntimeError) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to receive message: " << cRuntimeError.what();
        return false;
    }

    qkd::utility::memory cPublicPeerMemory;
    cMessage.data() >> cPublicPeerMemory;
    
    qkd::utility::bigint cPublicPeer(cPublicPeerMemory);
    
    qkd::utility::bigint cErrors = cPublicLocal ^ cPublicPeer;
    uint64_t nErrorsDetected = cErrors.bits_set();
    cKey.meta().nErrorRate = (double)nErrorsDetected / (double)cPositionsDisclosed.size();
    
    // set new detected value
    {
        std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
        d->cAvgError << cKey.meta().nErrorRate;
        d->nLastError = cKey.meta().nErrorRate;
    }
    
    // tell user
    qkd::utility::debug() << "key #" << cKey.id() << ", disclosed bits = " << cPositionsDisclosed.size() << ", errors detected = " << nErrorsDetected << ", error rate = " << cKey.meta().nErrorRate;
    
    // modify key: extrat discarded keybits
    qkd::utility::bigint cNewKeyBigint(nBits - cPositionsDisclosed.size());
    std::list<uint64_t>::iterator iter = cPositionsDisclosed.begin();
    for (uint64_t i = 0, j = 0; i < nBits; i++) {
        if (i == (*iter)) ++iter;
        else {
            cNewKeyBigint.set(j, cKeyBigint.get(i));
            j++;
        }
    }
    
    // fix key data
    cKey.data() = cNewKeyBigint.memory();
    
    return true;
}


/**
 * module work as bob
 * 
 * @param   cKey                    key with errors to estimate
 * @param   cIncomingContext        incoming crypto context
 * @param   cOutgoingContext        outgoing crypto context
 * @return  always true
 */
bool qkd_error_estimation::process_bob(qkd::key::key & cKey, qkd::crypto::crypto_context & cIncomingContext, qkd::crypto::crypto_context & cOutgoingContext) {

    // prepare
    qkd::utility::bigint cKeyBigint = qkd::utility::bigint(cKey.data());
    uint64_t nBits = cKeyBigint.bits();
    qkd::utility::bigint cMask(nBits);
    cMask.clear();

    // read from peer
    qkd::module::message cMessage;
    try {
        if (!recv(cMessage, cIncomingContext)) return false;
    }
    catch (std::runtime_error const & cRuntimeError) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to receive message: " << cRuntimeError.what();
        return false;
    }
    
    // positions to disclose
    std::list<uint64_t> cPositionsDisclosed;
    uint64_t nPeerBits = 0;
    uint64_t nPositionsDisclosed = 0;
    
    // get the header data
    qkd::key::key_id nPeerId;
    cMessage.data() >> nPeerId;
    cMessage.data() >> nPeerBits;
    
    // check if we are talking about the same key
    if ((nPeerId != cKey.id()) || (nPeerBits != nBits)) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "received data for wrong key and/or wrong key size: local id = " << cKey.id() << " peer id = " << nPeerId << " local bits = " << nBits << " peer bits = " << nPeerBits;
        terminate();
    }

    // get the disclosed positions
    cMessage.data() >> nPositionsDisclosed;
    for (uint64_t i = 0; i < nPositionsDisclosed; i++) {
        uint64_t nPosition = 0;
        cMessage.data() >> nPosition;
        cPositionsDisclosed.push_back(nPosition);
        cMask.set(nPosition, true);
    }
    
    // public peer disclosed key
    qkd::utility::memory cPublicPeerMemory;
    cMessage.data() >> cPublicPeerMemory;
    qkd::utility::bigint cPublicPeer(cPublicPeerMemory);
    
    // this needs to be sent back to alice
    qkd::utility::bigint cPublicLocal = cKeyBigint & cMask;
    
    // create message for peer
    cMessage = qkd::module::message();
    cMessage.data() << cPublicLocal.memory();
    try {
        send(cMessage, cOutgoingContext);
    }
    catch (std::runtime_error const & cRuntimeError) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to send message: " << cRuntimeError.what();
        return false;
    }
    
    // calculate error
    qkd::utility::bigint cErrors = cPublicLocal ^ cPublicPeer;
    uint64_t nErrorsDetected = cErrors.bits_set();
    cKey.meta().nErrorRate = (double)nErrorsDetected / (double)cPositionsDisclosed.size();

    // set new detected value
    {
        std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
        d->cAvgError << cKey.meta().nErrorRate;
        d->nLastError = cKey.meta().nErrorRate;
    }
    
    // tell user
    qkd::utility::debug() << "key #" << cKey.id() << ", disclosed bits = " << cPositionsDisclosed.size() << ", errors detected = " << nErrorsDetected << ", error rate = " << cKey.meta().nErrorRate;
    
    // modify key: extrat discarded keybits
    qkd::utility::bigint cNewKeyBigint(nBits - cPositionsDisclosed.size());
    std::list<uint64_t>::iterator iter = cPositionsDisclosed.begin();
    for (uint64_t i = 0, j = 0; i < nBits; i++) {
        if (i == (*iter)) ++iter;
        else {
            cNewKeyBigint.set(j, cKeyBigint.get(i));
            j++;
        }
    }
    
    // fix key data
    cKey.data() = cNewKeyBigint.memory();
    
    return true;
}


/**
 * sets the ratio of disclosed bits
 * 
 * @param   nRatio          the new ratio of disclosed bits
 */
void qkd_error_estimation::set_disclose(double nRatio) {
    
    // limit input ratio to reasonable values
    double nBoundedRatio = std::min<double>(std::max<double>(nRatio, 0.0), 1.0);
    if (nBoundedRatio != nRatio) qkd::utility::debug() << "input ratio " << nRatio << " set to " << nBoundedRatio;
    
    // get exclusive access to properties
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    d->nDisclose = nBoundedRatio;
}


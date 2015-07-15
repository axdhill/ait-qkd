/*
 * qkd-ping.cpp
 * 
 * The qkd-ping sends a series of messages back and forth 
 * to test remote module to module interconnection
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

#include <atomic>
#include <thread>

// ait
#include <qkd/crypto/engine.h>

#include "qkd-ping.h"
#include "qkd_ping_dbus.h"
#include "ping.h"


// ------------------------------------------------------------
// defs

#define MODULE_DESCRIPTION      "This is the qkd-ping QKD Module: it sends messages back and forth to test remote module to module connection capabilities."
#define MODULE_ORGANISATION     "(C)opyright 2012-2015 AIT Austrian Institute of Technology, http://www.ait.ac.at"


// ------------------------------------------------------------
// decl


/**
 * the qkd-ping pimpl
 */
class qkd_ping::qkd_ping_data {
    
public:

    
    /**
     * ctor
     */
    qkd_ping_data() : nRoundtrips(0) {
        nMaxRoundtrip = 0;
        nPackageSize = 1000;
        cSleepTime = std::chrono::milliseconds(1000);
    };
    
    std::recursive_mutex cPropertyMutex;        /**< property mutex */
    
    uint64_t nMaxRoundtrip;                     /**< maximum number of roundtrips */
    uint64_t nPackageSize;                      /**< size of package to sent */
    std::atomic<uint64_t> nRoundtrips;          /**< current number of roundtrips */
    std::chrono::milliseconds cSleepTime;       /**< wait time between calls */
};


// ------------------------------------------------------------
// code


/**
 * ctor
 */
qkd_ping::qkd_ping() : qkd::module::module("ping", qkd::module::module_type::TYPE_OTHER, MODULE_DESCRIPTION, MODULE_ORGANISATION) {

    d = boost::shared_ptr<qkd_ping::qkd_ping_data>(new qkd_ping::qkd_ping_data());
    
    // apply default values
    set_max_roundtrip(100);
    set_payload_size(1000);
    set_synchronize_keys(false);
    set_synchronize_ttl(0);
    set_urls("", "stdout://", "", "");
    
    new PingAdaptor(this);
}


/**
 * returns the maximum number of roundtrips to do
 * 
 * @return  the maximum number of roundtrips to do
 */
qulonglong qkd_ping::max_roundtrip() const {
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    return d->nMaxRoundtrip;
}


/**
 * returns the number of bytes of the payload sent back and forth
 * 
 * @return  the number of bytes sent each roundtrip
 */
qulonglong qkd_ping::payload_size() const {
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    return d->nPackageSize;
}


/**
 * module work as ALICE
 */
void qkd_ping::process_alice() {

    uint64_t nPackageSize = 0;
    {
        std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
        if (d->nPackageSize > 0) {
            nPackageSize = d->nPackageSize;
        }
    }

    qkd::crypto::crypto_context cIncomingContext = qkd::crypto::engine::create("null");
    qkd::crypto::crypto_context cOutgoingConetxt = qkd::crypto::engine::create("null");
    qkd::module::communicator cModuleComm = comm(cIncomingContext, cOutgoingConetxt);

    // real work here...
    if (!ping_alice(cModuleComm, nPackageSize)) return;
   
    d->nRoundtrips++;
    uint64_t nMaxRoundtrip = max_roundtrip();
    if ((nMaxRoundtrip != 0) && (d->nRoundtrips >= nMaxRoundtrip)) {
        
        // did enough work: bail out
        terminate();
        return;
    }
    
    // wait sleep time
    std::chrono::milliseconds cSlept(0);
    while (cSlept < std::chrono::milliseconds(sleep_time())) {
        
        // lay to bed
        std::chrono::time_point<std::chrono::system_clock> cBeforeSleep = std::chrono::system_clock::now();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::chrono::time_point<std::chrono::system_clock> cAfterSleep = std::chrono::system_clock::now();
        
        cSlept += std::chrono::duration_cast<std::chrono::milliseconds>(cAfterSleep - cBeforeSleep);
        
        // wake up! check for termination state
        if (is_dying_state()) break;
    }
}


/**
 * module work as BOB
 */
void qkd_ping::process_bob() {

    uint64_t nPackageSize = 0;
    {
        std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
        if (d->nPackageSize > 0) {
            nPackageSize = d->nPackageSize;
        }
    }
 
    // try (and test) the module_communicater facade instance
    qkd::crypto::crypto_context cIncomingContext = qkd::crypto::engine::create("null");
    qkd::crypto::crypto_context cOutgoingConetxt = qkd::crypto::engine::create("null");
    qkd::module::communicator cModuleComm = comm(cIncomingContext, cOutgoingConetxt);

    // real work here...
    if (!ping_bob(cModuleComm, nPackageSize)) return;
}


/**
 * returns the number of current rounndtrips
 * 
 * @return  the number of current roundtrips
 */
qulonglong qkd_ping::roundtrips() const {
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    return d->nRoundtrips;
}


/**
 * set a new maximum number of roundtrips to do
 * 
 * @param   nMaxRoundtrip       the new maximum number of roundtrips to do
 */
void qkd_ping::set_max_roundtrip(qulonglong nMaxRoundtrip) {
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    d->nMaxRoundtrip = nMaxRoundtrip;
}
    
    
/**
 * set a new number of bytes to send back and forth
 * 
 * @param   nPayloadSize    the new number of bytes to send and recv
 */    
void qkd_ping::set_payload_size(qulonglong nPayloadSize) {
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    d->nPackageSize = nPayloadSize;
}


/**
 * set a new number of milliseconds to wait between a roundtrip
 * this number must be a multple of timeout()
 * 
 * @param   nSleepTime      the new number of milliseconds to sleep
 */    
void qkd_ping::set_sleep_time(qulonglong nSleepTime) {
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    d->cSleepTime = std::chrono::milliseconds(nSleepTime);
}


/**
 * returns the number of milliseconds to wait between a roundtrip
 * 
 * @return  the time to sleep between calls
 */
qulonglong qkd_ping::sleep_time() const {
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    return d->cSleepTime.count();
}


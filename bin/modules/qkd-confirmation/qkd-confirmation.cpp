/*
 * qkd-confirmation.cpp
 * 
 * This is the implementation of the QKD postprocessing
 * confirmation facilities
 * 
 * Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *
 * Copyright (C) 2012-2016 AIT Austrian Institute of Technology
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

// ait
#include <qkd/utility/bigint.h>
#include <qkd/utility/syslog.h>

#include "qkd-confirmation.h"
#include "qkd_confirmation_dbus.h"


// ------------------------------------------------------------
// defs

#define MODULE_DESCRIPTION      "This is the qkd-confirmation QKD Module."
#define MODULE_ORGANISATION     "(C)opyright 2012-2016 AIT Austrian Institute of Technology, http://www.ait.ac.at"


// ------------------------------------------------------------
// decl


/**
 * the qkd-confirmation pimpl
 */
class qkd_confirmation::qkd_confirmation_data {
    
public:

    
    /**
     * ctor
     */
    qkd_confirmation_data() : nBadKeys(0), nConfirmedKeys(0) {};
    
    std::recursive_mutex cPropertyMutex;    /**< property mutex */

    std::atomic<uint64_t> nBadKeys;         /**< number of bad keys so far */
    std::atomic<uint64_t> nConfirmedKeys;   /**< number of confirmed keys so far */
    uint64_t nRounds;                       /**< number of confirmation rounds */
    
};


// ------------------------------------------------------------
// code


/**
 * ctor
 */
qkd_confirmation::qkd_confirmation() : qkd::module::module("confirmation", qkd::module::module_type::TYPE_CONFIRMATION, MODULE_DESCRIPTION, MODULE_ORGANISATION) {
    d = std::shared_ptr<qkd_confirmation::qkd_confirmation_data>(new qkd_confirmation::qkd_confirmation_data());
    set_rounds(10);
    new ConfirmationAdaptor(this);
}


/**
 * apply the loaded key value map to the module
 * 
 * @param   sURL            URL of config file loaded
 * @param   cConfig         map of key --> value
 */
void qkd_confirmation::apply_config(UNUSED std::string const & sURL, qkd::utility::properties const & cConfig) {
    
    for (auto const & cEntry : cConfig) {
        
        if (!is_config_key(cEntry.first)) continue;
        if (is_standard_config_key(cEntry.first)) continue;
        
        std::string sKey = cEntry.first.substr(config_prefix().size());
        if (sKey == "rounds") {
            set_rounds(atoll(cEntry.second.c_str()));
        }
        else {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "found unknown key: \"" << cEntry.first << "\" - don't know how to handle this.";
        }
    }
}


/**
 * get the number of bad keys so far
 * 
 * bad keys are keys not been successfully corrected
 * 
 * @return  the number of bad keys
 */
qulonglong qkd_confirmation::bad_keys() const {
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    return d->nBadKeys;
}


/**
 * get the number of confirmed keys so far
 * 
 * @return  the number of confirmation keys done
 */
qulonglong qkd_confirmation::confirmed_keys() const {
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    return d->nConfirmedKeys;
}


/**
 * module work
 * 
 * @param   cKey                    the key to confirm
 * @param   cIncomingContext        incoming crypto context
 * @param   cOutgoingContext        outgoing crypto context
 * @return  always true
 */
bool qkd_confirmation::process(qkd::key::key & cKey, qkd::crypto::crypto_context & cIncomingContext, qkd::crypto::crypto_context & cOutgoingContext) {
    
    // we pass on disclosed keys as-is
    if (cKey.meta().eKeyState == qkd::key::key_state::KEY_STATE_DISCLOSED) return true;

    if (is_alice()) return process_alice(cKey, cIncomingContext, cOutgoingContext);
    if (is_bob()) return process_bob(cKey, cIncomingContext, cOutgoingContext);
    
    throw std::logic_error("module acts neither as alice nor as bob");
}


/**
 * module work as alice
 * 
 * @param   cKey                    the key to confirm
 * @param   cIncomingContext        incoming crypto context
 * @param   cOutgoingContext        outgoing crypto context
 * @return  always true
 */
bool qkd_confirmation::process_alice(qkd::key::key & cKey, qkd::crypto::crypto_context & cIncomingContext, qkd::crypto::crypto_context & cOutgoingContext) {
    
    qkd::module::message cMessage;
    uint64_t nRounds = rounds();
    
    qkd::utility::bigint cKeyBI = qkd::utility::bigint(cKey.data());

    cMessage.data() << cKey.id();
    cMessage.data() << cKey.size();
    cMessage.data() << nRounds;

    std::list<bool> cParities;
    for (uint64_t i = 0; i < nRounds; i++) {
        
        // create a random bigint
        qkd::utility::memory cMemory(cKey.data().size());
        random() >> cMemory;
        qkd::utility::bigint cBI(cMemory);
        
        cBI &= cKeyBI;
        cParities.push_back(cBI.parity());
        
        // record random memory in message
        cMessage.data() << cMemory;
    }
    
    // finalize parities in message to send
    for (auto bParity : cParities) cMessage.data() << bParity;
    
    // send to bob
    try {
        send(cKey.id(), cMessage, cOutgoingContext);
    }
    catch (std::runtime_error const & cRuntimeError) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to send message: " << cRuntimeError.what();
        return false;
    }
    
    // recv from bob's parities
    try {
        if (!recv(cKey.id(), cMessage, cIncomingContext, qkd::module::message_type::MESSAGE_TYPE_DATA)) return false;
    }
    catch (std::runtime_error const & cRuntimeError) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to receive message: " << cRuntimeError.what();
        return false;
    }

    // compare parities
    bool bParitiesEqual = true;
    std::list<bool>::const_iterator iter = cParities.begin();
    for (uint64_t i = 0; i < nRounds; i++) {
        
        bool bParitiesBob;
        cMessage.data() >> bParitiesBob;
        bParitiesEqual = bParitiesEqual && ((*iter) == bParitiesBob);
        ++iter;
    }
    
    // match?
    if (!bParitiesEqual) {
        d->nBadKeys++;
        qkd::utility::syslog::info() << "confirmation for key " << cKey.id() << " failed";
    }
    else {
        cKey.meta().eKeyState = qkd::key::key_state::KEY_STATE_CONFIRMED;
        d->nConfirmedKeys++;
        qkd::utility::debug() << "confirmation for key " << cKey.id() << " ok";
    }

    return bParitiesEqual;
}


/**
 * module work as bob
 * 
 * @param   cKey                    the key to confirm
 * @param   cIncomingContext        incoming crypto context
 * @param   cOutgoingContext        outgoing crypto context
 * @return  always true
 */
bool qkd_confirmation::process_bob(qkd::key::key & cKey, qkd::crypto::crypto_context & cIncomingContext, qkd::crypto::crypto_context & cOutgoingContext) {
    
    qkd::module::message cMessage;
    
    qkd::key::key_id nPeerKeyId = 0;
    uint64_t nPeerKeySize = 0;
    uint64_t nRounds = 0;
    
    // recv data from alice
    try {
        if (!recv(cKey.id(), cMessage, cIncomingContext, qkd::module::message_type::MESSAGE_TYPE_DATA)) return false;
    }
    catch (std::runtime_error const & cRuntimeError) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to receive message: " << cRuntimeError.what();
        return false;
    }
    cMessage.data() >> nPeerKeyId;
    cMessage.data() >> nPeerKeySize;
    cMessage.data() >> nRounds;
    
    set_rounds(nRounds);
    
    // sanity check
    if ((cKey.id() != nPeerKeyId) || (cKey.data().size() != nPeerKeySize)) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "alice has wrong key id and/or different key size";
        return false;
    }
    
    // work on the received data
    std::list<bool> cParities;
    qkd::utility::bigint cKeyBI = qkd::utility::bigint(cKey.data());
    for (uint64_t i = 0; i < nRounds; i++) {
        
        qkd::utility::memory cMemory;
        cMessage.data() >> cMemory;
        
        qkd::utility::bigint cBI(cMemory);
        cBI &= cKeyBI;
        cParities.push_back(cBI.parity());
    }
    
    // compare local and peer parities
    bool bParitiesEqual = true;
    std::list<bool>::const_iterator iter = cParities.begin();
    for (uint64_t i = 0; i < nRounds; i++) {
        
        bool bParitiesAlice;
        cMessage.data() >> bParitiesAlice;
        bParitiesEqual = bParitiesEqual && ((*iter) == bParitiesAlice);
        ++iter;
    }
    
    // send our parities back to alice
    cMessage = qkd::module::message();
    for (auto bParity : cParities) cMessage.data() << bParity;
    
    try {
        send(cKey.id(), cMessage, cOutgoingContext);
    }
    catch (std::runtime_error const & cRuntimeError) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to send message: " << cRuntimeError.what();
        return false;
    }
    
    // match?
    if (!bParitiesEqual) {
        d->nBadKeys++;
        qkd::utility::syslog::info() << "confirmation for key " << cKey.id() << " failed";
    }
    else {
        cKey.meta().eKeyState = qkd::key::key_state::KEY_STATE_CONFIRMED;
        d->nConfirmedKeys++;
        qkd::utility::debug() << "confirmation for key " << cKey.id() << " ok";
    }
    
    return bParitiesEqual;
}


/**
 * get the number of confirmation rounds
 * 
 * @return  the number of confirmation rounds done
 */
qulonglong qkd_confirmation::rounds() const {
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    return d->nRounds;
}


/**
 * set the new number of confirmation rounds
 * 
 * @param   nRounds     the new number of confirmation rounds
 */
void qkd_confirmation::set_rounds(qulonglong nRounds) {
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    d->nRounds = nRounds;
}


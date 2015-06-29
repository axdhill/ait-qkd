/*
 * qkd-auth.cpp
 * 
 * This is the implementation of the QKD postprocessing
 * authentication facilities
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
#include <qkd/crypto/engine.h>
#include <qkd/q3p/db.h>
#include <qkd/utility/syslog.h>

#include "qkd-auth.h"
#include "qkd_auth_dbus.h"


// ------------------------------------------------------------
// defs

#define MODULE_DESCRIPTION      "This is the qkd-auth QKD Module."
#define MODULE_ORGANISATION     "(C)opyright 2012-2015 AIT Austrian Institute of Technology, http://www.ait.ac.at"


// ------------------------------------------------------------
// decl


/**
 * the qkd-auth pimpl
 */
class qkd_auth::qkd_auth_data {
    
public:

    
    /**
     * ctor
     */
    qkd_auth_data() : bChangeSchemeIncoming(false), bChangeSchemeOutgoing(false), nThreshold(1024) {
        cKeysIncoming = qkd::q3p::db::open("ram://");
        cKeysOutgoing = qkd::q3p::db::open("ram://");
    };
    
    std::recursive_mutex cPropertyMutex;            /**< property mutex */
    
    bool bChangeSchemeIncoming;                     /**< enforce incoming scheme change */
    bool bChangeSchemeOutgoing;                     /**< enforce outgoing scheme change */
    qkd::crypto::scheme cCurrentSchemeIncoming;     /**< the current incoming crypto scheme to use */
    qkd::crypto::scheme cCurrentSchemeOutgoing;     /**< the current outgoing crypto scheme to use */
    qkd::crypto::scheme cNextSchemeIncoming;        /**< the next incoming crypto scheme to use */
    qkd::crypto::scheme cNextSchemeOutgoing;        /**< the next outgoing crypto scheme to use */
    
    qkd::q3p::key_db cKeysIncoming;                 /**< incoming authentication key DB */
    qkd::q3p::key_db cKeysOutgoing;                 /**< outgoing authentication key DB */
    
    uint64_t nThreshold;                            /**< authentication key reserve limit in bytes */
    
};


// fwd
static void nibble(qkd::key::key & cKey, qkd::q3p::key_db & cKeyDB, uint64_t nThreshold);
static void store(qkd::utility::memory cMemory, qkd::q3p::key_db & cKeyDB);
static qkd::utility::memory tag(bool bAlice, qkd::crypto::crypto_context & cContext, qkd::q3p::key_db & cKeyDB, qkd::key::key_vector & cKeys);
static bool verify_scheme(qkd::crypto::scheme const & cScheme);


// ------------------------------------------------------------
// code


/**
 * ctor
 */
qkd_auth::qkd_auth() : qkd::module::module("auth", qkd::module::module_type::TYPE_OTHER, MODULE_DESCRIPTION, MODULE_ORGANISATION) {
    d = boost::shared_ptr<qkd_auth::qkd_auth_data>(new qkd_auth::qkd_auth_data());
    new AuthAdaptor(this);
}


/**
 * apply the loaded key value map to the module
 * 
 * @param   sURL            URL of config file loaded
 * @param   cConfig         map of key --> value
 */
void qkd_auth::apply_config(UNUSED std::string const & sURL, qkd::utility::properties const & cConfig) {
    
    for (auto const & cEntry : cConfig) {
        
        if (!is_config_key(cEntry.first)) continue;
        if (is_standard_config_key(cEntry.first)) continue;
        
        std::string sKey = cEntry.first.substr(config_prefix().size());

        // module specific config here
        if (sKey == "alice.key.incoming") {
            if (is_alice()) store_keys_incoming(QByteArray(cEntry.second.c_str()));
        }
        else
        if (sKey == "bob.key.incoming") {
            if (is_bob()) store_keys_incoming(QByteArray(cEntry.second.c_str()));
        }
        else
        if (sKey == "alice.key.outgoing") {
            if (is_alice()) store_keys_outgoing(QByteArray(cEntry.second.c_str()));
        }
        else
        if (sKey == "bob.key.outgoing") {
            if (is_bob()) store_keys_outgoing(QByteArray(cEntry.second.c_str()));
        }
        else
        if (sKey == "alice.scheme.incoming") {
            if (is_alice()) set_next_scheme_in(QString::fromStdString(cEntry.second));
        }
        else
        if (sKey == "bob.scheme.incoming") {
            if (is_bob()) set_next_scheme_in(QString::fromStdString(cEntry.second));
        }
        else
        if (sKey == "alice.scheme.outgoing") {
            if (is_alice()) set_next_scheme_out(QString::fromStdString(cEntry.second));
        }
        else
        if (sKey == "bob.scheme.outgoing") {
            if (is_bob()) set_next_scheme_out(QString::fromStdString(cEntry.second));
        }
        else {
            qkd::utility::syslog::warning() << __FILENAME__ 
                    << '@' 
                    << __LINE__ 
                    << ": " 
                    << "found unknown key: \"" 
                    << cEntry.first 
                    << "\" - don't know how to handle this.";
        }
    }
}


/**
 * run authentication
 * 
 * @param   cKey                    the key to authenticate
 * @param   cIncomingContext        incoming crypto context
 * @param   cOutgoingContext        outgoing crypto context
 * @return  true, if authentication run successfully
 */
bool qkd_auth::authenticate(qkd::key::key & cKey, 
        qkd::crypto::crypto_context & cIncomingContext, 
        qkd::crypto::crypto_context & cOutgoingContext) {

    // no security is ... well "authentic" ... if we lack the premise, anything goes
    if (cIncomingContext->null() && cOutgoingContext->null()) return true;
    
    // at least one crypto context is present

    qkd::utility::memory cTagIncomingAlice;
    qkd::utility::memory cTagOutgoingAlice;
    qkd::utility::memory cTagIncomingBob;
    qkd::utility::memory cTagOutgoingBob;
    
    qkd::key::key_vector cFinalKeysIncomingAlice;
    qkd::key::key_vector cFinalKeysOutgoingAlice;
    qkd::key::key_vector cFinalKeysIncomingBob;
    qkd::key::key_vector cFinalKeysOutgoingBob;
    
    {
        std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
        
        cTagIncomingAlice = tag(true, cIncomingContext, d->cKeysIncoming, cFinalKeysIncomingAlice);
        if (cTagIncomingAlice.is_null()) {
            pause();
            return false;
        }
        
        cTagOutgoingAlice = tag(true, cOutgoingContext, d->cKeysOutgoing, cFinalKeysOutgoingAlice);
        if (cTagOutgoingAlice.is_null()) {
            pause();
            return false;
        }
        
        cTagIncomingBob = tag(false, cIncomingContext, d->cKeysIncoming, cFinalKeysIncomingBob);
        if (cTagIncomingBob.is_null()) {
            pause();
            return false;
        }
        
        cTagOutgoingBob = tag(false, cOutgoingContext, d->cKeysOutgoing, cFinalKeysOutgoingBob);
        if (cTagOutgoingBob.is_null()) {
            pause();
            return false;
        }
        
    }
    
    if (!cIncomingContext->null() && (cTagIncomingAlice.size() == 0)) return false;
    if (!cIncomingContext->null() && (cTagIncomingBob.size() == 0)) return false;
    if (!cOutgoingContext->null() && (cTagOutgoingAlice.size() == 0)) return false;
    if (!cOutgoingContext->null() && (cTagOutgoingBob.size() == 0)) return false;
    
    static qkd::crypto::crypto_context cNullContext = qkd::crypto::engine::create("null");
    
    // send our tags to the peer and reqeust hers
    qkd::module::message cMessage;
    cMessage.data() << cKey.id();
    if (is_alice()) {
        cMessage.data() << cTagIncomingAlice;
        cMessage.data() << cTagOutgoingAlice;
        cMessage.data() << static_cast<uint64_t>(threshold());
    }
    if (is_bob()) {
        cMessage.data() << cTagIncomingBob;
        cMessage.data() << cTagOutgoingBob;
    }
    
    try {
        send(cMessage, cNullContext);
    }
    catch (std::runtime_error const & cRuntimeError) {
        qkd::utility::syslog::crit() << __FILENAME__ 
                << '@' 
                << __LINE__ 
                << ": " 
                << "failed to send message: " 
                << cRuntimeError.what();
        return false;
    }
    
    if (qkd::utility::debug::enabled()) {
        qkd::utility::debug() << "authentication running - sent: key = " 
                << cKey.id() 
                << " " 
                << "in-tag-alice = " 
                << cTagIncomingAlice.as_hex() 
                << " out-tag-alice = " 
                << cTagOutgoingAlice.as_hex() 
                << " " 
                << "in-tag-bob = " 
                << cTagIncomingBob.as_hex() 
                << " out-tag-bob = " 
                << cTagOutgoingBob.as_hex();
    }
    
    qkd::key::key_id nPeerKeyId;
    qkd::utility::memory cPeerTagIncoming;
    qkd::utility::memory cPeerTagOutgoing;
    try {
        if (!recv(cMessage, cNullContext)) return false;
    }
    catch (std::runtime_error const & cRuntimeError) {
        qkd::utility::syslog::crit() 
                << __FILENAME__ 
                << '@' 
                << __LINE__ 
                << ": " 
                << "failed to receive message: " 
                << cRuntimeError.what();
        return false;
    }
    
    cMessage.data() >> nPeerKeyId;
    cMessage.data() >> cPeerTagIncoming;
    cMessage.data() >> cPeerTagOutgoing;
    uint64_t nPeerThreshold = 0;
    if (is_bob()) cMessage.data() >> nPeerThreshold;
    
    if (qkd::utility::debug::enabled()) {
        qkd::utility::debug() << "authentication running - recv: key = " 
                << nPeerKeyId 
                << " in-tag = " 
                << cPeerTagIncoming.as_hex() 
                << " out-tag = " 
                << cPeerTagOutgoing.as_hex();
    }
    
    // this is the final test
    bool bAuthentic = (cKey.id() == nPeerKeyId);
    if (is_alice()) {
        bAuthentic = bAuthentic 
                && (cTagIncomingBob.equal(cPeerTagOutgoing)) 
                && (cTagOutgoingBob.equal(cPeerTagIncoming));
    }
    if (is_bob()) {
        bAuthentic = bAuthentic 
                && (cTagIncomingAlice.equal(cPeerTagOutgoing)) 
                && (cTagOutgoingAlice.equal(cPeerTagIncoming));
    }

    // if it is authentic we have to kick the keys from the databases
    if (bAuthentic) {

        d->cKeysIncoming->del(cFinalKeysIncomingAlice);
        d->cKeysIncoming->del(cFinalKeysIncomingBob);
        d->cKeysOutgoing->del(cFinalKeysOutgoingAlice);
        d->cKeysOutgoing->del(cFinalKeysOutgoingBob);
        
        if (is_bob() && (threshold() != nPeerThreshold)) set_threshold(nPeerThreshold);
    }
    
    return bAuthentic;
}


/**
 * get the amount of available key material for incoming authentication
 * 
 * @return  bytes of available key material for incoming authentication
 */
qulonglong qkd_auth::available_keys_incoming() const {
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    return (d->cKeysIncoming->count() * d->cKeysIncoming->quantum());
}


/**
 * get the amount of available key material for outgoing authentication
 * 
 * @return  bytes of available key material for outgoing authentication
 */
qulonglong qkd_auth::available_keys_outgoing() const {
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    return (d->cKeysOutgoing->count() * d->cKeysOutgoing->quantum());
}


/**
 * create new authenticate context 
 * 
 * @param   cIncomingContext        incoming crypto context
 * @param   cOutgoingContext        outgoing crypto context
 */
void qkd_auth::create_context(qkd::crypto::crypto_context & cIncomingContext, 
        qkd::crypto::crypto_context & cOutgoingContext) {

    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    
    // change to a new context if requested
    // TODO: this should be in sync with the peer: how to achieve this?
    if (d->bChangeSchemeIncoming && !d->cNextSchemeIncoming.null()) {
            d->cCurrentSchemeIncoming = d->cNextSchemeIncoming;
            d->cNextSchemeIncoming = qkd::crypto::scheme();
            d->bChangeSchemeIncoming = false;
    }
    if (d->bChangeSchemeOutgoing && !d->cNextSchemeOutgoing.null()) {
            d->cCurrentSchemeOutgoing = d->cNextSchemeOutgoing;
            d->cNextSchemeOutgoing = qkd::crypto::scheme();
            d->bChangeSchemeOutgoing = false;
    }
    
    try {
        if (!d->cCurrentSchemeIncoming.null()) {
            cIncomingContext = qkd::crypto::engine::create(d->cCurrentSchemeIncoming);
        }
    }
    catch (...) {
        qkd::utility::syslog::crit() << __FILENAME__ 
                << '@' 
                << __LINE__ 
                << ": " 
                << "failed to setup incoming crypto context";
    }
    try {
        if (!d->cCurrentSchemeOutgoing.null()) {
            cOutgoingContext = qkd::crypto::engine::create(d->cCurrentSchemeOutgoing);
        }
    }
    catch (...) {
        qkd::utility::syslog::crit() << __FILENAME__ 
                << '@' 
                << __LINE__ 
                << ": " 
                << "failed to setup outgoing crypto context";
    }
}


/**
 * get the current incoming authentication scheme
 * 
 * @return  the current incoming authentication scheme
 */
QString qkd_auth::current_scheme_in() const {
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    return QString::fromStdString(d->cCurrentSchemeIncoming.str());
}


/**
 * get the current outgoing authentication scheme
 * 
 * @return  the current outgoing authentication scheme
 */
QString qkd_auth::current_scheme_out() const {
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    return QString::fromStdString(d->cCurrentSchemeOutgoing.str());
}


/**
 * get the next incoming authentication scheme
 * 
 * @return  the next incoming authentication scheme
 */
QString qkd_auth::next_scheme_in() const {
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    return QString::fromStdString(d->cNextSchemeIncoming.str());
}


/**
 * get the next outgoing authentication scheme
 * 
 * @return  the next outgoing authentication scheme
 */
QString qkd_auth::next_scheme_out() const {
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    return QString::fromStdString(d->cNextSchemeOutgoing.str());
}


/**
 * module work
 * 
 * @param   cKey                    the key just read from the input pipe
 * @param   cIncomingContext        incoming crypto context
 * @param   cOutgoingContext        outgoing crypto context
 * @return  always true
 */
bool qkd_auth::process(qkd::key::key & cKey, 
        qkd::crypto::crypto_context & cIncomingContext, 
        qkd::crypto::crypto_context & cOutgoingContext) {
    
    //
    // Part I: authenticate any given crypto context created by
    //         modules *before* this authentication module
    //         
    //         This usually ends a pipeline processing       
    //
    
    // authenticate if we have a non-null context at hand
    if (!cIncomingContext->null() || !cOutgoingContext->null()) {
        
        std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
        
        // run authentication
        if (!authenticate(cKey, cIncomingContext, cOutgoingContext)) {
            
            // ############################################################
            //
            //              S E C U R I T Y    H A Z A R D
            //
            //                  Failed Authenticaion
            //
            // ############################################################
            
            pause();

            qkd::utility::syslog::crit() << __FILENAME__ 
                    << '@' 
                    << __LINE__ 
                    << ": " 
                    << "failed authentication verification for key " 
                    << cKey.id() 
                    << " - full stop";

            emit authentication_failed(cKey.id());
            
            return false;
        }
        else if (qkd::utility::debug::enabled()) {
            qkd::utility::debug() << "qkd post processing for key " << cKey.id() << " up to now has been authentic";
        }
        
        cIncomingContext = qkd::crypto::engine::create("null");
        cOutgoingContext = qkd::crypto::engine::create("null");
       
        refill_local_keystores(cKey);
    }
    
    //
    // Part II: apply a crypto context if we have one
    //
    //          this creates the initial crypto context here
    //         
    //          This usually starts pipeline processing       
    //

    create_context(cIncomingContext, cOutgoingContext);
    
    return (cKey.size() > 0);
}


/**
 * ensure the local key stores for authentication have enough keys
 *
 * @param   cKey        the key incoming
 */
void qkd_auth::refill_local_keystores(qkd::key::key & cKey) {

    if (cKey.meta().eKeyState == qkd::key::key_state::KEY_STATE_AMPLIFIED) {
        
        // alice starts filling incoming, bob starts filling outgoing
        
        if (is_alice()) {
            if (available_keys_incoming() < threshold()) {
                nibble(cKey, d->cKeysIncoming, threshold());
            }
            if (available_keys_outgoing() < threshold()) {
                nibble(cKey, d->cKeysOutgoing, threshold());
            }
        }
        
        if (is_bob()) {
            if (available_keys_outgoing() < threshold()) {
                nibble(cKey, d->cKeysOutgoing, threshold());
            }
            if (available_keys_incoming() < threshold()) {
                nibble(cKey, d->cKeysIncoming, threshold());
            }
        }
    }
        
    // still in need of keys?
    if ((available_keys_incoming() < threshold()) || (available_keys_outgoing() < threshold())) {
        qkd::utility::debug() << "key material famine in a key database: " 
                << "incoming: " 
                << available_keys_incoming() 
                << "/" 
                << threshold() 
                << " " 
                << "outgoing: " 
                << available_keys_outgoing() 
                << "/" 
                << threshold();

        emit starving();
    }
    
    // eat up all key? garfield?
    if (cKey.data().size() == 0) {
        qkd::utility::syslog::info() << "ate up the whole key by myself, nothing left to forward";
    }
}


/**
 * set the next incoming authentication scheme
 * 
 * @param   sScheme     the new next incoming authentication scheme
 */
void qkd_auth::set_next_scheme_in(QString const & sScheme) {
    
    qkd::crypto::scheme cScheme = qkd::crypto::scheme(sScheme.toStdString());
    if (!verify_scheme(cScheme)) return;

    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    d->cNextSchemeIncoming = cScheme;
    d->bChangeSchemeIncoming = true;
}


/**
 * set the next outgoing authentication scheme
 * 
 * @param   sScheme     the new next outgoing authentication scheme
 */
void qkd_auth::set_next_scheme_out(QString const & sScheme) {
    
    qkd::crypto::scheme cScheme = qkd::crypto::scheme(sScheme.toStdString());
    if (!verify_scheme(cScheme)) return;

    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    d->cNextSchemeOutgoing = cScheme;
    d->bChangeSchemeOutgoing = true;
}


/**
 * set the current authentication key material threshold in bytes
 *
 * @param   nThreshold  the new threshold to buffer key material for authentication
 */
void qkd_auth::set_threshold(qulonglong nThreshold) {
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    d->nThreshold = nThreshold;
}


/**
 * store authentication keys to use incoming
 * 
 * @param   cAuthenticationKey      the authentication key to use incoming
 */
void qkd_auth::store_keys_incoming(QByteArray cAuthenticationKey) {

    if (cAuthenticationKey.size() == 0) return;

    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    store(qkd::utility::memory::duplicate((unsigned char *)cAuthenticationKey.data(), 
            cAuthenticationKey.size()), 
            d->cKeysIncoming);

    if (available_keys_incoming() < threshold()) {
        qkd::utility::syslog::warning() << __FILENAME__ 
                << '@' 
                << __LINE__ 
                << ": " 
                << "authentication module may not have sufficient key material for incoming: " 
                << available_keys_incoming() 
                << "/" 
                << threshold() 
                << " bytes";
    }
}


/**
 * store authentication keys to use outgoing
 * 
 * @param   cAuthenticationKey      the authentication key to use outgoing
 */
void qkd_auth::store_keys_outgoing(QByteArray cAuthenticationKey) {
    
    if (cAuthenticationKey.size() == 0) return;
    
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    store(qkd::utility::memory::duplicate((unsigned char *)cAuthenticationKey.data(), 
            cAuthenticationKey.size()), 
            d->cKeysOutgoing);
    
    if (available_keys_outgoing() < threshold()) {
        qkd::utility::syslog::warning() << __FILENAME__ 
                << '@' 
                << __LINE__ 
                << ": " 
                << "authentication module may not have sufficient key material for outgoing: " 
                << available_keys_outgoing() 
                << "/" 
                << threshold() 
                << " bytes";
    }
}


/**
 * get the current authentication key material threshold in bytes
 *
 * @return  the threshold to buffer key material for authentication
 */
qulonglong qkd_auth::threshold() const {
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    return d->nThreshold;
}


/**
 * eat up some key material by moving it into a database
 * 
 * @param   cKey                    the key to consume
 * @param   cKeyDB                  the database to put the key into
 * @param   nThreshold              the maximum amount to eat
 */
void nibble(qkd::key::key & cKey, qkd::q3p::key_db & cKeyDB, uint64_t nThreshold) {

    uint64_t nKeySize = cKey.data().size();
    uint64_t nEat = std::min(nKeySize, nThreshold);

    if (nEat == 0) return;
        
    // cut from the high end (should be more efficient)
    qkd::utility::memory cFood = qkd::utility::memory::duplicate(cKey.data().get() + nKeySize - nEat, nEat);
    store(cFood, cKeyDB);
    
    cKey.data().resize(nKeySize - nEat);
    
    // TODO: how to deal with metadata?
    qkd::utility::debug() << "consumed " 
            << nEat 
            << " bytes of key material from bypassing key - tainting key meta data";
}


/**
 * store some bytes into a key database
 * 
 * @param   cMemory                 the memory block holding some key material
 * @param   cKeyDB                  the key database
 */
void store(qkd::utility::memory cMemory, qkd::q3p::key_db & cKeyDB) {
    
    qkd::key::key_ring cKeyRing(cKeyDB->quantum());
    cKeyRing << qkd::key::key(0, cMemory);
    
    // iterate over the keys to insert
    uint64_t nKeysInserted = 0;
    for (auto & cKey : cKeyRing) {
        
        if (cKey.size() == cKeyDB->quantum()) {
            
            // into the DB!
            qkd::key::key_id nKeyId = cKeyDB->insert(cKey);
            if (nKeyId != 0) {
                
                cKeyDB->set_injected(nKeyId);
                cKeyDB->set_real_sync(nKeyId);
                nKeysInserted++;
            }
            else {
                
                qkd::utility::syslog::warning() << __FILENAME__ 
                        << '@' 
                        << __LINE__ 
                        << ": " 
                        << "failed to injected key into database";
            }
        }
        else {
            
            // key does not fit ... give some message to user
            if (qkd::utility::debug::enabled()) {
                qkd::utility::debug() << "dropping "
                        << cKey.size()
                        << " bytes of key material - not a key quantum ("
                        << cKeyDB->quantum()
                        << " bytes)";
            }
        }
    }
}


/**
 * run authentication tag creation
 * 
 * the tag is created based on the crypto context and the
 * key database. the key vector is returned listening the keys
 * used in the tag creation. if the tag is correct, these keys
 * ought to be deleted.
 * 
 * @param   bAlice                  this is run as alice
 * @param   cContext                crypto context
 * @param   cKeyDB                  database holding keys to use
 * @param   cKeys                   the vector of keys utilized for the tag
 * @return  the tag, this is empty (size() == 0) in case of failure
 */
qkd::utility::memory tag(bool bAlice, 
        qkd::crypto::crypto_context & cContext, 
        qkd::q3p::key_db & cKeyDB, 
        qkd::key::key_vector & cKeys) {

    if (cContext->null()) return qkd::utility::memory();
    
    qkd::key::key cFinalKey;
    if (cContext->needs_final_key()) {
        
        cKeys = cKeyDB->find_continuous(cContext->final_key_size() * 2);
        if (cKeys.size() == 0) {
            
            // #############################################
            //
            //          Aaaarrrgghhh!!!
            // 
            // Unable to locate a consecutive series of 
            // key material to authenticate!
            //
            // This is the end. :-(
            // Can't proceed any further ... -.-
            // Over and out. I'm dead.
            //
            // #############################################
            
            qkd::utility::syslog::crit() << __FILENAME__ 
                    << '@' 
                    << __LINE__ 
                    << ": " 
                    << "cannot deduce enough key material for authentication tag creation ==> qkd post processing broken! "
                    << ":( please stop pipeline, provide this module with fresh new keys and restart... "
                    << "sorry for the inconvenience.";
            return qkd::utility::memory();
        }
        

        // get the final key: 
        // grab single key slices of quantum() size stored in cKeys
        // and aggregate them into a single key
        qkd::key::key_ring cKeysInDB = cKeyDB->ring(cKeys);
        qkd::key::key_ring cKeysRing(cContext->final_key_size());
        for (auto k : cKeysInDB) {
            cKeysRing << k;
        }
        
        // alice picks first key, bob second key
        if (bAlice) cFinalKey = cKeysRing[0];
        else cFinalKey = cKeysRing[1];
    }
    
    // final key, key DB and context ready: get the tag!
    qkd::utility::memory cTag;
    try {
        cTag = cContext->clone()->finalize(cFinalKey);
    }
    catch (...) {
        qkd::utility::syslog::crit() << __FILENAME__ 
                << '@' 
                << __LINE__ 
                << ": " 
                << "algorithm failed to create authentication tag";
    }

    return cTag;
}


/**
 * check a scheme
 * 
 * @return  true, if the scheme is applicable
 */
bool verify_scheme(qkd::crypto::scheme const & cScheme) {
    
    if (!qkd::crypto::engine::valid_scheme(cScheme)) {
        qkd::utility::syslog::warning() << __FILENAME__ 
                << '@' 
                << __LINE__ 
                << ": " 
                << "invalid scheme: " 
                << cScheme.str() 
                << " - refusing to apply scheme";
        return false;
    }
    
    // warn to use inproper schemes
    if ((cScheme.name() != "evhash") && (cScheme.name() != "null")) {
        qkd::utility::syslog::warning() << __FILENAME__ 
                << '@' 
                << __LINE__ 
                << ": " 
                << "scheme: " 
                << cScheme.str() 
                << " may not be used as an authentication scheme";
    }
    
    return true;
}



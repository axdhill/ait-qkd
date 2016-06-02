/* 
 * stash.cpp
 * 
 * QKD module key sync data
 *
 * Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *
 * Copyright (C) 2015-2016 AIT Austrian Institute of Technology
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

#include <algorithm>

#include <qkd/module/module.h>
#include <qkd/utility/debug.h>
#include <qkd/utility/syslog.h>

#include "stash.h"

using namespace qkd::module;


// ------------------------------------------------------------
// decl


/**
 * sync message commands
 */
enum class sync_command : uint32_t {
    
    SYNC_COMMAND_LIST,              /**< the message contains a list of stashed key ids */
    SYNC_COMMAND_PICK,              /**< the message contains the id of a key to pick */
    SYNC_COMMAND_NOPICK,            /**< there is no key to pick */
    SYNC_COMMAND_PICK_ACK,          /**< the peer acknowledges the key id */
    SYNC_COMMAND_PICK_NACK          /**< the peer does not acknowledge the key id */
};
    

// ------------------------------------------------------------
// fwd


/**
 * dumps a debug line about the expired keys
 * 
 * @param   cExpired        list of expired key ids
 */
static void debug_expired(std::list<qkd::key::key_id> const & cExpired);


/**
 * dumps a debug line about the current keys we have
 * 
 * @param   sHeader                 line header
 * @param   cStash                  the stash of current keys
 */
static void debug_sync(std::string const & sHeader, std::list<qkd::module::stash::stashed_key> const & cStash);


/**
 * dumps a debug line about the current keys we have
 * 
 * @param   sHeader                 line header
 * @param   cStash                  the stash of current keys
 */
static void debug_sync(std::string const & sHeader, std::list<qkd::key::key_id> const & cStash);


// ------------------------------------------------------------
// code


/**
 * ctor
 * 
 * @param   cModule     the parent module
 */
stash::stash(qkd::module::module * cModule) : 
        m_bSynchronize(true), 
        m_nTTL(10), 
        m_cModule(cModule) { 
            
    if (!m_cModule) throw std::invalid_argument("stash: parent module is null"); 
}


/**
 * choose a key from our stash knowledge
 * 
 * @return  the first key which is present in both stashes (or null key)
 */
qkd::key::key stash::choose() const {
    
    auto iter = std::find_first_of(
        m_cStash.begin(), 
        m_cStash.end(), 
        m_cPeerStash.begin(), 
        m_cPeerStash.end(), 
        [](stashed_key const & cKeyStash, qkd::key::key_id const & cPeerKey) { return (cKeyStash.cKey.id() == cPeerKey); });
    
    if (iter == m_cStash.end()) return qkd::key::key::null();
    
    return (*iter).cKey;
}



/**
 * pick a key which occurs first in both lists and remove it
 * 
 * If not such key exits, a key with is_null() == true is returned
 * 
 * @return  the first key in both lists
 */
qkd::key::key stash::pick() {
    if (m_cModule->is_alice()) return pick_alice();
    return pick_bob();
}
    

/**
 * pick a key as alice which occurs first in both lists and remove it
 * 
 * If not such key exits, a key with is_null() == true is returned
 * 
 * @return  the first key in both lists
 */
qkd::key::key stash::pick_alice() {
    
    qkd::key::key cKey = choose();

    qkd::module::message cMessage(0, qkd::module::message_type::MESSAGE_TYPE_KEY_SYNC);
    if (cKey.is_null()) {
        cMessage.data() << (uint32_t)sync_command::SYNC_COMMAND_NOPICK;
        if (m_cModule->debug_key_sync()) qkd::utility::debug() << "key-SYNC no key to pick";
    }
    else {
        cMessage.data() << (uint32_t)sync_command::SYNC_COMMAND_PICK;
        cMessage.data() << cKey.id();
        if (m_cModule->debug_key_sync()) qkd::utility::debug() << "key-SYNC pick key #" << cKey.id();
    }

    try {
        qkd::crypto::crypto_context cCryptoContext = qkd::crypto::context::null_context();
        m_cModule->send(cMessage, cCryptoContext);
    }
    catch (std::runtime_error & cException) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ 
                << ": failed to send pick of key to peer: " << cException.what();
        return qkd::key::key::null();
    }

    if (cKey.is_null()) return cKey;
    
    cMessage = qkd::module::message();
    try {
        qkd::crypto::crypto_context cCryptoContext = qkd::crypto::context::null_context();
        m_cModule->recv(cMessage, cCryptoContext, qkd::module::message_type::MESSAGE_TYPE_KEY_SYNC);
    }
    catch (std::runtime_error & cException) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ 
                << ": failed to receive acknowledge of key to pick: " << cException.what();
        return qkd::key::key::null();
    }
    
    if (cMessage.type() != qkd::module::message_type::MESSAGE_TYPE_KEY_SYNC) {
        throw std::runtime_error("received a non-sync message during key sync pick");
    }
    
    uint32_t nCmdSync = 0;
    cMessage.data() >> nCmdSync;
    
    switch ((sync_command)nCmdSync) {
        
    case sync_command::SYNC_COMMAND_PICK_ACK:
        break;
        
    case sync_command::SYNC_COMMAND_PICK_NACK:
        if (m_cModule->debug_key_sync()) qkd::utility::debug() << "key-SYNC key pick rejected by peer";
        return qkd::key::key::null();
        
    default:
        throw std::runtime_error("received a invalid answer for key pick assignment");
    }
    
    remove(cKey.id());
    
    return cKey;
}
    

/**
 * pick a key as bob which occurs first in both lists and remove it
 * 
 * If not such key exits, a key with is_null() == true is returned
 * 
 * @return  the first key in both lists
 */
qkd::key::key stash::pick_bob() {
   
    qkd::module::message cMessage;
    try {
        qkd::crypto::crypto_context cCryptoContext = qkd::crypto::context::null_context();
        m_cModule->recv(cMessage, cCryptoContext, qkd::module::message_type::MESSAGE_TYPE_KEY_SYNC);
    }
    catch (std::runtime_error & cException) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ 
                << ": failed to recv pick of key to peer: " << cException.what();
        return qkd::key::key::null();
    }
    
    if (cMessage.type() != qkd::module::message_type::MESSAGE_TYPE_KEY_SYNC) {
        throw std::runtime_error("accidentally received a non-sync message when waiting for key to pick");
    }
    
    uint32_t nCmdSync;
    cMessage.data() >> nCmdSync;
    switch ((sync_command)nCmdSync) {
        
    case sync_command::SYNC_COMMAND_PICK:
        break;
        
    case sync_command::SYNC_COMMAND_NOPICK:
        if (m_cModule->debug_key_sync()) qkd::utility::debug() << "key-SYNC no key to pick";
        return qkd::key::key::null();
        
    default:
        throw std::runtime_error("key sync message does not contain pick command");
    }
    
    qkd::key::key_id nKeyId;
    cMessage.data() >> nKeyId;
    auto iter = std::find_if(
            m_cStash.begin(), 
            m_cStash.end(), 
            [&](stashed_key const & cKeyStash) { return (cKeyStash.cKey.id() == nKeyId); });
    
    cMessage = qkd::module::message(0, qkd::module::message_type::MESSAGE_TYPE_KEY_SYNC);
    if (iter != m_cStash.end()) {
        cMessage.data() << (uint32_t)sync_command::SYNC_COMMAND_PICK_ACK;
    }
    else {
        cMessage.data() << (uint32_t)sync_command::SYNC_COMMAND_PICK_NACK;
    }
    
    try {
        qkd::crypto::crypto_context cCryptoContext = qkd::crypto::context::null_context();
        m_cModule->send(cMessage, cCryptoContext);
    }
    catch (std::runtime_error & cException) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ 
                << ": failed to send ack/nack of key to peer: " << cException.what();
        return qkd::key::key::null();
    }
    
    if (iter == m_cStash.end()) return qkd::key::key::null();
    
    qkd::key::key cKey = (*iter).cKey;
    remove(nKeyId);
    
    return cKey;
}
    

/**
 * removes keys which expired their TTL
 */
void stash::purge() {
    
    std::list<qkd::key::key_id> cExpiredKeys;
    for (auto const & k : m_cStash) {
        if (k.age() > m_nTTL) cExpiredKeys.push_back(k.cKey.id());
    }
    
    if (!cExpiredKeys.empty() && m_cModule->debug_key_sync()) debug_expired(cExpiredKeys);
    
    for (auto id : cExpiredKeys) {
        m_cStash.erase(
                std::find_if(
                        m_cStash.begin(), 
                        m_cStash.end(), 
                        [&](stashed_key const & k){ return (k.cKey.id() == id); }));
    }
}


/**
 * push a new key into our own current list
 * 
 * @param   cKey        key to push
 */
void stash::push(qkd::key::key & cKey) {
    if (cKey.is_null()) return;
    qkd::module::stash::stashed_key k = { cKey, std::chrono::system_clock::now() };
    m_cStash.push_back(k);
}


/**
 * process a received sync message
 * 
 * @param   cMessage        the message containing peer's keys
 */
void stash::recv(qkd::module::message & cMessage) {
    
    if (cMessage.type() != qkd::module::message_type::MESSAGE_TYPE_KEY_SYNC) {
        throw std::runtime_error("accidentally tried to sync keys based on a non-sync message");
    }

    m_cPeerStash.clear();
    
    uint32_t nSyncCmd;
    cMessage.data() >> nSyncCmd;
    if ((sync_command)nSyncCmd != sync_command::SYNC_COMMAND_LIST) {
        throw std::runtime_error("sync list expected, but other command received");
    }
    
    uint64_t nPeerStashKeys = 0;
    cMessage.data() >> nPeerStashKeys;
    
    for (uint64_t i = 0; i < nPeerStashKeys; ++i) {
        qkd::key::key_id cKeyId = 0;
        cMessage.data() >> cKeyId;
        m_cPeerStash.push_back(cKeyId);
    }

    if (m_cModule->debug_key_sync()) debug_sync("key-SYNC recv", m_cPeerStash);
}


/**
 * removes a key with a given id from both stashes
 * 
 * @param   nKeyId          id of key to remove
 */
void stash::remove(qkd::key::key_id nKeyId) {

    {
        auto iter = std::find_if(
                m_cStash.begin(), 
                m_cStash.end(), 
                [&](stashed_key const & cKeyStash) { return (cKeyStash.cKey.id() == nKeyId); });

        if (iter != m_cStash.end()) m_cStash.erase(iter);
    }
    
    {
        auto iter = std::find(m_cPeerStash.begin(), m_cPeerStash.end(), nKeyId);
        if (iter != m_cPeerStash.end()) m_cPeerStash.erase(iter);
    }
}


/**
 * bob: sends our keys to the peer
 */
void stash::send() {
    
    qkd::module::message cMessage(0, qkd::module::message_type::MESSAGE_TYPE_KEY_SYNC);
    cMessage.data() << (uint32_t)sync_command::SYNC_COMMAND_LIST;
    cMessage.data() << m_cStash.size();
    for (auto const & k : m_cStash) cMessage.data() << k.cKey.id();

    if (m_cModule->debug_key_sync()) debug_sync("key-SYNC send", m_cStash);
    
    try {
        qkd::crypto::crypto_context cCryptoContext = qkd::crypto::context::null_context();
        m_cModule->send(cMessage, cCryptoContext);
    }
    catch (std::runtime_error & cException) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ 
                << ": failed to send list of stashed keys to peer: " << cException.what();
    }
}


/**
 * does a sync step
 */
void stash::sync() {
    
    qkd::utility::debug() << "synchronizing keys...";

    purge();
    if (m_cModule->is_bob()) {
        send();
    }
    else {
        try {
            qkd::module::message cMessage;
            qkd::crypto::crypto_context cCryptoContext = qkd::crypto::context::null_context();
            if (m_cModule->recv(cMessage, cCryptoContext, qkd::module::message_type::MESSAGE_TYPE_KEY_SYNC)) {
                recv(cMessage);
            }
        }
        catch (std::runtime_error & cException) {}
    }
}


/**
 * dumps a debug line about the expired keys
 * 
 * @param   cExpired        list of expired key ids
 */
void debug_expired(std::list<qkd::key::key_id> const & cExpired) {
    
    if (!qkd::utility::debug::enabled()) return;
    
    std::stringstream ss;
    bool bFirst = true;
    for (auto k : cExpired) {
        if (!bFirst) ss << ", ";
        ss << k;
        bFirst = false;
    }

    qkd::utility::debug() << "key-SYNC purging expired keys " << "> [" << ss.str() << "]";
}


/**
 * dumps a debug line about the current keys we have
 * 
 * @param   sHeader                 line header
 * @param   cStash                  the stash of current keys
 */
void debug_sync(std::string const & sHeader, std::list<qkd::module::stash::stashed_key> const & cStash) {
    
    if (!qkd::utility::debug::enabled()) return;
    
    std::stringstream ss;
    bool bFirst = true;
    for (auto k : cStash) {
        if (!bFirst) ss << ", ";
        ss << k.cKey.id();
        bFirst = false;
    }

    qkd::utility::debug() << sHeader << " [" << ss.str() << "]";
}


/**
 * dumps a debug line about the current keys we have
 * 
 * @param   sHeader                 line header
 * @param   cStash                  the stash of current keys
 */
void debug_sync(std::string const & sHeader, std::list<qkd::key::key_id> const & cStash) {
            
    if (!qkd::utility::debug::enabled()) return;
    
    std::stringstream ss;
    bool bFirst = true;
    for (auto k : cStash) {
        if (!bFirst) ss << ", ";
        ss << k;
        bFirst = false;
    }

    qkd::utility::debug() << sHeader << " [" << ss.str() << "]";
}

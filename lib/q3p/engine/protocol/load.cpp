/*
 * load.cpp
 *
 * implement the Q3P KeyStore to Q3P KeyStore LOAD protocol
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

// ait
#include <qkd/key/key_ring.h>
#include <qkd/q3p/engine.h>
#include <qkd/utility/debug.h>
#include <qkd/utility/syslog.h>

#include "load.h"


using namespace qkd::q3p::protocol;


// ------------------------------------------------------------
// defs


#define TIMEOUT_SEC         5           /**< timeout in seconds for a load response */


// ------------------------------------------------------------
// const


/**
 * this rate of charge defines the sated condition
 * 
 * If the buffers are full up to this level
 * we don't trigger the load protocol.
 * 
 * This is to avoid to massive triggering of
 * tiny load issues. Since every LOAD does cost
 * twice authentication keys (1 "LOAD" and 1 "LOAD-ACK")
 * we try to avoid excessive loading of tiny
 * pieces of key chunks.
 */
static const double g_nSated = 0.90;


// ------------------------------------------------------------
// decl


/**
 * remember messages and keys to move
 */
class load_message_instance {
  
    
public:
    
    
    /**
     * the message sent 
     */
    qkd::q3p::message cMessage;
    
    qkd::key::key_vector cCommonStoreKeysForIncoming;          /**< keys to move from the CS to Incoming */
    qkd::key::key_vector cCommonStoreKeysForApplication;       /**< keys to move from the CS to Application */
    
    qkd::key::key_vector cIncomingBufferKeys;                       /**< new keys in incoming */
    qkd::key::key_vector cApplicationBufferKeys;                    /**< new keys in application */
};


/**
 * a smart pointer for load messages
 */
typedef boost::shared_ptr<load_message_instance> load_message;


/**
 * the load pimpl
 */
class qkd::q3p::protocol::load::load_data {
    
    
public:
    
    
    /**
     * ctor
     */
    load_data() { };
    
    
    /**
     * messages we sent and didn't get an answer yet 
     */
    std::map<uint32_t, load_message> cSent;     
   
};


// ------------------------------------------------------------
// code


/**
 * ctor
 * 
 * @param   cSocket     the socket we operate on
 * @param   cEngine     the parent engine
 * @throws  protocol_no_engine
 */
load::load(QAbstractSocket * cSocket, qkd::q3p::engine_instance * cEngine) : key_move(cSocket, cEngine) {
    // pimpl
    d = boost::shared_ptr<qkd::q3p::protocol::load::load_data>(new qkd::q3p::protocol::load::load_data());
}


/**
 * process a message received
 * 
 * @param   cMessage        the message read
 * @return  an protocol error variable
 */
protocol_error load::recv_internal(qkd::q3p::message & cMessage) {
    
    // sanity check
    if (!engine()) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "LOAD protocol without an engine! This is a bug.";
        emit failed((uint8_t)protocol_error::PROTOCOL_ERROR_ENGINE); 
        return protocol_error::PROTOCOL_ERROR_ENGINE;
    }

    // set the read position
    cMessage.seek_payload();

    // extract the very first string
    std::string sText;
    try {
        cMessage >> sText;
    }
    catch (...) { 
        emit failed((uint8_t)protocol_error::PROTOCOL_ERROR_SOCKET); 
        return protocol_error::PROTOCOL_ERROR_SOCKET; 
    }
    
    protocol_error eError = protocol_error::PROTOCOL_ERROR_NOT_IMPLEMENTED;
    
    // received a LOAD command
    if (sText == "LOAD") eError = recv_LOAD(cMessage);
    
    // received a LOAD ACKNOWLEDGEMENT command
    if (sText == "LOAD-ACK") eError = recv_LOAD_ACK(cMessage);
    
    return eError;
}


/**
 * process a message "LOAD" received
 * 
 * @param   cMessage        the message read
 * @return  an protocol error variable
 */
protocol_error load::recv_LOAD(qkd::q3p::message & cMessage) {

    // a "LOAD" may be only received by the slave
    if (!engine()->slave()) {
        emit failed((uint8_t)protocol_error::PROTOCOL_ERROR_ANSWER);
        return protocol_error::PROTOCOL_ERROR_ANSWER;
    }
    
    std::string sText;
    
    // the key mappings
    qkd::key::key_vector cCommonStoreKeysForIncoming;
    qkd::key::key_vector cCommonStoreKeysForApplication;
    qkd::key::key_vector cIncomingBufferKeys;
    qkd::key::key_vector cApplicationBufferKeys;
    
    try {

        // grab the keys for the outgoing (== incoming for the peer)
        cMessage >> sText;
        if (sText != "INCOMING") {
            emit failed((uint8_t)protocol_error::PROTOCOL_ERROR_ANSWER);
            return protocol_error::PROTOCOL_ERROR_ANSWER;
        }
        
        // extract the key mapping
        cMessage >> sText;
        if (sText != "C") {
            emit failed((uint8_t)protocol_error::PROTOCOL_ERROR_ANSWER);
            return protocol_error::PROTOCOL_ERROR_ANSWER;
        }
        cMessage >> cCommonStoreKeysForIncoming;
        
        cMessage >> sText;
        if (sText != "I") {
            emit failed((uint8_t)protocol_error::PROTOCOL_ERROR_ANSWER);
            return protocol_error::PROTOCOL_ERROR_ANSWER;
        }
        cMessage >> cIncomingBufferKeys;
        
        // grab the keys for the application
        cMessage >> sText;
        if (sText != "APPLICAT") {
            emit failed((uint8_t)protocol_error::PROTOCOL_ERROR_ANSWER);
            return protocol_error::PROTOCOL_ERROR_ANSWER;
        }
        
        // extract the key mapping
        cMessage >> sText;
        if (sText != "C") {
            emit failed((uint8_t)protocol_error::PROTOCOL_ERROR_ANSWER);
            return protocol_error::PROTOCOL_ERROR_ANSWER;
        }
        cMessage >> cCommonStoreKeysForApplication;
        
        cMessage >> sText;
        if (sText != "A") {
            emit failed((uint8_t)protocol_error::PROTOCOL_ERROR_ANSWER);
            return protocol_error::PROTOCOL_ERROR_ANSWER;
        }
        cMessage >> cApplicationBufferKeys;

    }
    catch (...) { 
        emit failed((uint8_t)protocol_error::PROTOCOL_ERROR_SOCKET); 
        return protocol_error::PROTOCOL_ERROR_SOCKET; 
    }

    // apply moves
    qkd::key::key_vector cMovedToOutgoing = move_outgoing(cCommonStoreKeysForIncoming, cIncomingBufferKeys);
    qkd::key::key_vector cMovedToApplication = move_application(cCommonStoreKeysForApplication, cApplicationBufferKeys);
    
    // create answer packet
    qkd::q3p::message cAckMessage(true, false);
    cAckMessage << std::string("LOAD-ACK");
    cAckMessage << cMessage.id();
    
    // place outgoing keys moved
    cAckMessage << std::string("OUTGOING");
    cAckMessage << cMovedToOutgoing;
    
    // place application keys moved
    cAckMessage << std::string("APPLICAT");
    cAckMessage << cMovedToApplication;
    
    // flush to peer
    protocol_error eError = send(cAckMessage);
    if (eError != protocol_error::PROTOCOL_ERROR_NO_ERROR) {
        emit failed((uint8_t)eError);
        return eError;
    }
    
    // debug
    if (qkd::utility::debug::enabled()) {
        
        qkd::utility::debug() 
            << "moved from common store to outgoing buffer: " << cMovedToOutgoing.size() 
            << " cs-keys; charge outgoing: " 
            << engine()->outgoing_buffer()->count() << "/" << engine()->outgoing_buffer()->amount();
        
        qkd::utility::debug() 
            << "moved from common store to application buffer: " << cMovedToApplication.size()
            << " cs-keys; charge application: " 
            << engine()->application_buffer()->count() << "/" << engine()->application_buffer()->amount();
            
        qkd::utility::debug() << "current charges: " << engine()->charge_string();
    }
    
    // DONE!
    emit success();
    
    return protocol_error::PROTOCOL_ERROR_NO_ERROR;
}


/**
 * process a message "LOAD-ACK" received
 * 
 * @param   cMessage        the message read
 * @return  an protocol error variable
 */
protocol_error load::recv_LOAD_ACK(qkd::q3p::message & cMessage) {
    
    // a "LOAD-ACK" may be only received by the master
    if (!engine()->master()) {
        emit failed((uint8_t)protocol_error::PROTOCOL_ERROR_ANSWER);
        return protocol_error::PROTOCOL_ERROR_ANSWER;
    }
    
    // this is an acknowledgement ... for which sent message?
    uint32_t nMessageId = 0;
    try {
        cMessage >> nMessageId;
    }
    catch (...) { 
        emit failed((uint8_t)protocol_error::PROTOCOL_ERROR_SOCKET); 
        return protocol_error::PROTOCOL_ERROR_SOCKET; 
    }
    
    // look up original message
    if (d->cSent.find(nMessageId) == d->cSent.end()) {
        
        // huh? received an acknowledgement for message we didn't sent?
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "received an acknowledgement for an unsent LOAD protocol message.";
        emit failed((uint8_t)protocol_error::PROTOCOL_ERROR_ANSWER); 
        return protocol_error::PROTOCOL_ERROR_ANSWER;
    }
    
    // pick original sent message
    load_message cLoadMessage = d->cSent[nMessageId];
    
    std::string sText;
    
    qkd::key::key_vector cMovedToIncoming;
    qkd::key::key_vector cMovedToApplication;
    
    try {
        
        // grab the keys for the incoming (== outgoing for the slave)
        cMessage >> sText;
        if (sText != "OUTGOING") {
            emit failed((uint8_t)protocol_error::PROTOCOL_ERROR_ANSWER);
            return protocol_error::PROTOCOL_ERROR_ANSWER;
        }
        
        // extract the set of keys moved
        cMessage >> cMovedToIncoming;

        // grab the keys for the application
        cMessage >> sText;
        if (sText != "APPLICAT") {
            emit failed((uint8_t)protocol_error::PROTOCOL_ERROR_ANSWER);
            return protocol_error::PROTOCOL_ERROR_ANSWER;
        }
        
        // extract the set of keys moved to the app buffer
        cMessage >> cMovedToApplication;
    }
    catch (...) { 
        emit failed((uint8_t)protocol_error::PROTOCOL_ERROR_SOCKET); 
        return protocol_error::PROTOCOL_ERROR_SOCKET; 
    }

    // move the keys
    move_incoming(cMovedToIncoming, cLoadMessage->cIncomingBufferKeys);
    move_application(cMovedToApplication, cLoadMessage->cApplicationBufferKeys);
    
    // collect all not moved and reset their count
    qkd::key::key_vector cNotMovedIncoming = cLoadMessage->cCommonStoreKeysForIncoming - cMovedToIncoming;
    qkd::key::key_vector cNotMovedApplication = cLoadMessage->cCommonStoreKeysForApplication - cMovedToApplication;
    engine()->common_store()->set_key_count(cNotMovedIncoming, 0);
    engine()->common_store()->set_key_count(cNotMovedApplication, 0);

    // clear those not moved: incoming
    uint64_t nKeysToClearIncoming = cNotMovedIncoming.size() * (engine()->common_store()->quantum() / engine()->incoming_buffer()->quantum());
    for (auto iter = cLoadMessage->cIncomingBufferKeys.end() - nKeysToClearIncoming; iter != cLoadMessage->cIncomingBufferKeys.end(); iter++) {
        engine()->incoming_buffer()->del(*iter);
    }
    
    // clear those not moved: application
    uint64_t nKeysToClearApplication = cNotMovedApplication.size() * (engine()->common_store()->quantum() / engine()->incoming_buffer()->quantum());
    for (auto iter = cLoadMessage->cApplicationBufferKeys.end() - nKeysToClearApplication; iter != cLoadMessage->cApplicationBufferKeys.end(); iter++) {
        engine()->application_buffer()->del(*iter);
    }
    
    // reset count for all assigned keys in the buffers
    engine()->incoming_buffer()->set_key_count(cLoadMessage->cIncomingBufferKeys, 0);
    engine()->application_buffer()->set_key_count(cLoadMessage->cApplicationBufferKeys, 0);
    
    // debug
    if (qkd::utility::debug::enabled()) {
        
        qkd::utility::debug() 
            << "moved from common store to incoming buffer: " << cMovedToIncoming.size() 
            << " cs-keys; charge incoming: " 
            << engine()->incoming_buffer()->count() << "/" << engine()->incoming_buffer()->amount();
        
        qkd::utility::debug() 
            << "moved from common store to application buffer: " << cMovedToApplication.size()
            << " cs-keys; charge application: " 
            << engine()->application_buffer()->count() << "/" << engine()->application_buffer()->amount();
            
        qkd::utility::debug() << "current charges: " << engine()->charge_string();
    }
    
    // remove pending message
    d->cSent.erase(nMessageId);
    
    // DONE!
    emit success();
    
    return protocol_error::PROTOCOL_ERROR_NO_ERROR;
}


/**
 * protocol starts
 */
void load::run_internal() {
    
    // sanity check
    if (!engine()) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "LOAD protocol without an engine! This is a bug.";
        return;
    }

    // this is a master only step here
    if (!engine()->master()) return;

    // if we have still messages ongoing: do not proceed - wait for responses
    if (d->cSent.size()) return;

    // for ease of reading the next lines
    key_db & cCommonStore = engine()->common_store();
    key_db & cIncomingBuffer = engine()->incoming_buffer();
    key_db & cOutgoingBuffer = engine()->outgoing_buffer();
    key_db & cApplicationBuffer = engine()->application_buffer();
    
    // calculate the number of bytes to be transferred from the
    // common store to the buffers.
    //
    // as this is the LOAD protocol we transfer key material from
    // the common store to the incoming buffer and the application
    // buffer, NOT the outgoing buffer.
    //
    // These are the constrains:
    //      - the incoming buffer is vital
    //      - the charge of the incoming buffer may be less or equal to
    //        the outgoing buffer
    //      - the charge of the application buffer 
    //        may not exceed any charge of the incoming or outgoing buffer
    
    // how many keys in buffer list for each common store key?
    uint64_t nCommonStoreToBufferRatio = cCommonStore->quantum() / cIncomingBuffer->quantum();
    
    // check number of keys needed: incoming buffer
    uint64_t nKeysIncoming = cIncomingBuffer->amount() - cIncomingBuffer->count();
    if (cIncomingBuffer->count() > cIncomingBuffer->amount() * g_nSated) nKeysIncoming = 0;
    if (cIncomingBuffer->count() > cOutgoingBuffer->count()) nKeysIncoming = 0;

    // check number of keys needed: application buffer (may not exceed incoming & outgoing buffers)
    uint64_t nKeysApplication = cApplicationBuffer->amount() - cApplicationBuffer->count();
    if (cApplicationBuffer->count() > cApplicationBuffer->amount() * g_nSated) nKeysApplication = 0;
    if (cApplicationBuffer->count() >= (cIncomingBuffer->count() + nKeysIncoming)) nKeysApplication = 0;
    if (cApplicationBuffer->count() >= cOutgoingBuffer->count()) nKeysApplication = 0;
    
    // do we need keys at all?
    if ((nKeysIncoming + nKeysApplication) == 0) return;
    
    // how many bytes are to spend for each buffer
    uint64_t nBytesAvailable = (cCommonStore->count() / 3) * cCommonStore->quantum();
    if (nBytesAvailable == 0) return;
    
    // check against available key material in the common store
    nKeysIncoming = std::min(nKeysIncoming, (uint64_t)(nBytesAvailable / cIncomingBuffer->quantum()));
    nKeysApplication = std::min(nKeysApplication, (uint64_t)(nBytesAvailable / cApplicationBuffer->quantum()));

    // go for multiples of common store quantums
    nKeysIncoming -= (nKeysIncoming % (cCommonStore->quantum() / cIncomingBuffer->quantum()));
    nKeysApplication -= (nKeysApplication % (cCommonStore->quantum() / cApplicationBuffer->quantum()));
    
    // don't proceed if no keys to fetch at all
    if ((nKeysIncoming == 0) && (nKeysApplication == 0)) return;
    
    // setup the message to send
    load_message cLoadMessage = boost::shared_ptr<load_message_instance>(new load_message_instance);
    
    // grab keys from the common store for incoming buffer and mark each of them
    cLoadMessage->cCommonStoreKeysForIncoming = cCommonStore->find_valid(nKeysIncoming * cIncomingBuffer->quantum(), 1);
    
    // grab keys from the incoming buffer and mark each of them
    cLoadMessage->cIncomingBufferKeys = cIncomingBuffer->find_spare(nKeysIncoming * cIncomingBuffer->quantum(), 1);
    
    // as common store quantum and buffer quantum are not equal
    // a single common store key is mapped on several buffer keys
    // so nCommonStoreToBufferRatio is a minimum, otherwise we would waste key material
    if (cLoadMessage->cIncomingBufferKeys.size() < nCommonStoreToBufferRatio) {
        
        // not enough keys: free them
        cCommonStore->set_key_count(cLoadMessage->cCommonStoreKeysForIncoming, 0);
        cIncomingBuffer->set_key_count(cLoadMessage->cIncomingBufferKeys, 0);
        
        cLoadMessage->cCommonStoreKeysForIncoming.resize(0);
        cLoadMessage->cIncomingBufferKeys.resize(0);
    }
    
    // as common store quantum and buffer quantum are not equal
    // a single common store key is mapped on several buffer keys
    // so nCommonStoreToBufferRatio is a minimum, otherwise we would waste key material
    cLoadMessage->cCommonStoreKeysForApplication = cCommonStore->find_valid(nKeysApplication * cApplicationBuffer->quantum(), 1);
    
    // grab keys from the incoming buffer and mark each of them
    cLoadMessage->cApplicationBufferKeys = cApplicationBuffer->find_spare(nKeysApplication * cApplicationBuffer->quantum(), 1);

    // nCommonStoreToBufferRatio is a minimum
    if (cLoadMessage->cApplicationBufferKeys.size() < nCommonStoreToBufferRatio) {
        
        // not enough keys: free them
        cCommonStore->set_key_count(cLoadMessage->cCommonStoreKeysForIncoming, 0);
        cApplicationBuffer->set_key_count(cLoadMessage->cApplicationBufferKeys, 0);
        
        cLoadMessage->cCommonStoreKeysForApplication.resize(0);
        cLoadMessage->cApplicationBufferKeys.resize(0);
    }

    // still something to send?
    if ((cLoadMessage->cCommonStoreKeysForIncoming.size() == 0) && (cLoadMessage->cCommonStoreKeysForApplication.size() == 0)) return;
    
    // check if sending such request do cost us more than we gain
    // that is: sending and receiving cost us authentication keys
    // we have to make more bytes into the buffers than consuming auth key bytes
    // presumably we should check for twice the outgoing keys dedicated
    // that is just a heuristic ... but fair enough?
    uint64_t nBytesNeededForAuthentication = engine()->channel().association().authentication().cOutgoing->result_size() / 8;
    if ((cLoadMessage->cCommonStoreKeysForIncoming.size() * cCommonStore->quantum()) < nBytesNeededForAuthentication * 2) {
        
        // *sigh* playing a single round of LOAD could cost us more
        // than we get into the buffers ... abort ... -.-
        cCommonStore->set_key_count(cLoadMessage->cCommonStoreKeysForIncoming, 0);
        cCommonStore->set_key_count(cLoadMessage->cCommonStoreKeysForApplication, 0);
        cIncomingBuffer->set_key_count(cLoadMessage->cIncomingBufferKeys, 0);
        cApplicationBuffer->set_key_count(cLoadMessage->cApplicationBufferKeys, 0);
        
        return;
    }
    
    // prepare LOAD message
    cLoadMessage->cMessage = qkd::q3p::message(true, false);
    cLoadMessage->cMessage << std::string("LOAD");
    
    // place incoming keys
    cLoadMessage->cMessage << std::string("INCOMING");
    cLoadMessage->cMessage << std::string("C");
    cLoadMessage->cMessage << cLoadMessage->cCommonStoreKeysForIncoming;
    cLoadMessage->cMessage << std::string("I");
    cLoadMessage->cMessage << cLoadMessage->cIncomingBufferKeys;
    
    // place application keys
    cLoadMessage->cMessage << std::string("APPLICAT");
    cLoadMessage->cMessage << std::string("C");
    cLoadMessage->cMessage << cLoadMessage->cCommonStoreKeysForApplication;
    cLoadMessage->cMessage << std::string("A");
    cLoadMessage->cMessage << cLoadMessage->cApplicationBufferKeys;

    // flush to peer
    protocol_error eError = send(cLoadMessage->cMessage);
    if (eError != protocol_error::PROTOCOL_ERROR_NO_ERROR) {
        
        // clear keys
        cCommonStore->set_key_count(cLoadMessage->cCommonStoreKeysForIncoming, 0);
        cCommonStore->set_key_count(cLoadMessage->cCommonStoreKeysForApplication, 0);
        cIncomingBuffer->set_key_count(cLoadMessage->cIncomingBufferKeys, 0);
        cApplicationBuffer->set_key_count(cLoadMessage->cApplicationBufferKeys, 0);
        
        emit failed((uint8_t)eError);
        return;
    }
    
    // copy the keys in advance
    // this is needed, since the peer may already take some of the assigned
    // keys to rightout authenticate his LOAD-ACK message ... 
    copy_incoming(cLoadMessage->cCommonStoreKeysForIncoming, cLoadMessage->cIncomingBufferKeys);
    copy_application(cLoadMessage->cCommonStoreKeysForApplication, cLoadMessage->cApplicationBufferKeys);

    // register message
    d->cSent.insert(std::pair<uint32_t, load_message>(cLoadMessage->cMessage.id(), cLoadMessage));
}


/**
 * timer event: check for timeout
 */
void load::timeout_internal() {
    
    // check all messages sent
    std::list<uint32_t> cMessagesTooOld;
    
    // check if a sent message is too old
    for (auto iter = d->cSent.begin();  iter != d->cSent.end(); iter++) {
        if (std::chrono::duration_cast<std::chrono::seconds>((*iter).second->cMessage.age()).count() > TIMEOUT_SEC) cMessagesTooOld.push_back((*iter).first);
    }
    
    // any to remove?
    if (cMessagesTooOld.empty()) return;
    
    // ok. peer missed some messages ... shit happens! *grml*
    for (auto & nMessageId : cMessagesTooOld) {
        
        // remove the message and clear all participating key counts in the buffers
        load_message cLoadMessage = d->cSent[nMessageId];
        
        // clear keys
        engine()->common_store()->set_key_count(cLoadMessage->cCommonStoreKeysForIncoming, 0);
        engine()->common_store()->set_key_count(cLoadMessage->cCommonStoreKeysForApplication, 0);
        engine()->incoming_buffer()->set_key_count(cLoadMessage->cIncomingBufferKeys, 0);
        engine()->application_buffer()->set_key_count(cLoadMessage->cApplicationBufferKeys, 0);
        
        // remove from the sent message store ...
        d->cSent.erase(nMessageId);
        
        // tell the environment
        qkd::utility::syslog::info() << "dropped pending LOAD message #" << nMessageId << " - peer didn't react";
    }
}

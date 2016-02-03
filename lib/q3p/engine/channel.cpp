/*
 * channel.cpp
 * 
 * implement the Q3P channel
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
#include <qkd/q3p/channel.h>
#include <qkd/q3p/engine.h>
#include <qkd/utility/syslog.h>
#include <qkd/utility/zip.h>

#include "protocol/protocol.h"


using namespace qkd::q3p;


// ------------------------------------------------------------
// code


/**
 * perform authentication on a message
 * 
 * @param   cMessage        the message to authenticate
 */
qkd::q3p::channel_error channel::authenticate(qkd::q3p::message & cMessage) {
    
    qkd::q3p::channel_error nError = channel_error::CHANNEL_ERROR_NO_ERROR;

    // the buffer to pick the keys from
    qkd::q3p::key_db cBuffer = engine()->outgoing_buffer();
    
    // pick the right crypto context
    qkd::crypto::crypto_context cCryptoContext = association().authentication().cOutgoing;
    
    // the key ids we use
    qkd::key::key_vector cKeys;
    
    // collect the number of bytes we need for authentication
    uint64_t nBytesNeeded = 0;
    if (cCryptoContext->needs_init_key() && !cCryptoContext->init_key_reusable()) nBytesNeeded += cCryptoContext->init_key_size();
    if (cCryptoContext->needs_final_key() && !cCryptoContext->final_key_reusable()) nBytesNeeded += cCryptoContext->final_key_size();
    
    // grab key material for operation
    cKeys = cBuffer->find_continuous(nBytesNeeded, 1);
    if ((cKeys.size() * cBuffer->quantum()) < nBytesNeeded) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to authenticate: message #" << cMessage.id() << " not enough key material left.";
        cBuffer->set_key_count(cKeys, 0);
        return channel_error::CHANNEL_ERROR_KEYS;
    }
    
    // split in init key and final key material
    auto cKeyIter = cKeys.begin();
    cMessage.set_authentication_key(cKeyIter[0]);

    // init key
    qkd::key::key cInitKey;
    if (cCryptoContext->needs_init_key() && !cCryptoContext->init_key_reusable()) {
        
        // extract the init key from the buffer keys
        qkd::key::key_ring cInitKeyRing(cCryptoContext->init_key_size());
        uint64_t nNumberOfBufferKeys = cCryptoContext->init_key_size() / cBuffer->quantum();
        for (uint64_t i = 0; i < nNumberOfBufferKeys; i++) {
            cInitKeyRing << cBuffer->get(*cKeyIter);
            cKeyIter++;
        }
        
        cInitKey = cInitKeyRing.at(0);
    }
    else {
        
        // take the old init key
        cInitKey = cCryptoContext->init_key();
    }
        
    // final key
    qkd::key::key cFinalKey;
    if (cCryptoContext->needs_final_key() && !cCryptoContext->final_key_reusable()) {
        
        // extract the final key from the buffer keys
        qkd::key::key_ring cFinalKeyRing(cCryptoContext->final_key_size());
        uint64_t nNumberOfBufferKeys = cCryptoContext->final_key_size() / cBuffer->quantum();
        for (uint64_t i = 0; i < nNumberOfBufferKeys; i++) {
            cFinalKeyRing << cBuffer->get(*cKeyIter);
            cKeyIter++;
        }
        
        cFinalKey = cFinalKeyRing.at(0);
    }

    // fix new size of message: we have an auth-tag added
    cMessage.set_length(cMessage.length() + cCryptoContext->result_size() / 8);
    
    // real crypto action start here
    qkd::utility::memory cTag;
    
    try {
        
        // reinit crypto context if need be
        cCryptoContext = qkd::crypto::engine::create(cCryptoContext->name(), cInitKey);

        // apply message
        cCryptoContext << cMessage;
        
        // get the result
        cTag = cCryptoContext->finalize(cFinalKey);
    }
    catch (...) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to authenticate: message #" << cMessage.id() << " exception during crypto context operation.";
        cBuffer->set_key_count(cKeys, 0);
        return channel_error::CHANNEL_ERROR_CONTEXT;
    }

    // set auth tag and delete keys
    cMessage.set_tag(cTag);
    engine()->outgoing_buffer()->del(cKeys);
    
    // add tag to message
    cMessage.resize(cMessage.length());
    memcpy(cMessage.get() + cMessage.size() - cTag.size(), cTag.get(), cTag.size());
    
    // trigger we changed the buffer
    engine()->outgoing_buffer()->emit_charge_change(0, cKeys.size());
    
    return nError;
}


/**
 * check the authentication of a message
 * 
 * @param   cMessage        the message to check
 */
qkd::q3p::channel_error channel::authentication_verify(qkd::q3p::message & cMessage) {
    
    qkd::q3p::channel_error nError = channel_error::CHANNEL_ERROR_NO_ERROR;

    // the buffer to pick the keys from
    qkd::q3p::key_db cBuffer = engine()->incoming_buffer();
    
    // pick the right crypto context
    qkd::crypto::crypto_context cCryptoContext = association().authentication().cIncoming;
    
    // the message MUST have at least the header + tag
    if (cMessage.size() < cMessage.header_size() + cCryptoContext->result_size() / 8) {
        
        // so a message has been tagged as authentic but can't hold a auth-tag at all!
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to verify authentication tag: message #" << cMessage.id() << " message length too short to hold header and tag.";
        return channel_error::CHANNEL_ERROR_AUTH;
    }
    
    // the key ids we use
    qkd::key::key_vector cKeys;
    
    // collect the number of bytes and therefore keys we need for authentication
    uint64_t nBytesNeeded = 0;
    if (cCryptoContext->needs_init_key() && !cCryptoContext->init_key_reusable()) nBytesNeeded += cCryptoContext->init_key_size();
    if (cCryptoContext->needs_final_key() && !cCryptoContext->final_key_reusable()) nBytesNeeded += cCryptoContext->final_key_size();
    uint64_t nKeysNeeded = nBytesNeeded / cBuffer->quantum();
    
    // decide the keys we need for check
    qkd::key::key_id nAuthenticationKeyId = cMessage.authentication_key();
    for (qkd::key::key_id i = 0; i < nKeysNeeded; i++) {
        
        // all keys must be valid
        if (!cBuffer->valid(nAuthenticationKeyId + i)) {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to verify authentication tag: message #" << cMessage.id() << " starting from auth-key-id " << nAuthenticationKeyId << " key #" << i << " (" << nAuthenticationKeyId + i << ") is not valid.";
            return channel_error::CHANNEL_ERROR_KEYS;
        }
        
        cKeys.push_back(nAuthenticationKeyId + i);
    }

    // extract the message tag and fix message tail
    cMessage.set_tag(qkd::utility::memory::duplicate(cMessage.get() + cMessage.length() - cCryptoContext->result_size() / 8, cCryptoContext->result_size() / 8));
    cMessage.resize(cMessage.length() - cCryptoContext->result_size() / 8);

    // split in init key and final key material
    auto cKeyIter = cKeys.begin();

    // init key
    qkd::key::key cInitKey;
    if (cCryptoContext->needs_init_key() && !cCryptoContext->init_key_reusable()) {
        
        // extract the init key from the buffer keys
        qkd::key::key_ring cInitKeyRing(cCryptoContext->init_key_size());
        uint64_t nNumberOfBufferKeys = cCryptoContext->init_key_size() / cBuffer->quantum();
        for (uint64_t i = 0; i < nNumberOfBufferKeys; i++) {
            cInitKeyRing << cBuffer->get(*cKeyIter);
            cKeyIter++;
        }
        
        cInitKey = cInitKeyRing.at(0);
    }
    else {
        
        // take the old init key
        cInitKey = cCryptoContext->init_key();
    }
        
    // final key
    qkd::key::key cFinalKey;
    if (cCryptoContext->needs_final_key() && !cCryptoContext->final_key_reusable()) {
        
        // extract the final key from the buffer keys
        qkd::key::key_ring cFinalKeyRing(cCryptoContext->final_key_size());
        uint64_t nNumberOfBufferKeys = cCryptoContext->final_key_size() / cBuffer->quantum();
        for (uint64_t i = 0; i < nNumberOfBufferKeys; i++) {
            cFinalKeyRing << cBuffer->get(*cKeyIter);
            cKeyIter++;
        }
        
        cFinalKey = cFinalKeyRing.at(0);
    }
        
    // real crypto action start here
    qkd::utility::memory cTag;
    
    try {
        
        // reinit crypto context if need be
        cCryptoContext = qkd::crypto::engine::create(cCryptoContext->name(), cInitKey);
        
        // apply message
        cCryptoContext << cMessage;
        
        // get the result
        cTag = cCryptoContext->finalize(cFinalKey);
    }
    catch (...) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to verify authentication tag: message #" << cMessage.id() << " exception during crypto context operation.";
        return channel_error::CHANNEL_ERROR_CONTEXT;
    }

    // this is the final authentication check
    bool bMessageIsAuthentic = cMessage.tag().equal(cTag);
    if (!bMessageIsAuthentic) {
        
        // This could be an attack!
        return channel_error::CHANNEL_ERROR_AUTH;
    }

    // set auth tag and delete keys
    engine()->incoming_buffer()->del(cKeys);
    
    // trigger we changed the buffer
    engine()->incoming_buffer()->emit_charge_change(0, cKeys.size());
    
    return nError;
}


/**
 * give a human readable description of the error
 * 
 * @param   eError      the error questioned
 * @return  a string describing the error
 */
std::string channel::channel_error_description(qkd::q3p::channel_error eError) {

    switch (eError) {
        
    case channel_error::CHANNEL_ERROR_NO_ERROR: return "no error";
    case channel_error::CHANNEL_ERROR_MESSAGE: return "the message object is malformed";
    case channel_error::CHANNEL_ERROR_KEYS: return "not enough keys in the buffers to perform action";
    case channel_error::CHANNEL_ERROR_AUTH: return "authentication failed. THIS IS CRITICAL! THIS MIGHT BE AN ATTACK!";
    case channel_error::CHANNEL_ERROR_CONTEXT: return "crypto operation failed internally";
        
    }
    
    return "unkown error";
}


/**
 * perform compression on a message
 * 
 * @param   cMessage        the message to compress
 */
qkd::q3p::channel_error channel::compress(qkd::q3p::message & cMessage) {
    
    uint64_t nHeaderSize = qkd::q3p::message::header_size();
    
    // deflate message
    qkd::utility::memory cPayload = qkd::utility::memory::wrap(cMessage.get() + nHeaderSize, cMessage.size() - nHeaderSize);
    qkd::utility::memory cCompressedMessage = qkd::utility::zip::deflate(cPayload);

    // copy the compress data
    cMessage.resize(nHeaderSize + cCompressedMessage.size());
    memcpy(cMessage.get() + nHeaderSize, cCompressedMessage.get(), cCompressedMessage.size());
    
    // fix new size of message
    cMessage.set_length(cMessage.size());
    cMessage.set_zipped(true);
    
    return channel_error::CHANNEL_ERROR_NO_ERROR;
}


/**
 * apply decryption and an authentication-check to a message
 * 
 * this is after a message has been received
 * 
 * @param   cMessage    message received
 * @return  a channel_error value
 */
qkd::q3p::channel_error channel::decode(qkd::q3p::message & cMessage) {
    
    qkd::q3p::channel_error nError = channel_error::CHANNEL_ERROR_NO_ERROR;

    // check header
    if (cMessage.length() != cMessage.size()) return channel_error::CHANNEL_ERROR_MESSAGE;
    if (cMessage.size() < qkd::q3p::message::header_size()) return channel_error::CHANNEL_ERROR_MESSAGE;

    // TODO: BUGS AHEAD-->encode->decode does not result to same message
    
    // // when authentication is set, verify authentication tag
    // if (cMessage.authentic()) nError = authentication_verify(cMessage);
    
    // // when decryption is needed, we decrypt the message
    // if (cMessage.encrypted()) nError = decrypt(cMessage);
    // if (nError != channel_error::CHANNEL_ERROR_NO_ERROR) return nError;
    
    // // check if we need to decompress the message
    // if (cMessage.zipped()) nError = decompress(cMessage);
    // if (nError != channel_error::CHANNEL_ERROR_NO_ERROR) return nError;

    return nError;
}


/**
 * perform decompression on a message
 * 
 * @param   cMessage        the message to decompress
 */
qkd::q3p::channel_error channel::decompress(qkd::q3p::message & cMessage) {
    
    uint64_t nHeaderSize = qkd::q3p::message::header_size();
    
    // deflate message
    qkd::utility::memory cPayload = qkd::utility::memory::wrap(cMessage.get() + nHeaderSize, cMessage.length() - nHeaderSize);
    qkd::utility::memory cDecompressedMessage = qkd::utility::zip::inflate(cPayload);

    // copy the compress data
    cMessage.resize(nHeaderSize + cDecompressedMessage.size());
    memcpy(cMessage.get() + nHeaderSize, cDecompressedMessage.get(), cDecompressedMessage.size());
    
    cMessage.set_zipped(false);

    return channel_error::CHANNEL_ERROR_NO_ERROR;
}


/**
 * perform decryption on a message
 * 
 * @param   cMessage        the message to decrypt
 */
qkd::q3p::channel_error channel::decrypt(UNUSED qkd::q3p::message & cMessage) {
    
    // sanity check
    if (cMessage.size() <= qkd::q3p::message::header_size()) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "refused to decrypt message with invalid size.";
        return channel_error::CHANNEL_ERROR_MESSAGE;
    }
    
    // the buffer to pick the keys from
    qkd::q3p::key_db cBuffer = engine()->incoming_buffer();
    
    // pick the right crypto context
    qkd::crypto::crypto_context cCryptoContext = association().encryption().cIncoming;

    // only "xor" is currently supported
    if (cCryptoContext->name() != "xor") {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "decryption with context '" << cCryptoContext->name() << "' currently not supported.";
        return channel_error::CHANNEL_ERROR_CONTEXT;
    }
    
    // the key ids we use
    qkd::key::key_vector cKeys;
    
    // found enough key material?
    uint64_t nBytesNeeded = cMessage.size() - cMessage.header_size();
    uint32_t nKeysNeeded = nBytesNeeded / cBuffer->quantum();
    if (nBytesNeeded % cBuffer->quantum()) nKeysNeeded++;
    
    // decide the keys we need for check
    qkd::key::key_id nEncryptionKeyId = cMessage.encryption_key();
    for (qkd::key::key_id i = 0; i < nKeysNeeded; i++) {
        
        // all keys must be valid
        if (!cBuffer->valid(nEncryptionKeyId + i)) {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to decrypt: message #" << cMessage.id() << " starting from encryption-key-id " << nEncryptionKeyId << " key #" << i << " (" << nEncryptionKeyId + i << ") is not valid.";
            return channel_error::CHANNEL_ERROR_KEYS;
        }
        
        cKeys.push_back(nEncryptionKeyId + i);
    }
    
    // construct the final key
    qkd::key::key_ring cFinalKeyRing(cKeys.size() * cBuffer->quantum());
    for (auto nKeyId : cKeys) {
        cFinalKeyRing << cBuffer->get(nKeyId);
    }
    qkd::key::key cFinalKey = cFinalKeyRing.at(0);
    
    // real crypto action start here
    qkd::utility::memory cTag;
    
    try {
        
        // reinit crypto context if need be
        cCryptoContext = qkd::crypto::engine::create(cCryptoContext->name());

        // apply message
        cCryptoContext << cMessage.payload();
        
        // get the result
        cTag = cCryptoContext->finalize(cFinalKey);
        
    }
    catch (...) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to encrypt: message #" << cMessage.id() << " exception during crypto context operation.";
        cBuffer->set_key_count(cKeys, 0);
        return channel_error::CHANNEL_ERROR_CONTEXT;
    }

    // set decrypt
    memcpy(cMessage.get() + cMessage.header_size(), cTag.get(), cTag.size());
    
    // set auth tag and delete keys
    engine()->incoming_buffer()->del(cKeys);
    
    // trigger we changed the buffer
    engine()->incoming_buffer()->emit_charge_change(0, cKeys.size());
    
    return channel_error::CHANNEL_ERROR_NO_ERROR;
}


/**
 * perform encryption on a message
 * 
 * @param   cMessage        the message to encrypt
 */
qkd::q3p::channel_error channel::encrypt(qkd::q3p::message & cMessage) {
    
    // sanity check
    if (cMessage.size() <= qkd::q3p::message::header_size()) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "refused to encrypt message with invalid size.";
        return channel_error::CHANNEL_ERROR_MESSAGE;
    }
    
    // the buffer to pick the keys from
    qkd::q3p::key_db cBuffer = engine()->outgoing_buffer();
    
    // pick the right crypto context
    qkd::crypto::crypto_context cCryptoContext = association().encryption().cOutgoing;

    // only "xor" is currently supported
    if (cCryptoContext->name() != "xor") {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "encryption with context '" << cCryptoContext->name() << "' currently not supported.";
        return channel_error::CHANNEL_ERROR_CONTEXT;
    }

    // found enough key material?
    uint64_t nBytesNeeded = cMessage.size() - cMessage.header_size();
    qkd::key::key_vector cKeys = cBuffer->find_continuous(nBytesNeeded, 1);
    if ((cKeys.size() * cBuffer->quantum()) < nBytesNeeded) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to encrypt: message #" << cMessage.id() << " not enough key material left.";
        cBuffer->set_key_count(cKeys, 0);
        return channel_error::CHANNEL_ERROR_KEYS;
    }
    
    // construct the final key
    qkd::key::key_ring cFinalKeyRing(cKeys.size() * cBuffer->quantum());
    for (auto nKeyId : cKeys) {
        cFinalKeyRing << cBuffer->get(nKeyId);
    }
    qkd::key::key cFinalKey = cFinalKeyRing.at(0);
    
    // set encryption key id
    cMessage.set_encrypted(true);
    cMessage.set_encryption_key(cKeys[0]);

    // real crypto action start here
    qkd::utility::memory cTag;
    
    try {
        
        // reinit crypto context if need be
        cCryptoContext = qkd::crypto::engine::create(cCryptoContext->name());

        // apply message
        cCryptoContext << cMessage.payload();
        
        // get the result
        cTag = cCryptoContext->finalize(cFinalKey);
        
    }
    catch (...) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to encrypt: message #" << cMessage.id() << " exception during crypto context operation.";
        cBuffer->set_key_count(cKeys, 0);
        return channel_error::CHANNEL_ERROR_CONTEXT;
    }

    // set cypher
    memcpy(cMessage.get() + cMessage.header_size(), cTag.get(), cTag.size());
    
    // delete keys
    engine()->outgoing_buffer()->del(cKeys);
    
    // trigger we changed the buffer
    engine()->outgoing_buffer()->emit_charge_change(0, cKeys.size());
    
    return channel_error::CHANNEL_ERROR_NO_ERROR;
}



/**
 * apply encryption and authentication to a message
 * 
 * this is for preparing a message to be sent
 * 
 * @param   cMessage    message to prepare
 * @return  a channel_error value
 */
qkd::q3p::channel_error channel::encode(qkd::q3p::message & cMessage) {
    
    qkd::q3p::channel_error nError = channel_error::CHANNEL_ERROR_NO_ERROR;
    
    // fix message length, id and channel number
    cMessage.set_length(cMessage.size());
    cMessage.set_channel_id(id());
    cMessage.set_id(message_id());
    
    // TODO: currently bugs: encode here, decode there does not result in same message

    // // check if we need to compress the message
    // if ((cMessage.size() >= max_uncompressed_payload()) || (cMessage.encrypted())) nError = compress(cMessage);
    // if (nError != channel_error::CHANNEL_ERROR_NO_ERROR) return nError;
    
    //// when encryption is needed, we encrypt the message
    // if (cMessage.encrypted()) nError = encrypt(cMessage);
    // if (nError != channel_error::CHANNEL_ERROR_NO_ERROR) return nError;
    
    // // when authentication is needed, we authenticate here
    // if (cMessage.authentic()) nError = authenticate(cMessage);
    // if (nError != channel_error::CHANNEL_ERROR_NO_ERROR) return nError;
    
    // increase message number
    m_nMessageId++;
    
    return nError;
}

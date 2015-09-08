/*
 * handshake.cpp
 *
 * implement the Q3P KeyStore to Q3P KeyStore HANDSHAKE protocol
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
#include <qkd/q3p/engine.h>
#include <qkd/utility/buffer.h>
#include <qkd/utility/debug.h>
#include <qkd/utility/random.h>
#include <qkd/utility/syslog.h>

#include "handshake.h"

using namespace qkd::q3p::protocol;



// ------------------------------------------------------------
// defs


#define TIMEOUT_SEC         5           /**< timeout in seconds for a handshake response */



// ------------------------------------------------------------
// decl


/**
 * the handshake pimpl
 */
class qkd::q3p::protocol::handshake::handshake_data {
    
    
public:
    
    
    /**
     * ctor
     */
    handshake_data() : m_nLocalNonce(0) { };
    
    uint32_t m_nLocalNonce;             /**< our local random number used */
    
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
handshake::handshake(QAbstractSocket * cSocket, qkd::q3p::engine_instance * cEngine) : protocol(cSocket, cEngine) {
    // pimpl
    d = boost::shared_ptr<qkd::q3p::protocol::handshake::handshake_data>(new qkd::q3p::protocol::handshake::handshake_data());
}


/**
 * decide the local role
 * 
 * @param   bPeerMaster         the peer's master role flag
 * @param   bPeerSlave          the peer's slave role flag
 * @param   nPeerNonce          the peer's random number
 * @return  true, if we have a decision
 */
bool handshake::choose_role(bool bPeerMaster, bool bPeerSlave, unsigned int nPeerNonce) {
    
    bool bLocalMaster = engine()->master();
    bool bLocalSlave = engine()->slave();
    uint32_t nLocalNonce = d->m_nLocalNonce;
    
    // go for proper role
    bool bLocalDecided = bLocalMaster ^ bLocalSlave;
    bool bPeerDecided = bPeerMaster ^ bPeerSlave;
    
    // both have a favorite role ...
    if (bLocalDecided && bPeerDecided) {
        
        if ((bLocalMaster == bPeerMaster) || (bLocalSlave == bPeerSlave)) {
            
            // both are master or both are slave ... :(
            // ---> role a dice!
            bLocalDecided = false;
            bPeerDecided = false;
            
            qkd::utility::debug() << "local and peer have the same role set ... rolling a dice";
        }
    }
    
    // we don't have a decision: role a dice
    if (!bLocalDecided && !bPeerDecided) {
        
        // the nonce MUST differ!
        if (nLocalNonce == nPeerNonce) {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "Woha! Nonce during handshake are equal! Local=" << nLocalNonce << " Peer=" << nPeerNonce << " ... that is very rare! Sorry, try again!";
            return false;
        }
        
        // get the sum of both numbers
        // if the sum is even, then the higher nonce
        // is master else slave
        uint32_t nSumOfNonce = nPeerNonce + nLocalNonce;
        bool bMasterIsHigher = (nSumOfNonce % 2 == 0);
        
        // get a decision
        if ((nLocalNonce > nPeerNonce) && bMasterIsHigher) engine()->set_master(true);
        if ((nLocalNonce < nPeerNonce) && bMasterIsHigher) engine()->set_master(false);
        if ((nLocalNonce > nPeerNonce) && !bMasterIsHigher) engine()->set_master(false);
        if ((nLocalNonce < nPeerNonce) && !bMasterIsHigher) engine()->set_master(true);
        
        // enforce slave flag
        engine()->set_slave(!engine()->master());
    }
    else if (!bLocalDecided && bPeerDecided) {
        
        // the peer had a decsion we had not: apply peer's one here
        engine()->set_master(!bPeerMaster);
        engine()->set_slave(!bPeerSlave);
        
        qkd::utility::debug() << "adjusting to peer role";
    }
    
    return true;
}


/**
 * process a message received
 * 
 * @param   cMessage        the message read
 * @return  an protocol error variable
 */
protocol_error handshake::recv_internal(qkd::q3p::message & cMessage) {

    // now read in the peer's specs
    try {

        bool bPeerMaster = false;
        bool bPeerSlave = false;
        uint32_t nPeerNonce = 0;
        std::string sPeerAuthenticationSchemeIncoming;
        std::string sPeerAuthenticationSchemeOutgoing;
        std::string sPeerEncryptionSchemeIncoming;
        std::string sPeerEncryptionSchemeOutgoing;
        
        // extract the specs
        try {
            
            cMessage >> bPeerMaster;
            cMessage >> bPeerSlave;
            cMessage >> nPeerNonce;
            cMessage >> sPeerAuthenticationSchemeIncoming;
            cMessage >> sPeerAuthenticationSchemeOutgoing;
            cMessage >> sPeerEncryptionSchemeIncoming;
            cMessage >> sPeerEncryptionSchemeOutgoing;
        }
        catch (...) { 
            emit failed((uint8_t)protocol_error::PROTOCOL_ERROR_ANSWER); 
            return protocol_error::PROTOCOL_ERROR_ANSWER; 
        }
        
        // decide a proper role
        if (!choose_role(bPeerMaster, bPeerSlave, nPeerNonce)) {
            qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to choose a role";
            emit failed((uint8_t)protocol_error::PROTOCOL_ERROR_CONFIG);
            return protocol_error::PROTOCOL_ERROR_CONFIG;
        }
        
        // check the auth schemes
        bool bAuthentication = (sPeerAuthenticationSchemeIncoming == engine()->authentication_scheme_outgoing().toStdString()) && (sPeerAuthenticationSchemeOutgoing == engine()->authentication_scheme_incoming().toStdString());
        bool bEncryption = (sPeerEncryptionSchemeIncoming == engine()->encryption_scheme_outgoing().toStdString()) && (sPeerEncryptionSchemeOutgoing == engine()->encryption_scheme_incoming().toStdString());
        if (!bAuthentication) {
            qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "authentication schemes mismatch";
            emit failed((uint8_t)protocol_error::PROTOCOL_ERROR_CONFIG);
            return protocol_error::PROTOCOL_ERROR_CONFIG;
        }
        if (!bEncryption) {
            qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "encryption schemes mismatch";
            emit failed((uint8_t)protocol_error::PROTOCOL_ERROR_CONFIG);
            return protocol_error::PROTOCOL_ERROR_CONFIG;
        }
        
        // check buffer configs
        uint64_t nCommonMinId = 0;
        uint64_t nCommonMaxId = 0;
        uint64_t nCommonQuantum = 0;
        uint64_t nInMinId = 0;
        uint64_t nInMaxId = 0;
        uint64_t nInQuantum = 0;
        uint64_t nOutMinId = 0;
        uint64_t nOutMaxId = 0;
        uint64_t nOutQuantum = 0;
        uint64_t nAppMinId = 0;
        uint64_t nAppMaxId = 0;
        uint64_t nAppQuantum = 0;
        
        // incoming buffer
        try {
            cMessage >> nCommonMinId;
            cMessage >> nCommonMaxId;
            cMessage >> nCommonQuantum;
        }
        catch (...) { 
            emit failed((uint8_t)protocol_error::PROTOCOL_ERROR_SOCKET); 
            return protocol_error::PROTOCOL_ERROR_SOCKET; 
        }
        if ((nCommonMinId != engine()->common_store()->min_id()) || (nCommonMaxId != engine()->common_store()->max_id()) || (nCommonQuantum != engine()->common_store()->quantum())) {
            qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "common store specification mismatch";
            emit failed((uint8_t)protocol_error::PROTOCOL_ERROR_CONFIG);
            return protocol_error::PROTOCOL_ERROR_CONFIG;
        }
        
        // incoming buffer
        try {
            cMessage >> nInMinId;
            cMessage >> nInMaxId;
            cMessage >> nInQuantum;
        }
        catch (...) { 
            emit failed((uint8_t)protocol_error::PROTOCOL_ERROR_SOCKET); 
            return protocol_error::PROTOCOL_ERROR_SOCKET; 
        }
        if ((nInMinId != engine()->incoming_buffer()->min_id()) || (nInMaxId != engine()->incoming_buffer()->max_id()) || (nInQuantum != engine()->incoming_buffer()->quantum())) {
            qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "incoming buffer specification mismatch";
            emit failed((uint8_t)protocol_error::PROTOCOL_ERROR_CONFIG);
            return protocol_error::PROTOCOL_ERROR_CONFIG;
        }
        
        // outgoing buffer
        try {
            cMessage >> nOutMinId;
            cMessage >> nOutMaxId;
            cMessage >> nOutQuantum;
        }
        catch (...) { 
            emit failed((uint8_t)protocol_error::PROTOCOL_ERROR_SOCKET); 
            return protocol_error::PROTOCOL_ERROR_SOCKET; 
        }
        if ((nOutMinId != engine()->outgoing_buffer()->min_id()) || (nOutMaxId != engine()->outgoing_buffer()->max_id()) || (nOutQuantum != engine()->outgoing_buffer()->quantum())) {
            qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "outgoing buffer specification mismatch";
            emit failed((uint8_t)protocol_error::PROTOCOL_ERROR_CONFIG);
            return protocol_error::PROTOCOL_ERROR_CONFIG;
        }
        
        // application buffer
        try {
            cMessage >> nAppMinId;
            cMessage >> nAppMaxId;
            cMessage >> nAppQuantum;
        }
        catch (...) { 
            emit failed((uint8_t)protocol_error::PROTOCOL_ERROR_SOCKET); 
            return protocol_error::PROTOCOL_ERROR_SOCKET; 
        }
        if ((nAppMinId != engine()->application_buffer()->min_id()) || (nAppMaxId != engine()->application_buffer()->max_id()) || (nAppQuantum != engine()->application_buffer()->quantum())) {
            qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "application buffer specification mismatch";
            emit failed((uint8_t)protocol_error::PROTOCOL_ERROR_CONFIG);
            return protocol_error::PROTOCOL_ERROR_CONFIG;
        }
        
        // cross check of limits: min_id
        if ((nInMinId != nOutMinId) || (nInMinId != nAppMinId) || (nOutMinId != nAppMinId)) {
            qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "minimum id cross check failed";
            emit failed((uint8_t)protocol_error::PROTOCOL_ERROR_CONFIG);
            return protocol_error::PROTOCOL_ERROR_CONFIG;
        }
            
        // cross check of limits: max_id
        if ((nInMaxId != nOutMaxId) || (nInMaxId != nAppMaxId) || (nOutMaxId != nAppMaxId)) {
            qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "maximum id cross check failed";
            emit failed((uint8_t)protocol_error::PROTOCOL_ERROR_CONFIG);
            return protocol_error::PROTOCOL_ERROR_CONFIG;
        }
            
        // cross check of limits: buffer quantums
        if ((nInQuantum != nOutQuantum) || (nInQuantum != nAppQuantum) || (nOutQuantum != nAppQuantum)) {
            qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "buffer quantum cross check failed";
            emit failed((uint8_t)protocol_error::PROTOCOL_ERROR_CONFIG);
            return protocol_error::PROTOCOL_ERROR_CONFIG;
        }
        
        // cross check of limits: common store to buffer: 
        // the quantum of the common store MUST be a full multiple of the buffer quantums
        if (((nCommonQuantum / nInQuantum) == 0) || ((nCommonQuantum % nInQuantum) != 0)) {
            qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "common store quantum not a multiple of buffer quantum";
            emit failed((uint8_t)protocol_error::PROTOCOL_ERROR_CONFIG);
            return protocol_error::PROTOCOL_ERROR_CONFIG;
        }
        
    }
    catch (...) {
        emit failed((uint8_t)protocol_error::PROTOCOL_ERROR_ANSWER);
        return protocol_error::PROTOCOL_ERROR_ANSWER;
    }
    
    // up here we have a success!
    emit success();
    return protocol_error::PROTOCOL_ERROR_NO_ERROR;
}


/**
 * protocol starts
 */
void handshake::run_internal() {

    protocol_error eError = protocol_error::PROTOCOL_ERROR_NO_ERROR;

    // sanity check
    if (!valid_socket()) {
        emit failed((uint8_t)protocol_error::PROTOCOL_ERROR_SOCKET); 
        return;
    }
    
    // send my specs
    qkd::q3p::message cMessage(false, false);
    qkd::utility::random_source::source() >> d->m_nLocalNonce;
    d->m_nLocalNonce %= 100000;
    
    // handshake packet
    cMessage << engine()->master();                                         // master bool
    cMessage << engine()->slave();                                          // slave bool
    cMessage << d->m_nLocalNonce;                                           // random number
    cMessage << engine()->authentication_scheme_incoming().toStdString();   // incoming auth scheme
    cMessage << engine()->authentication_scheme_outgoing().toStdString();   // outgoing auth scheme
    cMessage << engine()->encryption_scheme_incoming().toStdString();       // incoming encryption scheme
    cMessage << engine()->encryption_scheme_outgoing().toStdString();       // outgoing encryption scheme
    
    cMessage << (uint64_t)engine()->common_store()->min_id();               // common store specs: min_id
    cMessage << (uint64_t)engine()->common_store()->max_id();               // common store specs: max_id
    cMessage << (uint64_t)engine()->common_store()->quantum();              // common store specs: quantum
    cMessage << (uint64_t)engine()->incoming_buffer()->min_id();            // incoming specs: min_id
    cMessage << (uint64_t)engine()->incoming_buffer()->max_id();            // incoming specs: max_id
    cMessage << (uint64_t)engine()->incoming_buffer()->quantum();           // incoming specs: quantum
    cMessage << (uint64_t)engine()->outgoing_buffer()->min_id();            // outgoing specs: min_id
    cMessage << (uint64_t)engine()->outgoing_buffer()->max_id();            // outgoing specs: max_id
    cMessage << (uint64_t)engine()->outgoing_buffer()->quantum();           // outgoing specs: quantum
    cMessage << (uint64_t)engine()->application_buffer()->min_id();         // application specs: min_id
    cMessage << (uint64_t)engine()->application_buffer()->max_id();         // application specs: max_id
    cMessage << (uint64_t)engine()->application_buffer()->quantum();        // application specs: quantum

    qkd::utility::debug() << "local configuration:\n" << 
        "\t      master: " << engine()->master() << "\n" <<
        "\t       slave: " << engine()->slave() << "\n" <<
        "\t       nonce: " << d->m_nLocalNonce << "\n" <<
        "\t     auth-IN: " << engine()->authentication_scheme_incoming().toStdString() << "\n" <<
        "\t    auth-OUT: " << engine()->authentication_scheme_outgoing().toStdString() << "\n" <<
        "\t     encr-IN: " << engine()->encryption_scheme_incoming().toStdString() << "\n" <<
        "\t    encr-OUT: " << engine()->encryption_scheme_outgoing().toStdString() << "\n" <<
        "\tcommon-store: \n" <<
        "\t          min-id: " << engine()->common_store()->min_id() << "\n" <<
        "\t          max-id: " << engine()->common_store()->max_id() << "\n" <<
        "\t         quantum: " << engine()->common_store()->quantum() << "\n" <<
        "\t    incoming: \n" <<
        "\t          min-id: " << engine()->incoming_buffer()->min_id() << "\n" <<
        "\t          max-id: " << engine()->incoming_buffer()->max_id() << "\n" <<
        "\t         quantum: " << engine()->incoming_buffer()->quantum() << "\n" <<
        "\t    outgoing: \n" <<
        "\t          min-id: " << engine()->incoming_buffer()->min_id() << "\n" <<
        "\t          max-id: " << engine()->incoming_buffer()->max_id() << "\n" <<
        "\t         quantum: " << engine()->incoming_buffer()->quantum() << "\n" <<
        "\t application: \n" <<
        "\t          min-id: " << engine()->incoming_buffer()->min_id() << "\n" <<
        "\t          max-id: " << engine()->incoming_buffer()->max_id() << "\n" <<
        "\t         quantum: " << engine()->incoming_buffer()->quantum();
        
    // flush to peer
    eError = send(cMessage);
    if (eError != protocol_error::PROTOCOL_ERROR_NO_ERROR) emit failed((uint8_t)eError);
}


/**
 * timer event: check for timeout
 */
void handshake::timeout_internal() {
    
    // check for timeout
    static auto nStart = std::chrono::high_resolution_clock::now();
    auto nTimeDiff = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - nStart);
    if (nTimeDiff.count() > TIMEOUT_SEC) emit failed((uint8_t)protocol_error::PROTOCOL_ERROR_TIMEOUT);
}


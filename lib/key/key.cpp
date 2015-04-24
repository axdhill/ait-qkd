/*
 * key.cpp
 * 
 * QKD key implementation
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

#include <iostream>


// ait
#include <qkd/key/key.h>

using namespace qkd::key;


// ------------------------------------------------------------
// code


/**
 * access the key id counter
 * 
 * @return  the class wide key_id_counter
 */
qkd::key::key::key_id_counter & qkd::key::key::counter() {
    static qkd::key::key::key_id_counter cCounter;
    return cCounter;
}


/**
 * read from a buffer
 * 
 * @param   cBuffer     the buffer to read from
 */
void qkd::key::key::meta_data::read(qkd::utility::buffer & cBuffer) {
    
    cBuffer >> (uint8_t&)eKeyState;
    cBuffer >> nErrorBits;
    cBuffer >> nDisclosedBits;
    cBuffer >> nErrorRate;
    
    cBuffer >> sCryptoSchemeIncoming;
    cBuffer >> sCryptoSchemeOutgoing;
    
}


/**
 * read from stream
 * 
 * @param   cStream     the stream to read from
 */
void qkd::key::key::meta_data::read(std::istream & cStream) { 
    
    uint8_t nKeyState = 0;
    cStream.read((char *)&nKeyState, sizeof(nKeyState));
    this->eKeyState = static_cast<qkd::key::key_state>(nKeyState);
    
    // read error bits
    uint64_t nErrorBits = 0;
    cStream.read((char *)&nErrorBits, sizeof(nErrorBits));
    this->nErrorBits = be64toh(nErrorBits);
    
    // read disclosed bits
    uint64_t nDisclosedBits = 0;
    cStream.read((char *)&nDisclosedBits, sizeof(nDisclosedBits));
    this->nDisclosedBits = be64toh(nDisclosedBits);
    
    // read error rate
    cStream.read((char *)&(this->nErrorRate), sizeof(this->nErrorRate));
    
    // read the incoming crypto scheme string
    uint64_t nIncomingSchemeLength = 0;
    cStream.read((char *)&nIncomingSchemeLength, sizeof(nIncomingSchemeLength));
    nIncomingSchemeLength = be64toh(nIncomingSchemeLength);
    if (nIncomingSchemeLength) {
        char * sIncomingScheme = new char[nIncomingSchemeLength + 1];
        cStream.read(sIncomingScheme, nIncomingSchemeLength);
        sIncomingScheme[nIncomingSchemeLength] = 0;
        this->sCryptoSchemeIncoming = sIncomingScheme;
        delete [] sIncomingScheme;
    }
    else this->sCryptoSchemeIncoming = std::string();

    // read the outgoing crypto scheme string
    uint64_t nOutgoingSchemeLength = 0;
    cStream.read((char *)&nOutgoingSchemeLength, sizeof(nOutgoingSchemeLength));
    nOutgoingSchemeLength = be64toh(nOutgoingSchemeLength);
    if (nOutgoingSchemeLength) {
        char * sOutgoingScheme = new char[nOutgoingSchemeLength + 1];
        cStream.read(sOutgoingScheme, nOutgoingSchemeLength);
        sOutgoingScheme[nOutgoingSchemeLength] = 0;
        this->sCryptoSchemeOutgoing = sOutgoingScheme;
        delete [] sOutgoingScheme;
    }
    else this->sCryptoSchemeOutgoing = std::string();
}


/**
 * write to a buffer
 * 
 * @param   cBuffer     the buffer to write to
 */
void qkd::key::key::meta_data::write(qkd::utility::buffer & cBuffer) const {
    cBuffer << (uint8_t)eKeyState;
    cBuffer << nErrorBits;
    cBuffer << nDisclosedBits;
    cBuffer << nErrorRate;
    cBuffer << sCryptoSchemeIncoming;
    cBuffer << sCryptoSchemeOutgoing;
}


/**
 * write to stream
 * 
 * @param   cStream     the stream to write to
 */
void qkd::key::key::meta_data::write(std::ostream & cStream) const { 
    
    // write key state
    cStream.write((char*)&(this->eKeyState), sizeof(this->eKeyState));
    
    // write error bits
    uint64_t nErrorBits = htobe64(this->nErrorBits);
    cStream.write((char *)&nErrorBits, sizeof(nErrorBits));
    
    // write disclosed bits
    uint64_t nDislosedBits = htobe64(this->nDisclosedBits);
    cStream.write((char *)&nDislosedBits, sizeof(nDislosedBits));

    // write error rate
    cStream.write((char *)&(this->nErrorRate), sizeof(this->nErrorRate));
    
    // write crypto scheme strings
    uint64_t nIncomingSchemeLength = htobe64(this->sCryptoSchemeIncoming.length());
    cStream.write((char *)&nIncomingSchemeLength, sizeof(nIncomingSchemeLength));
    if (nIncomingSchemeLength) cStream.write(this->sCryptoSchemeIncoming.c_str(), this->sCryptoSchemeIncoming.length());
    
    uint64_t nOutgoingSchemeLength = htobe64(this->sCryptoSchemeOutgoing.length());
    cStream.write((char *)&nOutgoingSchemeLength, sizeof(nOutgoingSchemeLength));
    if (nOutgoingSchemeLength) cStream.write(this->sCryptoSchemeOutgoing.c_str(), this->sCryptoSchemeOutgoing.length());
}


/**
 * read from a buffer
 * 
 * if we fail to read a key the key is equal to null()
 * 
 * @param   cBuffer     the buffer to read from
 */
void qkd::key::key::read(qkd::utility::buffer & cBuffer) {
    
    cBuffer >> m_nId;
    m_cMeta.read(cBuffer);
    cBuffer >> m_cData;
    
    // record timestamp
    m_cMeta.cTimestampRead = std::chrono::high_resolution_clock::now();
}


/**
 * read from stream
 * 
 * if we fail to read a key the key is equal to null()
 * 
 * @param   cStream     the stream to read from
 */
void qkd::key::key::read(std::istream & cStream) { 

    // read key id
    key_id nId = 0;
    cStream.read((char*)&nId, sizeof(nId));
    m_nId = be32toh(nId);
    
    // read meta information
    m_cMeta.read(cStream);
    
    // read blob
    cStream >> m_cData; 
}


/**
 * give a stringified state
 * 
 * @param   eKeyState           the state of the key questioned
 * @return  a string holding the key state
 */
std::string qkd::key::key::state_string(qkd::key::key_state eKeyState) {
    
    std::string sState;
    switch (eKeyState) {
        
    case qkd::key::key_state::KEY_STATE_OTHER:
        sState = "other";
        break;
            
    case qkd::key::key_state::KEY_STATE_RAW:
        sState = "raw";
        break;

    case qkd::key::key_state::KEY_STATE_SIFTED:
        sState = "sifted";
        break;

    case qkd::key::key_state::KEY_STATE_CORRECTED:
        sState = "corrected";
        break;

    case qkd::key::key_state::KEY_STATE_UNCORRECTED:
        sState = "uncorrected";
        break;

    case qkd::key::key_state::KEY_STATE_CONFIRMED:
        sState = "confirmed";
        break;

    case qkd::key::key_state::KEY_STATE_UNCONFIRMED:
        sState = "unconfirmed";
        break;

    case qkd::key::key_state::KEY_STATE_AMPLIFIED:
        sState = "amplified";
        break;

    case qkd::key::key_state::KEY_STATE_AUTHENTICATED:
        sState = "authenticated";
        break;

    case qkd::key::key_state::KEY_STATE_DISCLOSED:
        sState = "disclosed";
        break;
    }
    
    return sState;
}


/**
 * write to stream
 * 
 * @param   cBuffer     the buffer to write to
 */
void qkd::key::key::write(qkd::utility::buffer & cBuffer) const {
    cBuffer << m_nId;
    m_cMeta.write(cBuffer);
    cBuffer << m_cData;
}


/**
 * write to stream
 * 
 * @param   cStream     the stream to write to
 */
void qkd::key::key::write(std::ostream & cStream) const { 
    
    // write key id
    key_id nId = htobe32(id());
    cStream.write((char*)&nId, sizeof(nId));
    
    // write meta information
    m_cMeta.write(cStream);
    
    // write key data
    cStream << m_cData; 
}


/**
 * substract one key_vector from the other
 * 
 * HENCE: lhs and rhs are meant to contain __sorted__ keys
 * 
 * @param   lhs     left hand side
 * @param   rhs     right hand side
 * @return  a key vector containing all key_ids in lhs not in rhs
 */
qkd::key::key_vector qkd::key::sub(qkd::key::key_vector const & lhs, qkd::key::key_vector const & rhs) {
    
    qkd::key::key_vector res;
    
    // sanity check
    if (lhs.size() == 0) return res;
    
    auto lhs_iter = lhs.begin();
    auto rhs_iter = rhs.begin();
    
    // walk over lhs
    while (lhs_iter != lhs.end()) {
        
        // rhs at end: easy
        if (rhs_iter == rhs.end()) {
            res.push_back(*lhs_iter);
            lhs_iter++;
            continue;
        }
        
        // both equal -> next step
        if ((*lhs_iter) == (*rhs_iter)) {
            lhs_iter++;
            rhs_iter++;
            continue;
        }
        
        // lhs < rhs?
        if ((*lhs_iter) < (*rhs_iter)) {
            res.push_back(*lhs_iter);
            lhs_iter++;
            continue;
        }
        
        // lhs > rhs?
        if ((*lhs_iter) > (*rhs_iter)) {
            rhs_iter++;
            continue;
        }
    }
    
    return res;
}
  


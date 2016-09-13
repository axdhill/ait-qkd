/*
 * key.cpp
 * 
 * QKD key implementation
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

#include <iostream>

#include <boost/version.hpp>
#include <boost/property_tree/xml_parser.hpp>

// ait
#include <qkd/utility/debug.h>
#include <qkd/key/key.h>

using namespace qkd::key;


// ------------------------------------------------------------
// code


/**
 * ctor
 */
qkd::key::key::key() : m_nId(0) { 
    init_metadata(); 
}


/**
 * copy ctor
 * 
 * this is a shallow copy
 * 
 * @param   rhs     right hand side
 */
qkd::key::key::key(key const & rhs) : 
        m_nId(rhs.m_nId), 
        m_cData(rhs.m_cData), 
        m_cMetaData(rhs.m_cMetaData), 
        m_cTimestampRead(rhs.m_cTimestampRead) 
{
}


/**
 * ctor
 * 
 * the given memory area is NOT(!) copied (shallow copy).
 * 
 * @param   nId         ID of the key
 * @param   cMemory     memory holding the key bits
 * @param   sEncoding   key data encoding
 */
qkd::key::key::key(key_id nId, qkd::utility::memory & cMemory, std::string const & sEncoding) : 
        m_nId(nId), 
        m_cData(cMemory), 
        m_cTimestampRead(std::chrono::high_resolution_clock::now()) 
{ 
    init_metadata(); 
    set_encoding(sEncoding);
}


/**
 * ctor
 * 
 * the given memory area is copied (deep copy).
 * 
 * @param   nId         ID of the key
 * @param   cMemory     memory holding the key bits
 * @param   sEncoding   key data encoding
 */
qkd::key::key::key(key_id nId, qkd::utility::memory const & cMemory, std::string const & sEncoding) : 
        m_nId(nId), 
        m_cData(cMemory.clone()), 
        m_cTimestampRead(std::chrono::high_resolution_clock::now()) 
{ 
    init_metadata(); 
    set_encoding(sEncoding);
}


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
 * inits the meta data
 */
void qkd::key::key::init_metadata() {
    m_cMetaData.put("key.<xmlattr>.id", m_nId);
    m_cMetaData.put("key.general.state", state_string(qkd::key::key_state::KEY_STATE_NEW));
    m_cMetaData.put("key.general.state.<xmlattr>.id", static_cast<int>(qkd::key::key_state::KEY_STATE_NEW));
    m_cMetaData.put("key.general.crypto.incoming", "null");
    m_cMetaData.put("key.general.crypto.outgoing", "null");
    m_cMetaData.put("key.general.bits", data().size() * 8);
    m_cMetaData.put("key.general.qber", 0.0);
    m_cMetaData.put("key.general.disclosed", 0);
    m_cMetaData.put("key.general.encoding", DEFAULT_DATA_ENCODING);
    m_cMetaData.put("key.modules", std::string());
}


/**
 * get current module section of the key's metadata
 * 
 * @return  the metadata property tree for the key's current module
 */
boost::property_tree::ptree & qkd::key::key::metadata_current_module() { 
    return metadata_modules().rbegin()->second; 
}


/**
 * get current module section of the key's metadata
 * 
 * @return  the metadata property tree for the key's current module
 */
boost::property_tree::ptree const & qkd::key::key::metadata_current_module() const { 
    return metadata_modules().rbegin()->second; 
}


/**
 * get modules metadata of the key
 * 
 * @return  the metadata property tree for the key's modules
 */
boost::property_tree::ptree & qkd::key::key::metadata_modules() { 
    return m_cMetaData.get_child("key.modules");
}


/**
 * get modules metadata of the key
 * 
 * @return  the metadata property tree for the key's modules
 */
boost::property_tree::ptree const & qkd::key::key::metadata_modules() const { 
    return m_cMetaData.get_child("key.modules");
}


/**
 * get metadata of the key as XML
 * 
 * @param   bPretty     pretty formatting enabled or not
 * @return  the metadata as XML
 */
std::string qkd::key::key::metadata_xml(bool bPretty) const {
    std::stringstream ss;
#if BOOST_VERSION < 105800    
    boost::property_tree::xml_writer_settings<char> cSettings(' ', (bPretty ? 4 : 0));
#else
    boost::property_tree::xml_writer_settings<std::string> cSettings(' ', (bPretty ? 4 : 0));
#endif
    boost::property_tree::write_xml(ss, m_cMetaData, cSettings);
    return ss.str();
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
    
    try {
        std::string sMetaDataXML;
        cBuffer >> sMetaDataXML;
        std::istringstream is(sMetaDataXML);
        boost::property_tree::read_xml(is, m_cMetaData, boost::property_tree::xml_parser::trim_whitespace);
    }
    catch (std::exception const & e) {
        qkd::utility::debug() << "Failed to parse key's XML metadata: " << e.what();
        throw std::runtime_error(e.what());
    }
    
    cBuffer >> m_cData;
    
    m_cTimestampRead = std::chrono::high_resolution_clock::now();
}


/**
 * read from stream
 * 
 * if we fail to read a key the key is equal to null()
 * 
 * @param   cStream     the stream to read from
 */
void qkd::key::key::read(std::istream & cStream) { 

    key_id nId = 0;
    cStream.read((char*)&nId, sizeof(nId));
    m_nId = be32toh(nId);
    
    uint64_t n = 0;
    cStream.read((char *)&n, sizeof(n));
    uint64_t nSize = be64toh(n);
    char * d = new char[nSize]; 
    cStream.read(d, nSize);
    std::string s(d, nSize); 
    
    std::istringstream xmlInputStream(s);
    boost::property_tree::ptree cXML;
    boost::property_tree::read_xml(xmlInputStream, m_cMetaData, boost::property_tree::xml_parser::trim_whitespace);
    
    delete [] d; 
    
    cStream >> m_cData; 
}


/**
 * sets the current crypto scheme used for this key
 * 
 * The crypto scheme string holds the algorithm, init-key
 * and current state of all incoming messages from the peer 
 * bound to this key (see qkd/crypto/scheme.h)
 * 
 * @param   sScheme     a string holding the new current crypto scheme for incoming
 */
void qkd::key::key::set_crypto_scheme_incoming(std::string sScheme) {
    m_cMetaData.put("key.general.crypto.incoming", sScheme);
}


/**
 * sets the current crypto scheme used for this key
 * 
 * The crypto scheme string holds the algorithm, init-key
 * and current state of all outgoing messages from the peer 
 * bound to this key (see qkd/crypto/scheme.h)
 * 
 * @param   sScheme     a string holding the new current crypto scheme for outgoing
 */
void qkd::key::key::set_crypto_scheme_outgoing(std::string sScheme) {
    m_cMetaData.put("key.general.crypto.outgoing", sScheme);
}


/**
 * sets the number of disclosed information bits of this key
 * 
 * @param   nDisclosed  the new number of disclosed information bits
 */
void qkd::key::key::set_disclosed(uint64_t nDisclosed) {
    m_cMetaData.put("key.general.disclosed", nDisclosed);
}


/**
 * sets the key bit encoding description
 * 
 * @param   sEncoding       a string describing the key bit encoding format
 */
void qkd::key::key::set_encoding(std::string sEncoding) {
    m_cMetaData.put("key.general.encoding", sEncoding);
}


/**
 * set a new key id
 * 
 * @param   nId         the new key id
 */
void key::set_id(key_id nId) { 
    m_nId = nId; 
    m_cMetaData.put("key.<xmlattr>.id", nId);
}


/**
 * set the key's QBER
 * 
 * @param   nQBER       the new key's QBER
 */
void qkd::key::key::set_qber(double nQBER) {
    m_cMetaData.put("key.general.qber", nQBER);
}


/**
 * set the key's state
 * 
 * @param   eKeyState   the new key state
 */
void qkd::key::key::set_state(key_state eKeyState) {
    m_cMetaData.put("key.general.state", state_string(eKeyState));
    m_cMetaData.put("key.general.state.<xmlattr>.id", static_cast<int>(eKeyState));
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
        
    case qkd::key::key_state::KEY_STATE_TAINTED:
        sState = "tainted";
        break;
        
    case qkd::key::key_state::KEY_STATE_NEW:
        sState = "new";
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
    cBuffer << metadata_xml(false);
    cBuffer << m_cData;
}


/**
 * write to stream
 * 
 * @param   cStream     the stream to write to
 */
void qkd::key::key::write(std::ostream & cStream) const { 
    
    key_id nId = htobe32(id());
    cStream.write((char*)&nId, sizeof(nId));

    std::string s = metadata_xml(false);
    uint64_t n = htobe64(s.size());
    cStream.write((char *)&n, sizeof(n));
    cStream << s;
    
    cStream << m_cData; 
}


/**
 * subtract one key_vector from the other
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

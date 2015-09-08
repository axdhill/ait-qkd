/*
 * scheme.cpp
 * 
 * crypto scheme class implementation
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

#include <sstream>

#include <boost/algorithm/string.hpp>

// ait
#include <qkd/common_macros.h>
#include <qkd/crypto/context.h>
#include <qkd/crypto/engine.h>

#include "crypto_evhash.h"
#include "crypto_null.h"
#include "crypto_xor.h"

using namespace qkd::crypto;


// ------------------------------------------------------------
// code


/**
 * ctor
 * 
 * @param   sScheme         the crypto scheme string
 */
scheme::scheme(std::string const sScheme) {
    
    // clear output
    m_sName = "";
    m_cInitKey = qkd::key::key();
    m_cState = qkd::utility::memory();
    
    // get the tokens
    std::vector<std::string> sTokenScheme;
    boost::split(sTokenScheme, sScheme, boost::is_any_of(":"));
    if (sTokenScheme.empty()) return;
    
    // parse the first token --> algorithm
    std::vector<std::string> sTokenAlgorithm;
    boost::split(sTokenAlgorithm, sTokenScheme[0], boost::is_any_of("-"));
    if (sTokenAlgorithm.empty()) return;
    m_sName = sTokenAlgorithm[0];
    
    // get the init key
    if (sTokenScheme.size() >= 2) {
        m_cInitKey.data() = qkd::utility::memory::from_hex(sTokenScheme[1]);
    }
    
    // test the variant
    if (sTokenAlgorithm.size() >= 2) {
        
        // get the length
        uint32_t nBytes = std::atoi(sTokenAlgorithm[1].c_str()) / 8;
        
        // already an init key?
        if (m_cInitKey.data().size() != 0) {
            if (m_cInitKey.data().size() != nBytes) {
                
                // variant differs from init key!
                m_sName = "";
                m_cInitKey = qkd::key::key();
                m_cState = qkd::utility::memory();
                return;
            }
        }
        else {
            m_cInitKey.data() = qkd::utility::memory(nBytes);
            m_cInitKey.data().fill(0);
        }
    }
    
    // get the state
    if (sTokenScheme.size() >= 3) {
        m_cState = qkd::utility::memory::from_hex(sTokenScheme[2]);
    }
}


/**
 * return a stringified version of this scheme
 * 
 * @return  a string which can be used to create this scheme again
 */
std::string scheme::str() const {
    
    std::stringstream ss;
    
    ss << m_sName;
    if (m_cInitKey.size()) {
        ss << "-" << m_cInitKey.size() * 8 << ":" << m_cInitKey.data().as_hex();
        if (m_cState.size()) {
            ss << ":" << m_cState.as_hex();
        }
    }
    
    return ss.str();
}


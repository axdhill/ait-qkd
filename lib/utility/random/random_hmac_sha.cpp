/*
 * random_hmac_sha.cpp
 * 
 * implement the random HMAC-SHA source
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

#include <chrono>

#include <boost/algorithm/string.hpp>

// ait
#include <qkd/utility/buffer.h>
#include <qkd/utility/environment.h>
#include <qkd/utility/random.h>

#include "random_hmac_sha.h"

using namespace qkd::utility;


// ------------------------------------------------------------
// code


/**
 * describe the random source
 * 
 * @return  a HR-string describing the random source
 */
std::string random_hmac_sha::describe() const {
    return std::string("random source using ") + m_sHMACSHA;
}


/**
 * get a block of random bytes
 * 
 * This function must be overwritten in derived classes
 * 
 * @param   cBuffer     buffer which will hold the bytes
 * @param   nSize       size of buffer in bytes
 */
void random_hmac_sha::get(char * cBuffer, uint64_t nSize) {

    // do not proceed if nothing to do
    if (!cBuffer) return;
    if (nSize == 0) return;
    
    // create a base input: "plaintext"
    qkd::utility::buffer cPlaintext;
    cPlaintext << qkd::utility::environment::process_id();
    cPlaintext << (uint64_t)(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())).count();
    
    // digest result
    unsigned char cDigest[EVP_MAX_MD_SIZE];
    unsigned int nDigestLen = 0;
    
    uint64_t nRead = 0;
    while (nRead < nSize) {
        
        // increase "plaintext"
        m_nCounter++;
        cPlaintext << m_nCounter;
        
        // re-init
        EVP_DigestInit_ex(&m_cMessageDigestContext, m_cMessageDigestAlgorithm, nullptr);
        EVP_DigestUpdate(&m_cMessageDigestContext, cPlaintext.get(), cPlaintext.size());
        
        nDigestLen = 0;
        EVP_DigestFinal_ex(&m_cMessageDigestContext, cDigest, &nDigestLen);
        memcpy(cBuffer + nRead, cDigest, std::min(nSize - nRead, (uint64_t)nDigestLen));
        
        nRead += std::min(nSize - nRead, (uint64_t)nDigestLen);
    }
}


/**
 * init the object
 */
void random_hmac_sha::init() {
    
    if (m_sHMACSHA.substr(0, std::string("hmac-sha").length()) != "hmac-sha") throw random_init_error();
    
    // get the tokens
    std::vector<std::string> sTokenScheme;
    boost::split(sTokenScheme, m_sHMACSHA, boost::is_any_of(":"));
    if (sTokenScheme.size() != 2) throw random_init_error();
    
    // parse the second token --> key
    qkd::utility::memory cKey = qkd::utility::memory::from_hex(sTokenScheme[1]);
    
    EVP_MD_CTX_init(&m_cMessageDigestContext);
    
    // select on key size
    switch (cKey.size()) {
        
    case 256 / 8:
        m_cMessageDigestAlgorithm = EVP_sha256();
        m_sHMACSHA = "hmac-sha-256";
        break;
        
    case 384 / 8:
        m_cMessageDigestAlgorithm = EVP_sha384();
        m_sHMACSHA = "hmac-sha-384";
        break;
        
    case 512 / 8:
        m_cMessageDigestAlgorithm = EVP_sha512();
        m_sHMACSHA = "hmac-sha-512";
        break;

    default:
        // unkown key size ==> unknown hmac-sha algorithm
        throw random_init_error();
    }
}

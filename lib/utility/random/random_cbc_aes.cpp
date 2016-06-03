/*
 * random_cbc_aes.cpp
 * 
 * implement the random CBC-AES source
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

#include <chrono>

#include <boost/algorithm/string.hpp>

// ait
#include <qkd/utility/buffer.h>
#include <qkd/utility/environment.h>
#include <qkd/utility/random.h>

#include "random_cbc_aes.h"


using namespace qkd::utility;


// ------------------------------------------------------------
// code


/**
 * describe the random source
 * 
 * @return  a HR-string describing the random source
 */
std::string random_cbc_aes::describe() const {
    return std::string("random source using ") + m_sCBCAES;
}


/**
 * get a block of random bytes
 * 
 * This function must be overwritten in derived classes
 * 
 * @param   cBuffer     buffer which will hold the bytes
 * @param   nSize       size of buffer in bytes
 */
void random_cbc_aes::get(char * cBuffer, uint64_t nSize) {
    
    // do not proceed if nothing to do
    if (!cBuffer) return;
    if (nSize == 0) return;
    
    // create enough "plaintext": concentrate the increasing counter with pid and current time stamp in ms
    qkd::utility::buffer cPlaintext;
    cPlaintext << m_nCounter++;
    cPlaintext << qkd::utility::environment::process_id();
    cPlaintext << (uint64_t)(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())).count();
    while (cPlaintext.size() < nSize) cPlaintext << m_nCounter++;

    // run CBC-AES
    qkd::utility::memory cCipher(cPlaintext.size());
    int nEncrypted = 0;
    EVP_EncryptInit_ex(&m_cCipherContext, nullptr, nullptr, nullptr, nullptr);
    EVP_EncryptUpdate(&m_cCipherContext, cCipher.get(), &nEncrypted, cPlaintext.get(), cCipher.size());
    
    // copy final result
    memcpy(cBuffer, cCipher.get(), nSize);
}


/**
 * init the object
 */
void random_cbc_aes::init() {
    
    if (m_sCBCAES.substr(0, std::string("cbc-aes").length()) != "cbc-aes") {
        throw qkd::exception::randomengine_error("wrong url syntax on init of cbc-aes random engine");
    }
    
    // get the tokens
    std::vector<std::string> sTokenScheme;
    boost::split(sTokenScheme, m_sCBCAES, boost::is_any_of(":"));
    if (sTokenScheme.size() != 2) {
        throw qkd::exception::randomengine_error("invalid url syntax for cbc-aes random engine scheme");
    }
    
    // parse the second token --> key
    qkd::utility::memory cKey = qkd::utility::memory::from_hex(sTokenScheme[1]);
    
    // create an init vector
    qkd::utility::buffer cIV;
    cIV << qkd::utility::environment::process_id();
    cIV << (uint64_t)(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())).count();
    
    EVP_CIPHER_CTX_init(&m_cCipherContext);
    
    // select on key size
    switch (cKey.size()) {
        
    case 128 / 8:
        EVP_EncryptInit(&m_cCipherContext, EVP_aes_128_cbc(), cKey.get(), cIV.get());
        m_sCBCAES = "cbc-aes-128";
        break;
        
    case 192 / 8:
        EVP_EncryptInit(&m_cCipherContext, EVP_aes_192_cbc(), cKey.get(), cIV.get());
        m_sCBCAES = "cbc-aes-192";
        break;
        
    case 256 / 8:
        EVP_EncryptInit(&m_cCipherContext, EVP_aes_256_cbc(), cKey.get(), cIV.get());
        m_sCBCAES = "cbc-aes-256";
        break;

    default:
        throw qkd::exception::randomengine_error("unknown key size yields unknown cbc-aes algorithm");
    }
}

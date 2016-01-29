/*
 * qauth.cpp
 * 
 * Implements the QAuth protocol parts as depicted at
 *
 *      http://www.iaria.org/conferences2015/awardsICQNM15/icqnm2015_a3.pdf
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

#include <qkd/crypto/context.h>
#include <qkd/crypto/engine.h>

    #include <qkd/utility/debug.h>

#include <qkd/utility/memory.h>

#include "qauth.h"


// ------------------------------------------------------------
// incs


/**
 * qauth-pimpl
 * 
 * this holds the H_kv and H_kp hash functions as well as the current v and p values
 * 
 * Note: we use evhash-32 here as our universal hashing. In order to mitigate any memory
 *       effects of the hash function, I (Oliver) create the hash function everytime anew. 
 *       This might not be necessary though.
 */
class qauth::qauth_data {
    
public:

    
    /**
     * ctor
     * 
     * @param   cQAuthInit      init values of qauth
     * @param   nModulus        m
     */
    qauth_data(qauth_init const & cQAuthInit, uint32_t nModulus) : m_nModulus(nModulus), m_cQAuthInit(cQAuthInit) {
        
        m_cCurrent.nPosition = cQAuthInit.nPosition0;
        m_cCurrent.nValue = cQAuthInit.nValue0;
                
//         std::string sHash_Kv_init = "evhash-32:" + qkd::utility::memory::wrap(reinterpret_cast<unsigned char *>(&cQAuthInit.nKv), sizeof(cQAuthInit.nKv)).as_hex();
// qkd::utility::debug() << __DEBUG_LOCATION__ << "sHash_Kv_init=" << sHash_Kv_init;        
//         m_cHash_Kv = qkd::crypto::engine::create(sHash_Kv_init);
//         
//         std::string sHash_Kp_init = "evhash-32:" + qkd::utility::memory::wrap(reinterpret_cast<unsigned char *>(&cQAuthInit.nKp), sizeof(cQAuthInit.nKp)).as_hex();
// qkd::utility::debug() << __DEBUG_LOCATION__ << "sHash_Kp_init=" << sHash_Kp_init;        
//         m_cHash_Kp = qkd::crypto::engine::create(sHash_Kp_init);
    }
    
    
    /**
     * make an iteration
     */
    void next() {
        
//         qkd::utility::memory m;
//         
//         // v_n+1 = H_kv(v_n)
//         m = qkd::utility::memory::wrap(reinterpret_cast<unsigned char *>(m_nValue), sizeof(m_nValue));
//         m_cHash_Kv << m;
//         m_cHash_Kv >> m;
//         
//         // p_n+1 = p_n + (1 + (H_kp(p_n) mod m))
//         uint64_t p;
//         m = qkd::utility::memory::wrap(reinterpret_cast<unsigned char *>(m_nValue), sizeof(m_nValue));
//         m_cHash_Kp << m_nPosition;
//         m_cHash_Kp >> p;
//         m_nPosition = m_nPosition + (1 + (p % m_nModulus));
    }
    
    
    
    /**
     * current qauth values
     */
    qauth_data_particle m_cCurrent;
    
    
    /**
     * the universial hash H_kp
     */
    qkd::crypto::crypto_context m_cHash_Kp;
    
    
    /**
     * the universial hash H_kv
     */
    qkd::crypto::crypto_context m_cHash_Kv;
    
    
    /**
     * the modules m
     */
    uint32_t m_nModulus;
    
    
    /**
     * the init values
     */
    qauth_init m_cQAuthInit;
};


// ------------------------------------------------------------
// code


/**
 * ctor
 * 
 * @param   cQAuthInit      init values of qauth
 * @param   nModulus        m
 */
qauth::qauth(qauth_init const & cQAuthInit, uint32_t nModulus) {
    d = std::shared_ptr<qauth::qauth_data>(new qauth::qauth_data(cQAuthInit, nModulus));
}


/**
 * dtor
 */
qauth::~qauth() {
}


/**
 * return the next qauth_data particle
 * 
 * @return  the next in qauth data in the series
 */
qauth_data_particle qauth::next() {
    qauth_data_particle res = d->m_cCurrent;
    d->next();
    return res;
}

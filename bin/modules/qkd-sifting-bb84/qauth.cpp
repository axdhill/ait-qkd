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

#include <sstream>

#include <qkd/crypto/context.h>
#include <qkd/crypto/engine.h>

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
 * 
 *       Using evhash-32 from the qkd/crypto lib is sure a wired hack here... =(
 */
class qauth::qauth_data {
    
public:

    
    /**
     * ctor
     * 
     * @param   cQAuthInit      init values of qauth
     */
    qauth_data(qauth_init const & cQAuthInit) : m_cQAuthInit(cQAuthInit) {
        
        m_cCurrent.nPosition = cQAuthInit.nPosition0;
        m_cCurrent.nValue = cQAuthInit.nValue0;

        std::string sHash_Kv_init = "evhash-32:" + qkd::utility::memory::wrap(reinterpret_cast<unsigned char *>(&m_cQAuthInit.nKv), sizeof(m_cQAuthInit.nKv)).as_hex();
        m_cHash_Kv = qkd::crypto::engine::create(qkd::crypto::scheme(sHash_Kv_init));
        
        std::string sHash_Kp_init = "evhash-32:" + qkd::utility::memory::wrap(reinterpret_cast<unsigned char *>(&m_cQAuthInit.nKp), sizeof(m_cQAuthInit.nKp)).as_hex();
        m_cHash_Kp = qkd::crypto::engine::create(qkd::crypto::scheme(sHash_Kp_init));
    }
    
    
    /**
     * perform a h_kp hash
     * 
     * @param   p       the value to hash
     * @return  h_kp(p)
     */
    uint64_t hash_kp(uint64_t p) {
        
        m_cHash_Kp << qkd::utility::memory::wrap(reinterpret_cast<unsigned char *>(&p), sizeof(p));
        qkd::utility::memory m = m_cHash_Kp->tag();
        
        return *(reinterpret_cast<uint64_t *>(m.get()));
    }
    
    
    /**
     * perform a h_kv hash
     * 
     * @param   v       the value to hash
     * @return  h_kv(v)
     */
    uint32_t hash_kv(uint32_t v) {
        
        m_cHash_Kp << qkd::utility::memory::wrap(reinterpret_cast<unsigned char *>(&v), sizeof(v));
        qkd::utility::memory m = m_cHash_Kp->tag();
        
        return *(reinterpret_cast<uint32_t *>(m.get()));
    }
    
    
    /**
     * make an iteration
     */
    void next() {
        
        // v_n+1 = H_kv(v_n)
        m_cCurrent.nValue = hash_kv(m_cCurrent.nValue);
        
        // p_n+1 = p_n + (1 + (H_kp(p_n) mod m))
        m_cCurrent.nPosition = m_cCurrent.nPosition + (1 + (hash_kp(m_cCurrent.nPosition) % m_cQAuthInit.nModulus));
    }
    
    
    /**
     * current qauth value
     */
    qauth_value m_cCurrent;
    
    
    /**
     * the universial hash H_kp
     */
    qkd::crypto::crypto_context m_cHash_Kp;
    
    
    /**
     * the universial hash H_kv
     */
    qkd::crypto::crypto_context m_cHash_Kv;
    
    
    /**
     * the init values
     */
    qauth_init m_cQAuthInit;
};


// ------------------------------------------------------------
// code


/**
 * dump value hr-readable into a stream
 * 
 * @param   cStream     the stream to dump to
 */
void qauth_value::dump(std::ostream & cStream) const {
    cStream << "<" << nPosition << ", " << nValue << ">";
}


/**
 * dump to a string
 * 
 * @return  a string containing the values
 */
std::string qauth_value::str() const {
    std::stringstream ss;
    dump(ss);
    return ss.str();
}


/**
 * == equality operator
 * 
 * deep check for each element of the values
 * 
 * @param   rhs         right hand side
 * @return  true, if each element of this is matched in rhs in the correct order
 */
bool qauth_values::operator==(qauth_values const & rhs) const {
    
    if (size() != rhs.size()) return false;
    auto li = cbegin();
    auto ri = rhs.cbegin();
    for (; li != cend(); ++li, ++ri) {
        if (((*li).nPosition != (*ri).nPosition) || ((*li).nValue != (*ri).nValue)) {
            return false;
        }
    }
    
    return true;
}


/**
 * dump the qauth particle list to a stream
 * 
 * @param   cStream     the stream to dump to
 * @param   sIndent     the indent on each line
 * @param   cList       the qauth particle list
 */
void qauth_values::dump(std::ostream & cStream, std::string const sIndent) const {
    
    bool bFirst = true;
    for (auto iter = cbegin(); iter != cend(); ++iter) {
        
        if (!bFirst) {
            cStream << ", ";
        }
        else {
            cStream << sIndent;
            bFirst = false;
        }
        (*iter).dump(cStream);
    }
}


/**
 * dump to a string
 * 
 * @param   sIndent     the indent on each line
 * @return  a string containing the values
 */
std::string qauth_values::str(std::string const sIndent) const {
    std::stringstream ss;
    dump(ss, sIndent);
    return ss.str();
}


/**
 * ctor
 * 
 * @param   cQAuthInit      init values of qauth
 */
qauth::qauth(qauth_init const & cQAuthInit) {
    d = std::shared_ptr<qauth::qauth_data>(new qauth::qauth_data(cQAuthInit));
}


/**
 * dtor
 */
qauth::~qauth() {
}


/**
 * create a series of data particles starting at position0
 * 
 * the amount of particles created will be such
 * that the hightest position value will be within
 * the set of elements of size nSize with the returned
 * list of data paticles.
 * 
 * That is
 * 
 *      l = create_max(m) ==> l.last().position < nSize
 * 
 * @param   nSize           size of container with mixed data particles within
 * @return  container with qauth data values
 */
qauth_values qauth::create_max(uint64_t nSize) {
    
    qauth_values res;
    for (;;) {
        
        qauth_value p = next();
        if (p.nPosition > nSize) break;
        p.nValue = (p.nValue % 2) ? (uint32_t)bb84_base::BB84_BASE_DIAGONAL : (uint32_t)bb84_base::BB84_BASE_RECTILINEAR;
        res.push_back(p);
    }

    return res;
}


/**
 * create a series of data particles starting at position0
 * 
 * the amount of particles created will be such
 * that the hightest position value will be within
 * the merged set of elements of size nSize with the returned
 * list of data paticles.
 * 
 * That is
 * 
 *      l = create_min(nSize) ==> l.last().position < (nSize + l.size())
 * 
 * @param   nSize           size of container to mix data particles into
 * @return  container with qauth data values
 */
qauth_values qauth::create_min(uint64_t nSize) {
    
    qauth_values res;
    for (;;) {
        
        qauth_value p = next();
        if (p.nPosition > (res.size() + nSize)) break;
        p.nValue = (p.nValue % 2) ? (uint32_t)bb84_base::BB84_BASE_DIAGONAL : (uint32_t)bb84_base::BB84_BASE_RECTILINEAR;
        res.push_back(p);
    }

    return res;
}


/**
 * return the next qauth_data particle
 * 
 * @return  the next in qauth data in the series
 */
qauth_value qauth::next() {
    qauth_value res = d->m_cCurrent;
    d->next();
    return res;
}


/**
 * stream into a memory
 * 
 * @param   lhs     left hand side
 * @param   rhs     right hand side
 * @return  memory object holding rhs
 */
qkd::utility::buffer & operator<<(qkd::utility::buffer & lhs, qauth_init const & rhs) {
    lhs << rhs.nKv;
    lhs << rhs.nKp;
    lhs << rhs.nModulus;
    lhs << rhs.nValue0;
    lhs << rhs.nPosition0;
    return lhs;
}


/**
 * stream out from memory
 * 
 * @param   lhs     left hand side
 * @param   rhs     right hand side
 * @return  memory object fromt which rhs has been retrieved
 */
qkd::utility::buffer & operator>>(qkd::utility::buffer & lhs, qauth_init & rhs) {
    lhs >> rhs.nKv;
    lhs >> rhs.nKp;
    lhs >> rhs.nModulus;
    lhs >> rhs.nValue0;
    lhs >> rhs.nPosition0;
    return lhs;
}

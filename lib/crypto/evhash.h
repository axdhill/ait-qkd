/*
 * evhash.h
 * 
 * implement the evaluation hash 
 *
 * Author: Thomas Themel - thomas.themel@ait.ac.at
 *         Oliver Maurhart, <oliver.maurhart@ait.ac.at>
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


#ifndef __QKD_CRYPTO_EVHASH_H_
#define __QKD_CRYPTO_EVHASH_H_


// ------------------------------------------------------------
// incs

#include <qkd/common_macros.h>
#include <qkd/key/key.h>
#include <qkd/utility/memory.h>

#include "gf2_fast_alpha.h"



// ------------------------------------------------------------
// decl


namespace qkd {
namespace crypto {


/**
 * class to deal with the template instances
 */
class evhash_abstract {


public:


    /**
     * dtor
     */
    virtual ~evhash_abstract() {};


    /**
     * add a memory BLOB to the algorithm
     *
     * @param   cMemory         memory block to be added
     */
    virtual void add(qkd::utility::memory const & cMemory) = 0; 


    /**
     * bit width of GF2
     *
     * @return  bit width of GF2
     */
    virtual unsigned int bits() const = 0;


    /**
     * get the current tag
     *
     * @return  the current tag
     */
    virtual qkd::utility::memory tag() const = 0;

};


/**
 * this class combines a modulus and a key with a certain GF2
 */
template <unsigned int GF_BITS> class evhash : public evhash_abstract {


public:


    /**
     * ctor
     */
    explicit evhash(qkd::key::key const & cKey) {
       
        unsigned int nModulus = 0;
        bool bTwoStepPrecalculation = false;

        switch (GF_BITS) {

        case 32:
            
            // GF(2^32) as GF(2)[x] mod x^32 + x^7 + x^3 + x^2 + 1
            // field element congruent with irreducible polynomial: 141 
            nModulus = 0x81;
            bTwoStepPrecalculation = false;
            break;

        case 64:
            
            // GF(2^64) as GF(2)[x] mod x^64 + x^4 + x^3 + x + 1
            // field element congruent with irreducible polynomial: 27 
            nModulus = 0x1b;
            bTwoStepPrecalculation = false;
            break;

        case 96:
            
            // GF(2^96) as GF(2)[x] mod x^96 + x^10 + x^9 + x^6 + 1
            // field element congruent with irreducible polynomial: 1601
            nModulus = 0x641;
            bTwoStepPrecalculation = false;
            break;

        case 128:
            
            // GF(2^128) as GF(2)[x] mod x^128 + x^7 + x^2 + x + 1
            // field element congruent with irreducible polynomial: 135
            nModulus = 0x87;
            bTwoStepPrecalculation = true;
            break;

        case 256:
            
            // GF(2^256) as GF(2)[x] mod x^128 + x^10 + x^5 + x^2 + 1
            // field element congruent with irreducible polynomial: 1061
            nModulus = 0x425;
            bTwoStepPrecalculation = true;
            break;

        default:

            throw std::invalid_argument("evaluation hash bit width not implemented.");

        }

        m_cGF2 = new gf2_fast_alpha<GF_BITS>(nModulus, bTwoStepPrecalculation, cKey.data());
        m_cGF2->blob_set_value(m_cTag, 0);
    }


    /**
     * dtor
     */
    virtual ~evhash() {
        delete m_cGF2;
    }


    /**
     * add a memory BLOB to the algorithm
     *
     * @param   cMemory         memory block to be added
     */
    void add(UNUSED qkd::utility::memory const & cMemory) {}


    /**
     * bit width of GF2
     *
     * @return  bit width of GF2
     */
    unsigned int bits() const { return GF_BITS; };


    /**
     * get the current tag
     *
     * @return  the current tag
     */
    qkd::utility::memory tag() const { return m_cGF2->blob_to_memory(m_cTag); };


private:


    /**
     * the GF2 to work on
     */
    gf2_fast_alpha<GF_BITS> * m_cGF2;


    /**
     * the current tag
     */
    typename gf2_fast_alpha<GF_BITS>::blob_t m_cTag;

};

}

}


#endif


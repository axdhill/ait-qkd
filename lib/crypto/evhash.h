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

#include <qkd/key/key.h>
#include <qkd/utility/buffer.h>
#include <qkd/utility/memory.h>

#include "gf2.h"


    #include <qkd/utility/debug.h>
    #include <qkd/utility/memory.h>

    #undef DUMP_BLOB
    #define DUMP_BLOB(b, s) qkd::utility::debug() << s << qkd::utility::memory::wrap((unsigned char *)b, m_cGF2->BLOB_BYTES).as_hex();


// ------------------------------------------------------------
// decl


namespace qkd {
namespace crypto {


/**
 * interface to the GF2 instances with necessary 
 * crypto methods to talk to gf2 in a uniform way 
 * regardless of current GF_BITS size
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
     * This transforms the tag stored in this object
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
     * number of blocks added so far
     *
     * @return  number of blocks in the tag
     */
    virtual uint64_t blocks() const = 0;


    /**
     * size of a single block
     *
     * @return  size of a single block in bytes
     */
    virtual unsigned int block_size() const = 0;


    /**
     * get the final tag
     *
     * @return  the final tag
     */
    virtual qkd::utility::memory finalize() = 0;


    /**
     * set the current state
     *
     * @param   cState      the state serialized
     */
    virtual void set_state(qkd::utility::buffer & cState) = 0;


    /**
     * get the current state
     *
     * @return  serialized current state
     */
    virtual qkd::utility::buffer state() const = 0;


    /**
     * get the current tag
     *
     * @return  the current tag
     */
    virtual qkd::utility::memory tag() const = 0;

};


/**
 * this class combines a modulus and a key with a certain GF2 plus interface methods to use it neatly
 */
template <unsigned int GF_BITS> class evhash : public evhash_abstract {


public:


    /**
     * ctor
     */
    explicit evhash(qkd::key::key const & cKey) : m_nBlocks(0), m_cRemainder(nullptr), m_nRemainderBytes(0) {
       
        unsigned int nModulus = 0;
        bool bTwoStepPrecalculation = false;

        switch (GF_BITS) {

        case 32:
            
            // GF(2^32) as GF(2)[x] mod x^32 + x^7 + x^3 + x^2 + 1
            // field element congruent with irreducible polynomial: 141 
            nModulus = 0x8d;
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

        // create the GF2 implementation with bit width, modulus and precalulcation tables
        m_cGF2 = new gf2_fast_alpha<GF_BITS>(nModulus, bTwoStepPrecalculation, cKey.data());
        m_cGF2->blob_set_value(m_cTag, 0);

        m_cRemainder = new char[block_size()];
        m_nRemainderBytes = 0;
    }


    /**
     * dtor
     */
    virtual ~evhash() {
        delete m_cGF2;
        delete [] m_cRemainder;
    }


    /**
     * add a memory BLOB to the algorithm
     *
     * This transforms the tag stored in this object
     *
     * @param   cMemory         memory block to be added
     */
    void add(qkd::utility::memory const & cMemory) {

        // we add pieces of blob_t to the GF2 field
        // when the memory exceeds a multiple of sizeof(blob_t)
        // the remainder is stored and added to the front
        // on the next run

        // --- the hashing ---
        // Horner Rule: tag_n = (tag_(n-1) + m) * k

        char * data = (char *)cMemory.get();
        int64_t nLeft = cMemory.size();


        // first block might have remainding bytes of the previous block prepended
        if (m_nRemainderBytes) {

            memcpy(m_cRemainder + m_nRemainderBytes, cMemory.get(), block_size() - m_nRemainderBytes);

            // first round with remainder
            typename gf2_fast_alpha<GF_BITS>::blob_t coefficient;
            m_cGF2->blob_from_memory(coefficient, m_cRemainder);
            m_cGF2->add(m_cTag, coefficient, m_cTag);
            m_cGF2->times_alpha(m_cTag, m_cTag);

            m_nBlocks++;
            data += block_size() - m_nRemainderBytes;
            nLeft -= block_size() - m_nRemainderBytes;
            m_nRemainderBytes = 0;
        }

        // walk over all blocks
        while (nLeft - block_size() >= 0) {

            typename gf2_fast_alpha<GF_BITS>::blob_t coefficient;
            m_cGF2->blob_from_memory(coefficient, data);
            m_cGF2->add(m_cTag, coefficient, m_cTag);
            m_cGF2->times_alpha(m_cTag, m_cTag);

            m_nBlocks++;
            data += block_size();
            nLeft -= block_size();
        }

        // remember the last block % block_size
        if (nLeft) {
            memcpy(m_cRemainder, data, nLeft);
            m_nRemainderBytes = nLeft;
        }
    }


    /**
     * bit width of GF2
     *
     * @return  bit width of GF2
     */
    inline unsigned int bits() const { return GF_BITS; }


    /**
     * number of blocks added so far
     *
     * @return  number of blocks in the tag
     */
    inline uint64_t blocks() const { return m_nBlocks; }


    /**
     * size of a single block
     *
     * @return  size of a single block in bytes
     */
    constexpr unsigned int block_size() { return GF_BITS / 8; }


    /**
     * get the final tag
     *
     * @return  the final tag
     */
    qkd::utility::memory finalize() { 

        // add the remainder (bytes not yet authenticated)
        if (m_nRemainderBytes > 0) {
            memset(m_cRemainder + m_nRemainderBytes, 0, block_size() - m_nRemainderBytes);
            m_nRemainderBytes = 0;
            add(qkd::utility::memory::wrap((qkd::utility::memory::value_t *)m_cRemainder, block_size()));
        }

        return tag(); 
    }


    /**
     * set the current state
     *
     * @param   cState      the state serialized
     */
    void set_state(qkd::utility::buffer & cState) {
        qkd::utility::memory m;
        cState >> m;
        m_cGF2->blob_from_memory(m_cTag, m);
        cState >> m;
        memcpy(m_cRemainder, m.get(), m.size());
        m_nRemainderBytes = m.size();
        cState >> m_nBlocks;
    }


    /**
     * get the current state
     *
     * @return  serialized current state
     */
    qkd::utility::buffer state() const {

        qkd::utility::buffer res;
        res << m_cGF2->blob_to_memory(m_cTag);
        res << qkd::utility::memory::wrap((qkd::utility::memory::value_t *)m_cRemainder, m_nRemainderBytes);
        res << m_nBlocks;
        return res;
    }


    /**
     * get the current tag
     *
     * @return  the current tag
     */
    qkd::utility::memory tag() const { return m_cGF2->blob_to_memory(m_cTag); }


private:


    /**
     * the GF2 to work on
     */
    gf2_fast_alpha<GF_BITS> * m_cGF2;


    /**
     * blocks done so far
     */
    uint64_t m_nBlocks;


    /**
     * remainder of last add (modulu blob size) 
     */
    char * m_cRemainder;


    /**
     * remainder of last add (modulu blob size) in bytes
     */
    unsigned int m_nRemainderBytes;


    /**
     * the current tag
     */
    typename gf2_fast_alpha<GF_BITS>::blob_t m_cTag;


};


}

}


#endif


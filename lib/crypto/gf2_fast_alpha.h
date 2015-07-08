/*
 * gf2_fast_alpha.h
 * 
 * Galois Field 2^n with optimizations for fast multiply of key alpha
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

 
#ifndef __QKD_CRYPTO_GF2_FAST_ALPHA_H_
#define __QKD_CRYPTO_GF2_FAST_ALPHA_H_


// ------------------------------------------------------------
// incs

#include "gf2.h"


// ------------------------------------------------------------
// decl


namespace qkd {

namespace crypto {


/**
 * this class represents a Galois Field 2 with optimizations to multiply a key alpha fast
 */
template <unsigned int GF_BITS> class gf2_fast_alpha : public gf2<GF_BITS> {


public:

    
    // make template depedend names explicit

    using typename gf2<GF_BITS>::blob_t;
    using typename gf2<GF_BITS>::word_t;
    
    using gf2<GF_BITS>::BLOB_INTS;
    using gf2<GF_BITS>::BLOB_BITS;
    using gf2<GF_BITS>::WORD_BITS;

    using gf2<GF_BITS>::blob_from_memory;
    using gf2<GF_BITS>::modulus;


    /**
     * ctor
     *
     * @param   nModules                    signature of the irreducible polynom
     * @param   bTwoStepPrecalculation      do a single or double precalculation table
     * @param   cKey                        the value for which fast multiplication is achieved
     */
    explicit gf2_fast_alpha(unsigned int nModulus, bool bTwoStepPrecalculation, qkd::utility::memory const & cKey) 
            : gf2<GF_BITS>(nModulus), bTwoStepPrecalculation(bTwoStepPrecalculation) {

        alpha = blob_from_memory(cKey);
        this->precalc_blob_multiplication();
    };



    /** 
     * Fast multiplication of a blob with alpha.
     *
     * @param   blob        the blob to multiply
     * @return  blob * alpha in this GF2
     */
    blob_t times_alpha(blob_t blob) const {

        blob_t res; 
        res.fill(0);

        word_t b[BLOB_BITS / HORNER_BITS]; 
        unsigned int k;

        k = 0 ;
        for (unsigned int i = 0; i < BLOB_INTS ; ++i) {

            word_t word = blob[i] ; 

            // Split word into the correct number 
            // of PRECALC_BITS sized chunks 
            for (int j = WORD_BITS / HORNER_BITS - 1; j >= 0; --j) {
                b[k++] = (word >> (HORNER_BITS * j)) & (HORNER_SIZE - 1);  
            }
        }

        for (unsigned int i = 0 ; i < BLOB_BITS / HORNER_BITS ; ++i) {

            assert(b[i]<HORNER_SIZE);

            precalc_shift(res);
            if (bTwoStepPrecalculation) {
                size_t index_v1 = b[i] >> PRECALC_BITS ;
                res = add(res, multiplication_table_2[index_v1]);
                b[i] &= PRECALC_SIZE - 1; 
            }

            res = add(res, multiplication_table[b[i]]);
        }      

        return res;
    };


protected:



    /**
     * Performs fast left-shifting of overflow-free blobs  by HORNER_BITS bits. 
     *
     * @param   blob        blob to shift
     * @return blob << HORNER_BITS
     */
    blob_t precalc_shift(blob_t blob) const {    

        blob_t res;
        res.fill(0);

        blob_t overflow_blob;
        res = blob_shift_left(blob, overflow_blob, HORNER_BITS);

        // get the lowest order (and only non-zero) 
        // overflow word 
        word_t overflow = overflow_blob[BLOB_INTS - 1] ;

        if (overflow != 0) {

            if (bTwoStepPrecalculation) {

                // v(x) = v1(x) * x^8 + v0(x) 
                size_t index_v1 = overflow >> PRECALC_BITS ; 
                assert(index_v1 < PRECALC_SIZE);

                // lookup v1(x) * x^8 * x^BLOB_BITS
                res[BLOB_INTS - 1] ^= overflow_table_2[index_v1];

                // and reduce v(x) to v0(x)
                overflow = overflow & (PRECALC_SIZE - 1);
            }
            else {
                assert(overflow < PRECALC_SIZE);
            }

            // lookup v(x) * x^BLOB_BITS
            res[BLOB_INTS - 1] ^= overflow_table[overflow];
        }

        return res;
    }



private:


    std::size_t HORNER_BITS;
    std::size_t HORNER_SIZE;

    bool bTwoStepPrecalculation;        /**< perform single or double pre calculation */


    /**
     *  multiplication table of alpha 
     */
    blob_t multiplication_table[PRECALC_SIZE];


    /**
     * multiplication table 2 of alpha 
     */
    blob_t multiplication_table_2[PRECALC_SIZE];
    
    
    /**
     * Precalculated mapping of v(x) -> v(x) * x^BLOB_BITS mod f(x) 
     * for deg(v(x)) < PRECALC_BITS 
     */
    unsigned int overflow_table[PRECALC_SIZE];


    /**
     * Precalculated mapping of v(x) -> v(x) * x^PRECALC_BITS * x^BLOB_BITS mod f(x)
     * for deg(v(x)) < PRECALC_BITS 
     */
    unsigned int overflow_table_2[PRECALC_SIZE];


    /**
     * the value for which we do fast multiply
     */
    blob_t alpha;


    /**
     * setup the precalulation tables
     */
    void precalc_blob_multiplication() {

        if (bTwoStepPrecalculation) {
            HORNER_BITS = 2 * PRECALC_BITS;
        }
        else {
            HORNER_BITS = PRECALC_BITS;
        }
        HORNER_SIZE = (1 << HORNER_BITS);

        setup_overflow_table();

        blob_t v ;
        for (int i = 0 ; i < PRECALC_SIZE; ++i) {

            blob_set_value(v, i);
            multiplication_table[i] = gf2_mul(alpha, v);
            if (bTwoStepPrecalculation) {

                blob_set_value(v, i << PRECALC_BITS);
                multiplication_table_2[i] = gf2_mul(alpha, v);
            }
        }
    };


    /** 
     * Initializes the field's overflow table
     */
    void setup_overflow_table() {

        blob_t tmp; 

        for(int i = 0 ; i < PRECALC_SIZE ; ++i) {

            blob_set_value(tmp, i);

            // MAXVALUE is identical to MODULUS_EQUIV in GF(2^N) 
            tmp = mul(tmp, modulus());
            overflow_table[i] = tmp[BLOB_INTS - 1];

            if (bTwoStepPrecalculation) {
                tmp.fill(i << PRECALC_BITS);
                tmp = mul(tmp, modulus());
                overflow_table_2[i] = tmp[BLOB_INTS - 1];
            }
        }
    };

};

}

}


#endif


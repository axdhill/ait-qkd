/*
 * gf2.h
 * 
 * Galois Field 2^n
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

 
#ifndef __QKD_CRYPTO_GF2_H_
#define __QKD_CRYPTO_GF2_H_


// ------------------------------------------------------------
// incs

#include <arpa/inet.h>

#include <boost/shared_ptr.hpp>

#include <qkd/utility/memory.h>


// ------------------------------------------------------------
// defs


/* 
 * Precalculation parameters - PRECALC_BITS must be a 
 * multiple of 8 and a divisor of BLOB_BITS - which probably
 * makes the only practical values 8 and 16 in 2006. 
 * PRECALC_BITS MUST BE <= WORD_BITS. If TWO_STEP_PRECALC
 * is active, 2*PRECALC_BITS must be a divisor of BLOB_BITS
 * as well. 
 */
#define PRECALC_BITS 8
#define PRECALC_SIZE (1 << PRECALC_BITS)


// ------------------------------------------------------------
// decl


namespace qkd {

namespace crypto {


/**
 * this class represents a Galois Field 2
 *
 * The class creates a Galois Field 2^x with x been the number of bits managed.
 * For beeing useful we need a irreducible polynom this GF2 is based on. This
 * polynom is given as a simple unsigned int value representing the coefficients of
 * the polynom. Note that the modulus omits the most signficant bit, since this is 
 * always set.
 *
 * E.g. 
 *      bits = 8
 *      modulus = 0xA7 ---> x^7 + x^5 + x^2 + x^1 + 1 
 *
 *          ==> x^8 + x^7 + x^5 + x^2 + x^1 + 1
 *              (no check if this here is really a irreducible polynom)
 *
 * addition is a simple XOR operation. However for multiplication we use
 * precalculated tables to speed up processing. This can be done in one step
 * or in two steps.
 */
template <unsigned int GF_BITS> class gf2 {


public:


    /**
     * a word_t is the single atom unit of processing within the GF2
     */
    typedef unsigned int word_t;

    std::size_t const WORD_BYTES = sizeof(word_t);
    std::size_t const WORD_BITS = sizeof(word_t) * 8;
    std::size_t const BLOB_INTS = GF_BITS / WORD_BITS;

    std::size_t BLOB_BYTES;
    std::size_t BLOB_BITS;


    /**
     * a blob_t holds a full value in the current GF2
     * congruent with the irreudcible polynom given in modulus_equiv
     */
    typedef std::array<word_t, GF_BITS / (sizeof(word_t) * 8)> blob_t;


    /**
     * ctor
     *
     * @param   nModules                    signature of the irreducible polynom
     */
    explicit gf2(unsigned int nModulus) {
        this->blob_set_ui(modulus_equiv, nModulus);
        this->setup_gf2();
    };


    /**
     * add two values
     *
     * this is a simple XOR
     *
     * @param   num1        first number
     * @param   num2        second number
     * @return  result of num1 + num2 in GF[2^GF_BITS]
     */
    inline blob_t add(blob_t const & num1, blob_t const & num2) const {
        blob_t res;
        for (unsigned int i = 0; i < BLOB_INTS; ++i) {
            res[i] = num1[i] ^ num2[i];
        }
        return res;
    };


    /**
     * Converts an arbitrary memory block to a blob and stores it in result. 
     *
     * Byte order for the conversion is network byte order, ie the string "01234567" 
     * is converted into the blob 0x3031323334353637, representing the number
     *  55561791730626147895 or the polynomial:
     *   x^61 + x^60 + x^53 + x^52 + x^48 + x^45 + x^44 + x^41 + x^37 + x^36
     *          + x^33 + x^32 + x^29 + x^28 + x^26 + x^21 + x^20 + x^18 + x^16
     *          + x^13 + x^12 + x^10 + x^9 + x^5 + x^4 + x^2 + x^1 + x^0 
     *
     * @param   bytes       the memory to convert
     * @return  a blob to use
     */
    inline blob_t blob_from_memory(char const * bytes) const {

        blob_t res;
        word_t * wordptr = dynamic_cast<word_t*>(bytes); 
        for (unsigned i = 0 ; i < BLOB_INTS ; ++i) {
            res[i] = htonl(*wordptr++);
        }
        return res;
    };


    /**
     * Converts an arbitrary memory block to a blob and stores it in result. 
     *
     * @param   cMemory     the memory to convert
     * @return  a blob to use
     */
    inline blob_t blob_from_memory(qkd::utility::memory const & cMemory) const {
        assert(cMemory.size() >= BLOB_BYTES);
        return blob_from_memory(cMemory.get());
    };


    /**
     * Converts a blob to a memory
     *
     * @param   blob        the blob to convert
     * @return  a memory object holding the blob
     */
    inline qkd::utility::memory blob_to_memory(blob_t const & blob) const {

        qkd::utility::memory res(BLOB_BYTES);
        
        word_t * wordptr = dynamic_cast<word_t*>(res.get()); 
        for (unsigned i = 0 ; i < BLOB_INTS ; ++i) {
            (*wordptr) = ntohl(res[i]);
            wordptr++;
        }
        return res;
    };


    /**
     * get the modulus
     *
     * @return  the modulus for this Galois Field
     */
    blob_t const & modulus() const { return modulus_equiv; };


    /**
     * multiply 
     *
     * @param   num1    first operand
     * @param   num2    second operand
     * @return  result of num1 * num2
     */
    blob_t mul(blob_t const & num1, blob_t const & num2) const {
    
        blob_t res;
        res.fill(0);

        for (int i = BLOB_INTS - 1; i >= 0; --i) {

            word_t word = num2[i];
            if (word != 0) {

                word_t mask;
                int j;
                for (mask = 1 << (WORD_BITS - 1), j = WORD_BITS - 1; mask > 0; mask = mask >> 1, --j) {

                    if (word & mask) {

                        blob_t shifted = shift((BLOB_INTS - 1 - i) * WORD_BITS + j, num1);
                        res = add(res, shifted);
                    }
                }
            }
        }

        return res;
    }


    /**
     * Reduce a value that doesn't fit the field  over the modulus to an equivalent field element. 
     *
     * @param   num         the value to reduce
     * @param   overflow    the value's overflow
     * @return  num % modules
     */
    blob_t reduce(blob_t const & num, blob_t & overflow) const {

        blob_t res = num;
        blob_t reduced_bit ;

        // for each bit in overflow ...
        for (unsigned int i = 0 ; i < BLOB_BITS; ++i) {     

            // ... that is set ...
            if (blob_tstbit(overflow, i)) {

                // ... add the equivalent field element
                reduce_bit(reduced_bit, i + BLOB_BITS);
                res = add(res, reduced_bit);
            }
        }

        return res;

    }


    /**
     * shift left
     *
     * @param   bits        number of steps to shift left
     * @param   num         number to shift left
     * @return  num << nSteps
     */
    blob_t shift(unsigned int bits, blob_t const & num) const {

        blob_t overflow;
        blob_t res = blob_shift_left(num, overflow, bits);
        res = reduce(res, overflow);

        return res;
    }


protected:


    /**
     * copy ctor
     */
    gf2(gf2 const & ) = delete;


    blob_t modulus_equiv;                   /**< modulus as blob */


    /**
     * Precalculated mapping of i -> x^i mod f(x). 
     */
    blob_t bitreduction_table[GF_BITS * 2];


    /**
     * Sets blob to an unsigned integer value.
     *
     * @param   blob        the blob to set
     * @param   value       the value to set
     */
    inline void blob_set_ui(blob_t & blob, unsigned int value) const {
        blob.fill(0);
        blob[BLOB_INTS - 1] = value; 
    };


    /**
     * shift a blob to the left for an amount of bits
     *
     * @param   num             the number to shift
     * @param   overflow        will contain the oveflow
     * @param   bits            amount of bits to shift
     * @return  num << bits plus oveflow
     */
    inline blob_t blob_shift_left(blob_t const & num, blob_t & overflow, unsigned int bits) const {

        assert(bits <= BLOB_BITS);

        unsigned int words = bits / WORD_BITS;
        unsigned int subbits = bits % WORD_BITS;

        blob_t res = num;
        if (words) {
            blob_shift_left_words(res, overflow, bits);
        }
        else {
            overflow.fill(0);
        }

        if (subbits) {
            if (words) {
                blob_shift_left_subbits(overflow, subbits);
            }

            overflow[BLOB_INTS - 1] ^=  blob_shift_left_subbits(res, subbits);
        }

        return res;
    };


    /** 
     * Shift rop to the left by a number of bits that doesn't fit a word boundary.
     *
     * @param   rop     the blob to work on
     * @param   bits    amount of bits to shift
     * @return  the resulting overflow 
     */
    inline unsigned int blob_shift_left_subbits(blob_t & rop, unsigned int bits) const {

        assert(bits < WORD_BITS);

        unsigned int carry = 0; 
        unsigned int antibits = WORD_BITS - bits; 

        for (int i = BLOB_INTS - 1; i >= 0 ; --i) {
            int newcarry = rop[i] >> antibits; 
            rop[i] = (rop[i] << bits) ^ carry;
            carry = newcarry; 
        }

        return carry; 
    }


    /** 
     * Shifts rop to the left by a number words, depositing the resulting overflow in overflow. 
     *
     * @param   rop         the blob to work on
     * @param   overflow    will receive the overflow
     * @param   words       number of words to shift
     */
    inline void blob_shift_left_words(blob_t & rop, blob_t & overflow, unsigned int words) const {

        assert(words <= BLOB_INTS);

        unsigned int srcidx = 0;
        int dstidx = srcidx - words; 

        overflow.fill(0);

        // shift blob -> overflow 
        while (dstidx < 0) {
            overflow[BLOB_INTS + dstidx] = rop[srcidx];
            ++dstidx; 
            ++srcidx; 
        }

        // shift inside blob 
        while (srcidx < BLOB_INTS) {
            rop[dstidx] = rop[srcidx]; 
            ++dstidx; 
            ++srcidx; 
        }

        // clear "shifted in" words 
        while (dstidx < (int) BLOB_INTS) {
            rop[dstidx++] = 0 ; 
        }
    }


    /** 
     * Return the value of a bit in op. 
     *
     * @param   blob        the blob with the bit
     * @param   bit         the bit position to test
     * @return  1 for set bit, 0 for not
     */
    inline unsigned int blob_tstbit(blob_t const & blob, unsigned int bit) const {
        assert(bit < BLOB_BITS);
        word_t mask = 1 << bit % WORD_BITS ;
        return blob[BLOB_INTS - 1 - bit / WORD_BITS] & mask;
    }


    /** 
     * Sets rop to the field element describing x^bit.
     *
     * Warning: Call only after setup_bitreduction_table is complete. 
     * Otherwise, use gf2_reduce_bit_slow!
     */
    inline void reduce_bit(blob_t rop, unsigned int bit) const {
        assert(bit <= BLOB_BITS * 2);
        blob_set_ui(rop, bitreduction_table[bit]);
    }


    /**
     * Sets rop to the field element describing x^bit. 
     *
     * @param   num     the blob we are working on
     * @param   bit     reduction bit
     */
    inline void reduce_bit_slow(blob_t & blob, unsigned int bit) const {

        blob_t overflow;
        blob_set_ui(blob, 1);

        if (bit < BLOB_BITS) {
            blob_shift_left(blob, overflow, bit);
        }
        else if (bit < BLOB_BITS * 2) {

            // Calling gf2_multiply_with here is slow and unnecessary, 
            //  but since we only use this code for precalc, let's keep 
            // it for the time being. 
            blob = blob_shift_left(blob, overflow, bit - BLOB_BITS);
            blob = mul(blob, modulus_equiv);
        }
        else {

            // recursion is possible, but only necesary for
            // bits that are > twice the field size, which we
            // never encounter in multiplication 
            assert(NULL) ;
        }
    }


private:


    /**
     * setup the GF2 with precalculation tables
     */
    void setup_gf2() {

        BLOB_BYTES = GF_BITS / 8;
        BLOB_BITS = GF_BITS;

        setup_bitreduction_table();
    };


    /** 
     * Computes a precalculation table for i -> x^i mod f(x) where i < 2 * BLOB_BITS. 
     *
     * This is required to adequately adjust for overflow in GF(2^BLOB_BITS) multiplication. 
     */
    void setup_bitreduction_table() {
        for(unsigned int i = 0 ; i < 2 * BLOB_BITS ; ++i) {
            reduce_bit_slow(bitreduction_table[i], i);  
        }
    };

};

}

}


#endif


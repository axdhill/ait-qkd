/*
 * evhash.cpp
 * 
 * implement the evaluation hash authentication
 *
 * Author: Thomas Themel - thomas.themel@ait.ac.at
 *         Oliver Maurhart, <oliver.maurhart@ait.ac.at>
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

#include <arpa/inet.h>

#include "evhash.h"


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


/**
 * this class represents a Galois Field 2
 *
 * The class creates a Galois Field 2^x with x been the number of bits managed.
 * For being useful we need an irreducible polynom this GF2 is based on. This
 * polynom is given as a simple unsigned int value representing the coefficients of
 * the polynom. Note that the modulus omits the most significant bit, since this is
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
template<unsigned int GF_BITS> class gf2 {


public:


    /**
     * a word_t is the single atom unit of processing within the GF2
     */
    typedef unsigned int word_t;

    static std::size_t const WORD_BYTES = sizeof(word_t);
    static std::size_t const WORD_BITS = sizeof(word_t) * 8;
    static std::size_t const BLOB_INTS = GF_BITS / WORD_BITS;
    static std::size_t const BLOB_BYTES = (GF_BITS / 8);
    static std::size_t const BLOB_BITS = GF_BITS;


    /**
     * a blob_t holds a full value in the current GF2
     * congruent with the irreducible polynom given in modulus
     */
    typedef word_t blob_t[GF_BITS / (8 * sizeof(word_t))];


    /**
     * ctor
     *
     * @param   nModules                    signature of the irreducible polynom
     */
    explicit gf2(unsigned int nModulus) {
        this->blob_set_value(modulus_equiv, nModulus);
        this->setup_gf2();
    }


    /**
     * add two values
     *
     * this is a simple XOR
     *
     * @param   res         result
     * @param   num1        first number
     * @param   num2        second number
     */
    inline void add(blob_t & res, blob_t const & num1, blob_t const & num2) const {
        for (unsigned int i = 0; i < BLOB_INTS; ++i) {
            res[i] = num1[i] ^ num2[i];
        }
    }


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
     * @param   res         result
     * @param   bytes       the memory to convert
     */
    inline void blob_from_memory(blob_t & res, char const * bytes) const {
        word_t const * wordptr = reinterpret_cast<word_t const *>(bytes); 
        for (unsigned i = 0 ; i < BLOB_INTS ; ++i) {
            res[i] = htonl(*wordptr++);
        }
    }


    /**
     * Converts an arbitrary memory block to a blob and stores it in result. 
     *
     * @param   res         result
     * @param   cMemory     the memory to convert
     * @return  a blob to use
     */
    inline void blob_from_memory(blob_t & res, qkd::utility::memory const & cMemory) const {
        assert(cMemory.size() >= BLOB_BYTES);
        blob_from_memory(res, (char const *)cMemory.get());
    }


    /**
     * Assign blob to another
     *
     * @param   res         the blob to set - dst
     * @param   value       the value to set - src
     */
    inline void blob_set(blob_t & blob, blob_t const & value) const { memcpy(blob, value, sizeof(blob_t)); }


    /**
     * Sets blob to an unsigned integer value.
     *
     * @param   blob        the blob to set
     * @param   value       the value to set
     */
    inline void blob_set_value(blob_t & blob, unsigned int value) const {
        memset(blob, 0, BLOB_BYTES);
        blob[BLOB_INTS - 1] = value; 
    }


    /**
     * Converts a blob to a memory
     *
     * @param   blob        the blob to convert
     * @return  a memory object holding the blob
     */
    inline qkd::utility::memory blob_to_memory(blob_t const & blob) const {

        qkd::utility::memory res(BLOB_BYTES);
        word_t * wordptr = reinterpret_cast<word_t *>(res.get()); 
        for (unsigned i = 0 ; i < BLOB_INTS ; ++i) {
            (*wordptr) = ntohl(blob[i]);
            wordptr++;
        }
        return res;
    }


    /**
     * get the cardinality of the GF2
     *
     * @return  the cardinality
     */
    unsigned int cardinality() const { return GF_BITS; }


    /**
     * get the modulus
     *
     * @return  the modulus for this Galois Field
     */
    blob_t const & modulus() const { return modulus_equiv; }


    /**
     * multiply 
     * 
     * This is the normal slow version. An enhances multiplication
     * with a constant value "alpha" (== key) is implemented in the
     * derived template gf2_fast_alpha below.
     *
     * @param   res     result
     * @param   num1    first operand
     * @param   num2    second operand
     */
    void mul(blob_t & res, blob_t const & num1, blob_t const & num2) const {
    
        blob_t tmp;
        blob_set_value(tmp, 0);

        for (int i = BLOB_INTS - 1; i >= 0; --i) {

            word_t word = num2[i];
            if (word != 0) {

                word_t mask;
                int j;
                for (mask = 1 << (WORD_BITS - 1), j = WORD_BITS - 1; mask > 0; mask = mask >> 1, --j) {

                    if (word & mask) {

                        blob_t shifted;
                        shift(shifted, (BLOB_INTS - 1 - i) * WORD_BITS + j, num1);
                        add(tmp, tmp, shifted);
                    }
                }
            }
        }

        blob_set(res, tmp);
    }


    /**
     * Reduce a value that doesn't fit the field over the modulus to an equivalent field element. 
     *
     * @param   res         num % modulus
     * @param   num         the value to reduce
     * @param   overflow    the value's overflow
     */
    void reduce(blob_t & res, blob_t const & num, blob_t & overflow) const {

        blob_set(res, num);
        blob_t reduced_bit ;

        // for each bit in overflow ...
        for (unsigned int i = 0 ; i < BLOB_BITS; ++i) {     

            // ... that is set ...
            if (blob_tstbit(overflow, i)) {

                // ... add the equivalent field element
                reduce_bit(reduced_bit, i + BLOB_BITS);
                add(res, res, reduced_bit);
            }
        }
    }


    /**
     * shift left
     *
     * @param   res         num << nSteps
     * @param   bits        number of steps to shift left
     * @param   num         number to shift left
     */
    void shift(blob_t & res, unsigned int bits, blob_t const & num) const {
        blob_t overflow;
        blob_shift_left(res, num, overflow, bits);
        reduce(res, res, overflow);
    }


protected:


    /**
     * copy ctor
     */
    gf2(gf2 const & ) = delete;


    /**
     * modulus as blob 
     */
    blob_t modulus_equiv;                   


    /**
     * Precalculated mapping of i -> x^i mod f(x). 
     */
    blob_t bitreduction_table[GF_BITS * 2];


    /**
     * shift a blob to the left for an amount of bits
     *
     * @param   res             num << bits plus oveflow
     * @param   num             the number to shift
     * @param   overflow        will contain the oveflow
     * @param   bits            amount of bits to shift
     */
    inline void blob_shift_left(blob_t & res, blob_t const & num, blob_t & overflow, unsigned int bits) const {

        assert(bits <= BLOB_BITS);

        unsigned int words = bits / WORD_BITS;
        unsigned int subbits = bits % WORD_BITS;

        blob_set(res, num);
        if (words) {
            blob_shift_left_words(res, overflow, words);
        }
        else {
            blob_set_value(overflow, 0);
        }

        if (subbits) {
            if (words) {
                blob_shift_left_subbits(overflow, subbits);
            }

            overflow[BLOB_INTS - 1] ^=  blob_shift_left_subbits(res, subbits);
        }
    }


    /** 
     * Shift blob to the left by a number of bits that doesn't fit a word boundary.
     *
     * @param   blob     the blob to work on
     * @param   bits    amount of bits to shift
     * @return  the resulting overflow 
     */
    inline unsigned int blob_shift_left_subbits(blob_t & blob, unsigned int bits) const {

        assert(bits < WORD_BITS);

        unsigned int carry = 0; 
        unsigned int antibits = WORD_BITS - bits; 

        for (int i = BLOB_INTS - 1; i >= 0; --i) {
            int newcarry = blob[i] >> antibits; 
            blob[i] = (blob[i] << bits) ^ carry;
            carry = newcarry; 
        }

        return carry; 
    }


    /** 
     * Shifts blob to the left by a number words, depositing the resulting overflow in overflow. 
     *
     * @param   blob        the blob to work on
     * @param   overflow    will receive the overflow
     * @param   words       number of words to shift
     */
    inline void blob_shift_left_words(blob_t & blob, blob_t & overflow, unsigned int words) const {

        assert(words <= BLOB_INTS);

        unsigned int srcidx = 0;
        int dstidx = srcidx - words; 

        blob_set_value(overflow, 0);

        // shift blob -> overflow 
        while (dstidx < 0) {
            overflow[BLOB_INTS + dstidx] = blob[srcidx];
            ++dstidx; 
            ++srcidx; 
        }

        // shift inside blob 
        while (srcidx < BLOB_INTS) {
            blob[dstidx] = blob[srcidx]; 
            ++dstidx; 
            ++srcidx; 
        }

        // clear "shifted in" words 
        while (dstidx < (int) BLOB_INTS) {
            blob[dstidx++] = 0 ; 
        }
    }


    /** 
     * Return the value of a bit in blob 
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
     * Sets blob to the field element describing x^bit.
     *
     * Warning: Call only after setup_bitreduction_table is complete. 
     * Otherwise, use gf2_reduce_bit_slow!
     */
    inline void reduce_bit(blob_t & blob, unsigned int bit) const {
        assert(bit <= BLOB_BITS * 2);
        blob_set(blob, bitreduction_table[bit]);
    }


    /**
     * Sets rop to the field element describing x^bit. 
     *
     * @param   num     the blob we are working on
     * @param   bit     reduction bit
     */
    inline void reduce_bit_slow(blob_t & blob, unsigned int bit) const {

        blob_t overflow;
        blob_set_value(blob, 1);

        if (bit < BLOB_BITS) {
            blob_shift_left(blob, blob, overflow, bit);
        }
        else if (bit < BLOB_BITS * 2) {

            // Calling gf2_multiply_with here is slow and unnecessary, 
            // but since we only use this code for precalc, let's keep 
            // it for the time being. 
            blob_shift_left(blob, blob, overflow, bit - BLOB_BITS);
            mul(blob, blob, modulus_equiv);
        }
        else {

            // recursion is possible, but only necessary for
            // bits that are > twice the field size, which we
            // never encounter in multiplication 
            throw std::length_error("bit number in bit reduction exceeds allowed size.");
        }
    }


private:


    /**
     * setup the GF2 with precalculation tables
     */
    void setup_gf2() { setup_bitreduction_table(); }


    /** 
     * Computes a precalculation table for i -> x^i mod f(x) where i < 2 * BLOB_BITS. 
     *
     * This is required to adequately adjust for overflow in GF(2^BLOB_BITS) multiplication. 
     */
    void setup_bitreduction_table() {
        for(unsigned int i = 0 ; i < 2 * BLOB_BITS ; ++i) {
            reduce_bit_slow(bitreduction_table[i], i);  
        }
    }

};


/**
 * this class represents a Galois Field 2 with optimizations to fast multiply a key alpha
 */
template<unsigned int GF_BITS> class gf2_fast_alpha : public gf2<GF_BITS> {


public:

    
    // make template depended names explicit

    using typename gf2<GF_BITS>::blob_t;
    using typename gf2<GF_BITS>::word_t;
    
    using gf2<GF_BITS>::BLOB_INTS;
    using gf2<GF_BITS>::BLOB_BITS;
    using gf2<GF_BITS>::BLOB_BYTES;
    using gf2<GF_BITS>::WORD_BITS;

    using gf2<GF_BITS>::blob_set;
    using gf2<GF_BITS>::blob_set_value;
    using gf2<GF_BITS>::blob_to_memory;


    /**
     * ctor
     * 
     * we precalculate k^n with n=2^x with x=1..20
     *
     * @param   nModules                    signature of the irreducible polynom
     * @param   bTwoStepPrecalculation      do a single or double precalculation table
     * @param   cKey                        the value for which fast multiplication is achieved
     */
    explicit gf2_fast_alpha(unsigned int nModulus, bool bTwoStepPrecalculation, qkd::utility::memory const & cKey) 
            : gf2<GF_BITS>(nModulus), MAX_POW(20), bTwoStepPrecalculation(bTwoStepPrecalculation), m_cAlphaPow(nullptr) {

        this->blob_from_memory(alpha, cKey);
        this->precalc_blob_multiplication();
        this->precalc_alpha_pow();
    }
    
    
    /**
     * dtor
     */
    ~gf2_fast_alpha() {
        if (m_cAlphaPow) delete [] m_cAlphaPow;
    }


    /** 
     * Fast multiplication of a blob with alpha.
     *
     * @param   res         blob * alpha in this GF2
     * @param   blob        the blob to multiply
     */
    void times_alpha(blob_t & res, blob_t const & blob) const {

        blob_t cProduct;
        this->blob_set_value(cProduct, 0);

        word_t b[GF_BITS / PRECALC_BITS * 2]; 
        unsigned int k;

        k = 0;
        for (unsigned int i = 0; i < BLOB_INTS ; ++i) {

            word_t word = blob[i] ; 

            // Split word into the correct number 
            // of PRECALC_BITS sized chunks 
            for (int j = WORD_BITS / HORNER_BITS - 1; j >= 0; --j) {
                b[k++] = (word >> (HORNER_BITS * j)) & (HORNER_SIZE - 1);
            }
        }

        for (unsigned int i = 0; i < BLOB_BITS / HORNER_BITS ; ++i) {

            assert(b[i] < HORNER_SIZE);
            precalc_shift(cProduct, cProduct);

            if (bTwoStepPrecalculation) {
                size_t index_v1 = b[i] >> PRECALC_BITS ;
                this->add(cProduct, cProduct, multiplication_table_2[index_v1]);
                b[i] &= PRECALC_SIZE - 1; 
            }

            this->add(cProduct, cProduct, multiplication_table[b[i]]);
        }      

        this->blob_set(res, cProduct);
    }


    /** 
     * Fast multiplication of a blob with alpha^n.
     * 
     * This uses the table of alpha^2^n stored in m_cAlphaPow.
     * The table has decreasing values starting at alpha^2^MAX_POW.
     *
     * @param   res         blob * alpha^n in this GF2
     * @param   blob        the blob to multiply
     * @param   n           exponent of alpha
     */
    void times_alpha_pow(blob_t & res, blob_t const & blob, uint64_t n) const {
        
        unsigned int nPowIndex = 0;
        blob_set(res, blob);
        while (n > 0) {
            
            if (n >= m_cAlphaPow[nPowIndex].nPow) {
                this->mul(res, res, m_cAlphaPow[nPowIndex].nValue);
                n -= m_cAlphaPow[nPowIndex].nPow;
            }
            else {
                nPowIndex++;
            }
        }
    }
    
    
protected:


    /**
     * Performs fast left-shifting of overflow-free blobs  by HORNER_BITS bits. 
     *
     * @param   res         blob << HORNER_BITS
     * @param   blob        blob to shift
     */
    void precalc_shift(blob_t & res, blob_t const & blob) const {    

        blob_t tmp;
        blob_set_value(tmp, 0);

        blob_t overflow_blob;
        this->blob_shift_left(tmp, blob, overflow_blob, HORNER_BITS);

        // get the lowest order (and only non-zero) 
        // overflow word 
        word_t overflow = overflow_blob[BLOB_INTS - 1] ;

        if (overflow != 0) {

            if (bTwoStepPrecalculation) {

                // v(x) = v1(x) * x^8 + v0(x) 
                size_t index_v1 = overflow >> PRECALC_BITS ; 
                assert(index_v1 < PRECALC_SIZE);

                // lookup v1(x) * x^8 * x^BLOB_BITS
                tmp[BLOB_INTS - 1] ^= overflow_table_2[index_v1];

                // and reduce v(x) to v0(x)
                overflow = overflow & (PRECALC_SIZE - 1);
            }
            else {
                assert(overflow < PRECALC_SIZE);
            }

            // lookup v(x) * x^BLOB_BITS
            tmp[BLOB_INTS - 1] ^= overflow_table[overflow];
        }

        blob_set(res, tmp);
    }



private:


    std::size_t HORNER_BITS;
    std::size_t HORNER_SIZE;
    std::size_t MAX_POW;
    

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
     * the value for which we do fast multiply (==> key k!)
     */
    blob_t alpha;


    /**
     * this holds a k^2^n value (k == alpha)
     */
    typedef struct {
        uint64_t nPow;          /**< this is 2^n --> 2, 4, 8, 16, ... */
        blob_t nValue;          /**< this is alpha^2^n */
    } alpha_pow;
    
    
    /**
     * table of k^2^n in reverse order: alpha^2^MAX_POW is at index 0
     */
    alpha_pow * m_cAlphaPow;
    

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

            this->blob_set_value(v, i);
            this->mul(multiplication_table[i], alpha, v);
            if (bTwoStepPrecalculation) {

                this->blob_set_value(v, i << PRECALC_BITS);
                this->mul(multiplication_table_2[i], alpha, v);
            }
        }
    }


    /**
     * setup the precalulated pow tables
     * 
     * This generates a series of k^n with n = 1...2^MAX_POW
     */
    void precalc_alpha_pow() {
        
        m_cAlphaPow = new alpha_pow[MAX_POW];
        
        blob_t value;
        blob_set(value, alpha);
        
        // init k^1
        m_cAlphaPow[MAX_POW - 1].nPow = 1;
        blob_set(m_cAlphaPow[MAX_POW - 1].nValue, value);
        
        // create k^2, k^4, k^8, ...
        for (int64_t i = MAX_POW - 2, j = 1; i >= 0; --i, ++j) {
            
            this->mul(value, value, value);
            m_cAlphaPow[i].nPow = (1 << j);
            blob_set(m_cAlphaPow[i].nValue, value);            
        }
    }


    /** 
     * Initializes the field's overflow table
     */
    void setup_overflow_table() {

        blob_t tmp; 

        for(int i = 0 ; i < PRECALC_SIZE ; ++i) {

            this->blob_set_value(tmp, i);

            // MAXVALUE is identical to MODULUS_EQUIV in GF(2^N) 
            this->mul(tmp, tmp, this->modulus());
            overflow_table[i] = tmp[BLOB_INTS - 1];
            if (bTwoStepPrecalculation) {
                blob_set_value(tmp, i << PRECALC_BITS);
                this->mul(tmp, tmp, this->modulus());
                overflow_table_2[i] = tmp[BLOB_INTS - 1];
            }
        }
    }

};


/**
 * this class combines a modulus and a key with a certain GF2 plus interface methods to use it neatly
 */
template <unsigned int GF_BITS> class evhash_impl : public evhash_abstract {


public:


    /**
     * ctor
     */
    explicit evhash_impl(qkd::key::key const & cKey) : m_nBlocks(0), m_cRemainder(nullptr), m_nRemainderBytes(0) {
       
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
    virtual ~evhash_impl() {
        delete m_cGF2;
        delete [] m_cRemainder;
    }


    /**
     * add a tag
     * 
     * This performs a simple add in the GF2
     *
     * @param   cTag        tag to add
     */
    void add(qkd::utility::memory const & cTag) {
        typename gf2_fast_alpha<GF_BITS>::blob_t operand;
        m_cGF2->blob_from_memory(operand, cTag);
        m_cGF2->add(m_cTag, operand, m_cTag);
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
    unsigned int block_size() const { return GF_BITS / 8; }


    /**
     * get the final tag
     *
     * @return  the final tag
     */
    qkd::utility::memory finalize() { 

        // add the remainder (bytes not yet calculated)
        if (m_nRemainderBytes > 0) {
            memset(m_cRemainder + m_nRemainderBytes, 0, block_size() - m_nRemainderBytes);
            m_nRemainderBytes = 0;
            update(qkd::utility::memory::wrap((qkd::utility::memory::value_t *)m_cRemainder, block_size()));
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


    /**
     * multiply the existing tag with the init key nRounds times
     *
     * this creates t_n = t_n-1 * k^nRounds
     *
     * this also increases the number of calculated blocks by nRounds
     *
     * @param   nRounds     number of rounds to multiply
     */
    void times(uint64_t nRounds) {
        m_cGF2->times_alpha_pow(m_cTag, m_cTag, nRounds);
        m_nBlocks += nRounds;
    }
    
    
    /**
     * add a memory BLOB to the algorithm
     *
     * This transforms the tag stored in this object
     * 
     * This is the working horse method of evaluation hash.
     * Here the actual tag is calculated.
     *
     * @param   cMemory         memory block to be added
     */
    void update(qkd::utility::memory const & cMemory) {

        // we add pieces of blob_t to the GF2 field
        // when the memory exceeds a multiple of sizeof(blob_t)
        // the remainder is stored and added to the front
        // on the next run

        char * data = (char *)cMemory.get();
        int64_t nLeft = cMemory.size();
        
        // maybe the given memory + remaining bytes are less than block_size()
        // so we store the data for next turn
        if (m_nRemainderBytes + nLeft < block_size()) {
            memcpy(m_cRemainder + m_nRemainderBytes, cMemory.get(), nLeft);
            m_nRemainderBytes += nLeft;
            return;
        }
        
        // we can do at least on block
        if (m_nRemainderBytes) {
            
            // try to add what's remaining of the previous runs first
            int64_t nAddToRemainder = block_size() - m_nRemainderBytes;
            memcpy(m_cRemainder + m_nRemainderBytes, cMemory.get(), nAddToRemainder);
            update_block(m_cRemainder);
            
            data += nAddToRemainder;
            nLeft -= nAddToRemainder;
            m_nRemainderBytes = 0;
        }
        
        // walk over all blocks
        while (nLeft >= block_size()) {
            
            update_block(data);
            
            data += block_size();
            nLeft -= block_size();
        }
        
        if (nLeft < 0) {
            throw std::logic_error("evaluation hash: consumed more bytes than available: nLeft < 0");
        }

        // remember the last block % block_size
        if (nLeft > 0) {
            memcpy(m_cRemainder, data, nLeft);
            m_nRemainderBytes = nLeft;
        }
    }


private:


    /**
     * adds a single block to the algorithm
     * 
     * data *must* point to a memory of at least block_size() size
     *
     * @param   data        memory block to be added
     */
    void update_block(char const * data) {

        // --- the hashing ---
        // Horner Rule: tag_n = (tag_(n-1) + m) * k

        typename gf2_fast_alpha<GF_BITS>::blob_t coefficient;
        m_cGF2->blob_from_memory(coefficient, data);
        m_cGF2->add(m_cTag, coefficient, m_cTag);
        m_cGF2->times_alpha(m_cTag, m_cTag);

        m_nBlocks++;
    }

    
    /**
     * the GF2 to work on
     */
    gf2_fast_alpha<GF_BITS> * m_cGF2;


    /**
     * blocks done so far
     */
    uint64_t m_nBlocks;


    /**
     * remainder of last add (modulo blob size)
     */
    char * m_cRemainder;


    /**
     * remainder of last add (modulo blob size) in bytes
     */
    unsigned int m_nRemainderBytes;


    /**
     * the current tag
     */
    typename gf2_fast_alpha<GF_BITS>::blob_t m_cTag;


};


// ------------------------------------------------------------
// code


/**
 * fabric method
 * 
 * The size of the init key also determines the size of the
 * ev-hash
 * 
 * @param   cKey        init key to create the evhash with
 * @return  an evaluation hash instance
 */
evhash evhash_abstract::create(qkd::key::key const & cKey) {
    
    switch (cKey.size() * 8) {
        
    case 32:
        return evhash(new evhash_impl<32>(cKey));
    case 64:
        return evhash(new evhash_impl<64>(cKey));
    case 96:
        return evhash(new evhash_impl<96>(cKey));
    case 128:
        return evhash(new evhash_impl<128>(cKey));
    case 256:
        return evhash(new evhash_impl<256>(cKey));
    }
    
    throw std::invalid_argument("no evhash available for this key size");
}

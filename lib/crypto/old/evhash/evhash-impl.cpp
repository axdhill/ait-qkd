/*
 * evhash-impl.c
 *
 * Implementation of the evaluation hash algorithm 
 * for the q3p crypto engine.
 * 
 * Author: Thomas Themel - thomas.themel@ait.ac.at
 *
 * Copyright (C) 2010-2015 Austrian Institute of Technology
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

#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

// ait
#include "../algorithm.h"
#include "../utility.h"
#include "../simple-state.h"


/* Precalculation parameters - PRECALC_BITS must be a 
   multiple of 8 and a divisor of BLOB_BITS - which probably
   makes the only practical values 8 and 16 in 2006. 
   PRECALC_BITS MUST BE <= WORD_BITS. If TWO_STEP_PRECALC
   is active, 2*PRECALC_BITS must be a divisor of BLOB_BITS
   as well. */
#ifndef PRECALC_BITS 
#define PRECALC_BITS 8
#endif 

#define PRECALC_SIZE (1 << PRECALC_BITS)

/* debug macro for examination of precalc'd blob_t tables */
#define DUMP_TABLE(name, length) { int i ;\
fprintf(stdout, "Dumping %s\n", #name);\
for(i = 0; i < length ; ++i) { blob_out_str(stdout, name[i]); fprintf(stdout, "\n");}\
fprintf(stdout, "End of %s\n", #name);}

#if !(defined GF_BITS)
#error Cannot compile this file directly, see evhash-<n>.c!
#endif


/* number of ints in a blob */

#define BLOB_BYTES sizeof(blob_t)
#define BLOB_BITS  (sizeof(blob_t)*8)
#define WORD_BYTES sizeof(word_t)
#define WORD_BITS (sizeof(word_t)*8)
#define BLOB_INTS (GF_BITS/WORD_BITS)

/* Number of bits to use for horner's rule. 
   If we use two-step precalculation, we can
   handle 2*PRECALC_BITS per step. */

#ifdef TWO_STEP_PRECALC
#define HORNER_BITS (2*PRECALC_BITS)
#else
#define HORNER_BITS PRECALC_BITS
#endif

#define HORNER_SIZE (1 << HORNER_BITS)

/* two macros necessary because DO_CONCAT
   doesn't expand arguments when called directly */
#define DO_CONCAT(a,b) a##b
#define CONCAT(a,b) DO_CONCAT(a,b)

#define DO_TO_STRING(a) #a
#define TO_STRING(a) DO_TO_STRING(a)

typedef unsigned int word_t ;
typedef word_t blob_t[BLOB_INTS];

/* a blob represenatation of the field element
   congruent with the irreducible polynomial */
static blob_t modulus_equiv;


#ifdef USE_GF2_96
/* GF(2^96) as GF(2)[x] mod x^96+x^10+x^9+x^6+1*/
/* field element congruent with irreducible polynomial: 1601 */
static blob_t modulus_equiv = {0,0,0x641};
#endif

#ifdef USE_GF2_128
/* GF(2^128) as GF(2)[x] mod x^128+x^7+x^2+x+1 */
/* field element congruent with irreducible polynomial: 135 */
static blob_t modulus_equiv = {0,0,0,0x87};
#endif

#ifdef USE_GF2_256
/* GF(2^256) as GF(2)[x] mod x^128+x^10+x^5+x^2+1 */
/* field element congruent with irreducible polynomial: 1061 */
static blob_t modulus_equiv = {0,0,0,0,0,0,0,0x425};
#endif

static void gf2_shift(blob_t rop, unsigned int bits, const blob_t num);
static void gf2_reduce_bit(blob_t rop, unsigned int bit);
static void gf2_reduce_bit_slow(blob_t rop, unsigned int bit);
static void gf2_add(blob_t rop, const blob_t num1, const blob_t num2);
static void gf2_reduce(blob_t rop, const blob_t overflow);
static void gf2_mul(blob_t result, const blob_t num1, const blob_t num2);
static void gf2_multiply_with(blob_t result,  const blob_t num1);

static int initialized = 0;

/* Precalculated mapping of v(x) -> v(x)*x^BLOB_BITS mod f(x) 
   for deg(v(x)) < PRECALC_BITS */ 
static unsigned int overflow_table[PRECALC_SIZE];

#ifdef TWO_STEP_PRECALC
/* Precalculated mapping of v(x) -> v(x)*x^PRECALC_BITS*x^BLOB_BITS mod f(x)
   for deg(v(x)) < PRECALC_BITS */
static unsigned int overflow_table_2[PRECALC_SIZE];
#endif

/* Precalculated mapping of i -> x^i mod f(x). Note that this is actually
   only dependent on the irreducible polynomial we use, so it could in 
   principle be stored statically. */
static blob_t bitreduction_table[BLOB_BITS*2];

/** Evaluation hash context. In addition to the usual hash_context
    data, we also need a copy of the key (alpha) and a table mapping
    v(x)->alpha*v(x) for all v(x) with deg(v(x))<PRECALC_BITS. If we 
    use the TWO_STEP_PRECALC method recommended by Fitzi, we need a 
    second multiplication table v(x)->alpha*x^PRECALC_BITS*v(x) for all
    v(x) with deg(v(x))<PRECALC_BITS.
*/
typedef struct 
{
    ce_context h;
    blob_t alpha ; 
    blob_t multiplication_table[PRECALC_SIZE] ;
#ifdef TWO_STEP_PRECALC
    blob_t multiplication_table_2[PRECALC_SIZE] ; 
#endif
} evaluation_hash_ctx;


/** Zeroes blob. */
static inline void blob_zero(blob_t blob)
{
    memset(blob, 0, BLOB_BYTES);
}

/** Sets blob to an unsigned integer value. */
static inline void blob_set_ui(blob_t blob, unsigned int value)
{
    memset(blob, 0, BLOB_BYTES - WORD_BYTES);
    blob[BLOB_INTS-1] = value ; 
}

/** Sets rop to op1. */
static inline void blob_set(blob_t rop, const blob_t op1)
{
    memcpy(rop, op1, BLOB_BYTES);
}

/** Shifts rop to the left by a number words, depositing the resulting
    overflow in overflow. */
static inline void blob_shift_left_words(blob_t rop, blob_t overflow, unsigned int words)
{
    unsigned int i ; 
    unsigned int srcidx = 0 ;
    int dstidx = srcidx - words ; 

    assert(words <= BLOB_INTS) ;

    /* clear overflow */
    for(i = 0 ; i < BLOB_INTS - words ; ++i)
    {
        overflow[i] = 0 ; 
    }

    /* shift blob -> overflow */
    while (dstidx < 0)
    {
        overflow[BLOB_INTS+dstidx] = rop[srcidx] ;
        ++dstidx ; 
        ++srcidx ; 
    }

    /* shift inside blob */
    while (srcidx < BLOB_INTS)
    {
        rop[dstidx] = rop[srcidx] ; 
        ++dstidx ; 
        ++srcidx ; 
    }

    /* clear "shifted in" words */
    while (dstidx < (int) BLOB_INTS)
    {
        rop[dstidx++] = 0 ; 
    }
}

/** Shift rop to the left by a number of bits that doesn't fit a word boundary.
    @return the resulting overflow. */
static inline unsigned int blob_shift_left_subbits(blob_t rop, unsigned int bits)
{
    unsigned int carry = 0 ; 
    unsigned int antibits = WORD_BITS - bits ; 
    int i; 

    assert(bits < WORD_BITS);

    for(i = BLOB_INTS-1; i >= 0 ; --i)
    {
        int newcarry = rop[i] >> antibits ; 
        rop[i] = (rop[i] << bits) ^ carry ;
        carry = newcarry ; 
    }

    return carry ; 
}

/* Shifts rop to the left by an arbitrary number of bits < BLOB_BITS. */
static inline void blob_shift_left(blob_t rop, blob_t overflow, unsigned int bits)
{
    assert(bits <= BLOB_BITS);    
    unsigned int words, subbits ;
    words = bits / WORD_BITS ;
    subbits = bits % WORD_BITS ; 

    if (words)
    {
        blob_shift_left_words(rop,overflow,words);
    }
    else
    {
        blob_zero(overflow);
    }

    if (subbits)
    { 
        if (words)
        {
            /* shift left overflow - this can be skipped 
             if words is zero since then overflow is also zero */
            blob_shift_left_subbits(overflow, subbits);
        }

        /* shift left actual data and append carry to overflow */
        overflow[BLOB_INTS-1] ^= blob_shift_left_subbits(rop, subbits);
    }
}

/** Compares op1 and op2. 
    @return -1 if op2 > op1
    @return 0 if op2 = op1 
    @return 1 if op1 > op2 */
static inline int blob_cmp(const blob_t op1, const blob_t op2)
{
    int i ; 

    for(i = BLOB_INTS - 1 ; i >= 0 ; --i)
    {
        if (op1[i] > op2[i]) return 1 ;
        if (op1[i] < op2[i]) return -1 ;
    }
    return 0 ; 
}

/** Clear a bit in rop. Bits are counted in "reverse network bit 
    order", with bit 0 the LSB. Another way to see this is that
    bit's weight in the bitstring-to-integer mapping is 2^bit.
*/
static inline void blob_clrbit(blob_t rop, unsigned int bit)
{
    assert(bit < BLOB_BITS);
    word_t mask = ~0 ;

    mask ^= 1 << bit % WORD_BITS ;

    rop[BLOB_INTS - 1 - bit / WORD_BITS] &= mask ;
}

/** Return the value of a bit in op. The same indexing as in
    @see blob_clrbit is used.
 */
static inline int blob_tstbit(const blob_t op, unsigned int bit)
{
    assert(bit < BLOB_BITS);
    word_t mask = 1 << bit % WORD_BITS ;

    return op[BLOB_INTS - 1 - bit / WORD_BITS] & mask ;
}

/*
** A debugging helper function that dumps a BLOB to a
    stream in hex format. *
static void blob_out_str(FILE* stream, const blob_t blob)
{
    int i ;
    for(i = 0 ; i < BLOB_INTS; ++i)
    {
        fprintf(stream, "%08x", blob[i]);
    }
}
*/

/** Initializes the field's overflow table,
    @see overflow_table */
static void setup_overflow_table()
{
    int i ;
    blob_t tmp ; 

    for(i = 0 ; i < PRECALC_SIZE ; ++i)
    {
        blob_set_ui(tmp, i);
        /* MAXVALUE is identical to MODULUS_EQUIV 
           in GF(2^N) */
        gf2_multiply_with(tmp,modulus_equiv);
        overflow_table[i] = tmp[BLOB_INTS-1];

#ifdef TWO_STEP_PRECALC
        blob_set_ui(tmp, i << PRECALC_BITS);
        gf2_multiply_with(tmp,modulus_equiv);
        overflow_table_2[i] = tmp[BLOB_INTS-1];
#endif
    }
}

/** Computes a precalculation table for alpha*v(x) mod f(x) where
    deg(v(x))<PRECALC_BITS. */

static void precalc_blob_multiplication(blob_t* multiplication_table, 
#ifdef TWO_STEP_PRECALC
                                        blob_t* multiplication_table_2,
#endif
                                        const blob_t alpha)
{
    int i ; 
    blob_t v ;

    for(i = 0 ; i< PRECALC_SIZE ; ++i)
    {
        blob_set_ui(v, i);
        gf2_mul(multiplication_table[i], alpha, v );
#ifdef TWO_STEP_PRECALC
        blob_set_ui(v, i << PRECALC_BITS);
        gf2_mul(multiplication_table_2[i], alpha, v );
#endif
    }
}

/** Computes a precalculation table for i -> x^i mod f(x)
    where i < 2*BLOB_BITS. This is required to adequately adjust 
    for overflow in GF(2^BLOB_BITS) multiplication. Note that this
    depends only on the irreducible polynomial and so could be done
    statically in principle.
*/
static void setup_bitreduction_table()
{
    unsigned int i ;
    for(i = 0 ; i < 2*BLOB_BITS ; ++i)
    {
        gf2_reduce_bit_slow(bitreduction_table[i], i);  
    }
}

/** Prepares precalculated data that is only needed
    once per field. */
static void setup_precalc()
{
    /* First: Initialize the modulus so that we
       can do reduction. */
    blob_set_ui(modulus_equiv, gf_modulus);

    /* IMPORTANT: set up the bit reduction table before
                  the others since gf2_mul depends on it!*/
    setup_bitreduction_table();
    setup_overflow_table() ;
}

/** Converts an arbitrary memory block to a blob and stores
    it in result. Byte order for the conversion is network
    byte order, ie the string "01234567" is converted into
    the blob 0x3031323334353637, representing the number
    55561791730626147895 or the polynomial 
    x^61+x^60+x^53+x^52+x^48+x^45+x^44+x^41+x^37+x^36
    +x^33+x^32+x^29+x^28+x^26+x^21+x^20+x^18+x^16+x^13
    +x^12+x^10+x^9+x^5+x^4+x^2+x^1+x^0 */
static void blob_from_memory(blob_t result, const char* bytes)
{
    /** \todo I know I'm asking for a SIGBUS or slow execution,
       but then again it's probably a reasonable expectation
       to have callers align their data on word boundaries. */
    word_t* wordptr = (word_t*) bytes ; 
    unsigned int i ;

    /* fill the blob with data from memory */
    for(i = 0 ; i < BLOB_INTS ; ++i)
    {
        result[i] = htonl(*wordptr++) ;
    }
}


/** Multiplies result with num1. */
static void gf2_multiply_with(blob_t result,  const blob_t num1)
{
    blob_t tmp;
    gf2_mul(tmp, result, num1);
    blob_set(result, tmp);
}


/** Multiplies num1*num2 mod f(x) in GF(2^BLOB_BITS).
    Danger: Result must not be the same oject as either
    num1 or num2 since it is mangled during calculation.
*/
static void gf2_mul(blob_t result, const blob_t num1, const blob_t num2)
{
    int i ;

    /* it's not safe to have result the same as the argument! */
    /*
    assert(result != num1);
    assert(result != num2);
    */
    blob_zero(result);

    for(i = BLOB_INTS - 1 ; i >= 0 ; --i)
    {
        word_t word = num2[i];

        if (word != 0)
        {
            word_t mask;
            int j;
            for(mask = 1 << (WORD_BITS -1), j = WORD_BITS - 1; 
                mask > 0 ; mask = mask >> 1, --j)
            {
                if (word & mask)
                {
                    blob_t shifted;
                    gf2_shift(shifted, (BLOB_INTS - 1 - i) * WORD_BITS + j, num1);
                    gf2_add(result, result, shifted) ;
                }
            }
        }       
    }
}

/** Store a bitwise xor of op1 and op2 in rop. rop may also
    be one of op1 or op2. */
static void blob_xor(blob_t rop, const blob_t op1, const blob_t op2)
{
    unsigned int i ;
    for(i = 0 ; i < BLOB_INTS ; ++i)
    {
        rop[i] = op1[i]^op2[i];
    }
}

/** Calculate num * x^bits mod f(x) and store the result in rop.
 */
static void gf2_shift(blob_t rop, unsigned int bits, const blob_t num)
{
    blob_t overflow ; 

    /* shift bits to the left */
    blob_set(rop, num);
    blob_shift_left(rop, overflow, bits);


    /* do modulus reduction */
    gf2_reduce(rop, overflow);
}

/** Reduce a value that doesn't fit the field 
    over the modulus to an equivalent field element. */
static void gf2_reduce(blob_t rop, const blob_t overflow)
{
    blob_t reduced_bit ;
    unsigned int i ; 

    /* for each bit in overflow */
    for(i = 0 ; i < BLOB_BITS; ++i)
    {     
        /* that is set */
        if (blob_tstbit(overflow,i))
        {
            /* add the equivalent field element */
            gf2_reduce_bit(reduced_bit, i+BLOB_BITS);
            gf2_add(rop, rop, reduced_bit);
        }
    }
}

/** Sets rop to the field element describing x^bit.
   Warning: Call only after setup_bitreduction_table 
   is complete. Otherwise, use gf2_reduce_bit_slow!
*/
static void gf2_reduce_bit(blob_t rop, unsigned int bit)
{
    assert(bit <= BLOB_BITS*2);
    blob_set(rop, bitreduction_table[bit]);
}

/** Sets rop to the field element describing x^bit. */
static void gf2_reduce_bit_slow(blob_t rop, unsigned int bit)
{
    blob_t overflow;

    blob_set_ui(rop, 1);
    if (bit < BLOB_BITS)
    {
        blob_shift_left(rop, overflow, bit) ;
    }
    else if (bit < BLOB_BITS*2)
    {
       /* Calling gf2_multiply_with here is slow and unnecessary, 
          but since we only use this code for precalc, let's keep 
          it for the time being. */
        blob_shift_left(rop, overflow, bit - BLOB_BITS) ;
        gf2_multiply_with(rop, modulus_equiv);
    }
    else
    {
        /* recursion is possible, but only necesary for
           bits that are > twice the field size, which we
           never encounter in multiplication */
        assert(NULL) ;
    }
}

/** Addition in GF(2^n): Simple XOR */
static void gf2_add(blob_t rop, const blob_t num1, const blob_t num2)
{
    blob_xor(rop, num1, num2);
}

/* Addition in GF(2^n): Simple XOR */
static void gf2_addto(blob_t rop, const blob_t op1)
{
    unsigned int i ;
    for(i = 0 ; i < BLOB_INTS ; ++i)
    {
        rop[i] ^= op1[i];
    }
}


/** Performs fast left-shifting of overflow-free blobs 
    by HORNER_BITS bits. 
    Warning: Call only after setup_overflow_table
       is complete! */
static void gf2_precalc_shift(blob_t rop)
{    
    blob_t overflow_blob;
    blob_shift_left(rop, overflow_blob, HORNER_BITS);

    /* get the lowest order (and only non-zero) 
       overflow word */
    word_t overflow = overflow_blob[BLOB_INTS-1] ;

    if (overflow != 0)
    {
#ifdef TWO_STEP_PRECALC
        // v(x) = v1(x)*x^8 + v0(x) 
        size_t index_v1 = overflow >> PRECALC_BITS ; 
        assert(index_v1 < PRECALC_SIZE);

        // lookup v1(x)*x^8*x^BLOB_BITS
        rop[BLOB_INTS-1] ^= overflow_table_2[index_v1];

        // and reduce v(x) to v0(x)
        overflow = overflow & (PRECALC_SIZE - 1);
#else
        assert(overflow < PRECALC_SIZE) ;
#endif
        // lookup v(x)*x^BLOB_BITS
        rop[BLOB_INTS-1] ^= overflow_table[overflow];
    }
}

/** Fast multiplication of rop with alpha.
   Warning: Call only after setup_multiplication_table
   is complete! Otherwise, use gf2_multiply_with. **/
static void gf2_times_alpha(blob_t rop, const evaluation_hash_ctx* pctx)
{
    word_t b[BLOB_BITS/HORNER_BITS] ; 
    unsigned int i,k ;
    int j ;
    blob_t result ; 

    blob_zero(result);

    k = 0 ;

    for(i = 0; i < BLOB_INTS ; ++i)
    {
        word_t word = rop[i] ; 
        /* Split word into the correct number of PRECALC_BITS 
           sized chunks */
        for(j = WORD_BITS/HORNER_BITS - 1 ; j >= 0;  --j)
        {
            b[k++] = (word >> (HORNER_BITS*j)) & (HORNER_SIZE - 1) ;  
        }
    }


    for(i = 0 ; i < BLOB_BITS/HORNER_BITS ; ++i)
    {
#ifdef TWO_STEP_PRECALC
        size_t index_v1 = b[i] >> PRECALC_BITS ;
#endif
        gf2_precalc_shift(result);
        assert(b[i]<HORNER_SIZE);
#ifdef TWO_STEP_PRECALC
        gf2_addto(result, pctx->multiplication_table_2[index_v1]);
        b[i] &= PRECALC_SIZE - 1 ; 
#endif
        gf2_addto(result, pctx->multiplication_table[b[i]]);       
    }      
    blob_set(rop, result);
}

/** Transformation function: Performs one round of the
    polynomial evaluation that underlies the evaluation hash. */
static int transform (ce_state* ps, const char* data, size_t nblocks)
{   
    size_t offset = 0 ;
    blob_t coefficient ;
    char* tag = ps->output ; 
    unsigned int i ;

    if (tag == NULL)
    {
        tag = static_cast<char *>(calloc(BLOB_BYTES, 1));
        ps->output = tag ; 
        assert(tag != NULL);
    }

    evaluation_hash_ctx* ctx = 
        (evaluation_hash_ctx*) ps->pctx ;

    for(i = 0 ; i < nblocks ; ++i)
    {
        blob_from_memory(coefficient,data+offset) ;
        gf2_addto((unsigned int*)tag, coefficient) ;
        gf2_times_alpha((unsigned int*)tag, ctx) ;
        offset += BLOB_BYTES ; 
    }

    return 0 ;
}

/** Destroys and frees an evaluation hash context. */
static void destroy(ce_context* ctx)
{
    free(ctx);
}

static ce_context* create_context(const char* key, size_t keysize);

ce_algorithm CONCAT(evhash_, GF_BITS) = 
{ 0x00 + GF_BITS,
  "evhash" TO_STRING(GF_BITS),
  TO_STRING(GF_BITS) "bit Evaluation Hash",
  GF_BITS, 
  GF_BITS,
  create_context
} ; 


/** Finalizes an evaluation hash computation. This basically does nothing
    on little endian archs and does byte flipping on little-endian archs. */
static char* finalize_state (struct _algorithm_state* state, size_t* output_length)
{

    unsigned int* tag = (unsigned int*) state->output ; 

    if(tag != NULL)
    {
        if (output_length)
            *output_length = BLOB_BYTES ;


        /* Byte order conversion: internally, we use host byte order so 
           that we can use C operators to do bit manipulation. The user,
           however, can think of his tag as a bitstring, ie all "network
           bit order". */
        for (unsigned int i = 0 ; i < BLOB_INTS ; ++i)
            tag[i] = htonl(tag[i]);
        
        /* Disown output */
        state->output = NULL ;        
    }
    return (char*) tag ; 
}


/** Creates an evaluation hash context. 
    @return Returns a pointer to the new context if keysize is correct.
    @return NULL if keysize is incorrect. */
static ce_context* create_context(const char* key, size_t keysize)
{
    if (keysize != BLOB_BYTES)
    {
        return NULL ; 
    }

    if (!initialized) 
    {
        setup_precalc();
    }

    evaluation_hash_ctx* pctx = static_cast<evaluation_hash_ctx *>(malloc(sizeof(evaluation_hash_ctx)));    

    /* Byte order conversion: internally, we use host byte order so 
       that we can use C operators to do bit manipulation. The user,
       however, can think of his key as a bitstring, ie all "network
       bit order". */
    blob_from_memory(pctx->alpha, key);

    precalc_blob_multiplication(pctx->multiplication_table,
#ifdef TWO_STEP_PRECALC
                                pctx->multiplication_table_2,
#endif
                                pctx->alpha);

    pctx->h.algorithm = &CONCAT(evhash_, GF_BITS);
    pctx->h.transform = transform ;
    pctx->h.create_state = simple_state_alloc ; 
    pctx->h.finalize_state = finalize_state ;
    pctx->h.destroy = destroy ; 

    return &pctx->h ;
}



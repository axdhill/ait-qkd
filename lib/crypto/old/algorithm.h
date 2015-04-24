/*
 * algorithm.h
 *
 * Algorithm information for the q3p crypto engine
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

#ifndef Q3P_CENGINE_ALGORITHM_H_INCLUDED
#define Q3P_CENGINE_ALGORITHM_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

struct _algorithm;
struct _algorithm_state ; 

/** Describes a context object binding a keyed algorithm and
    a specific key together. All operations on a algorithm context are
    read-only and thus inherently thread-safe. */
typedef struct _algorithm_ctx {
    /** Associated hash algorithm descriptor. */
    struct _algorithm* algorithm ; 
    /** "Destructor" function to release the context when
        it's no longer used. */
    void (*destroy) (struct _algorithm_ctx*);

    /** The transform function is the actual C function that 
        performs the algorithm. It takes a block that
        is nblocks * block_bits long and starts at data 
        and processes  it, transforming output as appropriate. 

        This is not intended to be called by users, but rather
        as an abstraction used in @see alg_update. */
    int (*transform) (struct _algorithm_state* ctx, const char* data, size_t nblocks);

    /** Creates and initializes a state object for this context. */
    struct _algorithm_state* (*create_state) (struct _algorithm_ctx* ctx);

    /** Creates (or extracts) the result from state. If output_length is
        non-NULL, *output_length is set to the number of bytes that the
        output is long. 
        @return A pointer to the algorithm's result. */
    char* (*finalize_state) (struct _algorithm_state* state, size_t* output_length);
} ce_context ; 


/** Describes an algorithm. */
typedef struct _algorithm
{
    /** Algorithm identifier - see ids.h for registry.*/
    unsigned int id ; 
    /** A short identifier - should conform to C identifier rules */
    const char* name ; 
    /** A human-readable description of the algorithm. */

    const char*  description ;   
    /** The size of the output produced by the algorithm.
        If this is 0, the algorithm has variable length output.
        The only cases where this currently occurs are encryption
        algorithms, where the length of the output is equal to the
        lenght of the input. This is probably a prudent assumption
        for clients looking to deal with this. */
    unsigned int output_bits ;

    /** The number of input bits required for one transformation
        step. Should be a multiple of 8 because sub-byte processing
        is not implemented. */
    unsigned int block_bits ; 

    /** Creates a context usable with this algorithm. This should not be 
        called by users, @see alg_context_create for the API function. */
    ce_context* (*create_context) (const char* key, size_t key_length);
} ce_algorithm ; 


/** Register an algorithm descriptor with the library.
    For standard algorithms, this is done in @see init_crypto_engine. */
void ce_register_algorithm(ce_algorithm* alg);

/** Retrieve an algorithm descriptor given the numeric algorithm id.
    See ids.h for identifiers of supplied algorithms. */
const ce_algorithm* ce_get_algorithm(unsigned int id);

/** Retrieve an algorithm descriptor given the string algorithm id. */
const ce_algorithm* ce_get_algorithm_by_name(const char* name);

/** Retrieve the list of all algorithm descriptors. */
const struct gb_list* ce_algorithm_list(void);

#ifdef __cplusplus
}
#endif

#endif // defined Q3P_CENGINE_ALGORITHM_H_INCLUDED

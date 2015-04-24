/*
 * utilities.h
 *
 * Helper functions for the q3p crypto engine
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

#ifndef Q3P_CENGINE_UTILITY_H_INCLUDED
#define Q3P_CENGINE_UTILITY_H_INCLUDED

#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif


/** A simple structure for buffering inputs that 
    are not a multiple of block size. */
typedef struct 
{
    char* hold ; 
    uint64_t capacity ; 
    uint64_t fill ; 
} ce_block_buffer ; 

/** A state object representing calculation state
    for a certain algorithm's context. Access to this
    object needs to be synchronized! */
typedef struct _algorithm_state
{
    ce_block_buffer buf ; 
    char* output ; 
    ce_context* pctx ; 
    void (*destroy)(struct _algorithm_state* ps);
} ce_state ;


typedef struct uhash_ctx *uhash_ctx_t;
  /* The uhash_ctx structure is defined by the implementation of the    */
  /* UHASH functions.                                                   */
 
struct uhash_state
{
    ce_state h;
    /* Used to deal with the reference
       implementation's limitation to 16MB 
       streams. */
    size_t bytes_done;
    uhash_ctx_t impl;    
};




/** Initialize a block buffer with a given capacity. */
void ce_block_buffer_init(ce_block_buffer* pbuf, size_t capacity);
/** Allocate and initialize a block buffer with the given capacity. */
ce_block_buffer* ce_block_buffer_alloc(size_t capacity);
/** Destroy the block buffer at pbuf. */
void ce_block_buffer_destroy(ce_block_buffer* pbuf);
/** Store up to length bytes starting from data in the block buffer at pbuf.
 *  @return Returns the number of bytes stored in the buffer.
 */
size_t ce_block_buffer_stow(ce_block_buffer* pbuf, const char* data, size_t length);
/** Clear the block buffer at pbuf. */
void ce_block_buffer_clear(ce_block_buffer* pbuf);
/** Returns 0 if the ce_block_buffer_stow would not return 0. */
int ce_block_buffer_full(ce_block_buffer* pbuf);

/** Returns the number of bytes required to store an output 
    for the given  context, or 0 if it is unknown. */
size_t ce_output_size(ce_context* ctx);

/** Allocates enough space for an output generated with the
    given context or returns NULL if it doesn't know what
    to allocate. */
char* ce_output_alloc(ce_context* ctx);

/** Allocates a new execution state object for ctx. */
ce_state* ce_state_alloc(ce_context* ctx);

/** Releases all resources held by *ps */
void ce_state_destroy(ce_state* ps);

/** Destroys *ps and frees ps. */
void ce_state_free(ce_state* ps);

/** Updates the state at ps with data_length bytes starting at
    data.
    @return 0 on success, < 0 otherwise.
*/
int ce_update(ce_state* ps, const char* data, size_t data_length);

/** Finalizes the transformation process on ps. If any unprocessed bytes
    remain, they are transformed  with the padding in the block_buffer.
    
    If output_length is non-NULL, the length of the returned data is
    stored in *output_length. 

    @return Returns a pointer to the result output. The caller owns this value
            and is responsible for calling free on it.
*/
char* ce_finalize(ce_state* ps, size_t* output_length) ; 


/** A convenience function that transforms a complete buffer at once.
    @param output_length If output_length is non-NULL, *output_length
    is set to the length of the result in bytes.
    @param input A pointer to the input data
    @param input_length The number of bytes of input data to process
    @param pctx A pointer to an initialized ce_context to use.
    @return Returns a pointer to the result of the transformation.
    It is the caller's responsibility to free this result using free(3).
    If an error occurs during processing (eg not enough key material), 
    then NULL is returned.
*/
char* ce_encode(ce_context* pctx, const char* input, size_t input_length,
                 size_t* output_length);

#ifdef __cplusplus
}
#endif


#endif // defined Q3P_CENGINE_UTILITY_H_INCLUDED

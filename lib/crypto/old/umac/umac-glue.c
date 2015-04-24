/*
 * umac-glue.c
 *
 * Interface code to connect the reference implementation of the 
 * UMAC algorithm to the q3p crypto engine.
 *
 * Author: Thomas Themel - thomas.themel@ait.ac.at
 * 
 * !! Not intended for immediate translation, see umac-nnn.c !!
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

#include <inttypes.h>
#include <stdlib.h>

// ait
#include "../algorithm.h"
#include "../ids.h"
#include "../utility.h"
#include "../simple-state.h"

/* Include the UMAC implementation. */
#include "umac.h"
#include "umac-impl.c"

/* two macros necessary because DO_CONCAT
   doesn't expand arguments when called directly */
#define DO_CONCAT(a,b) a##b
#define CONCAT(a,b) DO_CONCAT(a,b)

#define DO_TO_STRING(a) #a
#define TO_STRING(a) DO_TO_STRING(a)


#define UMAC_KEY_SIZE 16
#define MAX_STREAM_LENGTH (1 << 24)

/** uhash context. */
struct q3p_uhash_ctx  
{
    ce_context h;
    char* key ; 
} ;

// struct uhash_state
// {
//     ce_state h;
//     /* Used to deal with the reference
//        implementation's limitation to 16MB 
//        streams. */
//     size_t bytes_done;
//     uhash_ctx_t impl;    
// } umac_state ;

/** Transformation function: Passes date on to the internal UHASH buffering
    machinery. */
static int transform (ce_state* ps, const char* data, size_t nblocks)
{   
    struct uhash_state* pst = (struct uhash_state*) ps ; 
    size_t stream_length = pst->bytes_done + nblocks ;

    if (stream_length > MAX_STREAM_LENGTH)
        return -1 ; 

    uhash_update(pst->impl, (char*) data, nblocks) ;

    pst->bytes_done = stream_length ; 

    return 0 ; 
}

/** Destroys and frees an UHASH context. */
static void destroy(ce_context* ctx)
{
    if (!ctx) return;
    struct q3p_uhash_ctx* pctx = (struct q3p_uhash_ctx*)ctx;
    if (pctx->key) free(pctx->key);
    free(ctx);
}

static ce_context* create_context(const char* key, size_t keysize);

ce_algorithm CONCAT(uhash_, UMAC_BITS) = 
{ 0x01 + UMAC_BITS,
  "uhash" TO_STRING(UMAC_BITS),
  TO_STRING(UMAC_BITS) "bit uhash",
  UMAC_BITS, 
  8,
  create_context
} ; 


/** Finalizes an UHASH computation. */
static char* finalize_state (ce_state* state, size_t* output_length)
{
    char* ret = malloc(UMAC_OUTPUT_LEN);
    if (output_length != NULL)
        *output_length = UMAC_OUTPUT_LEN ;

    struct uhash_state* pst = (struct uhash_state*) state ; 
    uhash_final(pst->impl, ret);
    return ret ; 
}


static void uhash_state_destroy(ce_state* ps)
{
    struct uhash_state* pst = (struct uhash_state*) ps ; 
    uhash_free(pst->impl);
    simple_state_destroy(&pst->h);
}


static ce_state* uhash_state_alloc(ce_context* pctx)
{
    struct q3p_uhash_ctx* ctx = 
        (struct q3p_uhash_ctx*) pctx;
    struct uhash_state* pst = (struct uhash_state*)
        malloc(sizeof(struct uhash_state));
    
    simple_state_init(pctx, &pst->h);
 
    pst->bytes_done = 0;
    pst->impl = uhash_alloc(ctx->key);
    pst->h.destroy = uhash_state_destroy ; 
               
    return &pst->h;
}

/** Creates an evaluation hash context. 
    @return Returns a pointer to the new context if keysize is correct.
    @return NULL if keysize is incorrect. */
static ce_context* create_context(const char* key, size_t keysize)
{
    if (keysize != UMAC_KEY_SIZE)
    {
        return NULL ; 
    }

    struct q3p_uhash_ctx* pctx = malloc(sizeof(struct q3p_uhash_ctx));    
    pctx->key = malloc(UMAC_KEY_SIZE);
    memcpy(pctx->key, key, UMAC_KEY_SIZE);

    pctx->h.algorithm = &CONCAT(uhash_, UMAC_BITS);
    pctx->h.transform = transform ;
    pctx->h.create_state = uhash_state_alloc ; 
    pctx->h.finalize_state = finalize_state ;
    pctx->h.destroy = destroy ; 

    return &pctx->h ;
}


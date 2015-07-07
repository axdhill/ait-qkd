/*
 * utility.c
 *
 * TODO: comment here what this file does
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
#include <stdlib.h>
#include <string.h>

// ait
#include "algorithm.h"
#include "utility.h"

size_t ce_output_size(ce_context* ctx)
{
    return (ctx->algorithm->output_bits+7)/8;
}

char* ce_output_alloc(ce_context* ctx)
{
    size_t size = ce_output_size(ctx);

    if (size > 0)
    {
        return static_cast<char *>(malloc(size));
    }
    
    return NULL ; 
}

/** Initializes a block buffer of capacity capacity at *pbuf. */
void ce_block_buffer_init(ce_block_buffer* pbuf, size_t capacity)
{
    pbuf->capacity = capacity ; 
    pbuf->hold = (char*) calloc(capacity, 1) ; 
    pbuf->fill = 0 ; 
}

/** Allocates a block buffer with the given capacity and returns
    a pointer to the new buffer. */
ce_block_buffer* ce_block_buffer_alloc(size_t capacity)
{
    ce_block_buffer* ret = (ce_block_buffer*)malloc(sizeof(ce_block_buffer));
    if (ret) {
        ce_block_buffer_init(ret, capacity);
    }
    return ret ; 
}

/** Destroys the block buffer at *pbuf and frees pbuf. */
void ce_block_buffer_free(ce_block_buffer* pbuf)
{
    if (pbuf != NULL)
    {
        ce_block_buffer_destroy(pbuf);
        free(pbuf);
    }
}

/** Destroys the block buffer at *pbuf. */
void ce_block_buffer_destroy(ce_block_buffer* pbuf)
{
    free(pbuf->hold);
}

/** Stores data in a block buffer. If length bytes are free in the
    block buffer, length bytes are stored. Otherwise, the block
    buffer is filled to capacity.

    @return Returns the number of bytes stored in the block buffer. */
size_t ce_block_buffer_stow(ce_block_buffer* pbuf, const char* data, size_t length)
{
    size_t free = pbuf->capacity - pbuf->fill ;
    size_t tocopy = free > length ? length : free ;

    memcpy(pbuf->hold + pbuf->fill, data, tocopy);
    pbuf->fill += tocopy ; 

    return tocopy ; 
}
/** Resets a block buffer. Sets the fill count to zero
   and all the buffer bytes to 0.*/
void ce_block_buffer_reset(ce_block_buffer* pbuf)
{
    memset(pbuf->hold,0,pbuf->capacity);
    pbuf->fill = 0 ;
}

/** Boolean: Returns a value != 0 iff pbuf is filled to capacity. */
int ce_block_buffer_full(ce_block_buffer* pbuf)
{
    return pbuf->capacity == pbuf->fill ; 
}

ce_state* ce_state_alloc(ce_context* ctx)
{
    return ctx->create_state(ctx);
}

void ce_state_destroy(ce_state* ps)
{
   assert(ps);
   ps->destroy(ps);
}

/** Destroys the algorithm_state at *ps and frees ps. */
void ce_state_free(ce_state* ps)
{
    if (ps != NULL)
    {
        ce_state_destroy(ps);
        free(ps);
    }
}

int ce_update(ce_state* ps, const char* data, size_t data_length)
{
    int rc = 0 ; 

    if (ps->buf.fill != 0)
    {
        size_t added = ce_block_buffer_stow(&ps->buf, data, data_length) ;
        data += added ; 
        data_length -= added ; 
        if (ce_block_buffer_full(&ps->buf))
        {
            rc = ps->pctx->transform(ps, ps->buf.hold, 1);
            ce_block_buffer_reset(&ps->buf);
        }
    }


    if (rc != 0)
    {
        return rc  ;
    }

    if (data_length >= ps->buf.capacity)
    {
        unsigned int nblocks = data_length / ps->buf.capacity ; 
        rc = ps->pctx->transform(ps, data, nblocks);
        data += nblocks * ps->buf.capacity ; 
        data_length -= nblocks * ps->buf.capacity ; 
    }

    if (data_length)
    {
        ce_block_buffer_stow(&ps->buf, data, data_length);
    }

    return rc ;
}

char* ce_finalize(ce_state* ps, size_t* output_length)
{
    if (ps->buf.fill > 0)
    {
        if (ps->pctx->transform(ps, ps->buf.hold, 1) != 0)
        {
            return NULL ; 
        }
    }

    return ps->pctx->finalize_state(ps, output_length);
}

char* ce_encode(ce_context* pctx, const char* data, size_t data_length, 
             size_t *output_length)
{   
    int rc ;
    char* result ;
    ce_state* ps = pctx->create_state(pctx);

    rc = ce_update(ps, data, data_length) ;
    result = ce_finalize(ps, output_length);

    ce_state_free(ps);

    if (rc == 0)
        return result ;
   
    free(result);
    return NULL ; 
}

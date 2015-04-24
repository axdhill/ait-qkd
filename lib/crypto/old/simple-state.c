/*
 * simple-state.c
 *
 * Base class functionality for algorithm state objects
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

// ait
#include "algorithm.h"
#include "context.h"
#include "utility.h"

#include "simple-state.h"


ce_state* simple_state_alloc(ce_context* ctx)
{
    assert(ctx);

    ce_state* ps = (ce_state*)malloc(sizeof(ce_state));
    if (!ps) {
        return NULL ; 
    }

    simple_state_init(ctx, ps);

    return ps ; 
}

void simple_state_destroy(ce_state* ps)
{
   assert(ps);
   ce_block_buffer_destroy(&ps->buf);
   free(ps->output);
}

void simple_state_init(ce_context* ctx, ce_state* ps)
{
    ps->pctx = ctx ;
    ps->output = NULL ;
    ce_block_buffer_init(&ps->buf, (ctx->algorithm->block_bits+7)/8);
    ps->destroy = simple_state_destroy ; 
}

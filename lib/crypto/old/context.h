/*
 * context.h
 *
 * functions for thq q3p crypto engine context
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


#ifndef __Q3P_CE_CONTEXT_H
#define __Q3P_CE_CONTEXT_H

#ifdef __cplusplus
extern "C" {
#endif

/** Initializes the crypto engine. */
void ce_initialize(void);


/** Creates a new context for a given algorithm and key. */
ce_context* ce_context_create(unsigned int algorithm_id, 
                         const char* key, size_t keybytes);

/** Frees an algorithm context. */
void ce_context_free(ce_context* ctx); 

#ifdef __cplusplus
}
#endif


#endif // defined Q3P_CE_H_INCLUDED

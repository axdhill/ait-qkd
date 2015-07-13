/*
 * simple-state.h
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

#ifndef SIMPLE_STATE_H_INCLUDED
#define SIMPLE_STATE_H_INCLUDED

/** Allocates a new execution state object for ctx. */
ce_state* simple_state_alloc(ce_context* ctx);

/** Initializes a standard execution state object for ctx at ps.
    This should not be called explicitly by users but is rather
    intended as a "base constructor" for algorithm implementations. */
void simple_state_init(ce_context* ctx, ce_state* ps);

/** Releases all resources held by a standard execution state object.
    This should not be called explicitly by users but is rather
    intended as a "base destructor" for algorithm implementations. */
void simple_state_destroy(ce_state* ps);



#endif // !defined  SIMPLE_STATE_H_INCLUDED

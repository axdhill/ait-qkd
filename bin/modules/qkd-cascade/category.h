/*
 * category.h
 * 
 * A cascade category describes a quality during a cascade pass
 * 
 * Author: Philipp Grabenweger
 *         Christoph Pacher, <christoph.pacher@ait.ac.at>
 *         Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *
 * Copyright (C) 2014-2015 AIT Austrian Institute of Technology
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


#ifndef __QKD_MODULE_QKD_CASCADE_CATEGORY_H
#define __QKD_MODULE_QKD_CASCADE_CATEGORY_H


// ------------------------------------------------------------
// incs

#include <inttypes.h>


// ------------------------------------------------------------
// decl


/**
 * a category contains a pass qualification during cascade
 */
class category {


public:


    /**
     * ctor
     */
    category() : size(0), k(0), diffparity_must_be_even(false) {};


    /**
     * category size 
     */
    uint64_t size;                      


    /**
     * size of parity check blocks in which this category should be divided
     */
    uint64_t k;                             
   

    /**
     * states whether the total differential parity sum between Alice and Bob 
     * for all bits in this category must be even 
     */
    bool diffparity_must_be_even;       
};


#endif


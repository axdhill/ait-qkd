/*
 * random_c_api.cpp
 * 
 * implement the random source using srand() and rand()
 * 
 * Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
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

/*
 * TODO: optimize by reading in full sizeof(int) into the
 *       buffer at random_c_api::get instead of single chars
 */

 
// ------------------------------------------------------------
// incs

// ait
#include <qkd/utility/random.h>

#include "random_c_api.h"


using namespace qkd::utility;


// ------------------------------------------------------------
// code



/**
 * get a block of random bytes
 * 
 * This function must be overwritten in derived classes
 * 
 * @param   cBuffer     buffer which will hold the bytes
 * @param   nSize       size of buffer in bytes
 */
void random_c_api::get(char * cBuffer, uint64_t nSize) {

    // do not proceed if nothing to do
    if (!cBuffer) return;
    if (nSize == 0) return;
    
    // read in sequentially
    uint64_t nRead = 0;
    while (nRead < nSize) {
        cBuffer[nRead++] = rand();
    }
}


/**
 * init the object
 */
void random_c_api::init() {
    
    // hence: if you create more than one C API 
    //        random objects in the same second
    //        they will yield always the very same
    //        sequence of random numbers
    
    srand(time(nullptr));
}

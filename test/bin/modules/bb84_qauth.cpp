/*
 * bb84_qauth.cpp
 * 
 * This is a test file.
 *
 * TEST: this bb84 QAuth implementation
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


#if defined(__GNUC__) || defined(__GNUCPP__)
#   define UNUSED   __attribute__((unused))
#else
#   define UNUSED
#endif


// ------------------------------------------------------------
// incs

#include <fstream>
#include <iostream>

// include the all-in-one header
#include <qkd/qkd.h>

#include "../bin/modules/qkd-sifting-bb84/qauth.h"


// ------------------------------------------------------------
// code


int test() {
    
    qauth_values cQAuthA;
    qauth_values cQAuthB;
    
    qauth_init cQAuthInit1 = qauth_init{ 1, 2, 16, 4, (uint32_t)bb84_base::BB84_BASE_DIAGONAL };

    cQAuthA = qauth(cQAuthInit1).create_min(128);
    std::cerr << cQAuthA.str("cQAuthA: ") << std::endl;
    
    cQAuthB = qauth(cQAuthInit1).create_max(128 + 15);
    std::cerr << cQAuthB.str("cQAuthB: ") << std::endl;

    qauth_init cQAuthInit2 = qauth_init{ 1234, 287, 16, 0, (uint32_t)bb84_base::BB84_BASE_RECTILINEAR };

    cQAuthA = qauth(cQAuthInit2).create_min(128);
    std::cerr << cQAuthA.str("cQAuthA: ") << std::endl;
    
    cQAuthB = qauth(cQAuthInit2).create_max(128 + 15);
    std::cerr << cQAuthB.str("cQAuthB: ") << std::endl;

    return true;
}


int main(UNUSED int argc, UNUSED char** argv) {
    
    qkd::utility::debug::enabled() = true;
    
    return test();
}

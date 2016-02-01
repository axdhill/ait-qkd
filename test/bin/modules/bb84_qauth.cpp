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
    
    qauth_init cQAuthInit1 = qauth_init{ 1, 2, 16, 4, (bb84_base)bb84_base::BB84_BASE_DIAGONAL };
    qauth_ptr cQAuthAPtr;
    qauth_ptr cQAuthBPtr;
    qauth_data_particles cQAuthA;
    qauth_data_particles cQAuthB;

    cQAuthAPtr = qauth_ptr(new qauth(cQAuthInit1));
    cQAuthA = cQAuthAPtr->create_min(128);
    cQAuthA.dump(std::cerr, "cQAuthA: ");
    std::cerr << std::endl;
    
    cQAuthBPtr = qauth_ptr(new qauth(cQAuthInit1));
    cQAuthB = cQAuthBPtr->create_max(128 + 11);
    cQAuthB.dump(std::cerr, "cQAuthB: ");
    std::cerr << std::endl;

    qauth_init cQAuthInit2 = qauth_init{ 1234, 287, 16, 0, (bb84_base)bb84_base::BB84_BASE_RECTILINEAR };

    cQAuthAPtr = qauth_ptr(new qauth(cQAuthInit2));
    cQAuthA = cQAuthAPtr->create_min(128);
    cQAuthA.dump(std::cerr, "cQAuthA: ");
    std::cerr << std::endl;
    
    cQAuthBPtr = qauth_ptr(new qauth(cQAuthInit2));
    cQAuthB = cQAuthBPtr->create_max(128 + 11);
    cQAuthB.dump(std::cerr, "cQAuthB: ");
    std::cerr << std::endl;

    return true;
}

int main(UNUSED int argc, UNUSED char** argv) {
    return test();
}

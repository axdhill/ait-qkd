/*
 * retio.cpp
 * 
 * This is a test file.
 *
 * TEST: test the qkd::utility::ratio method
 *
 * Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *
 * Copyright (C) 2013-2015 AIT Austrian Institute of Technology
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

#include <iostream>
#include <ratio>

// include the all-in-one header
#include <qkd/qkd.h>


// ------------------------------------------------------------
// code


int test() {

    std::atto  a;
    std::femto f;
    std::pico  p;
    std::nano  n;
    std::micro mu;
    std::milli m;
    std::centi c;
    std::deci  d;
    std::deca  da;
    std::hecto h;
    std::kilo  k;
    std::mega  M;
    std::giga  G;
    std::tera  T;
    std::peta  P;
    std::exa   E;
    
    assert(qkd::utility::ratio_to_string(a) == "a");
    assert(qkd::utility::ratio_to_string(f) == "f");
    assert(qkd::utility::ratio_to_string(p) == "p");
    assert(qkd::utility::ratio_to_string(n) == "n");
    assert(qkd::utility::ratio_to_string(mu) == "mu");
    assert(qkd::utility::ratio_to_string(m) == "m");
    assert(qkd::utility::ratio_to_string(c) == "c");
    assert(qkd::utility::ratio_to_string(d) == "d");
    assert(qkd::utility::ratio_to_string(da) == "da");
    assert(qkd::utility::ratio_to_string(h) == "h");
    assert(qkd::utility::ratio_to_string(k) == "k");
    assert(qkd::utility::ratio_to_string(M) == "M");
    assert(qkd::utility::ratio_to_string(G) == "G");
    assert(qkd::utility::ratio_to_string(T) == "T");
    assert(qkd::utility::ratio_to_string(P) == "P");
    assert(qkd::utility::ratio_to_string(E) == "E");
    
    return 0;
}


int main(UNUSED int argc, UNUSED char** argv) {
    return test();
}


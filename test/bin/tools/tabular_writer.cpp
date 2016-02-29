/*
 * tabular_writer.cpp
 * 
 * This is a test file.
 *
 * TEST: test the tabular_writer class in qkd-view
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
#include <sstream>

#include <qkd/utility/properties.h>

#include "../../../bin/tools/qkd-view/tabular_writer.h"


// ------------------------------------------------------------
// code


inline std::string mul_str(std::string const & s, unsigned int i) {
    
    std::stringstream ss;
    for (unsigned j = 0; j < i; ++j) {
        ss << s;
    }
    return ss.str();
}


int test() {
    
    std::string sNumber[] = { "zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine" };
    
    std::map<std::string, qkd::utility::properties> d;
    for (int i = 0; i < 10; ++i) {
        
        qkd::utility::properties p;
        p["alpha"] = "eins " + mul_str("1", 1);
        p["beta"] = "zwei" + mul_str("2", 2);
        p["gamma"] = "drei " + mul_str("3", 3);
        p["delta"] = "vier " + mul_str("4", 4);
        p["epsilon"] = "fuenf " + mul_str("5", i + 5);
        p["zeta"] = "sechs " + mul_str("6", i + 6);
        
        d[sNumber[i]] = p;
    }
    
    std::list<std::string> f = { "gamma", "delta", "alpha", "zeta" };
    
    tabular_writer tw(std::cout, d, f, true, "test: ");
    
    return 0;
}

int main(UNUSED int argc, UNUSED char** argv) {
    return test();
}


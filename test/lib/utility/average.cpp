/*
 * average.cpp
 * 
 * This is a test file.
 *
 * TEST: test the qkd::average class
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

#include <iostream>

// include the all-in-one header
#include <qkd/qkd.h>


// ------------------------------------------------------------
// code


/**
 * Sometimes being approximately equal is enough. Plus, trying to avoid false errors due to the approximative nature
 * of float arithmetic.
 * 
 * @return  true iff the difference between both numbers is below 0.001; false otherwise.
 */
bool approximately_equal(double a, double b) {
    return std::abs(a - b) < 0.001;
}


void test() {
    
    double nAverage = 0.0;

    // sample values
    double nValues[20] = { 
        1.0, 2.0, 3.1, 200.3, 90.0, 23.4, 58.3, 834.0, 193.13, 98.321, 
        34.55, 48.358, 23.42, 7754.3, 2.40489, 13.4, 2.94, 0.323, 3.44, 0.0
    };
    
    // dump result into a stringstream
    std::stringstream ss;
    ss.setf(std::ios::fixed, std::ios::floatfield);
    ss.precision(4);
    
    // the average object tested
    qkd::utility::average cAverage;
    
    // --- average over the last N values ---

    // create a average value object of the last 5 inserted values
    cAverage = qkd::utility::average_technique::create("value", 5);
    
    // walk over the values and collect the results
    for (uint64_t i = 0; i < 20; i++) {
        cAverage << nValues[i];
        nAverage = cAverage->avg();
        ss << nAverage << std::endl;
    }

    // check result
    assert(ss.str() ==
"1.0000\n\
1.5000\n\
2.0333\n\
51.6000\n\
59.2800\n\
63.7600\n\
75.0200\n\
241.2000\n\
239.7660\n\
241.4302\n\
243.6602\n\
241.6718\n\
79.5558\n\
1591.7898\n\
1572.6066\n\
1568.3766\n\
1559.2930\n\
1554.6736\n\
4.5016\n\
4.0206\n");
    

    // --- average over the last 250 millisec values ---
    
    // create a average value object of the last 250 millisecs
    cAverage = qkd::utility::average_technique::create("time", 250);
    
    // empty output buffer
    ss.str("");
    
    // walk over the values and collect the results
    for (uint64_t i = 0; i < 20; i++) {
        
        cAverage << nValues[i];
        nAverage = cAverage->sum();
        ss << nAverage << std::endl;
        
        // sleep for 150 millisecs
        usleep(150 * 1000);
    }
    
    // check result
    assert(ss.str() ==
"1.0000\n\
3.0000\n\
5.1000\n\
203.4000\n\
290.3000\n\
113.4000\n\
81.7000\n\
892.3000\n\
1027.1300\n\
291.4510\n\
132.8710\n\
82.9080\n\
71.7780\n\
7777.7200\n\
7756.7049\n\
15.8049\n\
16.3400\n\
3.2630\n\
3.7630\n\
3.4400\n");
    
    // get unknown average algorithm
    try {
        cAverage = qkd::utility::average_technique::create("john_doe", 0);
        assert("unknown average algorithm");
    }
    catch (qkd::utility::average_technique::average_technique_unknown & cException) {}
}


void test_high_and_low() {
    
    qkd::utility::average cAverage;
    double series[] = { 3.14, 15.9, 26.53, 5.89, 7.93, 2.3, 84.6, 2.6, .433, 8.3 };

    // TEST WITH TIME-BASED AVERAGES, WINDOW SIZE 250ms, SLEEP 150ms PER ITERATION
    double expected_highs_t[] = { 3.14, 15.9, 26.53, 26.53, 7.93, 7.93, 84.6, 84.6, 2.6, 8.3 };
    double expected_lows_t [] = { 3.14, 3.14, 15.9, 5.89, 5.89, 2.3, 2.3, 2.6, .433, .433 };
    cAverage = qkd::utility::average_technique::create("time", 250);

    assert(cAverage->min() == 0.0);
    assert(cAverage->max() == 0.0);

    for (int i = 0; i < 10; i++) {
        cAverage << series[i];
        assert(approximately_equal(cAverage->max(), expected_highs_t[i]));
        assert(approximately_equal(cAverage->min(), expected_lows_t  [i]));
        usleep(150 * 1000);
    }

    // TEST WITH VALUE-BASED AVERAGES, WINDOW SIZE 3 VALUES
    double expected_highs_v3[] = { 3.14, 15.9, 26.53, 26.53, 26.53, 7.93, 84.6, 84.6, 84.6, 8.3 };
    double expected_lows_v3 [] = { 3.14, 3.14, 3.14, 5.89, 5.89, 2.3, 2.3, 2.3, .433, .433 };
    cAverage = qkd::utility::average_technique::create("value", 3);

    assert(cAverage->min() == 0.0);
    assert(cAverage->max() == 0.0);

    for (int i = 0; i < 10; i++) {
        cAverage << series[i];
        assert(approximately_equal(cAverage->max(), expected_highs_v3[i]));
        assert(approximately_equal(cAverage->min(), expected_lows_v3  [i]));
    }
}


int main(UNUSED int argc, UNUSED char** argv) {
    test();
    test_high_and_low();
    return 0;
}

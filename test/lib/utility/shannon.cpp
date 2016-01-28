/*
 * shannon.cpp
 * 
 * This is a test file.
 *
 * TEST: test the qkd::utility::shannon functions
 *
 * Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *
 * Copyright (C) 2013-2016 AIT Austrian Institute of Technology
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

#include <float.h>

#include <fstream>
#include <iostream>
#include <string>

// include the all-in-one header
#include <qkd/qkd.h>


// ------------------------------------------------------------
// code


int test() {

    // invalid/extrem cases
    assert(qkd::utility::binary_entropy(1.0) == 0);
    assert(qkd::utility::binary_entropy(0.0) == 0);
    assert(std::isnan(qkd::utility::binary_entropy(1.5)));
    assert(std::isnan(qkd::utility::binary_entropy(-0.1)));
    assert(!std::isnan(qkd::utility::binary_entropy(0.1)));
    
    // 1/2 error
    assert(qkd::utility::binary_entropy(0.5) == 1.0);
    
    // some erro rrates
    double nErrorRate[3] = { 0.10, 0.05, 0.01 };
    
    // calculate the shannon limits
    double nShannonLimit[3];
    for (int i = 0; i < 3; i++) nShannonLimit[i] = qkd::utility::binary_entropy(nErrorRate[i]);
    
    // verify shannon limits (to a fraction of 1000)
    double nShannonLimitResult[3] = { 0.468996, 0.286397, 0.0807931 };
    for (int i = 0; i < 3; i++) assert((int)(nShannonLimitResult[i] * 1000) == (int)(nShannonLimit[i] * 1000));
    
    // some disclosed rates
    double nDisclosedRate[4] = { 0.10, 0.20, 0.40, 0.60 };
    
    // expected efficiency by disclosed and error
    double nEfficiencyResult[4][3];
    
    nEfficiencyResult[0][0] = 0.213222;
    nEfficiencyResult[0][1] = 0.349166;
    nEfficiencyResult[0][2] = 1.23773;

    nEfficiencyResult[1][0] = 0.426443;
    nEfficiencyResult[1][1] = 0.698331;
    nEfficiencyResult[1][2] = 2.47546;
    
    nEfficiencyResult[2][0] = 0.852886;
    nEfficiencyResult[2][1] = 1.39666;
    nEfficiencyResult[2][2] = 4.95092;
    
    nEfficiencyResult[3][0] = 1.27933;
    nEfficiencyResult[3][1] = 2.09499;
    nEfficiencyResult[3][2] = 7.42637;
    
    // efficiency
    double nEfficiency[4][3];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 3; j++) {
            nEfficiency[i][j] = qkd::utility::shannon_efficiency(nErrorRate[j], nDisclosedRate[i]);
            assert((int)(nEfficiencyResult[i][j] * 1000) == (int)(nEfficiency[i][j] * 1000));
        }
    }
    
    return 0;
}


int main(UNUSED int argc, UNUSED char** argv) {
    return test();
}


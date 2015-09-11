/*
 * random_functions.cpp
 * 
 * random number distribution functions
 * 
 * Author: Mario Kahn
 *         Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *         Philipp Grabenweger
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


// ------------------------------------------------------------
// incs

// ait
#include <qkd/utility/random.h>

#include "random_functions.h"


using namespace qkd::simulate;


// -----------------------------------------------------------------
// code


/**
 * return a random number which is exponentional distributed
 * 
 * @param   nMu         exponentional distribution param (mean value of random number)
 */
double random_functions::random_exponential(double nMu) {
    
    double nRandom = 0.0;
    qkd::utility::random_source::source() >> nRandom;
    
    // avoid log(0.0) calculation afterwards
    if (nRandom == 0.0) nRandom = 1.0;
    
    return (-nMu * log(nRandom));
}


/**
 * return a random number which is normal distributed
 * 
 * @param   nMu         normal mu    (mean value)
 * @param   nSigma      normal sigma (standard deviation)
 */
double random_functions::random_gaussian(double nMu, const double nSigma) {

    double nUniform1 = 0.0;
    double nUniform2 = 0.0;
    double nRadius = 0.0;

    // polar method according to George Marsaglia and Thomas A. Bray
    
    do {

        // choose nUniform1, nUniform2 in uniform square (-1,-1) to (+1,+1) 
        qkd::utility::random_source::source() >> nUniform1;
        qkd::utility::random_source::source() >> nUniform2;
        
        nUniform1 = -1 + 2 * nUniform1;
        nUniform2 = -1 + 2 * nUniform2;

        // see if it is in the unit circle
        nRadius = nUniform1 * nUniform1 + nUniform2 * nUniform2;
        
    } while (nRadius > 1.0 || nRadius == 0);

    return nMu + nSigma * nUniform2 * sqrt(-2.0 * log(nRadius) / nRadius);
}


/**
 * return a random number which is uniformly distributed in the range 0 to 1
 */
double random_functions::random_uniform() {
    
    double nRandom = 0.0;
    
    qkd::utility::random_source::source() >> nRandom;
    
    return nRandom;
}


/**
 * return a random integer number which is uniformly distributed in the range 0 to (nVals - 1)
 * 
 * @param   nVals         number of possible values
 */
uint64_t random_functions::random_uniform_int(uint64_t nVals) {
    
    double nRandom = 0.0;
    
    qkd::utility::random_source::source() >> nRandom;
    
    return (static_cast<uint64_t>(nRandom * (double) nVals) % nVals);
}


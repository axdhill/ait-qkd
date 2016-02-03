/*
 * random_functions.h
 * 
 * random number distribution functions
 * 
 * Author: Mario Kahn
 *         Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *         Philipp Grabenweger
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


#ifndef __QKD_QKD_SIMULATE_RANDOM_FUNCTIONS_H_
#define __QKD_QKD_SIMULATE_RANDOM_FUNCTIONS_H_


// -----------------------------------------------------------------
// incs

#include <inttypes.h>


// -----------------------------------------------------------------
// decl


namespace qkd {
    
namespace simulate {

/**
 * random number distribution functions
 */
class random_functions {
    
    
public:
    
    
    /**
     * return a random number which is exponentially distributed
     * 
     * @param   nMu         exponential distribution param (mean value of random number)
     */
    static double random_exponential(double nMu);

    
    /**
     * return a random number which is normal distributed
     * 
     * @param   nMu         normal mu    (mean value)
     * @param   nSigma      normal sigma (standard deviation)
     */
    static double random_gaussian(double nMu, const double nSigma);
    
    
    /**
     * return a random number which is uniformly distributed in the range 0 to 1
     */
    static double random_uniform();
    
    
    /**
     * return a random integer number which is uniformly distributed in the range 0 to (nVals - 1)
     * 
     * @param   nVals         number of possible values
     */
    static uint64_t random_uniform_int(uint64_t nVals);
};
    
}
}

#endif

/*
 * shannon.h
 * 
 * handy Shannon limit calculus
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

 

#ifndef __QKD_UTILITY_SHANNON_H_
#define __QKD_UTILITY_SHANNON_H_


// ------------------------------------------------------------
// incl

#include <inttypes.h>
#include <math.h>

// ait
#include <qkd/utility/bigint.h>


// ------------------------------------------------------------
// decls


namespace qkd {
    
namespace utility {    


/**
 * caluclate the binary entropy
 * 
 * .. formerly known as "Shannon Limit"
 *
 * That is: the number of bits needed by a given error rate
 *          to successfully correct a single key bit
 * 
 * The resulted value is the lowest possible value:
 *      The Shannon Limit
 * 
 * @param   nErrorRate          error rate (or propability p(0))
 * @return  the binary entropy or std::numeric_limits< double >::quiet_NaN() for error
 * */
double binary_entropy(double nErrorRate);


/**
 * get the channel capacity with respect to an error rate
 * 
 * @param   nErrorRate      error rate of the channel
 * @return  the capacity of a channel
 */
double channel_capacity(double nErrorRate);


/**
 * get the disclosed rate 
 * 
 * (stupid easy but nifty)
 * 
 * @param   nBits           total bits
 * @param   nDisclosedBits  number of disclosed bits (nDisclosedBits <= nBits)
 * @return  the disclosed rate
 */
double disclosed_rate(uint64_t nBits, uint64_t nDisclosedBits);


/**
 * calculate the multiplicative gap
 * 
 * @param   nBits           total nbits
 * @param   nDisclosedBits  number of disclosed bits (nDisclosedBits <= nBits)
 * @param   nErrorRate      error rate of the channel
 * @return  the multiplicative_gap
 */
double multiplicative_gap(uint64_t nBits, uint64_t nDisclosedBits, double nErrorRate);


/**
 * calculate the relative inefficiency
 * 
 * @param   nBits           total nbits
 * @param   nDisclosedBits  number of disclosed bits (nDisclosedBits <= nBits)
 * @param   nErrorRate      error rate of the channel
 * @return  the relative inefficiency
 */
double relative_inefficiency(uint64_t nBits, uint64_t nDisclosedBits, double nErrorRate);


/**
 * get the shannon limit on a binary symetric channel
 * 
 * @param   nErrorRate      error rate of the binary symetric channel
 * @return  actually binary_entropy
 */
inline double shannon_limit_bsc(double nErrorRate) { return binary_entropy(nErrorRate); }


/**
 * caluclate the error correction efficiency
 *
 * @param   nErrorRate          error rate detected
 * @param   nDisclosedRate      bits disclosed compared to all bits to get there
 * @return  the efficiency compared to the Shannon limit
 * */
double shannon_efficiency(double nErrorRate, double nDisclosedRate);


}

}

#endif


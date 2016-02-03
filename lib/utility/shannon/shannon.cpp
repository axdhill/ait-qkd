/*
 * shannon.cpp
 * 
 * implementation handy Shannon limit calculus
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

 
// ------------------------------------------------------------
// incs

// ait
#include <qkd/utility/shannon.h>


// ------------------------------------------------------------
// code


/**
 * calculate the binary entropy
 * 
 * .. formerly known as "Shannon Limit"
 *
 * That is: the number of bits needed by a given error rate
 *          to successfully correct a single key bit
 * 
 * The resulted value is the lowest possible value:
 *      The Shannon Limit
 * 
 * @param   nErrorRate          error rate
 * @return  the binary entropy or std::numeric_limits< double >::quiet_Nan() for error
 * */
double qkd::utility::binary_entropy(double nErrorRate) {
    
    // error rate must not exceed 1.0 and must be greater to 0.0
    if (nErrorRate < 0.0) return std::numeric_limits<double>::quiet_NaN();
    if (nErrorRate > 1.0) return std::numeric_limits<double>::quiet_NaN();
    if (nErrorRate == 0.0) return 0.0;
    if (nErrorRate == 1.0) return 0.0;
    
    return (-nErrorRate * log2(nErrorRate) - (1.0 - nErrorRate) * log2(1.0 - nErrorRate));
}


/**
 * get the channel capacity with respect to an error rate
 * 
 * @param   nErrorRate      error rate of the channel
 * @return  the capacity of a channel
 */
double qkd::utility::channel_capacity(double nErrorRate) { 
    return 1.0 - shannon_limit_bsc(nErrorRate); 
}


/**
 * get the disclosed rate 
 * 
 * (stupid easy but nifty)
 * 
 * @param   nBits           total bits
 * @param   nDisclosedBits  number of disclosed bits (nDisclosedBits <= nBits)
 * @return  the disclosed rate
 */
double qkd::utility::disclosed_rate(uint64_t nBits, uint64_t nDisclosedBits) { 
    return (double)nDisclosedBits / (double)nBits; 
}


/**
 * calculate the multiplicative gap
 * 
 * @param   nBits           total nbits
 * @param   nDisclosedBits  number of disclosed bits (nDisclosedBits <= nBits)
 * @param   nErrorRate      error rate of the channel
 * @return  the multiplicative_gap
 */
double qkd::utility::multiplicative_gap(uint64_t nBits, uint64_t nDisclosedBits, double nErrorRate) { 
    return 1.0 - (1.0 - relative_inefficiency(nBits, nDisclosedBits, nErrorRate)); 
}


/**
 * calculate the relative inefficiency
 * 
 * @param   nBits           total nbits
 * @param   nDisclosedBits  number of disclosed bits (nDisclosedBits <= nBits)
 * @param   nErrorRate      error rate of the channel
 * @return  the relative inefficiency
 */
double qkd::utility::relative_inefficiency(uint64_t nBits, uint64_t nDisclosedBits, double nErrorRate) { 
    return disclosed_rate(nBits, nDisclosedBits) / shannon_limit_bsc(nErrorRate); 
}


/**
 * calculate the error correction efficiency
 *
 * @param   nErrorRate          error rate detected
 * @param   nDisclosedRate      bits disclosed compared to all bits to get there
 * @return  the efficiency compared to the Shannon limit
 * */
double qkd::utility::shannon_efficiency(double nErrorRate, double nDisclosedRate) {
    return (nDisclosedRate / binary_entropy(nErrorRate));
}



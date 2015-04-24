/* 
 * ntt.h
 * 
 * number theoretical transforms
 * 
 * Autor: Christoph Pacher
 *
 * Copyright (C) 2010-2015 AIT Austrian Institute of Technology
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


#ifndef __QKD_UTILITY_NTT_H
#define __QKD_UTILITY_NTT_H


// ------------------------------------------------------------
// incs


#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif

#include <inttypes.h>

// ait
#include <qkd/utility/bigint.h>


// ------------------------------------------------------------
// decl


typedef uint32_t mod;
typedef uint64_t longmod;


/**
 * returns ceil(ld(x))
 * returns 0xFFFFFFFF if input == 0
 * 
 * @param   x       input
 * @return  ceil(ld(x))
 */
uint32_t ld_ceil(uint32_t x);


/**
 * floor(ld(x))
 * Return 0xFFFFFFFFF if input == 0.
 *
 * @param   x       input
 * @return  floor(ld(x))
 */
uint32_t ld_floor(uint32_t x);


/**
 * copies a bigint into an array consisting of mod variables for NTT.
 * 
 * the bit at position pos of the BIGINT is written to position 
 * pos in the modArray.
 * 
 * if reverseOrder is set, it is written in reverse order.
 *
 * @param   nModArray       the output array
 * @param   cBI             the input bigint
 * @param   bReverseOrder   whether order should be reversed during the copy
 */
void mod_from_bigint(mod * nModArray, qkd::utility::bigint const & cBI, bool bReverseOrder);


/** 
 * Performs cyclic convolution with an ntt algorithm.
 * 
 * nArray1 folded with nArray2 = 1 / length * NTT^(-1)[NTT(nArray1) * NTT(nArray2)]
 * (ntt = fft over a finite field)
 *
 * @param   nArray1         first input array and result array
 * @param   nArray2         second input array
 * @param   nLog2Length     base-2 log of array lengths (of both nArray1 and nArray2)
 */
void ntt_convolution(mod * nArray1, mod * nArray2, const uint32_t nLog2Length);


#endif

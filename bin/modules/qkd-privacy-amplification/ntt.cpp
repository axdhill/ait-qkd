/* 
 * ntt.cpp
 * 
 * implementation file for number theoretical transforms

 * Author: Christoph Pacher
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


/*
 * details of finite field(s) used
 *
 * we consider finite fields with p elements (p is prime).
 * we search for primes p such that the identity
 *
 * a^(2^b) = 1 (mod p) holds for large b.
 *
 * a^(2^b)=1 ==> a^(2^(b-k)*2^k)=1 ==> [a^(2^(b-k))]^(2^k)=1.
 *
 * a^(2^b)=1 ==> a^(-2^(b-k)*(-2^k))=1 ==> [a^(-2^(b-k))]^(-2^k)=1.
 *
 * Given that one is able to find an appropriate a, this allows 
 * for NTTs with (power of two) lengths from 2^2 up to 2^b.
 *
 * 32 bit numbers 
 * (a) (original implementation)
 *     p = 13 * 2^20 + 1, where 2^(2^19) = 1 (mod p) holds, 
 *     i.e. a = 2, b = 19
 *
 * (b) p = 15 * 2^27 + 1, where 137^(2^27) = 1 (mod p) holds, 
 *     i.e. a = 137, b = 27
 *
 */


// ------------------------------------------------------------
// incs


// ------------------------------------------------------------
// defs

// following line for constants
#define MOD_C(value)    UINT32_C(value)

#define P13_20  1
#define P15_27  2


// choose one of P13_20 or P15_27 to be the MODULES for the implementation 
// #define MODULUS P13_20
#define MODULUS P15_27


// ------------------------------------------------------------
// incs

#include "ntt.h"


// ------------------------------------------------------------
// decl


#if MODULUS == P13_20 

    /*
     * size of field: p = 13 * 2^20 + 1 
     */

    static const mod g_nModulus = 13 * (1 << 20) + 1;           /**< size of finite field p */
    static const int g_nLdOrderPlus1 = 20;                      /**< possible NTT-lengths: 2^{2..19} */

    /**
     * 2^k-th and 2^(-k)-th roots of unity
     * k = 0 ... nLdOrderPlus1 - 1:                     g_nPower2RootsOfUnity[k]^(2^k)=1 (mod nModulus)
     * k = nLdOrderPlus1 ... 2 * nLdOrderPlus1 - 1:     g_nPower2RootsOfUnity[k]^(-2^k)=1 (mod nModulus)
     */
    static const mod g_nPower2RootsOfUnity[] = {
        
        1,          13631488,   1635631,    1598622,
        11792823,   7076190,    580251,     10270552,
        32346,      2803299,    10252398,   3341897,
        9153547,    3164342,    1048261,    65536,
        256,        16,         4,          2,
        
        /* start of negative roots */                   
        
        1,          13631488,   11995858,   9256520,
        1514586,    12710870,   695563,     2415013,
        7578528,    13516526,   265111,     8659501,
        3799463,    4259703,    43264,      13631281,
        13578241,   12779521,   10223617,   6815745
        
    };
    

    /**
     * inverses of powers of two
     * k = 0 ... nLdOrderPlus1 - 1:     g_nInverseOfPower2[k] = 2^(-k) (mod nModulus)
     */
    static const mod g_nInverseOfPower2[] = {
        1,          6815745,    10223617,   11927553,
        12779521,   13205505,   13418497,   13524993,
        13578241,   13604865,   13618177,   13624833,
        13628161,   13629825,   13630657,   13631073,
        13631281,   13631385,   13631437,   13631463
    };

    
#elif MODULUS == P15_27 
    
    /* 
     *size of field: p = 15 * 2^27 + 1 
     */

    static const mod g_nModulus = MOD_C(2013265921);    /**< size of finite field p = 15 * 2^27 + 1 */
    
    // note that the sum of two elements prior 
    // to reduction mod nModulus is between 2^31 and 2^32 
    
    static const int g_nLdOrderPlus1 = 28;              /**< possible NTT-lengths: 2^{2..27} */
    

    /**
     * 2^k-th and 2^(-k)-th roots of unity
     * k = 0 ... nLdOrderPlus1 - 1:                     g_nPower2RootsOfUnity[k]^(2^k)=1 (mod nModulus)
     * k = nLdOrderPlus1 ... 2 * nLdOrderPlus1 - 1:     g_nPower2RootsOfUnity[k]^(-2^k)=1 (mod nModulus)
     */
    static const mod g_nPower2RootsOfUnity[] = {
        MOD_C(1),           MOD_C(2013265920),  MOD_C(284861408),   MOD_C(1801542727), 
        MOD_C(567209306),   MOD_C(740045640),   MOD_C(918899846),   MOD_C(1881002012), 
        MOD_C(1453957774),  MOD_C(65325759),    MOD_C(1538055801),  MOD_C(515192888), 
        MOD_C(483885487),   MOD_C(157393079),   MOD_C(1695124103),  MOD_C(2005211659), 
        MOD_C(1540072241),  MOD_C(88064245),    MOD_C(1542985445),  MOD_C(1269900459), 
        MOD_C(1461624142),  MOD_C(825701067),   MOD_C(682402162),   MOD_C(1311873874), 
        MOD_C(1164520853),  MOD_C(352275361),   MOD_C(18769),       MOD_C(137),

        /* start of negative roots */                   
        
        MOD_C(1),           MOD_C(2013265920),  MOD_C(1728404513),  MOD_C(1592366214), 
        MOD_C(196396260),   MOD_C(1253260071),  MOD_C(72041623),    MOD_C(1091445674), 
        MOD_C(145223211),   MOD_C(1446820157),  MOD_C(1030796471),  MOD_C(2010749425), 
        MOD_C(1827366325),  MOD_C(1239938613),  MOD_C(246299276),   MOD_C(596347512), 
        MOD_C(1893145354),  MOD_C(246074437),   MOD_C(1525739923),  MOD_C(1194341128), 
        MOD_C(1463599021),  MOD_C(704606912),   MOD_C(95395244),    MOD_C(15672543), 
        MOD_C(647517488),   MOD_C(584175179),   MOD_C(137728885),   MOD_C(749463956)
    };
    
    
    /**
     * inverses of powers of two
     * k = 0 ... nLdOrderPlus1 - 1:     g_nInverseOfPower2[k] = 2^(-k) (mod nModulus)
     */
    static const mod g_nInverseOfPower2[] = {
        MOD_C(1),           MOD_C(1006632961),  MOD_C(1509949441),  MOD_C(1761607681), 
        MOD_C(1887436801),  MOD_C(1950351361),  MOD_C(1981808641),  MOD_C(1997537281), 
        MOD_C(2005401601),  MOD_C(2009333761),  MOD_C(2011299841),  MOD_C(2012282881), 
        MOD_C(2012774401),  MOD_C(2013020161),  MOD_C(2013143041),  MOD_C(2013204481), 
        MOD_C(2013235201),  MOD_C(2013250561),  MOD_C(2013258241),  MOD_C(2013262081), 
        MOD_C(2013264001),  MOD_C(2013264961),  MOD_C(2013265441),  MOD_C(2013265681), 
        MOD_C(2013265801),  MOD_C(2013265861),  MOD_C(2013265891),  MOD_C(2013265906)
    };

#else
                
#   error "please set the MODULE define correctly (neither P13_20 nor P15_27 has been set)"
                
#endif
                

// fwd
static inline mod mod_add(mod nModA, mod nModB);
static inline mod mod_mul(mod nModA, mod nModB);
static inline mod mod_sub(mod nModA, mod nModB);
static inline void mod_sum_diff(mod & nModA, mod & nModB);
static inline void multiply_val(mod * nModVector, uint32_t nLengthOfVector, mod nMultiplier);
static void ntt_dif4_core(mod * nArray, uint32_t nLog2Length);
static void ntt_dit4_core_inv(mod * nArray, uint32_t nLog2Length);

    
// ------------------------------------------------------------
// code
    
    
/**
 * returns ceil(ld(x))
 * returns 0xFFFFFFFF if input == 0
 * 
 * @param   x       input
 * @return  ceil(ld(x))
 */
uint32_t ld_ceil(uint32_t x) {
    
    if (0 == x) return 0xFFFFFFFF;

    uint32_t nLdFloor = ld_floor(x);
    uint32_t nPowerOf2 = 1UL << nLdFloor;
    
    if (nPowerOf2 == x) return nLdFloor;
    return nLdFloor + 1;
}


/**
 * floor(ld(x))
 * Return 0xFFFFFFFFF if input == 0.
 *
 * @param   x       input
 * @return  floor(ld(x))
 */
uint32_t ld_floor(uint32_t x) {

    if (0 == x) return 0xFFFFFFFF;

    uint32_t nLog2 = 0;
    
    if (x & 0xffff0000) { x >>= 16; nLog2 += 16; }
    if (x & 0x0000ff00) { x >>=  8; nLog2 +=  8; }
    if (x & 0x000000f0) { x >>=  4; nLog2 +=  4; }
    if (x & 0x0000000c) { x >>=  2; nLog2 +=  2; }
    if (x & 0x00000002) {           nLog2 +=  1; }
    
    return nLog2;
}


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
void mod_from_bigint(mod * nModArray, qkd::utility::bigint const & cBI, bool bReverseOrder) {

    if (!bReverseOrder) {
        for (uint32_t i = 0, j = cBI.bits() - 1; i < cBI.bits(); i++, j--) {
            nModArray[i] = cBI.get(j);
        }
    } 
    else {
        for (uint32_t i = 0; i < cBI.bits(); i++) nModArray[i] = cBI.get(i);
    } 
}


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
void ntt_convolution(mod * nArray1, mod * nArray2, const uint32_t nLog2Length) {
    
    assert((int32_t)nLog2Length < g_nLdOrderPlus1);

    // ntt of first array
    ntt_dif4_core(nArray1, nLog2Length);
    
    // ntt of second array
    ntt_dif4_core(nArray2, nLog2Length);  

    const uint32_t nLength = (1UL << nLog2Length);
    
    // multiply transforms
    for (uint32_t i = 0; i < nLength; ++i) nArray1[i] = mod_mul(nArray1[i], nArray2[i]);

    // inverse ntt of product
    ntt_dit4_core_inv(nArray1, nLog2Length); 

    // normalize with 1 / (length of array)    
    multiply_val(nArray1, nLength, g_nInverseOfPower2[nLog2Length]);
}


/**
 * addition in finite field
 *
 * @param   nModA   first operand
 * @param   nModB   second operand
 * @return  (nModA + nModB) modulo nModulus
 */
inline mod mod_add(mod nModA, mod nModB) {
    const mod nModC = nModA + nModB;
    return (nModC >= g_nModulus ? nModC - g_nModulus : nModC);
}


/**
 * multiplication in finite field
 *
 * @param   nModA   first operand
 * @param   nModB   second operand
 * @return  (nModA * nModB) modulo nModulus
 */
inline mod mod_mul(mod nModA, mod nModB) {
    
    if (nModA == 0) return 0;
    if (nModA == 1) return nModB;
    if (nModB == 1) return nModA;
    
    longmod nModC = static_cast<longmod>(nModA) * static_cast<longmod>(nModB);
    nModC %= g_nModulus;
    
    return static_cast<mod>(nModC);
}


/**
 * subtraction in finite field
 *
 * @param   nModA   first operand
 * @param   nModB   second operand
 * @return  (nModA - nModB) modulo nModulus
 */
inline mod mod_sub(mod nModA, mod nModB) {
    return (nModA >= nModB ? nModA - nModB : g_nModulus - nModB + nModA);
}


/**
 * sum and difference "in-place" in finite field
 *
 * @param   nModA   ptr to first operand, returns ptr to (nModA + nModB) mod nModulus
 * @param   nModB   ptr to second operand, returns ptr to (nModA - nModB) mod nModulus
 */
inline void mod_sum_diff(mod & nModA, mod & nModB) {

    /* {nModA, nModB}  <--| {nModA + nModB, nModA - nModB} */
 
    mod nDiff = mod_sub(nModA, nModB); 
    nModA += nModB; 
    
    if (nModA > g_nModulus) nModA -= g_nModulus; 
    nModB = nDiff; 
}


/**
 * multiplication of array in finite field with constant
 *
 * @param   nModVector          input vector
 * @param   nLengthOfVector     length of vector
 * @param   nMultiplier         multiplier
 * @return  (nModVector*nMultiplier) modulo nModulus
 */
inline void multiply_val(mod * nModVector, uint32_t nLengthOfVector, mod nMultiplier) {
    for (uint32_t i = 0; i < nLengthOfVector; i++) {
        nModVector[i] = mod_mul(nModVector[i], nMultiplier);
    }
}


// TODO: chris, please check this
#define nLX     2


/**
 * Decimation in frequency (DIF) radix-4 NTT.
 *
 * Output data is in permuted order.
 *
 * @param   nArray          array of input/output values (length has to be power of 2 !!)
 * @param   nLog2Length     base-2 log of array length
 */
void ntt_dif4_core(mod * nArray, uint32_t nLog2Length) {
    
    const mod nLength = (1UL << nLog2Length);

    // 2^(2^19)=1 (mod 2^20 13 +1) --> nLdOrderPlus1=19+1=20.
    const mod nImag = g_nPower2RootsOfUnity[2]; 

    uint32_t nLog2ActLength;
    
    // log n loops
    for (nLog2ActLength = nLog2Length; nLog2ActLength >= nLX; nLog2ActLength -= nLX) {
        
        const mod nActLength = (1UL << nLog2ActLength);
        const mod nActLength4 = (nActLength >> nLX);

        const mod nDRoot = g_nPower2RootsOfUnity[nLog2ActLength];
        mod nRoot = 1;
        mod nRoot2 = 1;
        mod nRoot3 = 1;
        uint32_t j;
        
        for (j=0; j < nActLength4; j++) {
            
            for (uint32_t r = 0, i0 = j + r; r < nLength; r += nActLength, i0 += nActLength) {
                
                const uint32_t i1 = i0 + nActLength4;
                const uint32_t i2 = i1 + nActLength4;
                const uint32_t i3 = i2 + nActLength4;

                mod nA0 = nArray[i0];
                mod nA1 = nArray[i1];
                mod nA2 = nArray[i2];
                mod nA3 = nArray[i3];

                mod nT02 = mod_add(nA0, nA2);
                mod nT13 = mod_add(nA1, nA3);

                nArray[i0] = mod_add(nT02, nT13);
                nArray[i1] = mod_mul(mod_sub(nT02, nT13), nRoot2);

                nT02 = mod_sub(nA0, nA2);
                nT13 = mod_mul(mod_sub(nA1, nA3), nImag);

                nArray[i2] = mod_mul(mod_add(nT02, nT13), nRoot);
                nArray[i3] = mod_mul(mod_sub(nT02, nT13), nRoot3);
            }

            nRoot = mod_mul(nRoot, nDRoot);
            nRoot2 = mod_mul(nRoot, nRoot);
            nRoot3 = mod_mul(nRoot, nRoot2);
        }
    }

    if (nLog2Length & 1) {
        
        // n is not a power of 4, need a radix-2 step
        for (uint32_t i = 0; i < nLength; i += 2) mod_sum_diff(nArray[i], nArray[i + 1]);
    }
}


/**
 * Inverse decimation in time (DIT) radix-4 NTT.
 *
 * Input data must be in permuted order.
 *
 * @param   nArray          array of input/output values (length has to be power of 2 !!)
 * @param   nLog2Length     base-2 log of array length
 */
void ntt_dit4_core_inv(mod * nArray, uint32_t nLog2Length) {
    
    const uint32_t nLength = (1UL << nLog2Length);

    if (nLog2Length & 1) {
        // n is not a power of 4, need a radix-2 step
        for (uint32_t i = 0; i < nLength; i += 2) mod_sum_diff(nArray[i], nArray[i+1]);
    }

    const mod nImag = g_nPower2RootsOfUnity[g_nLdOrderPlus1 + 2];

    uint32_t nLog2ActLength = nLX + (nLog2Length & 1);
    for ( ; nLog2ActLength <= nLog2Length ; nLog2ActLength += nLX) {
        
        const uint32_t nActLength = (1UL << nLog2ActLength);
        const uint32_t nActLength4 = (nActLength >> nLX);

        const mod nDRoot = g_nPower2RootsOfUnity[g_nLdOrderPlus1 + nLog2ActLength];
        mod nRoot = 1;
        mod nRoot2 = 1;
        mod nRoot3 = 1;
        
        for (uint32_t j = 0; j < nActLength4; j++) {
            
            for (uint32_t r = 0, i0 = j + r; r < nLength; r += nActLength, i0 += nActLength) {
                
                const uint32_t i1 = i0 + nActLength4;
                const uint32_t i2 = i1 + nActLength4;
                const uint32_t i3 = i2 + nActLength4;

                mod nA0 = nArray[i0];
                mod nA2 = mod_mul(nArray[i1], nRoot2);
                mod nA1 = mod_mul(nArray[i2], nRoot);
                mod nA3 = mod_mul(nArray[i3], nRoot3);

                mod nT02 = mod_add(nA0, nA2);
                mod nT13 = mod_add(nA1, nA3);

                nArray[i0] = mod_add(nT02, nT13);
                nArray[i2] = mod_sub(nT02, nT13);

                nT02 = mod_sub(nA0, nA2);
                nT13 = mod_sub(nA1, nA3);
                nT13 = mod_mul(nT13, nImag);

                nArray[i1] = mod_add(nT02, nT13);
                nArray[i3] = mod_sub(nT02, nT13);
            }

            nRoot = mod_mul(nDRoot, nRoot);
            nRoot2 = mod_mul(nRoot, nRoot);
            nRoot3 = mod_mul(nRoot2, nRoot);
        }
    }
}


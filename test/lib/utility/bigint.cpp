/*
 * bigint.cpp
 * 
 * This is a test file.
 *
 * TEST: test the qkd::bigint class
 *
 * Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *
 * Copyright (C) 2012-2015 AIT Austrian Institute of Technology
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

int test() {

    // create
    qkd::utility::bigint cBigintA;
    qkd::utility::bigint cBigintB;
    qkd::utility::bigint cBigintC;
    
    // memory vars
    qkd::utility::memory cMemoryA;
    qkd::utility::memory cMemoryB;
    qkd::utility::memory cMemoryC;
    
    // empty bigint
    cBigintA = qkd::utility::bigint(0);
    
    // out of range
    try {
        cBigintA.set(1000000, false);
        assert("out of range");
    }
    catch (qkd::utility::bigint::bigint_bit_out_of_range & cException) {}
    
    // bigint from and to memory conversions
    // Mem-A --> Bigint A --> Mem-B --> Bigint B --> Mem C
    // All most hold the same data
    cMemoryA = qkd::utility::memory::from_hex("a07c4df0520012a5c0de12701");
    cBigintA = qkd::utility::bigint(cMemoryA);
    cMemoryB = cBigintA.memory();
    cBigintB = qkd::utility::bigint(cMemoryB);
    cMemoryC = cBigintB.memory();
    
    assert(cMemoryA.equal(cMemoryB));
    assert(cMemoryA.equal(cMemoryC));
    assert(cBigintA == cBigintB);

    // memory is read from left to right, having the least significant byte first
    // memory checks
    cMemoryA = qkd::utility::memory::from_hex("a07c4df");
    cMemoryB = qkd::utility::memory::from_hex("04f3e58");
    cBigintA = qkd::utility::bigint(cMemoryA);
    
    // string represenations functions
    assert(cBigintA.as_dual().compare("11110000010011010111110010100000") == 0);
    assert(cBigintA.as_dec().compare("4031610016") == 0);
    assert(cBigintA.as_hex().compare("f04d7ca0") == 0);
    
    // resizing
    cBigintA = qkd::utility::bigint(cMemoryA);
    assert(cBigintA.as_dual().compare("11110000010011010111110010100000") == 0);
    cBigintA.resize(13);
    assert(cBigintA.as_dual().compare("1110010100000") == 0);
    cBigintA.resize(32);
    assert(cBigintA.as_dual().compare("00000000000000000001110010100000") == 0);
    
    // comparators
    cBigintA = qkd::utility::bigint(cMemoryA);
    cBigintB = qkd::utility::bigint(cMemoryB);
    cBigintC = cBigintA.clone();
    assert(cBigintA.as_dual().compare("11110000010011010111110010100000") == 0);
    assert(cBigintB.as_dual().compare("10000000111001011111001100000100") == 0);
    assert(cBigintC.as_dual().compare("11110000010011010111110010100000") == 0);
    assert(cBigintA == cBigintC);
    assert(cBigintA != cBigintB);
    assert(cBigintA > cBigintB);
    assert(cBigintA >= cBigintB);
    assert(cBigintA >= cBigintC);
    assert(cBigintB < cBigintA);
    assert(cBigintB <= cBigintA);
    assert(cBigintB != cBigintC);

    // shallow copy and bit manipulation
    cBigintA = qkd::utility::bigint(cMemoryA);
    cBigintB = cBigintA;
    cBigintB.set(13, false);
    // bit 13                                      here: +
    assert(cBigintA.as_dual().compare("11110000010011010101110010100000") == 0);
    assert(cBigintB.as_dual().compare("11110000010011010101110010100000") == 0);
    
    // deep copy and bit manipulation
    cBigintA = qkd::utility::bigint(cMemoryA);
    cBigintB = cBigintA.clone();
    cBigintB.set(13, false);
    // bit 13                                      here: +
    assert(cBigintA.as_dual().compare("11110000010011010111110010100000") == 0);
    assert(cBigintB.as_dual().compare("11110000010011010101110010100000") == 0);
    
    // prepare binary operations
    cBigintA = qkd::utility::bigint(cMemoryA);
    cBigintB = qkd::utility::bigint(cMemoryB);
    assert(cBigintA.as_dual().compare("11110000010011010111110010100000") == 0);
    assert(cBigintB.as_dual().compare("10000000111001011111001100000100") == 0);
    
    // binary AND
    cBigintC = cBigintA & cBigintB;
    assert(cBigintA.as_dual().compare("11110000010011010111110010100000") == 0);
    assert(cBigintB.as_dual().compare("10000000111001011111001100000100") == 0);
    assert(cBigintC.as_dual().compare("10000000010001010111000000000000") == 0);
    
    // binary OR
    cBigintC = cBigintA | cBigintB;
    assert(cBigintA.as_dual().compare("11110000010011010111110010100000") == 0);
    assert(cBigintB.as_dual().compare("10000000111001011111001100000100") == 0);
    assert(cBigintC.as_dual().compare("11110000111011011111111110100100") == 0);
    
    // binary XOR
    cBigintC = cBigintA ^ cBigintB;
    assert(cBigintA.as_dual().compare("11110000010011010111110010100000") == 0);
    assert(cBigintB.as_dual().compare("10000000111001011111001100000100") == 0);
    assert(cBigintC.as_dual().compare("01110000101010001000111110100100") == 0);

    // binary AND - assignment
    cBigintA = qkd::utility::bigint(cMemoryA);
    cBigintB = qkd::utility::bigint(cMemoryB);
    cBigintB &= cBigintA;
    assert(cBigintA.as_dual().compare("11110000010011010111110010100000") == 0);
    assert(cBigintB.as_dual().compare("10000000010001010111000000000000") == 0);

    // binary OR - assignment
    cBigintA = qkd::utility::bigint(cMemoryA);
    cBigintB = qkd::utility::bigint(cMemoryB);
    cBigintB |= cBigintA;
    assert(cBigintA.as_dual().compare("11110000010011010111110010100000") == 0);
    assert(cBigintB.as_dual().compare("11110000111011011111111110100100") == 0);

    // binary XOR - assignment
    cBigintA = qkd::utility::bigint(cMemoryA);
    cBigintB = qkd::utility::bigint(cMemoryB);
    cBigintB ^= cBigintA;
    assert(cBigintA.as_dual().compare("11110000010011010111110010100000") == 0);
    assert(cBigintB.as_dual().compare("01110000101010001000111110100100") == 0);
    
    // clear
    cBigintA = qkd::utility::bigint(cMemoryA);
    cBigintA.clear();
    assert(cBigintA.as_dual().compare("00000000000000000000000000000000") == 0);
    
    // fill
    cBigintA = qkd::utility::bigint(32);
    cBigintA.fill();
    assert(cBigintA.as_dual().compare("11111111111111111111111111111111") == 0);

    // NOT
    cBigintA = qkd::utility::bigint(cMemoryA);
    cBigintB = ~cBigintA;
    assert(cBigintA.as_dual().compare("11110000010011010111110010100000") == 0);
    assert(cBigintB.as_dual().compare("00001111101100101000001101011111") == 0);
    
    // big NOT
    cBigintA = qkd::utility::bigint(qkd::utility::memory::from_hex("7744774400ff00ffacac5353cdcd101026268d8d"));
    cBigintB = ~cBigintA;
    assert(cBigintA.as_hex().compare("8d8d26261010cdcd5353acacff00ff0044774477") == 0);
    assert(cBigintB.as_hex().compare("7272d9d9efef3232acac535300ff00ffbb88bb88") == 0);

    // shift right
    cBigintA = qkd::utility::bigint(cMemoryA);
    cBigintB = cBigintA >> 5;
    assert(cBigintA.as_dual().compare("11110000010011010111110010100000") == 0);
    assert(cBigintB.as_dual().compare("00000111100000100110101111100101") == 0);
    
    // shift right assignment
    cBigintA = qkd::utility::bigint(cMemoryA);
    cBigintA >>= 5;
    assert(cBigintA.as_dual().compare("00000111100000100110101111100101") == 0);
    
    // shift left
    cBigintA = qkd::utility::bigint(cMemoryA);
    cBigintB = cBigintA << 7;
    assert(cBigintA.as_dual().compare("11110000010011010111110010100000") == 0);
    assert(cBigintB.as_dual().compare("00100110101111100101000000000000") == 0);
    
    // shift left assignment
    cBigintA = qkd::utility::bigint(cMemoryA);
    cBigintA <<= 7;
    assert(cBigintA.as_dual().compare("00100110101111100101000000000000") == 0);
        
    // shift left and right assignment
    cBigintA = qkd::utility::bigint(cMemoryA);
    cBigintA <<= 5;
    cBigintA >>= 5;
    assert(cBigintA.as_dual().compare("00000000010011010111110010100000") == 0);

    // NOT + shift right
    cBigintA = qkd::utility::bigint(cMemoryA);
    cBigintB = ~cBigintA;
    cBigintB >>= 10;
    assert(cBigintA.as_dual().compare("11110000010011010111110010100000") == 0);
    assert(cBigintB.as_dual().compare("00000000000000111110110010100000") == 0);
    
    // bits set count
    cBigintA = qkd::utility::bigint(cMemoryA);
    assert(cBigintA.as_dual().compare("11110000010011010111110010100000") == 0);
    assert(cBigintA.bits_set() == 15);
    cBigintA.clear();
    assert(cBigintA.bits_set() == 0);
    cBigintA.op_not();
    assert(cBigintA.bits_set() == 32);
    
    // parity
    cBigintA = qkd::utility::bigint(cMemoryA);
    assert(cBigintA.as_dual().compare("11110000010011010111110010100000") == 0);
    assert(cBigintA.parity() == true);
    cBigintB = qkd::utility::bigint(cMemoryB);
    assert(cBigintB.as_dual().compare("10000000111001011111001100000100") == 0);
    assert(cBigintB.parity() == true);
    cBigintC = cBigintA ^ cBigintB;
    assert(cBigintC.as_dual().compare("01110000101010001000111110100100") == 0);
    assert(cBigintC.parity() == false);
    
    // sub
    cBigintA = qkd::utility::bigint(cMemoryA);
    cBigintB = cBigintA.sub(5, 10);
    assert(cBigintA.as_dual().compare("11110000010011010111110010100000") == 0);
    assert(cBigintB.as_dual().compare("1111100101") == 0);
    cBigintB = cBigintA.sub(0, 17);
    assert(cBigintB.as_dual().compare("10111110010100000") == 0);
    cBigintB = cBigintA.sub(0, 70);
    assert(cBigintB.as_dual().compare("11110000010011010111110010100000") == 0);
    cBigintB = cBigintA.sub(70, 70);
    assert(cBigintB.as_dual().compare("0") == 0);
    
    // masking
    cBigintA = qkd::utility::bigint::mask(18, 5, 0);
    assert(cBigintA.as_dual().compare("000000000000011111") == 0);
    cBigintA = qkd::utility::bigint::mask(18, 7, 3);
    assert(cBigintA.as_dual().compare("000000001111111000") == 0);
    cBigintA = qkd::utility::bigint::mask(18, 8, 10);
    assert(cBigintA.as_dual().compare("111111110000000000") == 0);
    cBigintA = qkd::utility::bigint::mask(18, 0, 13);
    assert(cBigintA.as_dual().compare("000000000000000000") == 0);
    
    return 0;
}

int main(UNUSED int argc, UNUSED char** argv) {
    return test();
}


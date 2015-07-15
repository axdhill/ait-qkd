/*
 * checksum.cpp
 * 
 * This is a test file.
 *
 * TEST: test the qkd::checksum class
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

    // create a memory area
    char const * sText = "The quick brown fox jumps over the lazy dog";
    qkd::utility::memory cMemory(strlen(sText));
    memcpy(cMemory.get(), sText, strlen(sText));

    // the algorithm and the digest
    qkd::utility::memory cDigest;
    qkd::utility::checksum cAlg;
    
    // --- CRC32 ---

    // create a CRC32 algorithm, fill in some stuff and get the digest
    cAlg = qkd::utility::checksum_algorithm::create("crc32");
    assert(cAlg->name() == "crc32");
    cAlg << cMemory;
    cAlg >> cDigest;
    assert(cDigest.as_hex() == "39a34f41");

    // create a SHA1 algorithm, fill in some stuff and get the digest
    cAlg = qkd::utility::checksum_algorithm::create("crc32");
    assert(cAlg->name() == "crc32");
    cAlg->add(cMemory);
    cDigest = cAlg->finalize();
    assert(cDigest.as_hex() == "39a34f41");

    // OVER-finalize: do not allow additional memory once we've finalized
    try {
        cAlg << cMemory;
        assert("OVER-finalize!");
    }
    catch (qkd::utility::checksum_algorithm::checksum_algorithm_final & cException) {}
    
    // --- SHA1 ---

    // create a SHA1 algorithm, fill in some stuff and get the digest
    cAlg = qkd::utility::checksum_algorithm::create("sha1");
    assert(cAlg->name() == "sha1");
    cAlg << cMemory;
    cAlg >> cDigest;
    assert(cDigest.as_hex() == "2fd4e1c67a2d28fced849ee1bb76e7391b93eb12");

    // create a SHA1 algorithm, fill in some stuff and get the digest
    cAlg = qkd::utility::checksum_algorithm::create("sha1");
    assert(cAlg->name() == "sha1");
    cAlg->add(cMemory);
    cDigest = cAlg->finalize();
    assert(cDigest.as_hex() == "2fd4e1c67a2d28fced849ee1bb76e7391b93eb12");

    // OVER-finalize: do not allow additional memory once we've finalized
    try {
        cAlg << cMemory;
        assert("OVER-finalize!");
    }
    catch (qkd::utility::checksum_algorithm::checksum_algorithm_final & cException) {}
    
    // --- MD5 ---
    
    // create a MD5 algorithm, fill in some stuff and get the digest
    cAlg = qkd::utility::checksum_algorithm::create("md5");
    assert(cAlg->name() == "md5");
    cAlg << cMemory;
    cAlg >> cDigest;
    assert(cDigest.as_hex() == "9e107d9d372bb6826bd81d3542a419d6");

    // create a MD5 algorithm, fill in some stuff and get the digest
    cAlg = qkd::utility::checksum_algorithm::create("md5");
    assert(cAlg->name() == "md5");
    cAlg->add(cMemory);
    cDigest = cAlg->finalize();
    assert(cDigest.as_hex() == "9e107d9d372bb6826bd81d3542a419d6");

    // OVER-finalize: do not allow additional memory once we've finalized
    try {
        cAlg << cMemory;
        assert("OVER-finalize");
    }
    catch (qkd::utility::checksum_algorithm::checksum_algorithm_final & cException) {}

    // --- <unknown> ---
    
    // get unkown algorithm
    try {
        cAlg = qkd::utility::checksum_algorithm::create("john_doe");
        assert("unknown algorithm");
    }
    catch (qkd::utility::checksum_algorithm::checksum_algorithm_unknown & cException) {}

    return 0;
}

int main(UNUSED int argc, UNUSED char** argv) {
    return test();
}


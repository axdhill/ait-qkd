/*
 * zip.cpp
 * 
 * This is a test file.
 *
 * TEST: test the qkd::utility::zip class
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

#include <fstream>
#include <iostream>

// include the all-in-one header
#include <qkd/qkd.h>


// ------------------------------------------------------------
// code


int test() {
    
    // input to zip
    char sText[] = "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";
    qkd::utility::memory cInput = qkd::utility::memory::wrap((unsigned char *)sText, strlen(sText));
    
    // compress
    qkd::utility::memory cCompressed = qkd::utility::zip::deflate(cInput);
    
    // ... and decrompress
    qkd::utility::memory cDecompressed = qkd::utility::zip::inflate(cCompressed);
    
    // check
    assert(!cInput.equal(cCompressed));
    assert(cInput.equal(cDecompressed));
    
    // large files
    //
    // File --> Memory A (Digest_A) --> Compress --> Uncompress --> Memory B (Digest_B)
    // 
    // Digest_A == Digest_B ?
    //
    qkd::utility::checksum cAlg;
    qkd::utility::memory cDigest_A;
    qkd::utility::memory cDigest_B;
    qkd::utility::memory cDigest_C;
    
    std::ifstream cFile("../test-data/test.jpg", std::ios::in | std::ios::binary);
    assert(cFile.is_open());
    
    // get size
    cFile.seekg (0, std::ios::end);
    uint64_t nLength = cFile.tellg();
    cFile.seekg (0, std::ios::beg);
    
    // get the file into memory
    char * cData = new char[nLength];
    cFile.read(cData, nLength);
    qkd::utility::memory cTestMem((unsigned char *)cData, nLength);
    
    // pick the checksum
    cAlg =  qkd::utility::checksum_algorithm::create("md5");
    cAlg << cTestMem;
    cAlg >> cDigest_A;
    assert(cDigest_A.as_hex() == "25bbbef662cc588f2d57e344aa5c305b");
    
    // zip the memory
    qkd::utility::memory cCompressedMemory = qkd::utility::zip::deflate(cTestMem);
    
    // pick the checksum
    cAlg =  qkd::utility::checksum_algorithm::create("md5");
    cAlg << cCompressedMemory;
    cAlg >> cDigest_B;
    assert(cDigest_B.as_hex() != "25bbbef662cc588f2d57e344aa5c305b");
    
    // unzip the memory
    qkd::utility::memory cUncompressedMemory = qkd::utility::zip::inflate(cCompressedMemory);
    
    // pick the checksum
    cAlg =  qkd::utility::checksum_algorithm::create("md5");
    cAlg << cUncompressedMemory;
    cAlg >> cDigest_C;
    assert(cDigest_C.as_hex() == "25bbbef662cc588f2d57e344aa5c305b");

    return 0;
}


int main(UNUSED int argc, UNUSED char** argv) {
    return test();
}


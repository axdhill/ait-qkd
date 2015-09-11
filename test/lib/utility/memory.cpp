/*
 * memory.cpp
 * 
 * This is a test file.
 *
 * TEST: test the qkd::utility::memory class
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

#include <fstream>
#include <iostream>

// include the all-in-one header
#include <qkd/qkd.h>


// ------------------------------------------------------------
// code


int test_const(qkd::utility::memory const & cMemory) {

    // this must be copy
    qkd::utility::memory cCopyMemory(cMemory);

    // access elements
    assert(cMemory[0] == '0');
    assert(cMemory[1] == '1');
    assert(cMemory[2] == '2');
    assert(cMemory[3] == '3');
    assert(cMemory[4] == '4');
    assert(cMemory[5] == '5');
    assert(cMemory[6] == '6');
    assert(cMemory[7] == '7');
    assert(cMemory[8] == '8');
    assert(cMemory[9] == '9');

    return 0;
}


int test() {
    
    char * area = nullptr;

    // empty constructor
    qkd::utility::memory cMemoryA;
    assert(cMemoryA.size() == 0);

    // size constructor
    cMemoryA = qkd::utility::memory(10);
    assert(cMemoryA.size() == 10);

    // wrap a memory area: do not take ownership
    area = new char[11];
    memcpy(area, "abcdefghij", 10);
    cMemoryA = qkd::utility::memory::wrap((unsigned char *)area, 10);
    assert(cMemoryA.size() == 10);

    // access elements
    assert(cMemoryA[0] == 'a');
    assert(cMemoryA[1] == 'b');
    assert(cMemoryA[2] == 'c');
    assert(cMemoryA[3] == 'd');
    assert(cMemoryA[4] == 'e');
    assert(cMemoryA[5] == 'f');
    assert(cMemoryA[6] == 'g');
    assert(cMemoryA[7] == 'h');
    assert(cMemoryA[8] == 'i');
    assert(cMemoryA[9] == 'j');

    // test checksum
    assert(cMemoryA.checksum().as_hex() == "3a708139");
    assert(cMemoryA.checksum("crc32").as_hex() == "3a708139");
    assert(cMemoryA.checksum("md5").as_hex() == "a925576942e94b2ef57a066101b48876");
    assert(cMemoryA.checksum("sha1").as_hex() == "d68c19a0a345b7eab78d5e11e991c026ec60db63");
    
    // release memory
    cMemoryA = qkd::utility::memory();
    
    // this must be now done, since the memory area has been wrapped
    delete [] area;

    // consume an existing memory
    area = new char[11];
    memcpy(area, "0123456789", 10);
    cMemoryA = qkd::utility::memory((unsigned char *)area, 10);
    assert(cMemoryA.size() == 10);

    // access elements
    assert(cMemoryA[0] == '0');
    assert(cMemoryA[1] == '1');
    assert(cMemoryA[2] == '2');
    assert(cMemoryA[3] == '3');
    assert(cMemoryA[4] == '4');
    assert(cMemoryA[5] == '5');
    assert(cMemoryA[6] == '6');
    assert(cMemoryA[7] == '7');
    assert(cMemoryA[8] == '8');
    assert(cMemoryA[9] == '9');
    
    // test const object
    int const_test = test_const(cMemoryA);
    if (const_test) return const_test;

    // copy object: increase ref count
    qkd::utility::memory cMemoryB(cMemoryA);
    assert(cMemoryA == cMemoryB);

    // make a new area
    area = new char[11];
    memcpy(area, "0123456789", 10);
    cMemoryB = qkd::utility::memory((unsigned char *)area, 10);
    assert(cMemoryB.size() == 10);

    // A and B are not identical but simliar
    assert(cMemoryA != cMemoryB);
    assert(cMemoryA.equal(cMemoryB));

    // both are the same: modify one thus detaching it
    cMemoryB = cMemoryA;
    assert(cMemoryA == cMemoryB);
    
    // when shallow, get() returns the same memory
    assert(cMemoryA.get() == cMemoryB.get());
    
    // when deep, get() returns a clone
    cMemoryA.set_shallow(false);
    assert(cMemoryA.get() != cMemoryB.get());

    cMemoryB = cMemoryA;
    assert(cMemoryA == cMemoryB);
    cMemoryB[0] = 'A';
    assert(cMemoryA != cMemoryB);
    assert(cMemoryA.get() != cMemoryB.get());

    // hex output
    cMemoryA = qkd::utility::memory(32);
    for (int i = 0; i < 32; i++) cMemoryA[i] = i;
    assert(cMemoryA.as_hex().compare("000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f") == 0);

    // clone: different objects but same content
    cMemoryB = cMemoryA.clone();
    assert(cMemoryA.unique());
    assert(cMemoryB.unique());
    assert(cMemoryA.get() != cMemoryB.get());
    assert(memcmp(cMemoryA.get(), cMemoryB.get(), cMemoryA.size()) == 0);
    
    // string conversions
    cMemoryA = qkd::utility::memory::from_hex("abcdef01234");
    assert(cMemoryA.as_hex().compare("abcdef012340") == 0);
    cMemoryA = qkd::utility::memory::from_hex("abcdef0123");
    assert(cMemoryA.as_hex().compare("abcdef0123") == 0);
    cMemoryA = qkd::utility::memory::from_hex("abcdef0123456789");
    assert(cMemoryA.as_hex().compare("abcdef0123456789") == 0);
    cMemoryA = qkd::utility::memory::from_hex("abcdef01234567890");
    assert(cMemoryA.as_hex().compare("abcdef012345678900") == 0);

    // resizing samples
    cMemoryA = qkd::utility::memory::from_hex("abcd0123abcd0123");
    cMemoryA.resize(4);
    assert(cMemoryA.as_hex().compare("abcd0123") == 0);
    cMemoryA.resize(8);
    cMemoryA.resize(12);
    
    // save and read
    char sTempNameTemplate[] = "memory_test_XXXXXX";
    close(mkstemp(sTempNameTemplate));
    std::string sTempFileName(sTempNameTemplate);

    cMemoryA = qkd::utility::memory::from_hex("abcd0123abcd0123");
    std::ofstream cFileOut(sTempFileName, std::ios::out | std::ios::binary | std::ios::trunc);
    assert(cFileOut.is_open());
    cFileOut << cMemoryA;
    cFileOut.close();

    cMemoryB = qkd::utility::memory();
    std::ifstream cFileIn(sTempFileName, std::ios::in | std::ios::binary);
    assert(cFileIn.is_open());
    cFileIn >> cMemoryB;
    cFileIn.close();
    
    assert(cMemoryB.as_hex().compare("abcd0123abcd0123") == 0);
    
    // grow memory
    cMemoryA = qkd::utility::memory::from_hex("abcd0123abcd0123");
    assert(cMemoryA.size() == 8);
    cMemoryA << qkd::utility::memory::from_hex("ab");
    cMemoryA << qkd::utility::memory::from_hex("cd");
    cMemoryA << qkd::utility::memory::from_hex("ef");
    cMemoryA << qkd::utility::memory::from_hex("01");
    cMemoryA << qkd::utility::memory::from_hex("23");
    cMemoryA << qkd::utility::memory::from_hex("45");
    cMemoryA << qkd::utility::memory::from_hex("67");
    cMemoryA << qkd::utility::memory::from_hex("89");
    assert(cMemoryA.size() == 16);
    assert(cMemoryA.as_hex() == "abcd0123abcd0123abcdef0123456789");
    
    // canonical test: read a file and check the canonical representation
    std::ifstream cFileCanonical("../test-data/shared-secret", std::ios::in | std::ios::binary);
    assert(cFileCanonical.is_open());
    
    // get length of file
    cFileCanonical.seekg(0, std::ios::end);
    uint64_t nSizeOfFile = cFileCanonical.tellg();
    cFileCanonical.seekg(0, std::ios::beg);
    
    cMemoryA = qkd::utility::memory(nSizeOfFile);
    cFileCanonical.read((char *)cMemoryA.get(), nSizeOfFile);
    cFileCanonical.close();
    
    // check canonical output
    assert(cMemoryA.canonical("abc ") == "\
abc 00000000   54 68 69 73 20 69 73 20  61 20 73 68 61 72 65 64   |This is  a shared|\n\
abc 00000010   20 73 65 63 72 65 74 2e  20 49 74 20 6f 75 67 68   | secret.  It ough|\n\
abc 00000020   74 20 74 6f 20 62 65 20  6c 6f 6e 67 65 72 20 74   |t to be  longer t|\n\
abc 00000030   68 61 6e 20 61 20 6b 65  79 20 71 75 61 6e 74 75   |han a ke y quantu|\n\
abc 00000040   6d 20 74 6f 20 68 61 76  65 20 6d 6f 72 65 20 74   |m to hav e more t|\n\
abc 00000050   68 61 6e 20 31 20 6b 65  79 20 69 6e 20 74 68 65   |han 1 ke y in the|\n\
abc 00000060   20 44 42 20 77 69 74 68  20 74 68 69 73 20 74 65   | DB with  this te|\n\
abc 00000070   78 74 2e 20 41 20 6b 65  79 20 71 75 61 6e 74 75   |xt. A ke y quantu|\n\
abc 00000080   6d 20 69 73 20 74 68 65  20 73 69 7a 65 20 6f 66   |m is the  size of|\n\
abc 00000090   20 61 20 6b 65 79 20 61  74 6f 6d 2e 20 54 68 65   | a key a tom. The|\n\
abc 000000a0   20 64 61 74 61 62 61 73  65 20 6f 6e 6c 79 20 68   | databas e only h|\n\
abc 000000b0   61 6e 64 6c 65 73 20 6b  65 79 73 20 6f 66 20 74   |andles k eys of t|\n\
abc 000000c0   68 61 74 20 73 69 7a 65  2e 20 4e 6f 20 6d 6f 72   |hat size . No mor|\n\
abc 000000d0   65 2c 20 6e 6f 20 6c 65  73 73 2e 0a               |e, no le ss..    |\
");    
    
    return 0;
}


int main(UNUSED int argc, UNUSED char** argv) {
    return test();
}


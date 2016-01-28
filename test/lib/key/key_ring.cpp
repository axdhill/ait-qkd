/*
 * key_ring.cpp
 * 
 * This is a test file.
 *
 * TEST: test the qkd::key::key_ring class
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


void const_iterator(qkd::key::key_ring const & cKeyRing) {
    
    std::stringstream ss;
    
    // iterate over const key ring
    for (auto iter = cKeyRing.begin(); iter != cKeyRing.end(); iter++) {
        ss << (*iter).id() << " " << (*iter).data().as_hex() << std::endl;
    }
    
    // check result
    assert(ss.str() == 
"231 8318c0138a4be932090d\n\
232 f0f2cf0ecfd33dc5344d\n\
233 799eedd3af3cb9573f9c\n\
234 09cf8a3d85afa032c3f0\n\
235 a6f56bdcc9f4df7aac02\n\
236 e7b635bc85bf2585eeb9\n\
237 12345678901234567890\n\
238 abcdef\n");
    
}

int test() {
    
    std::stringstream ss;
    qkd::key::key cKey;
    
    // create a key ring: 
    // - each key should have at max 10 bytes
    // - index starts at 231
    qkd::key::key_ring cKeyRing(10, 231);
    
    // add some keys to the key ring
    cKeyRing << qkd::key::key(1, qkd::utility::memory::from_hex("8318c0138a4be932090df"));
    cKeyRing << qkd::key::key(2, qkd::utility::memory::from_hex("f2cf0ecfd33dc5344d799eedd3af"));
    cKeyRing << qkd::key::key(3, qkd::utility::memory::from_hex("3cb9"));
    cKeyRing << qkd::key::key(4, qkd::utility::memory::from_hex("57"));
    cKeyRing << qkd::key::key(5, qkd::utility::memory::from_hex("3f9c09cf8a3d85afa032c3f0a6f56bdcc9f4df7aac02e7b635bc85bf2585eeb9"));
    cKeyRing << qkd::key::key(6, qkd::utility::memory::from_hex("123456789012345678"));
    cKeyRing << qkd::key::key(7, qkd::utility::memory::from_hex("90"));
    cKeyRing << qkd::key::key(8, qkd::utility::memory::from_hex("abcdef"));
    
    // iterate
    for (auto iter = cKeyRing.begin(); iter != cKeyRing.end(); iter++) {
        ss << (*iter).id() << " " << (*iter).data().as_hex() << std::endl;
    }
    
    // check result
    assert(ss.str() == 
"231 8318c0138a4be932090d\n\
232 f0f2cf0ecfd33dc5344d\n\
233 799eedd3af3cb9573f9c\n\
234 09cf8a3d85afa032c3f0\n\
235 a6f56bdcc9f4df7aac02\n\
236 e7b635bc85bf2585eeb9\n\
237 12345678901234567890\n\
238 abcdef\n");

    // do const iterator
    const_iterator(cKeyRing);

    // access any key at random
    cKey = cKeyRing.at(2);
    assert(cKey.id() == 233);
    cKey = cKeyRing.at(5);
    assert(cKey.id() == 236);
    
    // remove a key
    ss.str("");
    
    cKeyRing.erase(cKeyRing.begin() + 3);
    for (auto iter = cKeyRing.begin(); iter != cKeyRing.end(); iter++) {
        ss << (*iter).id() << " " << (*iter).data().as_hex() << std::endl;
    }
    
    // check result
    assert(ss.str() == 
"231 8318c0138a4be932090d\n\
232 f0f2cf0ecfd33dc5344d\n\
233 799eedd3af3cb9573f9c\n\
235 a6f56bdcc9f4df7aac02\n\
236 e7b635bc85bf2585eeb9\n\
237 12345678901234567890\n\
238 abcdef\n");
    
    cKey = cKeyRing.at(2);
    assert(cKey.id() == 233);
    cKey = cKeyRing.at(5);
    assert(cKey.id() == 237);
    
    
    // key-ring copy (this is a shallow copy!)
    qkd::key::key_ring cKeyRingCopy = cKeyRing;

    // modify copy content to check
    // the key memory blob is shared among
    // key ring instances
    // --> access the 3rd byte in key #2 and change it to 0xFF
    cKeyRingCopy.at(1).data()[2] = 0xff;
    
    ss.str("");
    for (auto iter = cKeyRing.begin(); iter != cKeyRing.end(); iter++) {
        ss << (*iter).id() << " " << (*iter).data().as_hex() << std::endl;
    }
    
    // check result (key #232 now has ff on the third position)
    assert(ss.str() == 
"231 8318c0138a4be932090d\n\
232 f0f2ff0ecfd33dc5344d\n\
233 799eedd3af3cb9573f9c\n\
235 a6f56bdcc9f4df7aac02\n\
236 e7b635bc85bf2585eeb9\n\
237 12345678901234567890\n\
238 abcdef\n");

    return 0;
}

int main(UNUSED int argc, UNUSED char** argv) {
    return test();
}


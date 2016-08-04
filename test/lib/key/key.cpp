/*
 * key.cpp
 * 
 * This is a test file.
 *
 * TEST: test the qkd::key::key class
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


qkd::key::key foo() {
    qkd::utility::memory cMemory = qkd::utility::memory::from_hex("0123456789abcdef");
    return qkd::key::key(1, cMemory);
}


int test() {
    
    // check bits 0x8318c013
    qkd::key::key cKey(1, qkd::utility::memory::from_hex("8318c013"));
    assert(cKey.metadata_xml(true) == std::string("\
<?xml version=\"1.0\" encoding=\"utf-8\"?>\n\
<key id=\"1\">\n\
    <general>\n\
        <state>99</state>\n\
        <state_name>new</state_name>\n\
        <crypto>\n\
            <incoming/>\n\
            <outgoing/>\n\
        </crypto>\n\
        <bits>32</bits>\n\
        <qber>0</qber>\n\
        <disclosed>0</disclosed>\n\
    </general>\n\
    <modules/>\n\
</key>\n"));

    assert(cKey.get_bit( 0) == true);      // 3 
    assert(cKey.get_bit( 1) == true);
    assert(cKey.get_bit( 2) == false);
    assert(cKey.get_bit( 3) == false);

    assert(cKey.get_bit( 4) == false);     // 8
    assert(cKey.get_bit( 5) == false);
    assert(cKey.get_bit( 6) == false);
    assert(cKey.get_bit( 7) == true);

    assert(cKey.get_bit( 8) == false);     // 8
    assert(cKey.get_bit( 9) == false);
    assert(cKey.get_bit(10) == false);
    assert(cKey.get_bit(11) == true);

    assert(cKey.get_bit(12) == true);      // 1
    assert(cKey.get_bit(13) == false);
    assert(cKey.get_bit(14) == false);
    assert(cKey.get_bit(15) == false);

    assert(cKey.get_bit(16) == false);     // 0
    assert(cKey.get_bit(17) == false);
    assert(cKey.get_bit(18) == false);
    assert(cKey.get_bit(19) == false);

    assert(cKey.get_bit(20) == false);     // c
    assert(cKey.get_bit(21) == false);
    assert(cKey.get_bit(22) == true);
    assert(cKey.get_bit(23) == true);

    assert(cKey.get_bit(24) == true);      // 3
    assert(cKey.get_bit(25) == true);
    assert(cKey.get_bit(26) == false);
    assert(cKey.get_bit(27) == false);

    assert(cKey.get_bit(28) == true);      // 1
    assert(cKey.get_bit(29) == false);
    assert(cKey.get_bit(30) == false);
    assert(cKey.get_bit(31) == false);

    // modify some bits

    cKey.set_bit( 3, true);
    cKey.set_bit( 6, true);
    cKey.set_bit( 7, false);
    cKey.set_bit(12, true);
    cKey.set_bit(15, true);
    cKey.set_bit(16, true);
    cKey.set_bit(18, false);
    cKey.set_bit(22, false);
    cKey.set_bit(28, false);

    // check modified bits

    assert(cKey.get_bit( 0) == true);      // b 
    assert(cKey.get_bit( 1) == true);
    assert(cKey.get_bit( 2) == false);
    assert(cKey.get_bit( 3) == true);

    assert(cKey.get_bit( 4) == false);     // 4
    assert(cKey.get_bit( 5) == false);
    assert(cKey.get_bit( 6) == true);
    assert(cKey.get_bit( 7) == false);

    assert(cKey.get_bit( 8) == false);     // 8
    assert(cKey.get_bit( 9) == false);
    assert(cKey.get_bit(10) == false);
    assert(cKey.get_bit(11) == true);

    assert(cKey.get_bit(12) == true);      // 9
    assert(cKey.get_bit(13) == false);
    assert(cKey.get_bit(14) == false);
    assert(cKey.get_bit(15) == true);

    assert(cKey.get_bit(16) == true);      // 1
    assert(cKey.get_bit(17) == false);
    assert(cKey.get_bit(18) == false);
    assert(cKey.get_bit(19) == false);

    assert(cKey.get_bit(20) == false);     // 8
    assert(cKey.get_bit(21) == false);
    assert(cKey.get_bit(22) == false);
    assert(cKey.get_bit(23) == true);

    assert(cKey.get_bit(24) == true);      // 3
    assert(cKey.get_bit(25) == true);
    assert(cKey.get_bit(26) == false);
    assert(cKey.get_bit(27) == false);

    assert(cKey.get_bit(28) == false);     // 0
    assert(cKey.get_bit(29) == false);
    assert(cKey.get_bit(30) == false);
    assert(cKey.get_bit(31) == false);

    assert(cKey.data().as_hex() == "4b988103");

    qkd::utility::memory cMemoryA = qkd::utility::memory::from_hex("8318c0138a4be932090df");
    qkd::utility::memory cMemoryB;
    qkd::utility::memory const cMemoryC = cMemoryA;
    
    // creation
    qkd::key::key cKeyA;
    qkd::key::key cKeyB(1, cMemoryA);
    assert(cKeyB.size() == 11);
   
    // memory checks
    cMemoryB = cKeyB.data();
    assert(cMemoryA.get() == cMemoryB.get());
    assert(cMemoryA.equal(cMemoryB));
    
    // init from const memory --> deep copy
    cKeyB = qkd::key::key(1, cMemoryC);
    cMemoryB = cKeyB.data();
    assert(cMemoryA.get() != cMemoryB.get());
    assert(cMemoryA.equal(cMemoryB));
    
    // copying and assignment
    cKeyA = cKeyB;
    cKeyB = foo();
    cKeyA = qkd::key::key(foo());

    // save and read
    char sTempNameTemplate[] = "key_test_XXXXXX";
    close(mkstemp(sTempNameTemplate));
    std::string sTempFileName(sTempNameTemplate);

    cKeyA = qkd::key::key(13, cMemoryA);
    cKeyA.set_disclosed(65);
    cKeyA.set_crypto_scheme_incoming("evhash-96:053f37b4f59af505c42ba169:64ac81010f6382824d1440e2");
    cKeyA.set_crypto_scheme_outgoing("evhash-96:44bc9c0137fae9190b76d4b3:0319ff9b6df7a7ede957428d");
    std::ofstream cFileOut(sTempFileName, std::ios::out | std::ios::binary | std::ios::trunc);
    assert(cFileOut.is_open());
    cFileOut << cKeyA;
    cFileOut.close();

    cKeyB = qkd::key::key();
    std::ifstream cFileIn(sTempFileName, std::ios::in | std::ios::binary);
    assert(cFileIn.is_open());
    cFileIn >> cKeyB;
    cFileIn.close();
    
    assert(cKeyA == cKeyB);
    assert(cKeyB.disclosed() == 65);
    assert(cKeyB.crypto_scheme_incoming() == "evhash-96:053f37b4f59af505c42ba169:64ac81010f6382824d1440e2");
    assert(cKeyB.crypto_scheme_outgoing() == "evhash-96:44bc9c0137fae9190b76d4b3:0319ff9b6df7a7ede957428d");
    assert(cKeyA.metadata_xml(false) == cKeyB.metadata_xml(false));
    assert(memcmp(cKeyA.data().get(), cKeyB.data().get(), cKeyA.data().size()) == 0);
    
    // retry with buffers
    memcpy(sTempNameTemplate, "key_test_XXXXXX", strlen("key_test_XXXXXX"));
    close(mkstemp(sTempNameTemplate));
    sTempFileName = std::string(sTempNameTemplate);
    
    qkd::utility::buffer cBufferA;
    cBufferA << cKeyA;
    std::ofstream cFileBufferOut(sTempFileName, std::ios::out | std::ios::binary | std::ios::trunc);
    assert(cFileBufferOut.is_open());
    cFileBufferOut << cBufferA;
    cFileBufferOut.close();
    
    qkd::utility::buffer cBufferB;
    std::ifstream cFileBufferIn(sTempFileName, std::ios::in | std::ios::binary);
    assert(cFileBufferIn.is_open());
    cFileBufferIn >> cBufferB;
    cFileBufferIn.close();
    
    // extract key from buffer
    qkd::key::key cKeyC;
    cBufferB >> cKeyC;
    assert(cKeyA == cKeyC);
    assert(cKeyC.disclosed() == 65);
    assert(cKeyC.crypto_scheme_incoming() == "evhash-96:053f37b4f59af505c42ba169:64ac81010f6382824d1440e2");
    assert(cKeyC.crypto_scheme_outgoing() == "evhash-96:44bc9c0137fae9190b76d4b3:0319ff9b6df7a7ede957428d");
    assert(cKeyA.metadata_xml(false) == cKeyC.metadata_xml(false));
    assert(memcmp(cKeyA.data().get(), cKeyC.data().get(), cKeyA.data().size()) == 0);

    // grow a key
    cKeyA = qkd::key::key(1, qkd::utility::memory(0));
    assert(cKeyA.size() == 0);
    cKeyA << qkd::utility::memory::from_hex("01");
    cKeyA << qkd::utility::memory::from_hex("23");
    cKeyA << qkd::utility::memory::from_hex("45");
    cKeyA << qkd::utility::memory::from_hex("67");
    cKeyA << qkd::utility::memory::from_hex("89");
    cKeyA << qkd::utility::memory::from_hex("abcdef");
    assert(cKeyA.size() == 8);
    assert(cKeyA.data().as_hex() == "0123456789abcdef");
    
    // check key counter
    qkd::key::key::counter() = qkd::key::key::key_id_counter(3, 7);
    qkd::key::key::counter().set_count(9);
    qkd::key::key_id nId = qkd::key::key::counter().inc();
    
    assert((((9 + 1) << 3) + 7) == nId);
    
    return 0;
}

int main(UNUSED int argc, UNUSED char** argv) {
    return test();
}


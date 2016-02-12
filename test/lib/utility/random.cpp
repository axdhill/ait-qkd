/*
 * random.cpp
 * 
 * This is a test file.
 *
 * TEST: test the qkd::utility::random class
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
    
    // creation: C API random source
    qkd::utility::random cRandom = qkd::utility::random_source::source();
    std::cout << cRandom->describe() << std::endl;
    
    char nR_c;
    unsigned char nR_uc;
    int32_t nR_i;
    uint32_t nR_ui;
    int64_t nR_l;
    uint64_t nR_ul;
    float nR_f;
    double nR_d;
    qkd::utility::memory cMemory(32);
    
    // read
    cRandom >> nR_c;
    std::cout << "         random char: " << (int)nR_c << std::endl;
    cRandom >> nR_uc;
    std::cout << "random unsigned char: " << (int)nR_uc << std::endl;
    cRandom >> nR_i;
    std::cout << "          random int: " << nR_i << std::endl;
    cRandom >> nR_ui;
    std::cout << " random unsigned int: " << nR_ui << std::endl;
    cRandom >> nR_l;
    std::cout << "         random long: " << nR_l << std::endl;
    cRandom >> nR_ul;
    std::cout << "random unsigned long: " << nR_ul << std::endl;
    cRandom >> nR_f;
    std::cout << "        random float: " << nR_f << std::endl;
    cRandom >> nR_d;
    std::cout << "       random double: " << nR_d << std::endl;
    cRandom >> cMemory;
    std::cout << "       random memory: " << cMemory.as_hex() << std::endl;
    
    // read from file
    char sTempNameTemplate[] = "random_test_XXXXXX.tmp";
    UNUSED int nFD = mkstemps(sTempNameTemplate, strlen(".tmp"));
    std::string sTempFileName = std::string(sTempNameTemplate);

    cMemory = qkd::utility::memory::from_hex("abcdef0123456789");
    std::ofstream cFileOut(sTempFileName, std::ios::out | std::ios::binary | std::ios::trunc);
    assert(cFileOut.is_open());
    cFileOut << cMemory;
    cFileOut.close();
    
    // construct the URL
    boost::filesystem::path cURLPath = qkd::utility::environment::current_path();
    cURLPath /= sTempFileName;
    std::string sURL = std::string("file://") + cURLPath.string();
    
    // create random source with file url
    cRandom = qkd::utility::random_source::create(sURL);
    std::cout << cRandom->describe() << std::endl;
    
    qkd::utility::memory cRandomMemory = qkd::utility::memory(16);
    cRandom >> cRandomMemory;
    std::cout << "       random memory: " << cMemory.as_hex() << std::endl;
    
    // bytes 0-7 are memory meta data
    assert(cRandomMemory[0x08] == 0xab);
    assert(cRandomMemory[0x09] == 0xcd);
    assert(cRandomMemory[0x0a] == 0xef);
    assert(cRandomMemory[0x0b] == 0x01);
    assert(cRandomMemory[0x0c] == 0x23);
    assert(cRandomMemory[0x0d] == 0x45);
    assert(cRandomMemory[0x0e] == 0x67);
    assert(cRandomMemory[0x0f] == 0x89);
    
    // create operating system random source
    cRandom = qkd::utility::random_source::create("file:///dev/urandom");
    std::cout << cRandom->describe() << std::endl;
    for (uint64_t i = 0; i < 10; i++) {
        cRandom >> nR_i;
        std::cout << "          random int: " << nR_i << std::endl;
    }
    
    // create CBC-AES random generators: 128 bit ==> 'cbc-aes:<KEY>' with |KEY| = 16 bytes
    cRandom = qkd::utility::random_source::create("cbc-aes:70f5b70e05747c6d30d6cb75a2b7a036");
    std::cout << cRandom->describe() << std::endl;
    for (uint64_t i = 0; i < 10; i++) {
        cRandom >> nR_i;
        std::cout << "  cbc-aes-128 random: " << nR_i << std::endl;
    }

    // create CBC-AES random generators: 192 bit ==> 'cbc-aes:<KEY>' with |KEY| = 24 bytes
    cRandom = qkd::utility::random_source::create("cbc-aes:14af81a6be5b90278f1e0c3ffaa974cbf9e34a7974939168");
    std::cout << cRandom->describe() << std::endl;
    for (uint64_t i = 0; i < 10; i++) {
        cRandom >> nR_i;
        std::cout << "  cbc-aes-192 random: " << nR_i << std::endl;
    }

    // create CBC-AES random generators: 256 bit ==> 'cbc-aes:<KEY>' with |KEY| = 32 bytes
    cRandom = qkd::utility::random_source::create("cbc-aes:2829656af176937a111eaf4192608d55a8a26db503f933051987492804eeca66");
    std::cout << cRandom->describe() << std::endl;
    for (uint64_t i = 0; i < 10; i++) {
        cRandom >> nR_i;
        std::cout << "  cbc-aes-256 random: " << nR_i << std::endl;
    }
    
    // create HMAC-SHA random generators: 256 bit ==> 'hmac-sha:<KEY>' with |KEY| = 32 bytes
    cRandom = qkd::utility::random_source::create("hmac-sha:42036fd1b857c03a35e1dbb0c8c6c458cf7c6fd74229a0519f941ae602ee07f0");
    std::cout << cRandom->describe() << std::endl;
    for (uint64_t i = 0; i < 10; i++) {
        cRandom >> nR_i;
        std::cout << " hmac-sha-256 random: " << nR_i << std::endl;
    }

    // create HMAC-SHA random generators: 384 bit ==> 'hmac-sha:<KEY>' with |KEY| = 48 bytes
    cRandom = qkd::utility::random_source::create("hmac-sha:d305dc7597b2f14c0256ad2e48344e03af2ae6df40681efe5f95fe9c0e24239ef21e274c932656660fff552d992f3f52");
    std::cout << cRandom->describe() << std::endl;
    for (uint64_t i = 0; i < 10; i++) {
        cRandom >> nR_i;
        std::cout << " hmac-sha-384 random: " << nR_i << std::endl;
    }
    
    // create HMAC-SHA random generators: 512 bit ==> 'hmac-sha:<KEY>' with |KEY| = 64 bytes
    cRandom = qkd::utility::random_source::create("hmac-sha:02bacda14a265a0b905c70baddc9c397ff78bb5d2080dabf8c177df1acce494bbb424bfabcdfed202dccbc5f2f3fe2984ed77009211c72ec97aaeb3c78fb3bed");
    std::cout << cRandom->describe() << std::endl;
    for (uint64_t i = 0; i < 10; i++) {
        cRandom >> nR_i;
        std::cout << " hmac-sha-512 random: " << nR_i << std::endl;
    }

    // create the C API's random generator
    cRandom = qkd::utility::random_source::create("c-api");
    std::cout << cRandom->describe() << std::endl;
    for (uint64_t i = 0; i < 10; i++) {
        cRandom >> nR_i;
        std::cout << "        c-api random: " << nR_i << std::endl;
    }

    // create the C API's random generator with a fixed seed
    // TODO: Verify that these expected values are actually consistent across different environments
    const int32_t expected[] = {
              691102790,
            -1662155164,
              130162647,
            -1574942683,
             -346380092,
             1229004811,
             -118387889,
              334573773,
             -130276716,
            -2137708120
    };
    cRandom = qkd::utility::random_source::create("c-api:42");
    std::cout << cRandom->describe() << std::endl;
    for (uint64_t i = 0; i < 10; i++) {
        cRandom >> nR_i;
        std::cout << " c-api/seeded random: " << nR_i << std::endl;
        assert(nR_i == expected[i]);
    }
    
    return 0;
}


int main(UNUSED int argc, UNUSED char** argv) {
    return test();
}


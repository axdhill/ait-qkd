/*
 * buffer.cpp
 * 
 * This is a test file.
 *
 * TEST: test the qkd::utility::buffer class
 *
 * Autor: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
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


int test() {

    // empty constructor
    qkd::utility::buffer cBuffer;
    
    // values to push
    char c = -12;
    unsigned char uc = 37;
    
    int16_t s = -678;
    uint16_t us = 5096;
    
    int32_t i = -3209;
    uint32_t ui = 93458;
    
    int64_t l = -4328498989;
    uint64_t ul = 133847382;
    
    float f = 0.13f;
    double d = 13.430490903652;
    
    std::string str = "The quick brown fox jumped over the lazy dog.";
    
    qkd::utility::memory cMemory(1000);
    cMemory[0] = 97;
    cMemory[100] = 138;
    cMemory[200] = 14;
    cMemory[500] = 234;
    cMemory[999] = 1;
    
    // srteam into buffer
    cBuffer << c;
    cBuffer << uc;
    assert(cBuffer.size() == 2);
    
    cBuffer << s;
    cBuffer << us;
    assert(cBuffer.size() == 6);
    
    cBuffer << i;
    cBuffer << ui;
    assert(cBuffer.size() == 14);
    
    cBuffer << l;
    cBuffer << ul;
    assert(cBuffer.size() ==30);
    
    cBuffer << f;
    assert(cBuffer.size() == 34);
    
    cBuffer << d;
    assert(cBuffer.size() == 42);
    
    cBuffer << str;
    assert(cBuffer.size() == 95);
    
    cBuffer << cMemory;
    assert(cBuffer.size() == 1103);
    
    // copy buffer from memory
    qkd::utility::buffer cBufferCopy((qkd::utility::memory)cBuffer);

    // values to pop
    char c_2;
    unsigned char uc_2;
    int16_t s_2;
    uint16_t us_2;
    int32_t i_2;
    uint32_t ui_2;
    int64_t l_2;
    uint64_t ul_2;
    float f_2;
    double d_2;
    std::string str_2;
    qkd::utility::memory cMemory_2;
    
    // read values from the buffer and verify
    cBufferCopy >> c_2;
    cBufferCopy >> uc_2;
    assert((c_2 == c) && (uc_2 == uc));
    
    cBufferCopy >> s_2;
    cBufferCopy >> us_2;
    assert((s_2 == s) && (us_2 == us));
    
    cBufferCopy >> i_2;
    cBufferCopy >> ui_2;
    assert((i_2 == i) && (ui_2 == ui));
    
    cBufferCopy >> l_2;
    cBufferCopy >> ul_2;
    assert((l_2 == l) && (ul_2 == ul));
    
    cBufferCopy >> f_2;
    assert(f_2 == f);
    
    cBufferCopy >> d_2;
    assert(d_2 == d);
    
    cBufferCopy >> str_2;
    assert(str_2 == str);
    
    cBufferCopy >> cMemory_2;
    assert(cMemory_2[0] == 97);
    assert(cMemory_2[100] == 138);
    assert(cMemory_2[200] == 14);
    assert(cMemory_2[500] == 234);
    assert(cMemory_2[999] == 1);
   
    // even more fancy streaming

    std::list<uint64_t> l1{ 1, 4, 2000, 39898 };
    std::list<uint64_t> l2;

    qkd::utility::buffer lB;
    lB << l1;
    lB.reset();
    lB >> l2;
    auto li = l2.begin();
    assert(*li++ == 1);
    assert(*li++ == 4);
    assert(*li++ == 2000);
    assert(*li++ == 39898);
    
    std::set<char> s1{ 'q', 'k', 'd' };
    std::set<char> s2;

    qkd::utility::buffer sB;
    sB << s1;
    sB.reset();
    sB >> s2;
    assert(s2.find('q') != s2.end());
    assert(s2.find('k') != s2.end());
    assert(s2.find('d') != s2.end());

    std::vector<std::string> v1{ "blue", "green", "red", "yellow", "white", "black" };
    std::vector<std::string> v2;

    qkd::utility::buffer vB;
    vB << v1;
    vB.reset();
    vB >> v2;
    auto vi = v2.begin();
    assert(*vi++ == "blue");
    assert(*vi++ == "green");
    assert(*vi++ == "red");
    assert(*vi++ == "yellow");
    assert(*vi++ == "white");
    assert(*vi++ == "black");

    // all in one streaming memory madness
    qkd::utility::buffer cCrazyStreamer;
    cCrazyStreamer << std::string("crazy memory streaming stuff") << std::vector<int>{ 1, 2, 3, 7, 11, 13, 17, 19 } << 'c' << 'r' << 'a' << 'z' << 'y' << std::list<char>{'s', 't', 'r', 'e', 'a', 'm'};

    // read and verify
    cCrazyStreamer.reset();
    std::string cs;
    cCrazyStreamer >> cs;
    assert(cs == "crazy memory streaming stuff");
    std::vector<int> cvi;
    cCrazyStreamer >> cvi;
    assert(cvi[0] == 1);
    assert(cvi[1] == 2);
    assert(cvi[2] == 3);
    assert(cvi[3] == 7);
    assert(cvi[4] == 11);
    assert(cvi[5] == 13);
    assert(cvi[6] == 17);
    assert(cvi[7] == 19);
    char cc;
    cCrazyStreamer >> cc;
    assert(cc == 'c');
    cCrazyStreamer >> cc;
    assert(cc == 'r');
    cCrazyStreamer >> cc;
    assert(cc == 'a');
    cCrazyStreamer >> cc;
    assert(cc == 'z');
    cCrazyStreamer >> cc;
    assert(cc == 'y');
    std::list<char> clc;
    cCrazyStreamer >> clc;
    auto clc_iter = clc.begin();
    assert(*clc_iter++ == 's');
    assert(*clc_iter++ == 't');
    assert(*clc_iter++ == 'r');
    assert(*clc_iter++ == 'e');
    assert(*clc_iter++ == 'a');
    assert(*clc_iter++ == 'm');

    return 0;
}


int main(UNUSED int argc, UNUSED char** argv) {
    return test();
}


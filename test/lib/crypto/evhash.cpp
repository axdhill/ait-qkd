/*
 * evhash.cpp
 * 
 * This is a test file.
 *
 * TEST: test the qkd::crypto::evhash functions more deeply
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

#include <inttypes.h>


#include <chrono>
#include <fstream>
#include <iostream>

// include the all-in-one header
#include <qkd/qkd.h>


// ------------------------------------------------------------
// vars


char const * g_sText = {
"\
Lorem ipsum dolor sit amet, consectetur adipiscing elit. Integer nec odio. \
Praesent libero. Sed cursus ante dapibus diam. Sed nisi. Nulla quis sem at \
nibh elementum imperdiet. Duis sagittis ipsum. Praesent mauris. Fusce nec \
tellus sed augue semper porta. Mauris massa. Vestibulum lacinia arcu eget nulla. \
Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos \
himenaeos. Curabitur sodales ligula in libero. Sed dignissim lacinia nunc."
};



/*
char const * g_sText = {
"\
Lorem ipsum dolor sit amet, consectetur adipiscing elit. Integer nec odio. \
Praesent libero. Sed cursus ante dapibus diam. Sed nisi. Nulla quis sem at \
nibh elementum imperdiet. Duis sagittis ipsum. Praesent mauris. Fusce nec \
tellus sed augue semper porta. Mauris massa. Vestibulum lacinia arcu eget nulla. \
Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos \
himenaeos. Curabitur sodales ligula in libero. Sed dignissim lacinia nunc. \
\
Curabitur tortor. Pellentesque nibh. Aenean quam. In scelerisque sem at dolor. \
Maecenas mattis. Sed convallis tristique sem. Proin ut ligula vel nunc egestas \
porttitor. Morbi lectus risus, iaculis vel, suscipit quis, luctus non, massa. \
Fusce ac turpis quis ligula lacinia aliquet. Mauris ipsum. Nulla metus metus, \
ullamcorper vel, tincidunt sed, euismod in, nibh. Quisque volutpat condimentum \
velit.\
\
Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos \
himenaeos. Nam nec ante. Sed lacinia, urna non tincidunt mattis, tortor neque \
adipiscing diam, a cursus ipsum ante quis turpis. Nulla facilisi. Ut fringilla. \
Suspendisse potenti. Nunc feugiat mi a tellus consequat imperdiet. Vestibulum \
sapien. Proin quam. Etiam ultrices. Suspendisse in justo eu magna luctus suscipit. \
Sed lectus.\
\
Integer euismod lacus luctus magna. Quisque cursus, metus vitae pharetra auctor, \
sem massa mattis sem, at interdum magna augue eget diam. Vestibulum ante ipsum primis \
in faucibus orci luctus et ultrices posuere cubilia Curae; Morbi lacinia molestie dui. \
Praesent blandit dolor. Sed non quam. In vel mi sit amet augue congue elementum. \
Morbi in ipsum sit amet pede facilisis laoreet. Donec lacus nunc, viverra nec, \
blandit vel, egestas et, augue. Vestibulum tincidunt malesuada tellus. Ut ultrices \
ultrices enim. Curabitur sit amet mauris. Morbi in dui quis est pulvinar ullamcorper. \
Nulla facilisi.\
\
Integer lacinia sollicitudin massa. Cras metus. Sed aliquet risus a tortor. Integer \
id quam. Morbi mi. Quisque nisl felis, venenatis tristique, dignissim in, ultrices \
sit amet, augue. Proin sodales libero eget ante. Nulla quam. Aenean laoreet. Vestibulum \
nisi lectus, commodo ac, facilisis ac, ultricies eu, pede. Ut orci risus, accumsan \
porttitor, cursus quis, aliquet eget, justo. Sed pretium blandit orci."

};
*/


// ------------------------------------------------------------
// code



int test() {


    //unsigned int const nInputLoop = 1000000;
    unsigned int const nInputLoop = 1;

    std::chrono::high_resolution_clock::time_point nStart;
    std::chrono::high_resolution_clock::time_point nStop;


    char const * sInitKeyText32 = "abcd";
    char const * sInitKeyText64 = "abcdabcd";
    char const * sInitKeyText96 = "abcdabcdabcd";
    char const * sInitKeyText128 = "abcdabcdabcdabcd";
    char const * sInitKeyText256 = "abcdabcdabcdabcdabcdabcdabcdabcd";

    char const * sFinalKeyText32 = "1234";
    char const * sFinalKeyText64 = "12341234";
    char const * sFinalKeyText96 = "123412341234";
    char const * sFinalKeyText128 = "1234123412341234";
    char const * sFinalKeyText256 = "12341234123412341234123412341234";

    qkd::utility::memory cInputData = qkd::utility::memory::duplicate((unsigned char const *)g_sText, strlen(g_sText));
    qkd::utility::memory cTag;

    qkd::key::key cKeyInit;
    qkd::key::key cKeyFinal;

    uint64_t nNanoSec;
    unsigned int nTotalBytes = strlen(g_sText) * nInputLoop;
    double nNanoSecPerBlock;

    // --- 32 

    // keys
    cKeyInit = qkd::key::key(101, qkd::utility::memory(32/8));
    memcpy(cKeyInit.data().get(), sInitKeyText32, 32/8);
    cKeyFinal.data().resize(32/8);
    memcpy(cKeyFinal.data().get(), sFinalKeyText32, 32/8);

    // get context
    nStart = std::chrono::high_resolution_clock::now();
    qkd::crypto::crypto_context cEvHash32 = qkd::crypto::engine::create("evhash", cKeyInit);
    assert(cEvHash32->name() == "evhash");
    
    // add some data
    for (unsigned int i = 0; i < nInputLoop; i++) cEvHash32 << cInputData;
    nStop = std::chrono::high_resolution_clock::now();
   
    // get the final tag
    cTag = cEvHash32->finalize(cKeyFinal);

    nNanoSec = std::chrono::duration_cast<std::chrono::nanoseconds>(nStop - nStart).count();
    nNanoSecPerBlock = nNanoSec / (double)(nTotalBytes / (32 / 8));
    std::cout << "evhash-32: " 
            << nTotalBytes << " bytes "
            << "in " << nNanoSec << " ns, "
            << nNanoSecPerBlock << " ns/block, "
            << "tag = " << cTag.as_hex() << std::endl;    
    //assert(cTag.as_hex() == "3fdd4e0a");

    
    // --- 64

    // keys
    cKeyInit = qkd::key::key(102, qkd::utility::memory(64/8));
    memcpy(cKeyInit.data().get(), sInitKeyText64, 64/8);
    cKeyFinal.data().resize(64/8);
    memcpy(cKeyFinal.data().get(), sFinalKeyText64, 64/8);
    
    // get context
    nStart = std::chrono::high_resolution_clock::now();
    qkd::crypto::crypto_context cEvHash64 = qkd::crypto::engine::create("evhash", cKeyInit);
    assert(cEvHash64->name() == "evhash");

    // add some data
    for (unsigned int i = 0; i < nInputLoop; i++) cEvHash64 << cInputData;
    nStop = std::chrono::high_resolution_clock::now();

    // get the final tag
    cTag = cEvHash64->finalize(cKeyFinal);

    nNanoSec = std::chrono::duration_cast<std::chrono::nanoseconds>(nStop - nStart).count();
    nNanoSecPerBlock = nNanoSec / (double)(nTotalBytes / (64 / 8));
    std::cout << "evhash-64: " 
            << nTotalBytes << " bytes "
            << "in " << nNanoSec << " ns, "
            << nNanoSecPerBlock << " ns/block, "
            << "tag = " << cTag.as_hex() << std::endl;    
    //assert(cTag.as_hex() == "8eda6d76209ad7c3");


    // --- 96

    // keys
    cKeyInit = qkd::key::key(103, qkd::utility::memory(96/8));
    memcpy(cKeyInit.data().get(), sInitKeyText96, 96/8);
    cKeyFinal.data().resize(96/8);
    memcpy(cKeyFinal.data().get(), sFinalKeyText96, 96/8);
    
    // get context
    nStart = std::chrono::high_resolution_clock::now();
    qkd::crypto::crypto_context cEvHash96 = qkd::crypto::engine::create("evhash", cKeyInit);
    assert(cEvHash96->name() == "evhash");

    // add some data
    for (unsigned int i = 0; i < nInputLoop; i++) cEvHash96 << cInputData;
    nStop = std::chrono::high_resolution_clock::now();

    // get the final tag
    cTag = cEvHash96->finalize(cKeyFinal);

    nNanoSec = std::chrono::duration_cast<std::chrono::nanoseconds>(nStop - nStart).count();
    nNanoSecPerBlock = nNanoSec / (double)(nTotalBytes / (96 / 8));
    std::cout << "evhash-96: " 
            << nTotalBytes << " bytes "
            << "in " << nNanoSec << " ns, "
            << nNanoSecPerBlock << " ns/block, "
            << "tag = " << cTag.as_hex() << std::endl;    
    //assert(cTag.as_hex() == "94562490caf21f74e970b6ea");


    // --- 128

    // keys
    cKeyInit = qkd::key::key(104, qkd::utility::memory(128/8));
    memcpy(cKeyInit.data().get(), sInitKeyText128, 128/8);
    cKeyFinal.data().resize(128/8);
    memcpy(cKeyFinal.data().get(), sFinalKeyText128, 128/8);
    
    // get context
    nStart = std::chrono::high_resolution_clock::now();
    qkd::crypto::crypto_context cEvHash128 = qkd::crypto::engine::create("evhash", cKeyInit);
    assert(cEvHash128->name() == "evhash");
    
    // add some data
    for (unsigned int i = 0; i < nInputLoop; i++) cEvHash128 << cInputData;
    nStop = std::chrono::high_resolution_clock::now();

    // get the final tag
    cTag = cEvHash128->finalize(cKeyFinal);

    nNanoSec = std::chrono::duration_cast<std::chrono::nanoseconds>(nStop - nStart).count();
    nNanoSecPerBlock = nNanoSec / (double)(nTotalBytes / (128 / 8));
    std::cout << "evhash-128: " 
            << nTotalBytes << " bytes "
            << "in " << nNanoSec << " ns, "
            << nNanoSecPerBlock << " ns/block, "
            << "tag = " << cTag.as_hex() << std::endl;    
    //assert(cTag.as_hex() == "1181efe0f3f97ea90c7f2f5bfe40a448");


    // --- 256

    // keys
    cKeyInit = qkd::key::key(105, qkd::utility::memory(256/8));
    memcpy(cKeyInit.data().get(), sInitKeyText256, 256/8);
    cKeyFinal.data().resize(256/8);
    memcpy(cKeyFinal.data().get(), sFinalKeyText256, 256/8);
    
    // get context
    nStart = std::chrono::high_resolution_clock::now();
    qkd::crypto::crypto_context cEvHash256 = qkd::crypto::engine::create("evhash", cKeyInit);
    assert(cEvHash256->name() == "evhash");
    
    // add some data
    for (unsigned int i = 0; i < nInputLoop; i++) cEvHash256 << cInputData;
    cTag = cEvHash256->finalize(cKeyFinal);

    // get the final tag
    nStop = std::chrono::high_resolution_clock::now();

    nNanoSec = std::chrono::duration_cast<std::chrono::nanoseconds>(nStop - nStart).count();
    nNanoSecPerBlock = nNanoSec / (double)(nTotalBytes / (256 / 8));
    std::cout << "evhash-256: " 
            << nTotalBytes << " bytes "
            << "in " << nNanoSec << " ns, "
            << nNanoSecPerBlock << " ns/block, "
            << "tag = " << cTag.as_hex() << std::endl;    
    //assert(cTag.as_hex() == "ef0fd5bff03091296466ac8dabdb3a9effeba59f82992750c48c95f3e79be7ce");

    return 0;
}


int main(UNUSED int argc, UNUSED char** argv) {
    return test();
}


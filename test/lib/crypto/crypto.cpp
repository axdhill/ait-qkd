/*
 * crypto.cpp
 * 
 * This is a test file.
 *
 * TEST: test the qkd::crypto class
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
    
    
    qkd::utility::memory cMemoryInput;
    qkd::utility::memory cMemoryOutput;
    qkd::utility::memory cMemoryKeyInit;
    qkd::utility::memory cMemoryKeyFinal;
    
    qkd::key::key cKeyInit;
    qkd::key::key cKeyFinal;
    
    char const * sInputKeyText32 = "abcd";
    char const * sInputKeyText64 = "abcdabcd";
    char const * sInputKeyText96 = "abcdabcdabcd";
    char const * sInputKeyText128 = "abcdabcdabcdabcd";
    char const * sInputKeyText256 = "abcdabcdabcdabcdabcdabcdabcdabcd";

    char const * sOutputKeyText32 = "1234";
    char const * sOutputKeyText64 = "12341234";
    char const * sOutputKeyText96 = "123412341234";
    char const * sOutputKeyText128 = "1234123412341234";
    char const * sOutputKeyText256 = "12341234123412341234123412341234";

    // create a memory input
    char const * sInputText = "The quick brown fox jumps over the lazy dog";
    cMemoryInput = qkd::utility::memory(strlen(sInputText));
    memcpy(cMemoryInput.get(), sInputText, strlen(sInputText));
    
    // create the key
    char const * sInputKeyText = "abcdefghijklmnopqrstuvwxyz0123456789abcdefg";
    cMemoryKeyFinal = qkd::utility::memory(strlen(sInputKeyText));
    memcpy(cMemoryKeyFinal.get(), sInputKeyText, strlen(sInputKeyText));
    cKeyFinal = qkd::key::key(1, cMemoryKeyFinal);
    
    qkd::crypto::crypto_context cCloneContext;
    
    
    // --- SCHEMES ---
    
    // arbitrary schemes
    std::string sScheme = "evhash-96:02cc942de299f4b0d86ffd53:fd2cf893f0cfe670d89183dd:12345";
    qkd::crypto::scheme cScheme(sScheme);
    
    assert(cScheme.str() == sScheme);
    assert(qkd::crypto::engine::valid_scheme(cScheme));
    assert(cScheme.init_key().data().as_hex() == "02cc942de299f4b0d86ffd53");
    assert(cScheme.state().as_hex() == "fd2cf893f0cfe670d89183dd");
    assert(cScheme.blocks() == 12345);
    
    assert(qkd::crypto::engine::valid_scheme(qkd::crypto::scheme("evhash-96::")));
    assert(qkd::crypto::engine::valid_scheme(qkd::crypto::scheme("evhash-96")));
    assert(!qkd::crypto::engine::valid_scheme(qkd::crypto::scheme("evhash-96:02cc942de299")));
    assert(!qkd::crypto::engine::valid_scheme(qkd::crypto::scheme("evhash")));
    
    // concrete schemes
    assert(qkd::crypto::engine::valid_scheme(qkd::crypto::scheme("null")));
    assert(qkd::crypto::engine::valid_scheme(qkd::crypto::scheme("evhash-32")));
    assert(qkd::crypto::engine::valid_scheme(qkd::crypto::scheme("evhash-64")));
    assert(qkd::crypto::engine::valid_scheme(qkd::crypto::scheme("evhash-96")));
    assert(qkd::crypto::engine::valid_scheme(qkd::crypto::scheme("evhash-128")));
    assert(qkd::crypto::engine::valid_scheme(qkd::crypto::scheme("evhash-256")));
    assert(qkd::crypto::engine::valid_scheme(qkd::crypto::scheme("xor")));
    
    
    // --- NULL ---
    
    // create a NULL context
    qkd::crypto::crypto_context cNULL = qkd::crypto::engine::create("null");
    assert(cNULL->name() == "null");
    

    // --- XOR ---
    
    // create a XOR context
    qkd::crypto::crypto_context cXOR = qkd::crypto::engine::create("xor");
    assert(cXOR->name() == "xor");
    
    // add some items to the crypto context
    cXOR << cMemoryInput;
    
    // get the XOR result
    cMemoryOutput = cXOR->finalize(cKeyFinal);
    
    // check data
    assert(cMemoryInput.as_hex() == "54686520717569636b2062726f776e20666f78206a756d7073206f76657220746865206c617a7920646f67");
    assert(cKeyFinal.data().as_hex() == "6162636465666768696a6b6c6d6e6f707172737475767778797a3031323334353637383961626364656667");
    assert(cMemoryOutput.as_hex() == "350a064414130e0b024a091e02190150171d0b541f031a080a5a5f47574114415e52185500181a44010900");
    
    // convert back
    cMemoryInput = cMemoryOutput;
    cXOR = qkd::crypto::engine::create("xor", cKeyInit);
    cXOR << cMemoryInput;
    cMemoryOutput = cXOR->finalize(cKeyFinal);
    
    // "The quick brown fox jumps over the lazy dog"
    assert(cMemoryOutput.as_hex() == "54686520717569636b2062726f776e20666f78206a756d7073206f76657220746865206c617a7920646f67");
    

    // --- evaluation hash ---

    // prepare input
    cMemoryInput = qkd::utility::memory(strlen(sInputText));
    memcpy(cMemoryInput.get(), sInputText, strlen(sInputText));
    
    
    // --- 32 

    // create init key
    cKeyInit = qkd::key::key(101, qkd::utility::memory(32/8));
    memcpy(cKeyInit.data().get(), sInputKeyText32, 32/8);
    
    // get context
    qkd::crypto::crypto_context cEvHash32 = qkd::crypto::engine::create("evhash", cKeyInit);
    assert(cEvHash32->name() == "evhash");
    
    // add 10 times some data
    for (unsigned int i = 0; i < 10; i++) cEvHash32 << cMemoryInput;

    // create final key
    cKeyFinal.data().resize(32/8);
    memcpy(cKeyFinal.data().get(), sOutputKeyText32, 32/8);
    
    // get the final tag
    cCloneContext = cEvHash32->clone();
    cMemoryOutput = cEvHash32->finalize(cKeyFinal);
    
    // check result
    assert(cMemoryOutput.as_hex() == "c32a0b7b");
    assert(cCloneContext->finalize(cKeyFinal).equal(cMemoryOutput));

    
    // --- 64

    // create init key
    cKeyInit = qkd::key::key(102, qkd::utility::memory(64/8));
    memcpy(cKeyInit.data().get(), sInputKeyText64, 64/8);
    
    // get context
    qkd::crypto::crypto_context cEvHash64 = qkd::crypto::engine::create("evhash", cKeyInit);
    assert(cEvHash64->name() == "evhash");

    // add 10 times some data
    for (uint32_t i = 0; i < 10; i++) cEvHash64 << cMemoryInput;

    // create final key
    cKeyFinal.data().resize(64/8);
    memcpy(cKeyFinal.data().get(), sOutputKeyText64, 64/8);
    
    // get the final tag
    cCloneContext = cEvHash64->clone();
    cMemoryOutput = cEvHash64->finalize(cKeyFinal);
    
    // check result
    assert(cMemoryOutput.as_hex() == "1982990231082d62");
    assert(cCloneContext->finalize(cKeyFinal).equal(cMemoryOutput));


    // --- 96

    // create init key
    cKeyInit = qkd::key::key(103, qkd::utility::memory(96/8));
    memcpy(cKeyInit.data().get(), sInputKeyText96, 96/8);
    
    // get context
    qkd::crypto::crypto_context cEvHash96 = qkd::crypto::engine::create("evhash", cKeyInit);
    assert(cEvHash96->name() == "evhash");

    // add 10 times some data
    for (uint32_t i = 0; i < 10; i++) cEvHash96 << cMemoryInput;

    // create final key
    cKeyFinal.data().resize(96/8);
    memcpy(cKeyFinal.data().get(), sOutputKeyText96, 96/8);
    
    // get the final tag
    cCloneContext = cEvHash96->clone();
    cMemoryOutput = cEvHash96->finalize(cKeyFinal);
    
    // check result
    assert(cMemoryOutput.as_hex() == "43ab557341855d972fcdeada");
    assert(cCloneContext->finalize(cKeyFinal).equal(cMemoryOutput));


    // --- 128

    // create init key
    cKeyInit = qkd::key::key(104, qkd::utility::memory(128/8));
    memcpy(cKeyInit.data().get(), sInputKeyText128, 128/8);
    
    // get context
    qkd::crypto::crypto_context cEvHash128 = qkd::crypto::engine::create("evhash", cKeyInit);
    assert(cEvHash128->name() == "evhash");
    
    // add 10 times some data
    for (uint32_t i = 0; i < 10; i++) cEvHash128 << cMemoryInput;

    // create final key
    cKeyFinal.data().resize(128/8);
    memcpy(cKeyFinal.data().get(), sOutputKeyText128, 128/8);
    
    // get the final tag
    cCloneContext = cEvHash128->clone();
    cMemoryOutput = cEvHash128->finalize(cKeyFinal);
    
    // check result
    assert(cMemoryOutput.as_hex() == "994d223422160f4cdcc79839cd3205d0");
    assert(cCloneContext->finalize(cKeyFinal).equal(cMemoryOutput));


    // --- 256

    // create init key
    cKeyInit = qkd::key::key(105, qkd::utility::memory(256/8));
    memcpy(cKeyInit.data().get(), sInputKeyText256, 256/8);
    
    // get context
    qkd::crypto::crypto_context cEvHash256 = qkd::crypto::engine::create("evhash", cKeyInit);
    assert(cEvHash256->name() == "evhash");
    
    // add 10 times some data
    for (uint32_t i = 0; i < 10; i++) cEvHash256 << cMemoryInput;
    
    // create final key
    cKeyFinal.data().resize(256/8);
    memcpy(cKeyFinal.data().get(), sOutputKeyText256, 256/8);
    
    // get the final tag
    cCloneContext = cEvHash256->clone();
    cMemoryOutput = cEvHash256->finalize(cKeyFinal);
    
    // check result
    assert(cMemoryOutput.as_hex() == "05df48f9ff890eb250b18178264ced0e8d311042bb3d3495f7bd195d79b44acc");
    assert(cCloneContext->finalize(cKeyFinal).equal(cMemoryOutput));
   

    // --- context reuse
    
    // here we create 3 evhash with the same init key
    // thus: all 3 evhash share the same context in the backend then
    // however: this is not visible at the frontend (here)
    
    cKeyInit = qkd::key::key(201, qkd::utility::memory(96/8));
    memcpy(cKeyInit.data().get(), sInputKeyText96, 96/8);
    
    // get context - #1
    cEvHash96 = qkd::crypto::engine::create("evhash", cKeyInit);

    // add 10 times some data
    for (uint32_t i = 0; i < 10; i++) cEvHash96 << cMemoryInput;
    
    cKeyFinal.data().resize(96/8);
    memcpy(cKeyFinal.data().get(), "123456789012", 96/8);
    cMemoryOutput = cEvHash96->finalize(cKeyFinal);

    // get context - #2
    cEvHash96 = qkd::crypto::engine::create("evhash", cKeyInit);

    // add 10 times some data
    for (uint32_t i = 0; i < 10; i++) cEvHash96 << cMemoryInput;
    
    cKeyFinal.data().resize(96/8);
    memcpy(cKeyFinal.data().get(), "abcdefghijkl", 96/8);
    cMemoryOutput = cEvHash96->finalize(cKeyFinal);

    // get context - #3
    cEvHash96 = qkd::crypto::engine::create("evhash", cKeyInit);

    // add 10 times some data
    for (unsigned int i = 0; i < 10; i++) cEvHash96 << cMemoryInput;
    
    cKeyFinal.data().resize(96/8);
    memcpy(cKeyFinal.data().get(), "123456abcdef", 96/8);
    cMemoryOutput = cEvHash96->finalize(cKeyFinal);
    

    // --- the unknown algorithm ---
    
    try {
        qkd::crypto::crypto_context cXOR = qkd::crypto::engine::create("john_doe");
        assert("unknown algorithm");
    }
    catch (qkd::crypto::engine::algorithm_unknown & cException) {}
    

    // --- crypto scheme string testing ---
        
    // scheme test data
    const char * sText_A = "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. ";
    const char * sText_B = "Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";
    
    cScheme = qkd::crypto::scheme("evhash-96:1e58217ab632751f02fa966c");
    qkd::key::key cFinalKey = qkd::key::key(1, qkd::utility::memory::from_hex("83c4db79fdf2c6e5b5d25889"));
    
    qkd::utility::memory cMemA(strlen(sText_A));
    qkd::utility::memory cMemB(strlen(sText_B));
    memcpy(cMemA.get(), sText_A, strlen(sText_A));
    memcpy(cMemB.get(), sText_B, strlen(sText_B));
    
    // EvHash (only 96 Bit as an example)
    
    // create via string and key
    qkd::crypto::crypto_context cEvHash96_Scheme_A = qkd::crypto::engine::create("evhash", cScheme.init_key());
    qkd::crypto::scheme cScheme_1 = cEvHash96_Scheme_A->scheme();
    cEvHash96_Scheme_A << cMemA;
    qkd::crypto::scheme cScheme_2 = cEvHash96_Scheme_A->scheme();
    cEvHash96_Scheme_A << cMemB;
    qkd::crypto::scheme cScheme_3 = cEvHash96_Scheme_A->scheme();
    
    qkd::utility::memory cTag_A = cEvHash96_Scheme_A->finalize(cKeyFinal);
    
    // scheme step 1
    qkd::crypto::crypto_context cEvHash96_Scheme_B = qkd::crypto::engine::create(cScheme);
    cEvHash96_Scheme_B << cMemA;
    cEvHash96_Scheme_B << cMemB;
    qkd::utility::memory cTag_B = cEvHash96_Scheme_B->finalize(cKeyFinal);
    
    assert(cTag_A.equal(cTag_B));
    
    // scheme step 2
    qkd::crypto::crypto_context cEvHash96_Scheme_C = qkd::crypto::engine::create(cScheme_1);
    cEvHash96_Scheme_C << cMemA;
    cEvHash96_Scheme_C << cMemB;
    qkd::utility::memory cTag_C = cEvHash96_Scheme_C->finalize(cKeyFinal);
    
    assert(cTag_A.equal(cTag_C));

    // scheme step 3
    qkd::crypto::crypto_context cEvHash96_Scheme_D = qkd::crypto::engine::create(cScheme_2);
    cEvHash96_Scheme_D << cMemB;
    qkd::utility::memory cTag_D = cEvHash96_Scheme_D->finalize(cKeyFinal);
    
    assert(cTag_A.equal(cTag_D));

    // scheme step 3
    qkd::crypto::crypto_context cEvHash96_Scheme_E = qkd::crypto::engine::create(cScheme_3);
    qkd::utility::memory cTag_E = cEvHash96_Scheme_E->finalize(cKeyFinal);
    
    assert(cTag_A.equal(cTag_E));
    
    return 0;
}


int main(UNUSED int argc, UNUSED char** argv) {
    return test();
}


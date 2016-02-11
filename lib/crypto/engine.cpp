/*
 * engine.cpp
 * 
 * crypto engine main interface implementation
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

 
// ------------------------------------------------------------
// incs

// ait
#include <qkd/common_macros.h>
#include <qkd/crypto/context.h>
#include <qkd/crypto/engine.h>

#include "crypto_evhash.h"
#include "crypto_null.h"
#include "crypto_xor.h"

using namespace qkd::crypto;


// ------------------------------------------------------------
// code


/**
 * factory method to create a crypto context
 * 
 * the given Algorithm identifies the algorithm used
 * 
 * currently supported:
 * 
 *      "null"          The empty NULL instance (does not do any crypto stuff)
 *      "evhash"        evaluation hash
 *      "xor"           binary xor encryption (init key is ignored)
 * 
 * Some algorithms need keys as input, some need keys as output and
 * some need keys at both stages. The nature of the algorithm is indicated
 * with the input key. That is: if you are going for an 96 bit evaluation
 * hash, you provide an 96 bit input key. If you want a 128 bit tag length
 * you need an 128 bit input key.
 * 
 *      e.g.:
 * 
 *              // create a 96 bit key
 *              qkd::key::key cKey;
 *              cKey.resize(96/8);
 * 
 *              // get the key data
 *              cKey.get()[0] = 0x12;
 *              cKey.get()[1] = 0x0C;
 *              ...
 * 
 *              // create the crypto context
 *              // this will now create a 96 bit evaluation hash
 *              qkd::crypto::crypto_context cContext = qkd::crypto::engine::create("ev-hash", cKey);
 * 
 * Currently available algorithms and tag sizes (== length of input key)
 * 
 *  name:   "null"
 *      need input key:         no
 *      need output key:        no
 *      input key bit size:     any
 *      remarks:                is an empty, void instance. does not encryption nor authentication
 * 
 *  name:   "evhash"
 *      need input key:         yes
 *      need output key:        yes
 *      input key bit size:     32, 64, 96, 128, 256
 *      remarks:                evhash with at least 96 bit is recommended
 * 
 * name:    "xor"
 *      need input key:         no
 *      need output key:        yes
 *      input key bit size:     -
 *      remarks:                this is the XOR encryption. the output key
 *                              must have a minimum length as the input data
 * 
 * Note, this interface could change in future.
 * 
 * The given key may or may not meet the algorithm requirements. If it does not
 * a context_wrong_key exception is thrown.
 * 
 * If during algorithm setup something goes awry a context_init exception
 * is thrown.
 * 
 * @param   sAlgorithm      string identifying algorithm 
 * @param   cKey            key used to setup the algorithm, the init key
 * @return  an initialized crypto context
 * @throws  algorithm_unknown
 * @throws  context_init
 * @throws  context_wrong_key
 */
crypto_context engine::create(std::string sAlgorithm, qkd::key::key const & cKey) {

    // treat different algorithms
    
    if (sAlgorithm == "null") {
        return std::shared_ptr<context>(new crypto_null(cKey));
    }
    
    if (sAlgorithm == "evhash") {
        if (!crypto_evhash::is_valid_input_key(cKey)) throw qkd::crypto::context::context_wrong_key();
        return std::shared_ptr<context>(new crypto_evhash(cKey));
    }
   
    if (sAlgorithm == "xor") {
        if (!crypto_xor::is_valid_input_key(cKey)) throw qkd::crypto::context::context_wrong_key();
        return std::shared_ptr<context>(new crypto_xor(cKey));
    }

    throw qkd::crypto::engine::algorithm_unknown();
}


/**
 * factory method to create a crypto context based on a scheme string
 * 
 * this instantiates an crypto context based on a crypto scheme string
 * 
 * A crypt scheme string has this syntax:
 * 
 *          "ALGORITHM[-VARIANT][:INITKEY[:STATE]]"
 * 
 * E.g.:
 * 
 *          "evhash-96"
 *          "evhash-96:87103893a579"
 *          "evhash-96:02cc942de299:f4b0d86ffd53"
 *          "xor"
 *          "null"
 * 
 * @param   cScheme         the crypto scheme string
 * @return  an initialized crypto context
 * @throws  scheme_invalid
 * @throws  algorithm_unknown
 * @throws  context_init
 * @throws  context_wrong_key
 */
crypto_context engine::create(qkd::crypto::scheme const & cScheme) {
    
    if (!valid_scheme(cScheme)) throw qkd::crypto::engine::scheme_invalid();
    
    // get the context based on the info we have now
    crypto_context cCryptoContext = create(cScheme.name(), cScheme.init_key());
    
    // apply state and blocks
    if (cScheme.state().get() != nullptr) {
        cCryptoContext->set_state(cScheme.state());
    }
    
    return cCryptoContext;
}


/**
 * check for a valid scheme name
 * 
 * @param   cScheme     the scheme to check
 * @return  true, if the scheme name is valid
 */
bool engine::valid_scheme(qkd::crypto::scheme const & cScheme) { 

    // check for the known algorithms
    try {
        create(cScheme.name(), cScheme.init_key());
        return true;
    }
    catch (...) { }
 
    return false;
}


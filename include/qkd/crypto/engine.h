/*
 * engine.h
 * 
 * crypto engine interface
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

 
#ifndef __QKD_CRYPTO_ENGINE_H_
#define __QKD_CRYPTO_ENGINE_H_


// ------------------------------------------------------------
// incs

#include <exception>
#include <string>

// ait
#include <qkd/key/key.h>
#include <qkd/crypto/context.h>
#include <qkd/crypto/scheme.h>


// ------------------------------------------------------------
// decls


namespace qkd {
    
namespace crypto {    


/**
 * this is the main crypto engine interface
 * 
 * Here a crypto context is created, which is capable of
 * performing cryptographic algorithms.
 * 
 * A crypto context might be identified by using a crypto "scheme"
 * and looks like:
 * 
 *          "ALGORITHM[-VARIANT][:INITKEY[:STATE]]"
 * 
 * E.g.:
 * 
 *          "evhash-96"
 *          "evhash-96:87103893a579"
 *          "evhash-96:02cc942de299:f4b0d86ffd53"
 *          "evhash-96:02cc942de299:f4b0d86ffd53:12345"
 *          "xor"
 *          "null"
 * 
 * see the qkd::crypto::scheme class for details.
 */
class engine {

    
public:


    /**
     * exception type thrown for unknown crypto algorithm
     */
    struct algorithm_unknown : virtual std::exception, virtual boost::exception { };
    
    
    /**
     * exception type thrown for an invalid scheme
     */
    struct scheme_invalid : virtual std::exception, virtual boost::exception { };
    
    
    /**
     * factory method to create a crypto context
     * 
     * the given Algorithm identifies the algorithm used
     * 
     * currently supported:
     * 
     *      "null"          The empty NULL instance (does not do any crpto stuff)
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
    static crypto_context create(std::string sAlgorithm, qkd::key::key const & cKey = qkd::key::key::null());
    
    
    /**
     * factory method to create a crypto context based on a scheme string
     * 
     * this instantiates an crypto context based on a crypto scheme string
     * 
     * A crytp scheme string has this syntax:
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
     * @param   cScheme         a crypto scheme
     * @return  an initialized crypto context
     * @throws  algorithm_unknown
     * @throws  scheme_invalid
     * @throws  context_init
     * @throws  context_wrong_key
     */
    static crypto_context create(qkd::crypto::scheme const & cScheme);
    
    
    /**
     * check for a valid scheme
     * 
     * @param   cScheme     the scheme to check
     * @return  true, if the scheme is valid
     */
    static bool valid_scheme(qkd::crypto::scheme const & cScheme);

    
private:
    
    
    /**
     * ctor
     */
    engine() {};
    
};
  

}

}

#endif


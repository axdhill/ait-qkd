/*
 * scheme.h
 * 
 * a crypto scheme
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

 
#ifndef __QKD_CRYPTO_SCHEME_H_
#define __QKD_CRYPTO_SCHEME_H_


// ------------------------------------------------------------
// incs

#include <exception>
#include <string>

// ait
#include <qkd/key/key.h>
#include <qkd/crypto/context.h>


// ------------------------------------------------------------
// decls


namespace qkd {
    
namespace crypto {    


/**
 * this is a crypto scheme.
 * 
 * A scheme is initialized by a string which has this syntax
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
 */
class scheme {

    
public:


    /**
     * ctor
     * 
     * @param   sScheme         the crypto scheme string
     */
    explicit scheme(std::string const sScheme = "null");
    
    
    /**
     * returns the init key stored
     * 
     * @return  the key stored in a scheme
     */
    inline qkd::key::key const & init_key() const { return m_cInitKey; };
    
    
    /**
     * return the algorithm name
     * 
     * @return  the name of the algorithm
     */
    inline std::string const & name() const { return m_sName; };
    
    
    /**
     * checks if this is the NULL scheme
     * 
     * @return  true if this is the NULL scheme
     */
    inline bool null() const { return m_sName == "null"; };
    
    
    /**
     * returns the state stored
     * 
     * @return  the state in a scheme
     */
    inline qkd::utility::memory const & state() const { return m_cState; };
    
    
    /**
     * return a stringified version of this scheme
     * 
     * @return  a string which can be used to create this scheme again
     */
    std::string str() const;
    

private:    
    
    
    /**
     * the init key stored
     */
    qkd::key::key m_cInitKey;
    
    
    /**
     * the algorithm name
     */
    std::string m_sName;
    
    
    /**
     * the algorithm state
     */
    qkd::utility::memory m_cState;
    
};
  

}

}

#endif


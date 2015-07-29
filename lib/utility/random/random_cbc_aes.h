/*
 * random_cbc_aes.h
 * 
 * random number generator interface CBC-AES
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

 
#ifndef __QKD_UTLITY_RANDOM_CBC_AES_H_
#define __QKD_UTLITY_RANDOM_CBC_AES_H_


// ------------------------------------------------------------
// incs

#include <exception>
#include <fstream>
#include <string>

#include <openssl/x509.h>

// Qt
#include <QtCore/QUrl>

// ait
#include <qkd/utility/random.h>


// ------------------------------------------------------------
// decls


namespace qkd {
    
namespace utility {    


/**
 * this class creates random numbers based on CBC-AES-128, CBC-AES-192, CBC-AES-256
 */
class random_cbc_aes : public qkd::utility::random_source {


public:


    /**
     * ctor
     */
    explicit random_cbc_aes(std::string const & sCBCAES) : m_sCBCAES(sCBCAES), m_nCounter(0) { init(); };


    /**
     * describe the random source
     * 
     * @return  a HR-string describing the random source
     */
    virtual std::string describe() const;
    

private:

    
    /**
     * get a block of random bytes
     * 
     * This function must be overwritten in derived classes
     * 
     * @param   cBuffer     buffer which will hold the bytes
     * @param   nSize       size of buffer in bytes
     */
    virtual void get(char * cBuffer, uint64_t nSize);
    
    
    /**
     * init the object
     */
    void init();
    
    
    /**
     * the CBS-AES init
     */
    std::string m_sCBCAES;
    

    /**
     * the counter
     */
    uint64_t m_nCounter;
    
    
    /**
     * key used
     */
    qkd::utility::memory m_cKey;
    
    
    /**
     * OpenSSL cypher context
     */
    EVP_CIPHER_CTX m_cCipherContext;    
    
};


}

}

#endif


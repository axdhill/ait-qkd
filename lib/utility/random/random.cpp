/*
 * random.cpp
 * 
 * implement the main random object
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

 
// ------------------------------------------------------------
// incs

// ait
#include <qkd/utility/random.h>

#include "random_c_api.h"
#include "random_cbc_aes.h"
#include "random_file.h"
#include "random_hmac_sha.h"


using namespace qkd::utility;


// ------------------------------------------------------------
// decl


namespace qkd {
    
namespace utility {

/**
 * this class constructs the main random singelton source
 */
class random_singelton {
    
    
public:
    
    
    /**
     * ctor
     */
    random_singelton() { m_cRandom = random_source::create(); };
    
    
    /**
     * return the main random singelton
     * 
     * @return  the main random singelton
     */
    qkd::utility::random & get() { return m_cRandom; };
    
    
    /**
     * sets the main random singelton
     * 
     * @param   cRandom     the new main random singelton
     */
    void set(qkd::utility::random & cRandom) { m_cRandom = cRandom; };


private:    
    
    
    /**
     * the main random singelton
     */
    qkd::utility::random m_cRandom;
    
};

}

}


// ------------------------------------------------------------
// vars


/**
 * the main random singelton
 */
static random_singelton g_cRandomSingelton;



// ------------------------------------------------------------
// code



/**
 * factory method to create a random source
 *
 * @param   sURL        a URL string indicating the random source
 * @return  an initialized random object
 */
qkd::utility::random random_source::create(std::string sURL) {

    // check for empty url
    if (sURL.empty()) return boost::shared_ptr<random_source>(new qkd::utility::random_c_api());
    
    // check what we have
    QUrl cURL(QString::fromStdString(sURL), QUrl::TolerantMode);
    
    // switch for the correct scheme
    if (cURL.isLocalFile()) {
        return boost::shared_ptr<random_source>(new qkd::utility::random_file(cURL));
    }
    else if (sURL.substr(0, std::string("cbc-aes").length()) == "cbc-aes") {
        return boost::shared_ptr<random_source>(new qkd::utility::random_cbc_aes(sURL));
    }
    else if (sURL.substr(0, std::string("hmac-sha").length()) == "hmac-sha") {
        return boost::shared_ptr<random_source>(new qkd::utility::random_hmac_sha(sURL));
    }
    else {
        throw random_url_scheme_unknown();
    }
    
    return boost::shared_ptr<random_source>(new qkd::utility::random_source());
}


/**
 * sets the main random singelton source
 *
 * @param   cRandom     the new random singelton source
 */
void random_source::set_source(qkd::utility::random & cRandom) {
    g_cRandomSingelton.set(cRandom);
}


/**
 * returns the main random singelton source
 *
 * @return  the main random singelton
 */
qkd::utility::random & random_source::source() {
    return g_cRandomSingelton.get();
}

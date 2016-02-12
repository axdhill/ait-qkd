/*
 * random_c_api.h
 * 
 * random number generator using srand() and rand() from the C API
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

 
#ifndef __QKD_UTLITY_RANDOM_C_API_H_
#define __QKD_UTLITY_RANDOM_C_API_H_


// ------------------------------------------------------------
// incs

#include <exception>
#include <string>
#include <boost/algorithm/string.hpp>

// Qt
#include <QtCore/QFile>
#include <QtCore/QUrl>

// ait
#include "qkd/utility/random.h"


// ------------------------------------------------------------
// decls


namespace qkd {
    
namespace utility {    


/**
 * this class uses the C API rand() and srand() to retrieve random numbers
 */
class random_c_api : public qkd::utility::random_source {


public:


    /**
     * ctor
     */
    explicit random_c_api();

    /**
     * ctor
     */
    explicit random_c_api(std::string const & sURL);

    
    /**
     * describe the random source
     * 
     * @return  a HR-string describing the random source
     */
    virtual std::string describe() const { return "random source using POSIX C API rand() function"; };

    /**
     * Enforces usage of the specified random seed.
     *
     * @param  seed  the seed value to use.
     */
    void seed(unsigned int seed) const;
    

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
     * 
     * @param   nSize       size of buffer in bytes
     */
    void init();
};


}

}

#endif


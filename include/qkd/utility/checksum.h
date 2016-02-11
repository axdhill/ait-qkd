/*
 * checksum.h
 * 
 * create and manage checksums
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

 
#ifndef __QKD_UTILITY_CHECKSUM_H_
#define __QKD_UTILITY_CHECKSUM_H_


// ------------------------------------------------------------
// incs

#include <exception>
#include <memory>
#include <string>

// ait
#include <qkd/utility/memory.h>


// ------------------------------------------------------------
// decls


namespace qkd {
    
namespace utility {    


// fwd
class checksum_algorithm;
typedef std::shared_ptr<checksum_algorithm> checksum;


/**
 * this class can create checksums for arbitrary objects
 * 
 * This is actually a factory class. Concrete checksum instances are created by
 * invoking the checksum_algorithm::create function providing the a checksum name.
 * Currently we support:
 * 
 *  - "crc32"
 *  - "md5"
 *  - "sha1"
 * 
 * Usage example:
 * 
 *          qkd::utility::checksum algorithm = qkd::utility::checksum_algorithm::create("md5");
 * 
 *          qkd::utility::buffer data;
 *          data << std::string("The big brown fox jumped over the lazy dog");
 *          data << 3.1415;
 *          
 *          algorithm << data;
 *          qkd::utility::memory checksum;
 * 
 *          algorithm >> checksum;
 * 
 *          std::cout << "The checksum is: " << checksum.as_hex() << std::endl;
 */
class checksum_algorithm {


public:


    /**
     * dtor
     */
    virtual ~checksum_algorithm() {}


    /**
     * stream into
     *
     * @param   cMemory         memory block to stream into algorithm
     * @return  algorithm
     */
    inline checksum_algorithm & operator<<(memory const & cMemory) { 
        add(cMemory); 
        return *this; 
    }


    /**
     * stream out
     *
     * @param   cMemory         memory block to stream out from the algorithm
     * @return  algorithm
     */
    inline checksum_algorithm & operator>>(memory & cMemory) { 
        cMemory = finalize(); 
        return *this; 
    }


    /**
     * add a memory area to the calculation
     *
     * @param   cMemory         memory block to be added
     * @throws  checksum_algorithm_final, if the algorithm has finished and does not allow another addition
     */
    virtual void add(memory const & cMemory) = 0;


    /**
     * factory method to create a known algorithm
     * 
     * @param   sName       lower case of the algorithm name (e.g. "sha1", "md5", ...)
     * @return  an initialized checksum algorithm object
     * @throws  checksum_algorithm_unknown
     */
    static checksum create(std::string sName);


    /**
     * finalize the algorithm and get the checksum value
     * 
     * @return  the final authentication tag or cypher
     */
    virtual memory finalize() = 0;
    
    
    /**
     * name of the checksum
     * 
     * @return  the name of the checksum
     */
    virtual std::string name() = 0;


protected:


    /**
     * ctor
     */
    checksum_algorithm() {}

};


/**
 * stream into
 * 
 * Add some memory data to the checksum
 * 
 * @param   lhs     the checksum object
 * @param   rhs     the memory to add
 * @return  the checksum object
 */
inline checksum & operator<<(checksum & lhs, memory const & rhs) { 
    (*lhs) << rhs; 
    return lhs; 
}


/**
 * stream out
 * 
 * Calculate the checksum value (as memory block)
 * 
 * @param   lhs     the checksum object
 * @param   rhs     the memory holding the checksum value
 * @return  the checksum object
 */
inline checksum & operator>>(checksum & lhs, memory & rhs) { 
    (*lhs) >> rhs; 
    return lhs; 
}


}

}

#endif


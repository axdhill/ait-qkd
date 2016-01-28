/*
 * checksum_crc32.h
 * 
 * interface for the CRC32 checksum
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

 
#ifndef __QKD_CHECKSUM_CRC32_H_
#define __QKD_CHECKSUM_CRC32_H_


// ------------------------------------------------------------
// incs

// ait
#include <qkd/utility/checksum.h>


// ------------------------------------------------------------
// decls


namespace qkd {

namespace utility {


/**
 * this class creates CRC32 checksums
 */
class checksum_algorithm_crc32 : public checksum_algorithm {


public:


    /**
     * ctor
     */
    checksum_algorithm_crc32();


    /**
     * add a memory area to the calculation
     *
     * @param   cMemory         memory block to be added
     * @throws  checksum_algorithm_final, if the algorithm has finished and does not allow another addition
     */
    virtual void add(memory const & cMemory);


    /**
     * finalize the algorithm and get the checksum value
     */
    virtual memory finalize();


    /**
     * name of the checksum
     * 
     * @return  the name of the checksum
     */
    virtual std::string name() { return "crc32"; };
    

protected:


    /**
     * pimpl
     */
    class data;
    boost::shared_ptr<data> d;

};


}
    
    
}

#endif


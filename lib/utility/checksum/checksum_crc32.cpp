/*
 * checksum_crc32.cpp
 * 
 * implement the CRC32 checksum
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

 
// ------------------------------------------------------------
// incs

#include <boost/crc.hpp>

// ait
#include <qkd/utility/checksum.h>

#include "checksum_crc32.h"

using namespace qkd::utility;


// ------------------------------------------------------------
// decls

/**
 * pimpl
 */
class checksum_algorithm_crc32::data {


public:

    /**
     * ctor
     */
    data() : m_bFinalized(false) { m_cDigest = memory(4); };
    
    
    /**
     * get the algorithm implementation
     **/
    boost::crc_32_type & crc32() { return m_cCRC32; };


public:
    
    
    /**
     * the crc 32 algorithm
     */
    boost::crc_32_type m_cCRC32;


    /**
     * the digest
     */
    memory m_cDigest;


    /**
     * check if finalized
     */
    bool m_bFinalized;


};


// ------------------------------------------------------------
// code


/**
 * ctor
 */
checksum_algorithm_crc32::checksum_algorithm_crc32() : d(new checksum_algorithm_crc32::data()) {
}


/**
 * add a memory area to the calculation
 *
 * @param   cMemory         memory block to be added
 * @throws  checksum_algorithm_final, if the algorithm has finished and does not allow another addition
 */
void checksum_algorithm_crc32::add(memory const & cMemory) {
    if (d->m_bFinalized) throw checksum_algorithm_final();
    d->crc32().process_bytes(cMemory.get(), cMemory.size());
}


/**
 * finalize the algorithm and get the checksum value
 */
memory checksum_algorithm_crc32::finalize() {

    if (!d->m_bFinalized) {
        unsigned int nChecksum = d->crc32().checksum();
        memcpy(d->m_cDigest.get(), &nChecksum, 4);
        d->m_bFinalized = true;
    }

    return d->m_cDigest;
}


/*
 * checksum_md5.cpp
 * 
 * implement the MD5 checksum
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

#include <openssl/md5.h>

// ait
#include <qkd/utility/checksum.h>

#include "checksum_md5.h"


using namespace qkd::utility;


// ------------------------------------------------------------
// decls

/**
 * pimpl
 */
class checksum_algorithm_md5::data {


public:

    /**
     * ctor
     */
    data() : m_bFinalized(false) { MD5_Init(&m_cMD5Context); m_cDigest = memory(MD5_DIGEST_LENGTH); };


public:


    /**
     * the digest
     */
    memory m_cDigest;


    /**
     * check if finalized
     */
    bool m_bFinalized;


    /**
     * the SHA context
     */
    MD5_CTX m_cMD5Context;

};


// ------------------------------------------------------------
// code


/**
 * ctor
 */
checksum_algorithm_md5::checksum_algorithm_md5() : d(new checksum_algorithm_md5::data()) {
}


/**
 * add a memory area to the calculation
 *
 * @param   cMemory         memory block to be added
 * @throws  checksum_algorithm_final, if the algorithm has finished and does not allow another addition
 */
void checksum_algorithm_md5::add(memory const & cMemory) {
    if (d->m_bFinalized) throw checksum_algorithm_final();
    MD5_Update(&(d->m_cMD5Context), cMemory.get(), cMemory.size());
}


/**
 * finalize the algorithm and get the checksum value
 */
memory checksum_algorithm_md5::finalize() {

    if (!d->m_bFinalized) {
        MD5_Final((unsigned char *)d->m_cDigest.get(), &(d->m_cMD5Context));
        d->m_bFinalized = true;
    }

    return d->m_cDigest;
}


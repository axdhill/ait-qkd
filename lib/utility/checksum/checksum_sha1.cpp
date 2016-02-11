/*
 * checksum_sha1.cpp
 * 
 * implement the SHA1 checksum
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

#include <openssl/sha.h>

// ait
#include <qkd/utility/checksum.h>

#include "checksum_sha1.h"


using namespace qkd::utility;


// ------------------------------------------------------------
// decls

/**
 * pimpl
 */
class checksum_algorithm_sha1::data {


public:

    /**
     * ctor
     */
    data() : m_bFinalized(false) { SHA1_Init(&m_cSHAContext); m_cDigest = memory(SHA_DIGEST_LENGTH); };


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
    SHA_CTX m_cSHAContext;

};


// ------------------------------------------------------------
// code


/**
 * ctor
 */
checksum_algorithm_sha1::checksum_algorithm_sha1() : d(new checksum_algorithm_sha1::data()) {
}


/**
 * add a memory area to the calculation
 *
 * @param   cMemory         memory block to be added
 * @throws  checksum_algorithm_final, if the algorithm has finished and does not allow another addition
 */
void checksum_algorithm_sha1::add(memory const & cMemory) {
    if (d->m_bFinalized) {
        throw std::runtime_error("checksum algorithm instance already finalized");
    }
    SHA1_Update(&(d->m_cSHAContext), cMemory.get(), cMemory.size());
}


/**
 * finalize the algorithm and get the checksum value
 */
memory checksum_algorithm_sha1::finalize() {

    if (!d->m_bFinalized) {
        SHA1_Final((unsigned char *)d->m_cDigest.get(), &(d->m_cSHAContext));
        d->m_bFinalized = true;
    }

    return d->m_cDigest;
}


/*
 * checksum.cpp
 * 
 * implement the main checksum object
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

// ait
#include <qkd/utility/checksum.h>

#include "checksum_crc32.h"
#include "checksum_md5.h"
#include "checksum_sha1.h"


using namespace qkd::utility;


// ------------------------------------------------------------
// code


/**
 * factory method to create a known algorithm
 *
 * @param   sName       lower case of the algorithm name (e.g. "sha1", "md5", ...)
 * @return  an initialized checksum algorithm object
 * @throws  checksum_algorithm_unknown
 */
checksum checksum_algorithm::create(std::string sName) {

    // crc32
    if (sName == "crc32") return std::shared_ptr<checksum_algorithm>(new checksum_algorithm_crc32());

    // MD5
    if (sName == "md5") return std::shared_ptr<checksum_algorithm>(new checksum_algorithm_md5());

    // SHA1
    if (sName == "sha1") return std::shared_ptr<checksum_algorithm>(new checksum_algorithm_sha1());

    throw std::invalid_argument("checksum algorithm unknown");
}


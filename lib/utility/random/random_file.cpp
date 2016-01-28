/*
 * random_file.cpp
 * 
 * implement the random file source
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
#include <qkd/utility/random.h>

#include "random_file.h"

using namespace qkd::utility;


// ------------------------------------------------------------
// code


/**
 * get a block of random bytes
 * 
 * This function must be overwritten in derived classes
 * 
 * @param   cBuffer     buffer which will hold the bytes
 * @param   nSize       size of buffer in bytes
 */
void random_file::get(char * cBuffer, uint64_t nSize) {
    
    // do not proceed if nothing to do
    if (!cBuffer) return;
    if (nSize == 0) return;
    if (!m_cFileInStream.is_open()) return;
    
    uint64_t nRead = 0;
    while (nRead < nSize) {
        
        // reset if at end
        if (m_cFileInStream.eof()) {
            m_cFileInStream.clear();
            m_cFileInStream.seekg(0, std::ios::beg);
        }

        // read in some bytes
        m_cFileInStream.read(cBuffer + nRead, nSize - nRead);
        long nCurrentRead = m_cFileInStream.gcount();
        if (nCurrentRead == 0) {
            throw random_get_unknown();
        }
        
        nRead += nCurrentRead;
    }
}


/**
 * init the object
 */
void random_file::init() {
    
    m_cFileInStream.open(m_sFileName, std::ios::in | std::ios::binary);
    if (!m_cFileInStream.is_open()) {
        throw random_init_error();
    }
}

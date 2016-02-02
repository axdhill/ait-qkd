/*
 * bb84_base.cpp
 * 
 * BB84 Base values
 *
 * Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *
 * Copyright (C) 2016 AIT Austrian Institute of Technology
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
// inc

#include <sstream>

#include "bb84_base.h"


// ------------------------------------------------------------
// code

/**
 * dump a (sparse) base table
 * 
 * @param   cStream     the stream to dump to
 * @param   cBases      the base table to dump
 * @param   sIndent     the indent
 */
void dump_bb84(std::ostream & cStream, qkd::utility::memory const & cBase, std::string sIndent) {
    
    static char const sBB84[4] = { '?', 'x', '+', '?' };
    
    cStream << sIndent;
    unsigned char const * nEvent = cBase.get();    
    for (uint64_t i = 0; i < cBase.size(); ++i, ++nEvent) {
        cStream << sBB84[(*nEvent) & 0x03];
    }
}


/**
 * dump a (sparse) base table
 * 
 * @param   cBases      the base table to dump
 * @return  a string containing the bases as depicted in cBases
 */
std::string dump_bb84_str(qkd::utility::memory const & cBase) {
    std::stringstream ss;
    dump_bb84(ss, cBase);
    return ss.str();
}

/*
 * entangled.cpp
 * 
 * continue creation for entangled data
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
// incs

#include <sstream>

// ait
#include <qkd/common_macros.h>

#include "entangled.h"

using namespace qkd::cv;


// ------------------------------------------------------------
// code


/**
 * add program arguments to our mode settings
 * 
 * @param   cArguments      the arguments as passed from the command line
 * @return  true, if all arguments are ok
 */
bool entangled::consume_arguments(UNUSED boost::program_options::variables_map const & cArguments) {
    
    return true;
}


/**
 * report some help on this key generation mode
 * 
 * @return  help shown to the user on stdout about this mode
 */
std::string entangled::help() {
    
    std::stringstream ss;
    
    ss << "mode: 'entangled'\n";
    ss << "This mode creates a pair of entangled CV pseudo keys.\n";
    ss << "The resulting keys do have ENCODING_BASE_FLOAT encoding: the \n";
    ss << "first 32 bits hold either Q (== 0) or P (== 1) and the next \n";
    ss << "32 bits hold the measurment.\n\n";
    
    ss << "Parameters needed for mode 'entangled':\n";
    ss << "    --sigma-alice-q\n";
    ss << "    --sigma-alice-p\n";
    ss << "    --sigma-bob-q\n";
    ss << "    --sigma-bob-p\n";
    ss << "    --rho";
    
    return ss.str();
}


/**
 * produce a set of pseudo random cv-keys
 * 
 * @param   cKeyAlice       alice key
 * @param   cKeyBob         bob key
 */
void entangled::produce(UNUSED qkd::key::key & cKeyAlice, UNUSED qkd::key::key & cKeyBob) {
}

/*
 * netlink_message.cpp
 * 
 * a single netlink message to be sent or received
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


// this is only Linux code
#if defined(__linux__)
 

// ------------------------------------------------------------
// incs

#include <sstream>

#include "netlink_message.h"

using namespace qkd::q3p;


// ------------------------------------------------------------
// code


/**
 * turn the whole netlink message into a JSON string
 *
 * @return  the netlink message as JSON string
 */
std::string netlink_message::str() const {
    
    std::stringstream ss;
    
    bool bFirst = true;
    ss << "[ ";
    for (auto const & m: (*this)) {
        
        if (!bFirst) ss << ", ";
        else bFirst = false;
        
        ss << m->str();
    }
    ss << " ]";
    
    return ss.str();
}


#endif

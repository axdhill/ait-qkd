/*
 * netlink_nlmsgerr.cpp
 * 
 * nlmsgerr wrapper
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

#include <string.h>
#include <sstream>

#include <sys/socket.h>

#include "netlink_nlmsgerr.h"
#include "netlink_nlmsghdr.h"

using namespace qkd::q3p;


// ------------------------------------------------------------
// code


/**
 * ctor
 */
netlink_nlmsgerr::netlink_nlmsgerr() : netlink_base() {
    reset();
}
    
    
/**
 * ctor
 * 
 * @param   cNlMsgErr       the nlmsgerr kernel object to wrap
 */
netlink_nlmsgerr::netlink_nlmsgerr(nlmsgerr const & cNlMsgErr) : netlink_base() {
    memcpy(&m_cNlMsgErr, &cNlMsgErr, sizeof(m_cNlMsgErr));
}


/**
 * the name of the netlink kernel struct managed
 * 
 * @return  the name of the kernek struct managed
 */
std::string const & netlink_nlmsgerr::name() const { 
    static std::string const sName = "nlmsgerr"; 
    return sName;
}


/**
 * reset/init the netlink message to an empty (but valid) state
 */
void netlink_nlmsgerr::reset() {
    memset(&m_cNlMsgErr, 0, sizeof(m_cNlMsgErr));
}

    
/**
 * this method provides a JSON representation
 *
 * @return  a string holding a JSON of the current values
 */
std::string netlink_nlmsgerr::str() const {
    
    std::stringstream ss;
   
    ss << "{ \"" << name() << "\": { ";
    ss << "\"error\": " << m_cNlMsgErr.error << ", ";
    ss << "\"msg\": " << netlink_nlmsghdr(m_cNlMsgErr.msg).str();
    ss << " } }";
    
    return ss.str();
}


#endif

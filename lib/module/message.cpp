/* 
 * message.cpp
 * 
 * QKD module message implementation
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

#include <boost/format.hpp>

// ait
#include <qkd/module/message.h>


using namespace qkd::module;


// ------------------------------------------------------------
// incs


/**
 * message id counter
 */
uint32_t qkd::module::message::m_nLastId = 0;


// ------------------------------------------------------------
// code


/**
 * ctor
 */
message::message() {
    m_cHeader.nId = 0;
    m_cHeader.eType = qkd::module::message_type::MESSAGE_TYPE_DATA;
}


/**
 * give a debug string
 * 
 * @return  a debug string describing the message
 */
std::string message::string() const {
    
    boost::format cLineFormater = boost::format("<%10u><%-8s><%10u><%08x>\n%s");

    cLineFormater % id();
    
    switch (type()) {
        
    case qkd::module::message_type::MESSAGE_TYPE_DATA:
        cLineFormater % "DATA";
        break;
    
    case qkd::module::message_type::MESSAGE_TYPE_KEY_SYNC:
        cLineFormater % "KEY_SYNC";
        break;
        
    default:
        cLineFormater % "UNKNOWN";
        break;
    }
    
    cLineFormater % data().size();
    cLineFormater % data().crc32();
    cLineFormater % data().canonical("        ");
    
    return cLineFormater.str();
}




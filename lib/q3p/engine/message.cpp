/*
 * message.cpp
 * 
 * a Q3P message
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

#include <boost/format.hpp>

// ait
#include <qkd/q3p/message.h>
#include <qkd/utility/syslog.h>

#include "protocol/protocol.h"


using namespace qkd::q3p;


// ------------------------------------------------------------
// code


/**
 * ctor
 * 
 * @param   bAuthentic      authentic flag
 * @param   bEncrypted      encryption flag
 */
message::message(bool bAuthentic, bool bEncrypted) : qkd::utility::buffer() {
    
    // position already at the payload data
    resize(header_size());
    memset(get(), 0, header_size());
    
    set_authentic(bAuthentic);
    set_encrypted(bEncrypted);
    set_version();
    set_zipped(false);
    
    seek_payload();
}


/**
 * return the payload
 * 
 * @return  the payload
 */
qkd::utility::memory message::payload() {
    if (size() <= header_size()) return qkd::utility::memory(0);
    return qkd::utility::memory::wrap(get() + header_size(), size() - header_size());
}


/**
 * set the authentic flag
 * 
 * @param   bAuthentic      the new authentic flag
 */
void message::set_authentic(bool bAuthentic) {
    ensure_header(); 
    if (bAuthentic) header().nFlagsAndVersion |= 0x02;
    else header().nFlagsAndVersion &= ~0x02;
}


/**
 * set the encrypted flag
 * 
 * @param   bEncrypted      the new encrypted flag
 */
void message::set_encrypted(bool bEncrypted) {
    ensure_header(); 
    if (bEncrypted) header().nFlagsAndVersion |= 0x01;
    else header().nFlagsAndVersion &= ~0x01;
}


/**
 * set the Q3P version
 */
void message::set_version() {
    ensure_header(); 
    header().nFlagsAndVersion &= ~0xe0;
    header().nFlagsAndVersion |= (2 << 5);
}


/**
 * set the zipped flag
 * 
 * @param   bZipped         the new zipped flag
 */
void message::set_zipped(bool bZipped) {
    ensure_header(); 
    if (bZipped) header().nFlagsAndVersion |= 0x04;
    else header().nFlagsAndVersion &= ~0x04;
}


/**
 * return a small string describing the header, a bit payload and the tag
 * 
 * useful for debugging
 *
 * @return  a string describing the message
 */
std::string message::str() const {
    
    // generate a header signature
    boost::format cHeaderFormatter = boost::format("<%|10|><%|10|><%|1|%|1|%|1|><%|1|><%|-12|><%|5|><%|10|><%|10|>");
    cHeaderFormatter % length();
    cHeaderFormatter % id();
    cHeaderFormatter % (encrypted() ? 'E' : ' ');
    cHeaderFormatter % (authentic() ? 'A' : ' ');
    cHeaderFormatter % (zipped() ? 'Z' : ' ');
    cHeaderFormatter % 2;
    cHeaderFormatter % qkd::q3p::protocol::protocol::protocol_id_name(protocol_id());
    cHeaderFormatter % channel_id();
    cHeaderFormatter % encryption_key();
    cHeaderFormatter % authentication_key();

    // the header string
    std::string sHeaderSignature = cHeaderFormatter.str();
    
    // combine all elements
    boost::format cMessageFormatter = boost::format("%1%<---DATA---><%2%>") % sHeaderSignature % tag().as_hex();
    
    return cMessageFormatter.str();
}

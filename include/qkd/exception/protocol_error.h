/* 
 * protocol_error.h
 * 
 * Exception thrown on protocol errors between peers
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


#ifndef __QKD_EXCEPTION_PROTCOL_ERROR_H_
#define __QKD_EXCEPTION_PROTCOL_ERROR_H_


// ------------------------------------------------------------
// incs

#include <stdexcept>


// ------------------------------------------------------------
// decl

namespace qkd {

namespace exception {


/**
 * this exception is thrown on protocol errors with the peer.
 * 
 * This error indicates that the network itself is ok, i.e. sending and
 * receiving of raw messages works - we do have a living connection. This
 * implies that the configuration management also is set up accordingly.
 * However, the messages received are out of scope of the current protocol
 * used. That is: we received a message which is *NOT* within the scope
 * scope of the current protocol. E.g. we expected a ACK or NO-ACK 
 * message from the peer, but we received something else we can not
 * handle at the current stage of processing.
 */
class protocol_error : public std::runtime_error {
    
    
public:

    
    /**
     * ctor
     * 
     * @param   sError      text describing the error
     */       
    protocol_error(std::string const & sError) : std::runtime_error("[qkd::exception::protocol_error] " + sError) {}
};


}

}

#endif


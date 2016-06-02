/* 
 * network_error.h
 * 
 * Exception thrown when errors appear in the network communication.
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


#ifndef __QKD_EXCEPTION_NETWORK_ERROR_H_
#define __QKD_EXCEPTION_NETWORK_ERROR_H_


// ------------------------------------------------------------
// incs

#include <stdexcept>


// ------------------------------------------------------------
// decl

namespace qkd {

namespace exception {


/**
 * this exception is thrown on errors on the network connection.
 * 
 * This error indicates that a send/recv operation has failed due to network failures.
 */
class network_error : public std::runtime_error {
    
    
public:
    

    /**
     * ctor
     * 
     * @param   sError      text describing the error
     */       
    network_error(std::string const & sError) : std::runtime_error("[qkd::exception::network_error] " + sError) {}
};


}

}

#endif


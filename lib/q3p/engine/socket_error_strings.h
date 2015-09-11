/*
 * socket_error_strings.h
 * 
 * this resolves the string for a QAbstractSocket::SocketError enum
 *
 * Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
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

 
#ifndef __QKD_Q3P_SOCKET_ERROR_STRINGS_H_
#define __QKD_Q3P_SOCKET_ERROR_STRINGS_H_


// ------------------------------------------------------------
// incs

// Qt
#include <QtNetwork/QAbstractSocket>


// ------------------------------------------------------------
// decls


namespace qkd {
    
namespace q3p {    

   
/**
 * This class stores the QAbstractSocket::SocketError enum to string functionality
 */
class socket_error_strings {
    

public:


    /**
     * return a string for a QAbstractSocket::SocketError enum
     * 
     * @param   eError      the error
     * @return  string for the error
     */
    static std::string str(QAbstractSocket::SocketError eError) {
        switch (eError) {
        case QAbstractSocket::ConnectionRefusedError: return "ConnectionRefusedError";
        case QAbstractSocket::RemoteHostClosedError: return "RemoteHostClosedError";
        case QAbstractSocket::HostNotFoundError: return "HostNotFoundError";
        case QAbstractSocket::SocketAccessError: return "SocketAccessError";
        case QAbstractSocket::SocketResourceError: return "SocketResourceError";
        case QAbstractSocket::SocketTimeoutError: return "SocketTimeoutError";
        case QAbstractSocket::DatagramTooLargeError: return "DatagramTooLargeError";
        case QAbstractSocket::NetworkError: return "NetworkError";
        case QAbstractSocket::AddressInUseError: return "AddressInUseError";
        case QAbstractSocket::SocketAddressNotAvailableError: return "SocketAddressNotAvailableError";
        case QAbstractSocket::UnsupportedSocketOperationError: return "UnsupportedSocketOperationError";
        case QAbstractSocket::ProxyAuthenticationRequiredError: return "ProxyAuthenticationRequiredError";
        case QAbstractSocket::SslHandshakeFailedError: return "SslHandshakeFailedError";
        case QAbstractSocket::UnfinishedSocketOperationError: return "UnfinishedSocketOperationError";
        case QAbstractSocket::ProxyConnectionRefusedError: return "ProxyConnectionRefusedError";
        case QAbstractSocket::ProxyConnectionClosedError: return "ProxyConnectionClosedError";
        case QAbstractSocket::ProxyConnectionTimeoutError: return "ProxyConnectionTimeoutError";
        case QAbstractSocket::ProxyNotFoundError: return "ProxyNotFoundError";
        case QAbstractSocket::ProxyProtocolError: return "ProxyProtocolError";
        default: return "unknown error";
        }
    }


private:
    
    
    /**
     * ctor
     */
    socket_error_strings() {};
    
    
    /**
     * dtor
     */
    ~socket_error_strings() {};
    
};
  

}

}


#endif

/* 
 * connection.h
 * 
 * Module internal connection object
 *
 * Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *
 * Copyright (C) 2015 AIT Austrian Institute of Technology
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


#ifndef __QKD_MODULE_CONNECTION_H_
#define __QKD_MODULE_CONNECTION_H_


// ------------------------------------------------------------
// incs

#include "config.h"

#include <atomic>
#include <string>

// Qt
#include <QtCore/QUrl>

#include <qkd/key/key.h>
#include <qkd/module/message.h>


// ------------------------------------------------------------
// decl

namespace qkd {

namespace module {


/**
 * different connections we know
 */
enum class connection_type : uint8_t {
    
    PIPE_IN,            /**< pipe in, incoming keystream */
    PIPE_OUT,           /**< pipe out, outgoing keystream */
    LISTEN,             /**< bob's server socket */
    PEER                /**< alice's client connection */
};
    
    
/**
 * the internal used connection object
 */
class connection {
    
public:


    /**
     * ctor
     * 
     * @param   eType           type of the connection
     */
    explicit connection(connection_type eType);
    

    /**
     * copy ctor
     * 
     * @param   rhs     right hand side
     */
    connection(connection const & rhs) = delete;
    

    /**
     * dtor
     */
    ~connection();
    
    
    /**
     * deduce a correct, proper URL from a would-be URL
     * 
     * this returns "stdin://" and "stdout://" for these schemes.
     * 
     * on "ipc:// it checks for ambiguity and for the existance and 
     * access to the ipc socket file.
     * 
     * on "tcp://" it also checks for ambiguity and tries to deduce
     * the IP adress for a given hostname.
     * 
     * @param   sURL        an url
     * @return  a good, real, usable url (or empty() in case of failure)
     */
    static std::string fix_url(std::string const & sURL);
    
    
    /**
     * check if the given URL holds an ambiguous address
     *
     * This is only valid for tcp schemes. So this returns true
     * on 
     *          tcp:// *:PORT/...
     *
     * @param   cURL        an url to check
     * @return  true, if the URL contains an ambiguous host
     */
    static bool is_ambiguous(QUrl const & cURL);
    
    
    /**
     * checks if we can read from this connection
     * 
     * @return  true, if we can read or receive
     */
    bool is_incoming() const { return m_eType != connection_type::PIPE_OUT; }
    
    
    /**
     * checks if we can send to this connection
     * 
     * @return  true, if we can send of write
     */
    bool is_outgoing() const { return m_eType != connection_type::PIPE_IN; }
    
    
    /**
     * checks if this is our standard input
     * 
     * @return  true, if this connection can read from stdin
     */
    bool is_stdin() const { return m_bStdIn; }
    
    
    /**
     * checks if this is stdout
     * 
     * @return  true, if this connection pushes to stdout
     */
    bool is_stdout() const { return m_bStdOut; }
    
    
    /**
     * checks if this connection is void
     * 
     * @return  true, if there is nothing to send or receive from
     */
    bool is_void() const { return m_bVoid; }
    
    
    /**
     * checks if this connection needs to be setup (again)
     * 
     * @return  true, if we should run setup() again
     */
    bool needs_setup() const { return m_bSetup; }
    
    
    /**
     * get a next key from PIPE_IN
     * 
     * @param   cKey        this will receive the next key
     * @return  true, if reading was successful
     */
    bool read_key(qkd::key::key & cKey);
    
    
    /**
     * read a message
     *
     * this call is blocking (with respect to timeout)
     * 
     * The nTimeOut value is interpreted in these ways:
     * 
     *      n ...   wait n milliseconds for an reception of a message
     *      0 ...   do not wait: get the next message and return
     *     -1 ...   wait infinite (must be interrupted: see interrupt_worker())
     *     
     * The given message object will be deleted with delet before assigning new values.
     * Therefore if message receive has been successful the message is not NULL
     * 
     * This call waits explcitly for the next message been of type eType. If this
     * is NOT the case a exception is thrown.
     * 
     * @param   cMessage            this will receive the message
     * @param   nTimeOut            timeout in ms
     * @return  true, if we have received a message
     */
    bool recv_message(qkd::module::message & cMessage, int nTimeOut);
    
    
    /**
     * resets the connection to an empty void state
     * 
     * This closes all sockets.
     */
    void reset();
    
    
    /**
     * set the number of milliseconds for network send/recv timeout
     * 
     * @param   nTimeout        the new number of milliseconds for network send/recv timeout
     */
    void set_timeout(qlonglong nTimeout);
    
    
    /**
     * sets the stored url
     * 
     * @param   sURL        the new URL
     */
    void set_url(std::string sURL);
    

    /**
     * send a message to the peer
     * 
     * this call is blocking (with respect to timout)
     * 
     * The nTimeOut value is interpreted in these ways:
     * 
     *      n ...   wait n milliseconds
     *      0 ...   do not wait
     *     -1 ...   wait infinite (must be interrupted: see interrupt_worker())
     *     
     * Note: this function takes ownership of the message's data sent! 
     * Afterwards the message's data will be void
     * 
     * Sending might fail on interrupt.
     *
     * @param   cMessage            the message to send
     * @param   nTimeOut            timeout in ms
     * @returns true, if successfully sent
     */
    bool send_message(qkd::module::message & cMessage, int nTimeOut);
        
        
    /**
     * setup the connection with the given url
     * 
     * @param   sURL                a string holding the URL of the connection
     * @param   nHighWaterMark      high water mark value
     * @param   nTimeout            initial timeout in millisec on this connection
     * @param   sIPCPrefix          ipc socket file prefix (if ipc:// * is used)
     * @param   sIPCSuffix          ipc socket file suffix (if ipc:// * is used)
     * 
     * @return  true, for success
     */
    bool setup(std::string sURL, 
            int nHighWaterMark = 1000, 
            int nTimeout = 1000, 
            std::string sIPCPrefix = std::string(), 
            std::string sIPCSuffix = std::string());
    
    
    /**
     * setup the connection with the stored url
     * 
     * @param   nHighWaterMark      high water mark value
     * @param   nTimeout            initial timeout in millisec on this connection
     * @param   sIPCPrefix          ipc socket file prefix (if ipc:// * is used)
     * @param   sIPCSuffix          ipc socket file suffix (if ipc:// * is used)
     * 
     * @return  true, for success
     */
    bool setup(int nHighWaterMark, 
            int nTimeout, 
            std::string sIPCPrefix = std::string(), 
            std::string sIPCSuffix = std::string()) { 
        return setup(url(), nHighWaterMark, nTimeout, sIPCPrefix, sIPCSuffix); 
    }
    
    
    /**
     * setup the connection with the stored url
     * 
     * @return  true, for success
     */
    bool setup() { 
        return setup(url(), m_nHighWaterMark, m_nTimeout); 
    }
    
    
    /**
     * returns the 0MQ socket
     * 
     * @return  the 0MQ socket
     */
    void * socket() { return m_cSocket; }
    
    
    /**
     * get the connection type
     * 
     * @return  the connection type
     */
    connection_type type() const { return m_eType; }
    
    
    /**
     * return the url
     * 
     * @return  the URL used for this connection
     */
    std::string url() const { return m_sURL; }
    
    
    /**
     * write a key to the next module
     * 
     * @param   cKey        key to pass to the next module
     * @return  true, if writing was successful
     */
    bool write_key(qkd::key::key const & cKey);
    

private:
    
    
    /**
     * create an IPC socket file
     * 
     * The file will be created within a fixed folder (/tmp/qkd or /run/qkd).
     * The filename will have [prefix.]PID[.suffix]
     * 
     * @param   sPrefix         prefix name of socket
     * @param   sSuffix         suffix name of socket
     */
    static boost::filesystem::path create_ipc_socket(std::string sPrefix, std::string sSuffix);
    
    
    /**
     * create an IPC outgoing path
     */
    boost::filesystem::path create_ipc_out() const;
    
    
    /**
     * deduce a correct, proper IPC-URL from a would-be IPC-URL
     * 
     * @param   sURL        an url
     * @return  a good, real, usable url (or empty() in case of failure)
     */
    static std::string fix_url_ipc(std::string const & sURL);
    
    
    /**
     * deduce a correct, proper TCP-URL from a would-be TCP-URL
     * 
     * @param   sURL        an url
     * @return  a good, real, usable url (or empty() in case of failure)
     */
    static std::string fix_url_tcp(std::string const & sURL);
    
    
    /**
     * setup socket with high water mark and timeout
     *
     * also linger will be set to 0
     *
     * @param   cSocket             socket to modify
     * @param   nHighWaterMark      high water mark
     * @param   nTimeout            timeout on socket
     */
    void prepare_socket(void * & cSocket, int nHighWaterMark, int nTimeout);
        
    
    /**
     * return the proper 0MQ socket type for this connection
     * 
     * @return  0MQ socket type value
     */
    int zmq_socket_type() const;
    
    
    std::string m_sURL;                 /**< the URL to connect */
    std::atomic<bool> m_bSetup;         /**< if the connection needs to be setup */
    
    connection_type m_eType;            /**< connection type */
    
    bool m_bStdIn;                      /**< true, if this connection is stdin */
    bool m_bStdOut;                     /**< true, if this connection is stdout */
    bool m_bVoid;                       /**< true, if this connection is void */
    
    int m_nHighWaterMark;               /**< last HighWaterMark stored for re-setup */
    int m_nTimeout;                     /**< last Timeout stored for re-setup */
    
    void * m_cSocket;                   /**< 0MQ socket */
    
    
};


}

}

#endif


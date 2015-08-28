/* 
 * path.h
 * 
 * a single path from this module to a remote point
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


#ifndef __QKD_MODULE_PATH_H_
#define __QKD_MODULE_PATH_H_


// ------------------------------------------------------------
// incs

#include <string>

// boost
#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>

// 0MQ
#include <zmq.h>



// ------------------------------------------------------------
// decl

namespace qkd {

namespace module {
    
    
/**
 * a single ZMQ message neatly encapsulated
 */
class zmq_msg {
    
    
public:
    
    
    zmq_msg_t m_cZMQMessage;
    
    
    /**
     * ctor
     */
    zmq_msg() { if (zmq_msg_init(&m_cZMQMessage) == -1) throw std::runtime_error("unable to init 0MQ message"); }
    
    
    /**
     * copy ctor
     * 
     * @param   rhs     right hand side
     */
    zmq_msg(zmq_msg const & rhs) = delete;
    
    
    /**
     * dtor
     */
    ~zmq_msg() { zmq_msg_close(&m_cZMQMessage); }
    
    
    /**
     * return the data of the message
     * 
     * @return  data of the message
     */
    void * data() { return zmq_msg_data(&m_cZMQMessage); }
    
    
    /**
     * check if there is more to fetch
     * 
     * @return  true, if there is more on the path to get
     */
    bool more() { return zmq_msg_more(&m_cZMQMessage) == 1; }
    
    
    /**
     * get the message
     * 
     * @return  the ZMQ Message
     */
    zmq_msg_t & msg() { return m_cZMQMessage; }
    
    
    /**
     * get the message
     * 
     * @return  the ZMQ Message
     */
    zmq_msg_t const & msg() const { return m_cZMQMessage; }
    
    
    /**
     * return the size of the message
     * 
     * @return  size of the message
     */
    size_t size() { return zmq_msg_size(&m_cZMQMessage); }
};


/**
 * a path is single connection to a remote point
 */
class path {
    
public:
    
    
    /**
     * ctor
     */
    path();
    
    
    /**
     * copy ctor
     */
    path(path const & rhs) = delete;
    
    
    /**
     * dtor
     */
    ~path();
    
    
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
     * return the high water mark for this path
     * 
     * @return  the amount of messages in transit
     */
    int highwatermark() const { return m_nHighWaterMark; }
    
    
    /**
     * check if the given URL holds an ambiguous address
     *
     * This is only valid for tcp schemes. So this returns true
     * on 
     *          tcp:// *:PORT/...
     *
     * @param   sURL        an url to check
     * @return  true, if the URL contains an ambiguous host
     */
    static bool is_ambiguous(std::string sURL);
    
    
    /**
     * is this ipc?
     * 
     * @return  true, if URL is "ipc://"
     */
    bool is_ipc() const { return m_bIPC; }
    
    
    /**
     * is this stdin?
     * 
     * @return  true, if URL is "stdin://"
     */
    bool is_stdin() const { return m_bStdIn; }
    
    
    /**
     * is this stdout?
     * 
     * @return  true, if URL is "stdout://"
     */
    bool is_stdout() const { return m_bStdOut; }
    
    
    /**
     * is this tcp?
     * 
     * @return  true, if URL is "tcp://"
     */
    bool is_tcp() const { return m_bTCP; }
    
    
    /**
     * is this a void path
     * 
     * @return  true, if this path is void
     */
    bool is_void() const { return m_bVoid; }
    
    
    /**
     * receive data on the path
     * 
     * @param   cMessage        message to be received
     * @param   nZMQFlags       flags to use on receiving
     * @return  number of bytes read (or -1 in case or error)
     */
    int recv(zmq_msg & cMessage, int nZMQFlags = 0) { return zmq_msg_recv(&cMessage.msg(), socket(), nZMQFlags); }
    
    
    /**
     * reset the path to void
     */
    void reset();
    
    
    /**
     * sets the high water mark for this path
     * 
     * @param   nHighWaterMark      the new amount of messages in transit
     */
    void set_highwatermark(int nHighWaterMark) { m_nHighWaterMark = nHighWaterMark; }
    
    
    /**
     * set incoming (recv) timeout
     * 
     * @param   nTimeout    in millisec, -1 for infinite
     * @return  true, if set, false for error
     */
    bool set_timeout_incoming(int nTimeout);
    
    
    /**
     * set incoming (recv) timeout
     * 
     * @param   nTimeout    in millisec, -1 for infinite
     * @return  true, if set, false for error
     */
    bool set_timeout_outgoing(int nTimeout);
    

    /**
     * sets the path's URL
     * 
     * Valid URLs are:
     * 
     *  ""              -   the 'void', NULL URL: reading and writing does always succeed!
     *  "stdin://"      -   standard input
     *  "stdout://"     -   standout output
     *  "ipc://"        -   Interprocess Communication (via UNIX Domain Sockets)
     *  "tcp://"        -   TCP/IP socket Communication
     * 
     * Hence the char ';' serves as a delimiter of concatenated URLs.
     * ... and we serve only one single URL here. So something like
     * "ipc:///this;main;file" will yield an exception.
     * 
     * The server flag, socket type, high water mark and the timeout are only relevant
     * for ipc:// and tcp:// paths.
     * 
     * @param   sURL                the new URL of the path
     * @param   bServer             do a "bind()" (instead of a "connect()")
     * @param   nSocketType         the ZMQ socket type
     * @param   nHighWaterMark      the number of messages in transit for this path
     * @param   nTimeout            the timeout in milliseconds for actions on this path
     * @param   sIPCHint            proper file name to use for IPC paths (if ipc:// is ambiguous)
     */
    void set_url(std::string sURL, bool bServer, int nSocketType, int nHighWaterMark = 1000, int nTimeout = 1000,  std::string sIPCHint = "");
    
    
    /**
     * send data over the path
     * 
     * @param   cBuffer     the data
     * @param   nLength     size of data
     * @param   nZMQFlags   0MQ flags to use while sending
     * @return  number of bytes sent or -1 for error
     */
    int send(void * cBuffer, size_t nLength, int nZMQFlags = 0) { return zmq_send(m_cSocket, cBuffer, nLength, nZMQFlags); }
    
    
    /**
     * get the ZMQ socket
     * 
     * @return  the ZMQ socket
     */
    void * socket() { return m_cSocket; }
    
    
    /**
     * get the url
     * 
     * @return  the url stored
     */
    std::string const & url() const { return m_sURL; }
    
    
private:    
    

    /**
     * create an IPC socket file
     * 
     * @param   sIPCSocketFileName      the intended socket file name
     * @return  a full path to the socket file
     */
    static boost::filesystem::path create_ipc_socket(std::string sIPCSocketFileName);
    
    
    /**
     * create an IPC outgoing path
     */
    boost::filesystem::path create_ipc_out() const;
    
    
    /**
     * setup path with high water mark and timeout
     *
     * also linger will be set to 0
     *
     * @param   nHighWaterMark      high water mark
     * @param   nTimeout            timeout on socket
     */
    void prepare(int nHighWaterMark, int nTimeout);
    
    
    /**
     * setup the path
     * 
     * @param   bServer         rather use "bind()" for socket and not "connect()"
     * @param   nSocketType     the ZMQ socket type
     * @param   nHighWaterMark      the number of messages in transit for this path
     * @param   nTimeout            the timeout in milliseconds for actions on this path
     */
    void setup(bool bServer, int nSocketType, int nHighWaterMark, int nTimeout);
    
    
    std::string m_sURL;                 /**< the URL to connect */
    bool m_bSetup;                      /**< if the path needs to be setup */
    
    bool m_bTCP;                        /**< true, if this path is tcp:// */
    bool m_bIPC;                        /**< true, if this path is ipc:// */
    bool m_bStdIn;                      /**< true, if this path is stdin:// */
    bool m_bStdOut;                     /**< true, if this path is stdout:// */
    bool m_bVoid;                       /**< true, if this path is void */
    
    int m_nHighWaterMark;               /**< last HighWaterMark stored */
    
    void * m_cSocket;                   /**< 0MQ sockets */
};


/**
 * smart pointer to a path instance
 */
typedef boost::shared_ptr<path> path_ptr;


}

}

#endif

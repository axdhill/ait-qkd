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

#include <algorithm>
#include <deque>
#include <string>

#include <qkd/key/key.h>
#include <qkd/module/message.h>

#include "path.h"


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
 * different ways to touch the sockets we manage
 */
enum class socket_access : uint8_t {
    
    // TODO: BEST_EFFORT,        /**< pick the next send/recv which is available */ does not work currently (zmq_poll broken?)
    ROUND_ROBIN,        /**< send/recv messages on a single socket, but use round robin */
    ALL                 /**< use all available sockets to send/recv the same message */
};
    
    
/**
 * the internal used connection object
 * 
 * a connection maintains at least one single path. 
 * a path is used for sending and/or receiving.
 * if there are more than one path available, a ROUND_ROBIN scheduling is used for send/recv.
 * 
 * a connection is typed as being one of
 * - pipe input
 * - pipe output
 * - bob listener
 * - alice peer
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
     * add a path for the connection with the given url
     * 
     * @param   sURL                the URLs of the connection
     * @param   nHighWaterMark      high water mark value
     * @param   sIPCPrefix          ipc socket file prefix (if ipc:// * is used)
     * @param   sIPCSuffix          ipc socket file suffix (if ipc:// * is used)
     * 
     * @return  true, for success
     */
    bool add(std::string sURL, int nHighWaterMark = 1000, std::string sIPCPrefix = std::string(), std::string sIPCSuffix = std::string());
    
    
    /**
     * clears all paths
     * 
     * This closes all sockets and drops them.
     */
    void clear();
    
    
    /**
     * check if this connection is void (for all paths)
     * 
     * @return  true, if we ain't got a single valid path not void
     */
    bool is_void() const { return std::all_of(m_cPaths.cbegin(), m_cPaths.cend(), [](path_ptr const & p) { return p->is_void(); }); }
    

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
     * this call is blocking
     * 
     * The given message object will be deleted with delet before assigning new values.
     * Therefore if message receive has been successful the message is not NULL
     * 
     * This call waits explcitly for the next message been of type eType. If this
     * is NOT the case a exception is thrown.
     * 
     * @param   cMessage            this will receive the message
     * @return  true, if we have received a message
     */
    bool recv_message(qkd::module::message & cMessage);
    
    
    /**
     * resets the connection to an empty void state
     * 
     * This closes all sockets.
     */
    void reset();
    
    
    /**
     * send a message
     * 
     * this call is blocking (with respect to timout)
     * 
     * Note: this function takes ownership of the message's data sent! 
     * Afterwards the message's data will be void
     * 
     * Sending might fail on interrupt.
     *
     * @param   cMessage            the message to send
     * @returns true, if successfully sent
     */
    bool send_message(qkd::module::message & cMessage);
        
        
    /**
     * splt a list of urls sperarated by semicolon into a list
     * of url string
     * 
     * @param   sURLs       a string holding many URLs, spearated by ';'
     * @return  the URLs as given in the sURLs
     */
    static std::list<std::string> split_urls(std::string sURLs);

    
    /**
     * return the urls inside this connection as a string
     * 
     * @return  the URLs used for this connection as a string
     */
    std::string urls_string() const;
    
    
    /**
     * return the urls inside this connection
     * 
     * @return  the URLs used for this connection
     */
    std::list<std::string> urls() const;
    
    
    /**
     * write a key
     * 
     * @param   cKey        key to pass
     * @return  true, if writing was successful
     */
    bool write_key(qkd::key::key const & cKey);
    

private:
    
    
    /**
     * get the next paths to work on
     * 
     * @return  a list of current paths to work on
     */
    std::list<path_ptr> get_next_paths();
    
    
    /**
     * get a next key from a path
     * 
     * @param   cPath       the path to read
     * @param   cKey        the key to read
     * @return  true, if we read a key
     */
    bool read_key(qkd::module::path & cPath, qkd::key::key & cKey);
    
    
    /**
     * read a message from a path
     *
     * @param   cPath       the path to read
     * @param   cMessage    the message to be received
     * @return  true, if cMessage is received
     */
    bool recv_message(qkd::module::path & cPath, qkd::module::message & cMessage);
    
        
    /**
     * send a message on a path
     * 
     * @param   cPath               the path to send the message on
     * @param   cMessage            the message to send
     * @returns true, if successfully sent
     */
    bool send_message(qkd::module::path & cPath, qkd::module::message & cMessage);
    
        
    /**
     * write a key
     * 
     * @param   cKey        key to pass
     * @param   cPath       the path to write key on
     * @return  true, if writing was successful
     */
    bool write_key(qkd::module::path & cPath, qkd::key::key const & cKey);

    
    /**
     * return true if we should act as server
     * 
     * @return  true, if we should bind as listener
     */
    bool zmq_socket_server() const;
    
    
    /**
     * return the proper 0MQ socket type for this connection
     * 
     * @return  0MQ socket type value
     */
    int zmq_socket_type() const;
    
    
    connection_type m_eType;                                    /**< connection type */
    enum socket_access m_eSocketAccess;                         /**< outgoing mode */
    
    std::vector<path_ptr> m_cPaths;                             /**< the paths we use */
    unsigned int m_nCurrentPathIndex;                           /**< current path index */
    
    std::deque<qkd::key::key> m_cKeysInStock;                   /**< read keys not yet delivered */
    std::deque<qkd::module::message> m_cMessagesInStock;        /**< read messages not yet delivered */
        
};


}

}

#endif


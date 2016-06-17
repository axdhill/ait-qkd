/* 
 * connection.cpp
 * 
 * Module internal connection object
 *
 * Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *
 * Copyright (C) 2015-2016 AIT Austrian Institute of Technology
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

#include <algorithm>
#include <iostream>

// boost
#include <boost/tokenizer.hpp>

// Qt
#include <QtCore/QString>
#include <QtCore/QUrl>

#include <qkd/exception/connection_error.h>
#include <qkd/exception/network_error.h>
#include <qkd/utility/buffer.h>
#include <qkd/utility/debug.h>
#include <qkd/utility/environment.h>
#include <qkd/utility/syslog.h>

#include <qkd/module/connection.h>


using namespace qkd::module;


// ------------------------------------------------------------
// decl


/**
 * internal used connection object details
 */
class qkd::module::connection::connection_internal {
    
    
public:
    
    
    /**
     * ctor
     * 
     * @param   cConnection         the parent connection
     * @param   eType               connection type
     */
    connection_internal(qkd::module::connection * cConnection, connection_type eType) : 
    
            m_eType(eType), 
            m_eSocketSendMode(socket_send_mode::ROUND_ROBIN), 
            m_nCurrentPathIndex(0),
            m_cConnection(cConnection) {
                
        if (!m_cConnection) {
            throw std::invalid_argument("Invalid parent connection object for internal connection object.");
        }
    }


    
    connection_type m_eType;                                    /**< connection type */
    enum socket_send_mode m_eSocketSendMode;                    /**< outgoing mode */
    
    std::vector<path_ptr> m_cPaths;                             /**< the paths we use */
    unsigned int m_nCurrentPathIndex;                           /**< current path index */
    
    std::deque<qkd::key::key> m_cKeysInStock;                   /**< read keys not yet delivered */
    std::deque<qkd::module::message> m_cMessagesInStock;        /**< read messages not yet delivered */

        
private:
    
    
    /**
     * the parent connection
     */
    qkd::module::connection * m_cConnection;
    
};


// ------------------------------------------------------------
// code


/**
 * ctor
 * 
 * @param   eType           type of the connection
 */
connection::connection(connection_type eType)  {

    d = std::shared_ptr<qkd::module::connection::connection_internal>(
        new qkd::module::connection::connection_internal(this, eType)
    );
}


/**
 * dtor
 */
connection::~connection() {
    reset();
}


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
bool connection::add(std::string sURL, int nHighWaterMark, std::string sIPCPrefix, std::string sIPCSuffix) {
    
    // try to work on an already added instance
    path_ptr cPath;
    auto cFound = std::find_if(d->m_cPaths.begin(), d->m_cPaths.end(), [&](path_ptr & p) { return (p->url() == sURL); });
    if (cFound != d->m_cPaths.end()) {
        cPath = *cFound;
    }
    else {
        cPath = path_ptr(new path());
    }

    // new URL somehow valid?    
    cPath->reset();
    try {
        
        std::stringstream ss;
        if (!sIPCPrefix.empty()) ss << sIPCPrefix << ".";
        ss << qkd::utility::environment::process_id();
        if (!sIPCSuffix.empty()) ss << "." << sIPCSuffix;
        
        int nTimeout = -1;
        if ((d->m_eType == connection_type::PIPE_IN) || (d->m_eType == connection_type::PIPE_OUT)) nTimeout = 1000;
        
        // this creates the real socket/path/connection underneath
        cPath->set_url(sURL, zmq_socket_server(), zmq_socket_type(), nTimeout, nHighWaterMark, ss.str());
    }
    catch (std::exception & e) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ 
                << ": unable to set url - " << e.what();
        return false;
    }

    // stdin:// on pipe out is not allowed
    if (cPath->is_stdin()) {    
        if (d->m_eType == connection_type::PIPE_OUT) {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ 
                    << ": url can't be 'stdin' for this connection";
            return false;
        }
    }
    
    // stdout:// on pipe in is not allowed
    if (cPath->is_stdout()) {
        if (d->m_eType == connection_type::PIPE_IN) {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ 
                    << ": url can't be 'stdout' for this connection";
            return false;
        }
    }
    
    if (cFound == d->m_cPaths.end()) d->m_cPaths.push_back(cPath);
    
    return true;
}


/**
 * clears all paths
 * 
 * This closes all sockets and drops them.
 */
void connection::clear() {
    reset();
    d->m_cPaths.clear();
    d->m_nCurrentPathIndex = 0;
}


/**
 * get the next paths to send on
 * 
 * @return  a list of current paths to send next message to
 */
std::list<path_ptr> connection::get_next_paths() {
    
    std::list<path_ptr> res;
    if (d->m_cPaths.empty()) return res;
        
    switch (d->m_eSocketSendMode) {
        
    case socket_send_mode::ROUND_ROBIN:
        res.push_back(d->m_cPaths[d->m_nCurrentPathIndex % d->m_cPaths.size()]);
        d->m_nCurrentPathIndex++;
        break;
        
    // TODO: case socket_access::BEST_EFFORT:
    case socket_send_mode::ALL:
        for (auto p : d->m_cPaths) res.push_back(p);
        break;
    
    default:
        throw qkd::exception::connection_error("cannot deduce paths for next action on current connection");
    }
    
    return res;
}


/**
 * check if this connection is void (for all paths)
 * 
 * @return  true, if we ain't got a single valid path not void
 */
bool connection::is_void() const { 
    return std::all_of(d->m_cPaths.cbegin(), d->m_cPaths.cend(), [](path_ptr const & p) { return p->is_void(); }); 
}


/**
 * return the paths
 * 
 * @return  the paths of this connection
 */
std::vector<path_ptr> const & connection::paths() const { 
    return d->m_cPaths; 
}


/**
 * get a next key from PIPE_IN
 * 
 * @param   cKey        this will receive the next key
 * @return  true, if reading was successful
 */
bool connection::read_key(qkd::key::key & cKey) {

    if (d->m_eType != connection_type::PIPE_IN) {
        throw qkd::exception::connection_error("tried to read key on a non-pipe-in connection");
    }
    
    // empty in/out key
    cKey = qkd::key::key();
    
    // hand out keys we already have read
    if (d->m_cKeysInStock.size() > 0) {
        cKey = d->m_cKeysInStock.front();
        d->m_cKeysInStock.pop_front();
        return true;
    }
    
    std::list<path_ptr> cPaths = get_next_paths();
    if (cPaths.empty()) return true;
    
    // all void paths ---> no key (but it's okay...)
    if (std::all_of(cPaths.begin(), cPaths.end(), [](path_ptr & p) { return p->is_void(); })) return true;
    
    // iterate over all sockets
    // NOTE: this should be zmq_poll, however this does not properly work
    //       if we have several different polls and send/recv sockets 
    //       in the process
    for (auto & p : cPaths) {
        
        qkd::key::key cReadKey;
        if (read_key(*p.get(), cReadKey)) {
            if (cKey.is_null()) cKey = cReadKey;
            else d->m_cKeysInStock.push_back(cReadKey);
        }
    }
    
    return (!cKey.is_null());
}


/**
 * get a next key from a path
 * 
 * @param   cPath       the path to read
 * @param   cKey        the key to read
 * @return  true, if we read a key
 */
bool connection::read_key(qkd::module::path & cPath, qkd::key::key & cKey) {
    
    if (cPath.is_void()) return false;
    if (cPath.is_stdin()) {
        std::cin >> cKey;
        return true;
    }
    
    zmq_msg m;
    int nRead = cPath.recv(m);
    if (nRead == -1) {

        // EAGAIN and EINTR are not critical
        if ((zmq_errno() == EAGAIN) || (zmq_errno() == EINTR)) {
            return false;
        }

        std::stringstream ss;
        ss << "failed reading key: " << strerror(zmq_errno());
        throw qkd::exception::network_error(ss.str());
    }

    qkd::utility::buffer cData = qkd::utility::buffer(qkd::utility::memory::wrap((unsigned char *)m.data(), m.size()));
    cData >> cKey;

    return true;
}


/**
 * read a message
 *
 * this call is blocking
 * 
 * The given message object will be deleted with delete before assigning new values.
 * Therefore if message receive has been successful the message is not NULL
 * 
 * This call waits explicitly for the next message been of type eType. If this
 * is NOT the case an exception is thrown.
 * 
 * @param   cMessage            this will receive the message
 * @return  true, if we have received a message
 */
bool connection::recv_message(qkd::module::message & cMessage) {

    // empty in/out message
    cMessage = qkd::module::message();
    
    // hand out message we already have read
    if (d->m_cMessagesInStock.size() > 0) {
        cMessage = d->m_cMessagesInStock.front();
        d->m_cMessagesInStock.pop_front();
        return true;
    }
    
    std::list<path_ptr> cPaths = get_next_paths();
    if (cPaths.empty()) return false;
    if (std::all_of(cPaths.begin(), cPaths.end(), [](path_ptr & p) { return p->is_void(); })) return false;
    
    // iterate over all sockets
    // NOTE: this should be zmq_poll, however this does not properly work
    //       if we have several different polls and send/recv sockets 
    //       in the process
    bool bMessageSet = false;
    for (auto & p : cPaths) {
        
        qkd::module::message cReadMessage;
        if (recv_message(*p.get(), cReadMessage)) {
            if (!bMessageSet) {
                cMessage = cReadMessage;
                bMessageSet = true;
            }
            else d->m_cMessagesInStock.push_back(cReadMessage);
        }
    }
    
    return bMessageSet;
}    
    

/**
 * read a message from a path
 *
 * @param   cPath       the path to read
 * @param   cMessage    the message to be received
 * @return  true, if cMessage is received
 */
bool connection::recv_message(qkd::module::path & cPath, qkd::module::message & cMessage) {
    
    if (cPath.is_void()) return false;
    if (cPath.is_stdin()) {
        throw qkd::exception::connection_error("don't know how to read a message from stdin");
    }

    // --> get the message header    
    zmq_msg cMsgHeader;
    int nReadHeader = 0;
    do {
        nReadHeader = cPath.recv(cMsgHeader, ZMQ_RCVMORE);
        if (nReadHeader == -1) {

            // EAGAIN and EINTR are not critical
            if (zmq_errno() == EAGAIN) continue;
            if (zmq_errno() == EINTR) return false;

            std::stringstream ss;
            ss << "Failed reading message header from peer: " << strerror(zmq_errno());
            throw qkd::exception::network_error(ss.str());
        }
        
    } while (nReadHeader <= 0);
    if (!cMsgHeader.more() || (cMsgHeader.size() != sizeof(cMessage.m_cHeader))) {
        throw qkd::exception::network_error("Received invalid message header");
    }
    memcpy(&(cMessage.m_cHeader), (unsigned char *)cMsgHeader.data(), sizeof(cMessage.m_cHeader));

    // --> get the message data
    zmq_msg cMsgData;
    int nReadData = 0;
    do {
        nReadData = cPath.recv(cMsgData);
        if (nReadData == -1) {

            // EAGAIN and EINTR are not critical
            if (zmq_errno() == EAGAIN) continue;
            if (zmq_errno() == EINTR) return false;

            std::stringstream ss;
            ss << "Failed reading message data from peer: " << strerror(zmq_errno());
            throw qkd::exception::network_error(ss.str());
        }
        
    } while (nReadData <= 0);
    cMessage.data().resize(cMsgData.size());
    memcpy(cMessage.data().get(), cMsgData.data(), cMsgData.size());
    cMessage.data().set_position(0);
    
    return true;
}
    
        
/**
 * resets the connection to an empty void state
 */
void connection::reset() {
    for (auto & cPath: d->m_cPaths) cPath->reset();
}


/**
 * send a message
 * 
 * this call is blocking
 * 
 * Note: this function takes ownership of the message's data sent! 
 * Afterwards the message's data will be void
 * 
 * Sending might fail on interrupt.
 *
 * The path index holds the number of the path to choose. 
 * On -1 the next suitable path(s) are taken.
 * 
 * @param   cMessage            the message to send
 * @param   nPath               the path number to send
 * @returns true, if successfully sent
 */
bool connection::send_message(qkd::module::message & cMessage, int nPath) {
    
    std::list<path_ptr> cPaths;
    if (nPath == -1) {
        cPaths = get_next_paths();
    }
    else {
        if (static_cast<size_t>(nPath) >= d->m_cPaths.size()) {
            throw qkd::exception::connection_error("path index out of range");
        }
        cPaths.push_back(d->m_cPaths[nPath]);
    }
    
    if (cPaths.empty()) return false;
    if (std::all_of(cPaths.begin(), cPaths.end(), [](path_ptr & p) { return p->is_void(); })) return false;
    
    // iterate over all sockets
    // NOTE: this should be zmq_poll, however this does not properly work
    //       if we have several different polls and send/recv sockets 
    //       in the process
    bool bMessageSent = false;
    for (auto & p : cPaths) {
        if (send_message(*p.get(), cMessage)) {
            bMessageSent |= true;
        }
    }
    
    return bMessageSent;
}
    
        
/**
 * send a message on a path
 * 
 * @param   cPath               the path to send the message on
 * @param   cMessage            the message to send
 * @returns true, if successfully sent
 */
bool connection::send_message(qkd::module::path & cPath, qkd::module::message & cMessage) {
    
    if (cPath.is_void()) return false;
    if (cPath.is_stdout()) {
        throw qkd::exception::connection_error("don't know how to send a qkd peer module message on stdout");
    }
    
    cMessage.m_cHeader.nId = htobe32(++qkd::module::message::m_nLastId);
    cMessage.m_cTimeStamp = std::chrono::high_resolution_clock::now();

    int nSentHeader = cPath.send(&(cMessage.m_cHeader), sizeof(cMessage.m_cHeader), ZMQ_SNDMORE);
    if (nSentHeader == -1) {

        // EINTR is not critical
        if (zmq_errno() == EINTR) {
            return false;
        }

        std::stringstream ss;
        ss << "failed sending message header to peer: " << strerror(zmq_errno());
        throw qkd::exception::network_error(ss.str());
    }

    int nSentData = cPath.send(cMessage.data().get(), cMessage.data().size());
    if (nSentData == -1) {

        // EINTR is not critical
        if (zmq_errno() == EINTR) {
            return false;
        }
        std::stringstream ss;
        ss << "failed sending message data to peer: " << strerror(zmq_errno());
        throw qkd::exception::network_error(ss.str());
    }

    return true;
}


/**
 * split a list of urls separated by semicolon into a list
 * of url string
 * 
 * @param   sURLs       a string holding many URLs, separated by ';'
 * @return  the URLs as given in the sURLs
 */
std::list<std::string> connection::split_urls(std::string sURLs) {
    std::list<std::string> res;
    boost::char_separator<char> cSeparator(";");
    boost::tokenizer<boost::char_separator<char>> cTokens(sURLs, cSeparator);
    for (auto & u : cTokens) res.push_back(u);
    return res;
}


/**
 * return the urls inside this connection
 * 
 * @return  the URLs used for this connection
 */
std::list<std::string> connection::urls() const {
    std::list<std::string> res;
    for (auto & cPath : d->m_cPaths) { res.push_back(cPath->url()); };
    return res;
}


/**
 * return the urls inside this connection as a string
 * 
 * @return  the URLs used for this connection as a string
 */
std::string connection::urls_string() const {
    
    std::stringstream ss;
    bool bURLWritten = false;
    
    for (auto sURL : urls()) {
        if (bURLWritten) ss << ";";
        ss << sURL;
    }

    return ss.str();    
}


/**
 * write a key
 * 
 * @param   cKey        key to pass
 * @param   nPath       path index of connection
 * @return  true, if writing was successful
 */
bool connection::write_key(qkd::key::key const & cKey, int nPath) {
    
    if (d->m_eType != connection_type::PIPE_OUT) {
        throw qkd::exception::connection_error("tried to write key to a non-pipe-out connection");
    }
    
    std::list<path_ptr> cPaths;
    if (nPath == -1) {
        cPaths = get_next_paths();
    }
    else {
        if (static_cast<size_t>(nPath) >= d->m_cPaths.size()) {
            throw qkd::exception::connection_error("path index out of range");
        }
        cPaths.push_back(d->m_cPaths[nPath]);
    }
    if (cPaths.empty()) return true;
    if (std::all_of(cPaths.begin(), cPaths.end(), [](path_ptr & p) { return p->is_void(); })) return true;
    
    // iterate over all sockets
    // NOTE: this should be zmq_poll, however this does not properly work
    //       if we have several different polls and send/recv sockets 
    //       in the process
    bool bKeyWritten = false;
    for (auto & cPath : cPaths) {
        if (write_key(*cPath.get(), cKey)) {
            bKeyWritten |= true;
        }
    }

    return bKeyWritten;
}


/**
 * write a key
 * 
 * @param   cPath       the path to write key on
 * @param   cKey        key to pass
 * @return  true, if writing was successful
 */
bool connection::write_key(qkd::module::path & cPath, qkd::key::key const & cKey) {
    
    if (cPath.is_void()) return false;
    if (cPath.is_stdout()) {
        std::cout << cKey;
        return true;
    }
    
    qkd::utility::buffer cBuffer;
    cBuffer << cKey;

    int nWritten = 0;
    do {
        
        nWritten = cPath.send(cBuffer.get(), cBuffer.size());
        if (nWritten == -1) {

            // EAGAIN: currently we are not able to send: try again
            if (zmq_errno() == EAGAIN) continue;

            // EINTR is not critical
            if (zmq_errno() == EINTR) {
                return false;
            }

            std::stringstream ss;
            ss << "failed writing key to next module: " << strerror(zmq_errno());
            throw qkd::exception::network_error(ss.str());
        }
        
    } while (nWritten <= 0);
    
    return true;
}


/**
 * return true if we should act as server
 * 
 * @return  true, if we should bind as listener
 */
bool connection::zmq_socket_server() const {
    
    switch (d->m_eType) {
        
    case connection_type::LISTEN:
    case connection_type::PIPE_IN:
        return true;
    
    case connection_type::PEER:
    case connection_type::PIPE_OUT:
        return false;
        
    default:
        throw std::logic_error("cannot deduce 0MQ socket server mode from connection type");
            
    }
}


/**
 * return the proper 0MQ socket type for this connection
 * 
 * @return  0MQ socket type value
 */
int connection::zmq_socket_type() const {
    
    switch (d->m_eType) {
        
    case connection_type::LISTEN:
        return ZMQ_DEALER;
    
    case connection_type::PEER:
        return ZMQ_DEALER;
    
    case connection_type::PIPE_IN:
        return ZMQ_PULL;
        
    case connection_type::PIPE_OUT:
        return ZMQ_PUSH;
        
    default:
        throw std::logic_error("cannot deduce 0MQ socket type from connection type");
            
    }
    
    return -1;
}


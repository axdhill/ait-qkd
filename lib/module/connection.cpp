/* 
 * connection.cpp
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


// ------------------------------------------------------------
// incs

#include <algorithm>

// boost
#include <boost/tokenizer.hpp>

// Qt
#include <QtCore/QString>
#include <QtCore/QUrl>

#include <qkd/utility/buffer.h>
#include <qkd/utility/debug.h>
#include <qkd/utility/environment.h>
#include <qkd/utility/syslog.h>

#include <qkd/module/connection.h>

using namespace qkd::module;


// ------------------------------------------------------------
// code


/**
 * ctor
 * 
 * @param   eType           type of the connection
 */
connection::connection(connection_type eType) : m_eType(eType), m_eSocketAccess(socket_access::ROUND_ROBIN), m_nCurrentPathIndex(0) {
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
    
    // try to work on an allready added instance
    path_ptr cPath;
    auto cFound = std::find_if(m_cPaths.begin(), m_cPaths.end(), [&](path_ptr & p) { return (p->url() == sURL); });
    if (cFound != m_cPaths.end()) {
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
        if ((m_eType == connection_type::PIPE_IN) || (m_eType == connection_type::PIPE_OUT)) nTimeout = 1000;
        
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
        if (m_eType == connection_type::PIPE_OUT) {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ 
                    << ": url can't be 'stdin' for this connection";
            return false;
        }
    }
    
    // stdout:// on pipe in is not allowed
    if (cPath->is_stdout()) {
        if (m_eType == connection_type::PIPE_IN) {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ 
                    << ": url can't be 'stdout' for this connection";
            return false;
        }
    }
    
    if (cFound == m_cPaths.end()) m_cPaths.push_back(cPath);
    
    return true;
}


/**
 * clears all paths
 * 
 * This closes all sockets and drops them.
 */
void connection::clear() {
    reset();
    m_cPaths.clear();
    m_nCurrentPathIndex = 0;
}


/**
 * get the next paths to work on
 * 
 * @return  a list of current paths to work on
 */
std::list<path_ptr> connection::get_next_paths() {
    
    std::list<path_ptr> res;
    if (m_cPaths.size() == 0) return res;
        
    switch (m_eSocketAccess) {
        
    case socket_access::ROUND_ROBIN:
        res.push_back(m_cPaths[m_nCurrentPathIndex % m_cPaths.size()]);
        m_nCurrentPathIndex++;
        break;
        
    // TODO: case socket_access::BEST_EFFORT:
    case socket_access::ALL:
        for (auto p : m_cPaths) res.push_back(p);
        break;
    
    default:
        throw std::runtime_error("cannot deduce paths for next action on current connection");
    }
    
    return res;
}


/**
 * get a next key from PIPE_IN
 * 
 * @param   cKey        this will receive the next key
 * @return  true, if reading was successful
 */
bool connection::read_key(qkd::key::key & cKey) {

    if (m_eType != connection_type::PIPE_IN) {
        throw std::runtime_error("tried to read key on a non-pipe-in connection");
    }
    
    // empty in/out key
    cKey = qkd::key::key();
    
    // hand out keys we already have read
    if (m_cKeysInStock.size() > 0) {
        cKey = m_cKeysInStock.front();
        m_cKeysInStock.pop_front();
        return true;
    }
    
    std::list<path_ptr> cPaths = get_next_paths();
    if (cPaths.size() == 0) return true;
    
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
            else m_cKeysInStock.push_back(cReadKey);
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
        throw std::runtime_error(ss.str());
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
 * The given message object will be deleted with delet before assigning new values.
 * Therefore if message receive has been successful the message is not NULL
 * 
 * This call waits explcitly for the next message been of type eType. If this
 * is NOT the case a exception is thrown.
 * 
 * @param   cMessage            this will receive the message
 * @return  true, if we have received a message
 */
bool connection::recv_message(qkd::module::message & cMessage) {

    // empty in/out key
    cMessage = qkd::module::message();
    
    // hand out keys we already have read
    if (m_cMessagesInStock.size() > 0) {
        cMessage = m_cMessagesInStock.front();
        m_cMessagesInStock.pop_front();
        return true;
    }
    
    std::list<path_ptr> cPaths = get_next_paths();
    if (cPaths.size() == 0) return false;
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
            else m_cMessagesInStock.push_back(cReadMessage);
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
        throw std::runtime_error("don't know how to read a message from stdin");
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
            ss << "failed reading message header from peer: " << strerror(zmq_errno());
            throw std::runtime_error(ss.str());
        }
        
    } while (nReadHeader <= 0);
    if (!cMsgHeader.more() || (cMsgHeader.size() != sizeof(cMessage.m_cHeader))) {
        throw std::runtime_error("received invalid message header");
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
            ss << "failed reading message data from peer: " << strerror(zmq_errno());
            throw std::runtime_error(ss.str());
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
    for (auto & cPath: m_cPaths) cPath->reset();
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
        if (static_cast<size_t>(nPath) >= m_cPaths.size()) throw std::out_of_range("path index out of range");
        cPaths.push_back(m_cPaths[nPath]);
    }
    
    if (cPaths.size() == 0) return false;
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
        throw std::runtime_error("don't know how to send a message on stdout");
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
        throw std::runtime_error(ss.str());
    }

    int nSentData = cPath.send(cMessage.data().get(), cMessage.data().size());
    if (nSentData == -1) {

        // EINTR is not critical
        if (zmq_errno() == EINTR) {
            return false;
        }
        std::stringstream ss;
        ss << "failed sending message data to peer: " << strerror(zmq_errno());
        throw std::runtime_error(ss.str());
    }

    return true;
}


/**
 * splt a list of urls sperarated by semicolon into a list
 * of url string
 * 
 * @param   sURLs       a string holding many URLs, spearated by ';'
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
    for (auto & cPath : m_cPaths) { res.push_back(cPath->url()); };
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
    
    if (m_eType != connection_type::PIPE_OUT) {
        throw std::runtime_error("tried to write key to a non-pipe-out connection");
    }
    
    std::list<path_ptr> cPaths;
    if (nPath == -1) {
        cPaths = get_next_paths();
    }
    else {
        if (static_cast<size_t>(nPath) >= m_cPaths.size()) throw std::out_of_range("path index out of range");
        cPaths.push_back(m_cPaths[nPath]);
    }
    if (cPaths.size() == 0) return true;
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
            throw std::runtime_error(ss.str());
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
    
    bool res = false;
    
    switch (m_eType) {
        
    case connection_type::LISTEN:
    case connection_type::PIPE_IN:
        res = true;
        break;
    
    case connection_type::PEER:
    case connection_type::PIPE_OUT:
        res =  false;
        break;
        
    default:
        throw std::logic_error("cannot deduce 0MQ socket server mode from connection type");
            
    }
    
    return res;
}


/**
 * return the proper 0MQ socket type for this connection
 * 
 * @return  0MQ socket type value
 */
int connection::zmq_socket_type() const {
    
    switch (m_eType) {
        
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


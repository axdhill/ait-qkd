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

// boost
#include <boost/filesystem.hpp>

// Qt
#include <QtCore/QString>

// 0MQ
#include <zmq.h>

#include <qkd/utility/buffer.h>
#include <qkd/utility/environment.h>
#include <qkd/utility/syslog.h>

#include "connection.h"

using namespace qkd::module;


// ------------------------------------------------------------
// decl


/**
 * 0MQ initializer (singelton)
 */
class zmq_init {
    
    
public:
    
    
    /**
     * the single ZeroMQ context used
     * 
     * @return  the 0MQ context
     */
    static void * ctx() { 
        static zmq_init z;
        return z.m_cZMQContext; 
    }
    

private:
    
    
    /**
     * ctor
     */
    zmq_init() {
        m_cZMQContext = zmq_ctx_new();
        if (m_cZMQContext == nullptr) throw std::runtime_error("unable to create 0MQ context");
    }
    
    
    /**
     * copy ctor
     */
    zmq_init(zmq_init const & rhs) = delete;
    
    
    /**
     * dtor
     */
    ~zmq_init() {
        zmq_ctx_term(m_cZMQContext);
        m_cZMQContext = nullptr;
    }
    
    
    /**
     * our single ZMQ context used
     */
    void * m_cZMQContext;
    
};


// ------------------------------------------------------------
// code


/**
 * ctor
 * 
 * @param   eType           type of the connection
 */
connection::connection(connection_type eType) : 
        m_bSetup(true), 
        m_eType(eType), 
        m_bStdIn(false), 
        m_bStdOut(false), 
        m_bVoid(true), 
        m_nHighWaterMark(1000),
        m_nTimeout(1000),
        m_cSocket(nullptr) {
}


/**
 * dtor
 */
connection::~connection() {
    reset();
}



/**
 * create an IPC socket file
 * 
 * The file will be created within a fixed folder (/tmp/qkd or /run/qkd).
 * The filename will have [prefix.]PID[.suffix]
 * 
 * @param   sPrefix         prefix name of socket
 * @param   sSuffix         suffix name of socket
 */
boost::filesystem::path connection::create_ipc_socket(std::string sPrefix, std::string sSuffix) {
    
    // create some /tmp/qkd/id-pid.in file
    // TODO: this should reside soemwhere in the /run folder: FHS!
    boost::filesystem::path cIPCPath = boost::filesystem::temp_directory_path() / "qkd";
    if (!boost::filesystem::exists(cIPCPath)) {
        if (!boost::filesystem::create_directory(cIPCPath)) {
            
            // fail
            qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ 
                    << ": failed to create folder " << cIPCPath.string();
                    
            std::stringstream ss;
            ss << "unable to create IPC socket file folder '" << cIPCPath.string() << "'";
            throw std::runtime_error(ss.str());
        }
    }
    
    std::stringstream ss;
    if (!sPrefix.empty()) ss << sPrefix << ".";
    ss << qkd::utility::environment::process_id();
    if (!sSuffix.empty()) ss << "." << sSuffix;
    cIPCPath /= ss.str();
    
    return cIPCPath;
}


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
std::string connection::fix_url(std::string const & sURL) {

    if (sURL == "stdin://") return sURL;
    if (sURL == "stdout://") return sURL;

    QUrl cURL(QString::fromStdString(sURL));
    if (cURL.scheme() == "ipc") {
        return fix_url_ipc(sURL);
    }
    if (cURL.scheme() == "tcp") {
        return fix_url_tcp(sURL);
    }
        
    qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "unknown URL scheme: " << sURL;
    return std::string();
}

    
/**
 * deduce a correct, proper IPC-URL from a would-be IPC-URL
 * 
 * @param   sURL        an url
 * @return  a good, real, usable url (or empty() in case of failure)
 */
std::string connection::fix_url_ipc(std::string const & sURL) {

    static const std::string::size_type nSchemeHeader = std::string("ipc://").size();
    std::string sAddress = sURL.substr(nSchemeHeader);
    if (sAddress.empty() || sAddress == "*") {
        
        // we got an unspecified socket file to bind
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ 
                << ": failed to setup url: this is a unspecified IPC url: " << sURL;
        return std::string();
    }
    
    // check that the parent folder exists
    boost::filesystem::path cPath(sAddress);
    cPath = cPath.parent_path();
    if (!boost::filesystem::exists(cPath)) {
        if (!create_directory(cPath)) {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ 
                    << ": failed to setup url: can't access ipc location: " << sURL;
            return std::string();
        }
    }
    
    return sURL;
}


/**
 * deduce a correct, proper TCP-URL from a would-be TCP-URL
 * 
 * @param   sURL        an url
 * @return  a good, real, usable url (or empty() in case of failure)
 */
std::string connection::fix_url_tcp(std::string const & sURL) {

    // decuce proper IP of host
    QUrl cURL(QString::fromStdString(sURL));
    QString sAddress = cURL.host();
    if (sAddress.isEmpty() || sAddress == "*") {
        
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ 
                << ": provided '*' as host to listen on";
        sAddress = QString::fromStdString("0.0.0.0");
    }
    
    // turn any (possible) hostname into an IP address
    std::set<std::string> cAddressesForHost = qkd::utility::environment::host_lookup(sAddress.toStdString());
    if (cAddressesForHost.empty()) {
        qkd::utility::syslog::warning() << "failed to listen: unable to get IP address for hostname: " 
                << sAddress.toStdString();
        return std::string();
    }
    
    // pick the first
    sAddress = QString::fromStdString(*cAddressesForHost.begin());
    
    std::stringstream ss;
    ss << "tcp://";
    ss << sAddress.toStdString();
    if (cURL.port() != -1) {
        ss << ":";
        ss << cURL.port();
    }
    
    return ss.str();
}


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
bool connection::is_ambiguous(QUrl const & cURL) {

    if (cURL.scheme() != "tcp") return false;

    if (cURL.host().isEmpty()) return true;
    if (cURL.host() == "*") return true;
    if (cURL.host() == "0.0.0.0") return true;

    return false;
}


/**
 * setup socket with high water mark and timeout
 *
 * also linger will be set to 0
 *
 * @param   cSocket             socket to modify
 * @param   nHighWaterMark      high water mark
 * @param   nTimeout            timeout on socket
 */
void connection::prepare_socket(void * & cSocket, int nHighWaterMark, int nTimeout) {

    if (zmq_setsockopt(cSocket, ZMQ_RCVHWM, &nHighWaterMark, sizeof(nHighWaterMark)) == -1) {
        std::stringstream ss;
        ss << "failed to set high water mark on socket: " << strerror(zmq_errno());
        zmq_close(cSocket);
        cSocket = nullptr;
        throw std::runtime_error(ss.str());
    }
    if (zmq_setsockopt(cSocket, ZMQ_SNDHWM, &nHighWaterMark, sizeof(nHighWaterMark)) == -1) {
        std::stringstream ss;
        ss << "failed to set high water mark on socket: " << strerror(zmq_errno());
        zmq_close(cSocket);
        cSocket = nullptr;
        throw std::runtime_error(ss.str());
    }
    if (zmq_setsockopt(cSocket, ZMQ_RCVTIMEO, &nTimeout, sizeof(nTimeout)) == -1) {
        std::stringstream ss;
        ss << "failed to set receive timeout on socket: " << strerror(zmq_errno());
        zmq_close(cSocket);
        cSocket = nullptr;
        throw std::runtime_error(ss.str());
    }
    if (zmq_setsockopt(cSocket, ZMQ_SNDTIMEO, &nTimeout, sizeof(nTimeout)) == -1) {
        std::stringstream ss;
        ss << "failed to set send timeout on socket: " << strerror(zmq_errno());
        zmq_close(cSocket);
        cSocket = nullptr;
        throw std::runtime_error(ss.str());
    }

    int nLinger = 0;
    if (zmq_setsockopt(cSocket, ZMQ_LINGER, &nLinger, sizeof(nLinger)) == -1) {
        std::stringstream ss;
        ss << "failed to set linger on socket: " << strerror(zmq_errno());
        throw std::runtime_error(ss.str());
    }

}


/**
 * get a next key from PIPE_IN
 * 
 * @param   cKey        this will receive the next key
 * @return  true, if reading was successful
 */
bool connection::read_key(qkd::key::key & cKey) {
    
    if (type() != connection_type::PIPE_IN) {
        throw std::runtime_error("tried to read key on a non-pipe-in connection");
    }
    
    if (needs_setup()) setup();
    if (is_void()) return true;
    if (is_stdin()) {
        std::cin >> cKey;
        return true;
    }
    
    if (m_cSocket) {
        
        zmq_msg_t cZMQMessage;
        assert(zmq_msg_init(&cZMQMessage) == 0);
        int nRead = zmq_msg_recv(&cZMQMessage, m_cSocket, 0);
        if (nRead == -1) {

            // EAGAIN and EINTR are not critical
            if ((zmq_errno() == EAGAIN) || (zmq_errno() == EINTR)) {
                return false;
            }

            std::stringstream ss;
            ss << "failed reading key: " << strerror(zmq_errno());
            zmq_msg_close(&cZMQMessage);
            throw std::runtime_error(ss.str());
        }

        qkd::utility::buffer cData = qkd::utility::buffer(
                qkd::utility::memory::wrap((unsigned char *)zmq_msg_data(&cZMQMessage), zmq_msg_size(&cZMQMessage)));
        cData >> cKey;

        zmq_msg_close(&cZMQMessage);
        
        return true;
    }
    
    // when we wind up here, there has been a bad config of this connection
    throw std::runtime_error("pipe-in connection lacks ability to read key");
    return false;
}


/**
 * resets the connection to an empty void state
 */
void connection::reset() {
    if (m_cSocket) zmq_close(m_cSocket);
    m_cSocket = nullptr;
    m_bStdIn = false;
    m_bStdOut = false;
    m_bVoid = true;
    m_bSetup = true;
}


/**
 * set the number of milliseconds for network send/recv timeout
 * 
 * @param   nTimeout        the new number of milliseconds for network send/recv timeout
 */
void connection::set_timeout(qlonglong nTimeout) {
    
    m_nTimeout = nTimeout;
    
    if (m_cSocket) {
        if (zmq_setsockopt(m_cSocket, ZMQ_RCVTIMEO, &m_nTimeout, sizeof(m_nTimeout)) == -1) {
            std::stringstream ss;
            ss << "failed to set timeout on socket: " << strerror(zmq_errno());
            throw std::runtime_error(ss.str());
        }
        if (zmq_setsockopt(m_cSocket, ZMQ_SNDTIMEO, &m_nTimeout, sizeof(m_nTimeout)) == -1) {
            std::stringstream ss;
            ss << "failed to set timeout on socket: " << strerror(zmq_errno());
            throw std::runtime_error(ss.str());
        }
    }
}


/**
 * sets the stored url
 * 
 * @param   sURL        the new URL
 */
void connection::set_url(std::string sURL) {
    if (m_sURL == sURL) return;
    m_sURL = sURL;
    m_bSetup = true;
}


/**
 * setup the connection with the given url
 * 
 * @param   sURL                a string holding the URL of the connection
 * @param   nHighWaterMark      high water mark value
 * @param   nTimeout            initial timeout in millisec on this connection
 * @param   sIPCPrefix          ipc socket file prefix
 * @param   sIPCSuffix          ipc socket file suffix
 */
bool connection::setup(std::string sURL, 
        int nHighWaterMark, 
        int nTimeout, 
        std::string sIPCPrefix, 
        std::string sIPCSuffix) {
    
    reset();
    
    m_sURL = sURL;
    m_bSetup = false;
    
    m_nHighWaterMark = nHighWaterMark;
    m_nTimeout = nTimeout;

    // void connection
    if (sURL.empty()) return true;
    
    QUrl cURL(QString::fromStdString(sURL));
    
    // stdin://
    if (cURL.scheme() == "stdin") {
        if (!is_incoming()) {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ 
                    << ": url can't be 'stdin' for this connection";
            return false;
        }
        
        m_bStdIn = true;
        m_bVoid = false;
        return true;
    }
    
    // stdout://
    if (cURL.scheme() == "stdout") {
        if (!is_outgoing()) {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ 
                    << ": url can't be 'stdout' for this connection";
            return false;
        }
        
        m_bStdOut = true;
        m_bVoid = false;
        return true;
    }
    
    // ipc:// ... fix filesystem access
    if (cURL.scheme() == "ipc") {
        
        // pick the correct IPC path
        boost::filesystem::path cIPC(cURL.path().toStdString());
        if (cIPC.empty()) cIPC = create_ipc_socket(sIPCPrefix, sIPCSuffix);
        if (cIPC.empty()) {
            qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ 
                    << ": failed to create input IPC for '" << sURL << "'";
            return false;
        }
        
        sURL = fix_url_ipc("ipc://" + cIPC.string());
        cURL = QUrl(QString::fromStdString(sURL));
    }
        
    // ipc:// or tcp://
    if ((cURL.scheme() == "ipc") || (cURL.scheme() == "tcp")) {
        
        m_bStdIn = false;
        m_bStdOut = false;
        m_bVoid = false;
        
        m_cSocket = zmq_socket(zmq_init::ctx(), zmq_socket_type());
        prepare_socket(m_cSocket, nHighWaterMark, nTimeout); 
        
        switch (type()) {
            
        case connection_type::PIPE_IN:
        case connection_type::LISTEN:
            
            // warn if we use a "*" or empty host here
            if (is_ambiguous(cURL)) {
                qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ 
                    << ": warning: URL '" << sURL << "' contains ambiguous address - this may fail!";
            }
            
            if (zmq_bind(m_cSocket, sURL.c_str()) == -1) {
                std::stringstream ss;
                ss << "url: '" << m_sURL << "' - failed to bind socket: " << strerror(zmq_errno());
                throw std::runtime_error(ss.str());
            }
            return true;
            
        case connection_type::PIPE_OUT:
        case connection_type::PEER:
            zmq_connect(m_cSocket, sURL.c_str());
            return true;
            
        default:
            throw std::logic_error("implementation ipc:// or tcp:// for current connection type is missing");
            
        }
    }
    
    // at this line: we do not know how to handle this type of URL
    qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ 
            << ": url scheme not recognized: " << cURL.scheme().toStdString();
    
    return false;
    
}


/**
 * write a key to the next module
 * 
 * @param   cKey        key to pass to the next module
 * @return  true, if writing was successful
 */
bool connection::write_key(qkd::key::key const & cKey) {
    
    if (type() != connection_type::PIPE_OUT) {
        throw std::runtime_error("tried to write key on a non-pipe-out connection");
    }
    
    if (needs_setup()) setup();
    if (is_void()) return true;
    if (is_stdout()) {
        std::cout << cKey;
        return true;
    }
    
    if (m_cSocket) {
        
        qkd::utility::buffer cBuffer;
        cBuffer << cKey;

        int nWritten = zmq_send(m_cSocket, cBuffer.get(), cBuffer.size(), 0);
        if (nWritten == -1) {

            // EINTR is not critical
            if (zmq_errno() == EINTR) {
                return false;
            }

            // EAGAIN: currently the next peer is not able to send
            if (zmq_errno() == EAGAIN) {
                return false;
            }

            std::stringstream ss;
            ss << "failed writing key to next module: " << strerror(zmq_errno());
            throw std::runtime_error(ss.str());
        }
        
        return true;
    }
    
    // when we wind up here, there has been a bad config of this connection
    throw std::runtime_error("pipe-out connection lacks ability to write key");
    return false;
}


/**
 * return the proper 0MQ socket type for this connection
 * 
 * @return  0MQ socket type value
 */
int connection::zmq_socket_type() const {
    
    switch (type()) {
        
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

    

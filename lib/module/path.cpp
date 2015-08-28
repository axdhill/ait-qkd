/* 
 * path.cpp
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


// ------------------------------------------------------------
// incs

// Qt
#include <QtCore/QUrl>

#include <qkd/utility/environment.h>
#include <qkd/utility/syslog.h>

#include "path.h"

using namespace qkd::module;


// ------------------------------------------------------------
// decl


/**
 * 0MQ initializer (singelton)
 */
class zmq_init {
    
    
public:
    
    
    /**
     * dtor
     */
    ~zmq_init() {
        zmq_ctx_term(m_cZMQContext);
        m_cZMQContext = nullptr;
    }
    
    
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
     * our single ZMQ context used
     */
    void * m_cZMQContext;
    
};


// ------------------------------------------------------------
// code


/**
 * ctor
 */
path::path() : m_nHighWaterMark(1000), m_cSocket(nullptr) {
    reset();
}


/**
 * dtor
 */
path::~path() {
    reset();
}


/**
 * create an IPC socket file
 * 
 * @param   sIPCSocketFileName      the intended socket file name
 * @return  a full path to the socket file
 */
boost::filesystem::path path::create_ipc_socket(std::string sIPCSocketFileName) {
    
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
    
    cIPCPath /= sIPCSocketFileName;
    
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
std::string path::fix_url(std::string const & sURL) {

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
std::string path::fix_url_ipc(std::string const & sURL) {

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
std::string path::fix_url_tcp(std::string const & sURL) {

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
 * @param   sURL        an url to check
 * @return  true, if the URL contains an ambiguous host
 */
bool path::is_ambiguous(std::string sURL) {
    
    QUrl cURL(QString::fromStdString(sURL));

    if (cURL.scheme() != "tcp") return false;
    if (cURL.host().isEmpty()) return true;
    if (cURL.host() == "*") return true;
    if (cURL.host() == "0.0.0.0") return true;

    return false;
}


/**
 * setup path with high water mark and timeout
 *
 * also linger will be set to 0
 *
 * @param   nHighWaterMark      high water mark
 * @param   nTimeout            timeout on socket
 */
void path::prepare(int nHighWaterMark, int nTimeout) {
    
    if (m_cSocket == nullptr) return;

    if (zmq_setsockopt(m_cSocket, ZMQ_RCVHWM, &nHighWaterMark, sizeof(nHighWaterMark)) == -1) {
        reset();
        std::stringstream ss;
        ss << "failed to set high water mark on socket: " << strerror(zmq_errno());
        throw std::runtime_error(ss.str());
    }
    if (zmq_setsockopt(m_cSocket, ZMQ_SNDHWM, &nHighWaterMark, sizeof(nHighWaterMark)) == -1) {
        reset();
        std::stringstream ss;
        ss << "failed to set high water mark on socket: " << strerror(zmq_errno());
        throw std::runtime_error(ss.str());
    }
    if (!set_timeout_incoming(nTimeout)) {
        reset();
        std::stringstream ss;
        ss << "failed to set receive timeout on socket: " << strerror(zmq_errno());
        throw std::runtime_error(ss.str());
    }
    if (!set_timeout_outgoing(nTimeout)) {
        reset();
        std::stringstream ss;
        ss << "failed to set send timeout on socket: " << strerror(zmq_errno());
        throw std::runtime_error(ss.str());
    }

    int nLinger = 0;
    if (zmq_setsockopt(m_cSocket, ZMQ_LINGER, &nLinger, sizeof(nLinger)) == -1) {
        std::stringstream ss;
        ss << "failed to set linger on socket: " << strerror(zmq_errno());
        throw std::runtime_error(ss.str());
    }
}


/**
 * reset the path to void
 */
void path::reset() {
    if (m_cSocket) zmq_close(m_cSocket);
    m_cSocket = nullptr;
    m_bIPC = false;
    m_bStdIn = false;
    m_bStdOut = false;
    m_bTCP = false;
    m_bVoid = true;
    m_bSetup = true;
}


/**
 * set incoming (recv) timeout
 * 
 * @param   nTimeout    in millisec, -1 for infinite
 * @return  true, if set, false for error
 */
bool path::set_timeout_incoming(int nTimeout) {
    if (m_cSocket == nullptr) return false;
    return (zmq_setsockopt(m_cSocket, ZMQ_RCVTIMEO, &nTimeout, sizeof(nTimeout)) != -1);
}


/**
 * set incoming (recv) timeout
 * 
 * @param   nTimeout    in millisec, -1 for infinite
 * @return  true, if set, false for error
 */
bool path::set_timeout_outgoing(int nTimeout) {
    if (m_cSocket == nullptr) return false;
    return (zmq_setsockopt(m_cSocket, ZMQ_SNDTIMEO, &nTimeout, sizeof(nTimeout)) != -1);
}


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
 * @param   nTimeout            the timeout in milliseconds for actions on this path
 * @param   nHighWaterMark      the number of messages in transit for this path
 * @param   sIPCHint            proper file name to use for IPC paths (if ipc:// is ambiguous)
 */
void path::set_url(std::string sURL, bool bServer, int nSocketType, int nTimeout, int nHighWaterMark,  std::string sIPCHint) {
    
    if (sURL.find(';') != std::string::npos) {
        throw std::invalid_argument("given URL '" + sURL + "' contains illegal char ';'");
    }
    m_sURL = sURL;
    m_bSetup = true;
    reset();
    
    // void connection
    if (sURL.empty()) return;
    
    QUrl cURL(QString::fromStdString(sURL));
    
    // stdin://
    if (cURL.scheme() == "stdin") {
        m_bIPC = false;
        m_bStdIn = true;
        m_bStdOut = false;
        m_bTCP = false;
        m_bVoid = false;
    }
    
    // stdout://
    if (cURL.scheme() == "stdout") {
        m_bIPC = false;
        m_bStdIn = false;
        m_bStdOut = true;
        m_bTCP = false;
        m_bVoid = false;
    }
    
    // ipc:// ... fix filesystem access
    if (cURL.scheme() == "ipc") {
        
        // pick the correct IPC path
        boost::filesystem::path cIPC(cURL.path().toStdString());
        if (cIPC.empty()) cIPC = create_ipc_socket(sIPCHint);
        if (cIPC.empty()) {
            qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ 
                    << ": failed to create input IPC for '" << sURL << "'";
            return;
        }
        
        m_sURL = fix_url_ipc("ipc://" + cIPC.string());
        
        m_bIPC = true;
        m_bStdIn = false;
        m_bStdOut = false;
        m_bTCP = false;
        m_bVoid = false;
    }
        
    // tcp://
    if (cURL.scheme() == "tcp") {
        m_bIPC = false;
        m_bStdIn = false;
        m_bStdOut = false;
        m_bTCP = true;
        m_bVoid = false;
    }
    
    // now that URL is clarified, run the setup
    setup(bServer, nSocketType, nTimeout, nHighWaterMark);
}


/**
 * setup the path
 * 
 * @param   bServer         rather use "bind()" for socket and not "connect()"
 * @param   nSocketType     the ZMQ socket type
 * @param   nTimeout            the timeout in milliseconds for actions on this path
 * @param   nHighWaterMark      the number of messages in transit for this path
 */
void path::setup(bool bServer, int nSocketType, int nTimeout, int nHighWaterMark) {
    
    m_bSetup = false;
    if (is_void()) return;
    if (is_stdin()) return;
    if (is_stdout()) return;
    
    // neither void, stdin, stdout now ...
    
    m_cSocket = zmq_socket(zmq_init::ctx(), nSocketType);
    prepare(nHighWaterMark, nTimeout); 
    
    if (is_ambiguous(m_sURL)) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ 
            << ": warning: URL '" << m_sURL << "' contains ambiguous address - this may fail!";
    }
    
    if (bServer) {
        
        // warn if we use a "*" or empty host here
        if (zmq_bind(m_cSocket, m_sURL.c_str()) == -1) {
            std::stringstream ss;
            ss << "url: '" << m_sURL << "' - failed to bind socket: " << strerror(zmq_errno());
            throw std::runtime_error(ss.str());
        }
        
    }
    else {
        if (zmq_connect(m_cSocket, m_sURL.c_str()) == -1) {
            std::stringstream ss;
            ss << "url: '" << m_sURL << "' - failed to connect socket: " << strerror(zmq_errno());
            throw std::runtime_error(ss.str());
        }
    }
}


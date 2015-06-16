/* 
 * module_internal.cpp
 * 
 * QKD module internal implementation
 *
 * Autor: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
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


#include <boost/format.hpp>

#include <qkd/module/module.h>
#include <qkd/utility/syslog.h>

#include "module_internal.h"

using namespace qkd::module;


// ------------------------------------------------------------
// decl


/**
 * module lib initilizer
 */
class module_init {
    
    
public:
    
    
    /**
     * ctor
     */
    module_init();
    
    
    /**
     * copy ctor
     */
    module_init(module_init const & rhs) = delete;
    
    
    /**
     * dtor
     */
    ~module_init();
    
    
    /**
     * the single ZeroMQ context used
     * 
     * @return  the 0MQ context
     */
    inline zmq::context_t & zmq_ctx() { return *m_cZMQContext; };
    

private:
    
    
    /**
     * our single ZMQ context used
     */
    zmq::context_t * m_cZMQContext;
    
};


// ------------------------------------------------------------
// vars


/**
 * create a static instance of the initilizer
 */
static module_init g_cInit;


// ------------------------------------------------------------
// code


/**
 * ctor
 */
module_init::module_init() : m_cZMQContext(nullptr) {
    
    // this is the place to run module framework 
    // init code.
    //
    // this is a singelton and won't run twice
    
    m_cZMQContext = new zmq::context_t(2);
    assert(m_cZMQContext != nullptr);
}


/**
 * dtor
 */
module_init::~module_init() {
    
    // this is run, when the process exits once.
    // include here correct and graceful framwork
    // and resource rundown.
    
    delete m_cZMQContext;
}


/**
 * add key statistics for incoming
 * 
 * @param   cKey        new key arrived
 */
void module::module_internal::add_stats_incoming(qkd::key::key const & cKey) {

    std::lock_guard<std::recursive_mutex> cLock(cStat.cMutex);

    cStat.nKeysIncoming++;
    cStat.nKeyBitsIncoming += cKey.size() * 8;
    cStat.nDisclosedBitsIncoming += cKey.meta().nDisclosedBits;
    cStat.cKeysIncomingRate << cStat.nKeysIncoming;
    cStat.cKeyBitsIncomingRate << cStat.nKeyBitsIncoming;
    cStat.cDisclosedBitsIncomingRate << cStat.nDisclosedBitsIncoming;
}


/**
 * add key statistics for outgoing
 * 
 * @param   cKey        key sent
 */
void module::module_internal::add_stats_outgoing(qkd::key::key const & cKey) {

    std::lock_guard<std::recursive_mutex> cLock(cStat.cMutex);

    cStat.nKeysOutgoing++;
    cStat.nKeyBitsOutgoing += cKey.size() * 8;
    cStat.nDisclosedBitsOutgoing += cKey.meta().nDisclosedBits;
    cStat.cKeysOutgoingRate << cStat.nKeysOutgoing;
    cStat.cKeyBitsOutgoingRate << cStat.nKeyBitsOutgoing;
    cStat.cDisclosedBitsOutgoingRate << cStat.nDisclosedBitsOutgoing;
}


/**
 * connect to remote instance
 * 
 * @param   sPeerURL        the remote instance URL
 */
void module::module_internal::connect(std::string sPeerURL) {
    this->sURLPeer = sPeerURL;
}


/**
 * create an IPC incoming path
 */
boost::filesystem::path module::module_internal::create_ipc_in() const {
    
    // create some /tmp/qkd/id-pid.in file
    // TODO: this should reside soemwhere in the /run folder: FHS!
    boost::filesystem::path cIPCPath = boost::filesystem::temp_directory_path() / "qkd";
    if (!boost::filesystem::exists(cIPCPath)) {
        if (!boost::filesystem::create_directory(cIPCPath)) {
            
            // fail
            qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to create folder " << cIPCPath.string();
            return boost::filesystem::path();
        }
    }
    
    // now add id and pid()
    std::stringstream ss;
    ss << sId << "-" << qkd::utility::environment::process_id() << ".in";
    cIPCPath /= ss.str();
    
    return cIPCPath;
}


/**
 * create an IPC outgoing path
 */
boost::filesystem::path module::module_internal::create_ipc_out() const {
    
    // create some /tmp/qkd/id-pid.out file
    // TODO: this should reside soemwhere in the /run folder: FHS!
    boost::filesystem::path cIPCPath = boost::filesystem::temp_directory_path() / "qkd";
    if (!boost::filesystem::exists(cIPCPath)) {
        if (!boost::filesystem::create_directory(cIPCPath)) {
            
            // fail
            qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to create folder " << cIPCPath.string();
            return boost::filesystem::path();
        }
    }
    
    // now add id and pid()
    std::stringstream ss;
    ss << sId << "-" << qkd::utility::environment::process_id() << ".out";
    cIPCPath /= ss.str();
    
    return cIPCPath;
}


/**
 * dump current module config
 */
void module::module_internal::debug_config() {

    qkd::utility::debug() << "current module config:";
    qkd::utility::debug() << "              role: " << cModule->role_name().toStdString();
    qkd::utility::debug() << "          url_peer: " << cModule->url_peer().toStdString();
    qkd::utility::debug() << "       url_pipe_in: " << cModule->url_pipe_in().toStdString();
    qkd::utility::debug() << "      url_pipe_out: " << cModule->url_pipe_out().toStdString();
    qkd::utility::debug() << "        url_listen: " << cModule->url_listen().toStdString();
    qkd::utility::debug() << "          pipeline: " << cModule->pipeline().toStdString();
    qkd::utility::debug() << "              hint: " << cModule->hint().toStdString();
    qkd::utility::debug() << "        random_url: " << cModule->random_url().toStdString();
    qkd::utility::debug() << "  synchronize_keys: " << (cModule->synchronize_keys() ? "true" : "false");
    qkd::utility::debug() << "   synchronize_ttl: " << cModule->synchronize_ttl();
    qkd::utility::debug() << "   timeout_network: " << cModule->timeout_network();
    qkd::utility::debug() << "      timeout_pipe: " << cModule->timeout_pipe();
}


/**
 * dump a message to stderr
 *
 * @param   bSent       message has been sent
 * @param   cMessage    message itself
 */
void module::module_internal::debug_message(bool bSent, qkd::module::message const & cMessage) {

    if (!bDebugMessageFlow) return;
    if (bSent) {
        qkd::utility::debug() << "<MOD-SENT>" << cMessage.string("          ");
    }
    else {
        qkd::utility::debug() << "<MOD-RECV>" << cMessage.string("          ");
    }
 }


/**
 * dump key PULL to stderr
 *
 * @param   cKey        key to dump
 */
void module::module_internal::debug_key_pull(qkd::key::key const & cKey) {

    // if not needed, then performance is wasted here
    boost::format cLineFormater = 
            boost::format("key-PULL [%015ums] id: %010u bits: %010u err: %6.4f dis: %010u crc: %08x state: %-13s");
    
    auto cTimePoint = std::chrono::duration_cast<std::chrono::milliseconds>(cModule->age());
    cLineFormater % cTimePoint.count();
    cLineFormater % cKey.id();
    cLineFormater % (cKey.size() * 8);
    cLineFormater % cKey.meta().nErrorRate;
    cLineFormater % cKey.meta().nDisclosedBits;
    cLineFormater % cKey.data().crc32();
    cLineFormater % cKey.state_string();
    
    qkd::utility::debug() << cLineFormater.str();
}


/**
 * dump key PUSH  to stderr
 *
 * @param   cKey        key to dump
 */
void module::module_internal::debug_key_push(qkd::key::key const & cKey) {

    // if not needed, then performance is wasted here
    boost::format cLineFormater = 
            boost::format("key-PUSH [%015ums] id: %010u bits: %010u err: %6.4f dis: %010u crc: %08x state: %-13s dur: %012u ns (%06u ms)");

    auto cTimePoint = std::chrono::duration_cast<std::chrono::milliseconds>(cModule->age());
    cLineFormater % cTimePoint.count();
    cLineFormater % cKey.id();
    cLineFormater % (cKey.size() * 8);
    cLineFormater % cKey.meta().nErrorRate;
    cLineFormater % cKey.meta().nDisclosedBits;
    cLineFormater % cKey.data().crc32();
    cLineFormater % cKey.state_string();
    auto cNanoSeconds =  std::chrono::duration_cast<std::chrono::nanoseconds>(cKey.dwell());
    cLineFormater % cNanoSeconds.count();
    cLineFormater % (uint64_t)(std::floor(cNanoSeconds.count() / 1000000.0 + 0.5));
    
    qkd::utility::debug() << cLineFormater.str();
}


/**
 * deduce a correct, proper URL from a would-be URL
 * 
 * @param   sURL        an url
 * @return  a good, real, usable url (or empty() in case of failure)
 */
std::string module::module_internal::fix_url(std::string const & sURL) {

    // check for standard urls
    if (sURL == "stdin://") return sURL;
    if (sURL == "stdout://") return sURL;

    // check URL
    QUrl cURL(QString::fromStdString(sURL));
    
    // ipc
    if (cURL.scheme() == "ipc") {
        return fix_url_ipc(sURL);
    }
    
    // tcp
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
std::string module::module_internal::fix_url_ipc(std::string const & sURL) {

    // decuce proper filename from "ipc://" part
    static const std::string::size_type nSchemeHeader = std::string("ipc://").size();
    std::string sAddress = sURL.substr(nSchemeHeader);
    if (sAddress.empty() || sAddress == "*") {
        
        // we got an unspecified socket file to bind
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to setup url: this is a unspecified IPC url: " << sURL;
        return std::string();
    }
    
    // check that the parent folder exists
    boost::filesystem::path cPath(sAddress);
    cPath = cPath.parent_path();
    if (!boost::filesystem::exists(cPath)) {
        if (!create_directory(cPath)) {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to setup url: can't access ipc location: " << sURL;
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
std::string module::module_internal::fix_url_tcp(std::string const & sURL) {

    // decuce proper IP of host
    QUrl cURL(QString::fromStdString(sURL));
    QString sAddress = cURL.host();
    if (sAddress.isEmpty() || sAddress == "*") {
        
        // we got an unspecified IP to bind
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "provided '*' as host to listen on";
        sAddress = QString::fromStdString("0.0.0.0");
    }
    
    // turn any (possible) hostname into an IP address
    std::set<std::string> cAddressesForHost = qkd::utility::environment::host_lookup(sAddress.toStdString());
    if (cAddressesForHost.empty()) {
        qkd::utility::syslog::warning() << "failed to listen: unable to get IP address for hostname: " << sAddress.toStdString();
        return std::string();
    }
    
    // pick the first
    sAddress = QString::fromStdString(*cAddressesForHost.begin());
    
    // construct good url
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
 * get the current module state
 * 
 * @return  the current module state
 */
module_state module::module_internal::get_state() const {
    std::unique_lock<std::mutex> cLock(cStateMutex);
    return eState;
}


/**
 * clean any resources left
 */
void module::module_internal::release() {
    
    set_state(module_state::STATE_TERMINATING);
    
    // free ZMQ stuff (if any)
    
    // by setting the LINGER to 0, we
    // kick all pending messages
    
    if (cSocketListener != nullptr) {
        int nLinger = 0;
        cSocketListener->setsockopt(ZMQ_LINGER, &nLinger, sizeof(nLinger));
        delete cSocketListener;
    }
    cSocketListener = nullptr;
    
    if (cSocketPeer != nullptr) {
        int nLinger = 0;
        cSocketPeer->setsockopt(ZMQ_LINGER, &nLinger, sizeof(nLinger));
        delete cSocketPeer;
    }
    cSocketPeer = nullptr;
    
    if (cSocketPipeIn != nullptr) {
        int nLinger = 0;
        cSocketPipeIn->setsockopt(ZMQ_LINGER, &nLinger, sizeof(nLinger));
        delete cSocketPipeIn;
    }
    cSocketPipeIn = nullptr;
    
    if (cSocketPipeOut != nullptr) {
        int nLinger = 0;
        cSocketPipeOut->setsockopt(ZMQ_LINGER, &nLinger, sizeof(nLinger));
        delete cSocketPipeOut;
    }
    cSocketPipeOut = nullptr;
    
    // reset connection settings to initial
    bPipeInStdin = false;
    bPipeInVoid = true;
    bPipeOutStdout = false;
    bPipeOutVoid = true;
    
    // if restarted we have to indicate to start connections anew
    bSetupListen = true;
    bSetupPeer = true;
    bSetupPipeIn = true;
    bSetupPipeOut = true;
    
    set_state(module_state::STATE_TERMINATED);
}


/**
 * set a new module state
 * 
 * the working thread will be notified (if waiting)
 * 
 * @param   eNewState       the new module state
 */
void module::module_internal::set_state(module_state eNewState) {
    
    std::unique_lock<std::mutex> cLock(cStateMutex);
    eState = eNewState;
    cStateCondition.notify_all();
}


/**
 * runs all the setup code for the module worker thread
 * 
 * @return  true, if all is laid out properly
 */
bool module::module_internal::setup() {
    
    bool bRes = true;
    
    // setup the connection endpoints
    bRes = bRes && setup_pipe_in();
    bRes = bRes && setup_pipe_out();
    bRes = bRes && setup_listen();
        
    return bRes;
}


/**
 * setup listen
 * 
 * @return  true, for success
 */
bool module::module_internal::setup_listen() {

    // do not change URL settings yet
    std::lock_guard<std::mutex> cLock(cURLMutex);
    
    // from now on we have tried to setup listen
    bSetupListen = false;
    
    // reset peer connection stuff
    if (cSocketListener) delete cSocketListener;
    cSocketListener = nullptr;
    
    // if we ain't got a URL then we are already finished
    if (!sURLListen.empty()) sURLListen = fix_url(sURLListen);
    if (sURLListen.empty()) return true;
    
    try {
        
        // create the ZMQ socket
        cSocketListener = new zmq::socket_t(g_cInit.zmq_ctx(), ZMQ_DEALER);
        
#if (ZMQ_VERSION_MAJOR == 3)
        // set HWM to 1000
        int nHighWaterMark = 1000;
        cSocketListener->setsockopt(ZMQ_RCVHWM, &nHighWaterMark, sizeof(nHighWaterMark));
        cSocketListener->setsockopt(ZMQ_SNDHWM, &nHighWaterMark, sizeof(nHighWaterMark));
#else
        // set HWM to 1000
        uint64_t nHighWaterMark = 1000;
        cSocketListener->setsockopt(ZMQ_HWM, &nHighWaterMark, sizeof(nHighWaterMark));
#endif        
        
        // set send/recv time out
        cSocketListener->setsockopt(ZMQ_RCVTIMEO, &nTimeoutNetwork, sizeof(nTimeoutNetwork));
        cSocketListener->setsockopt(ZMQ_SNDTIMEO, &nTimeoutNetwork, sizeof(nTimeoutNetwork));
        
        // bind!
        qkd::utility::syslog::info() << "binding module listen on " << sURLListen;
        cSocketListener->bind(sURLListen.c_str());
        
        // debug
        if (qkd::utility::debug::enabled()) qkd::utility::debug() << "listen set to '" << sURLListen << "'";
        
    }
    catch (zmq::error_t & cZMQError) {
        
        // fail
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to setup listen endpoint with url: " << sURLListen << " error: " << cZMQError.what(); 
        
        if (cSocketListener) delete cSocketListener;
        cSocketListener = nullptr;
        
        return false;
    }

    return true;
}


/**
 * setup peer
 * 
 * @return  true, for success
 */
bool module::module_internal::setup_peer() {

    // do not change URL settings yet
    std::lock_guard<std::mutex> cLock(cURLMutex);
    
    // from now on we have tried to setup listen
    bSetupPeer = false;
    
    // reset peer connection stuff
    if (cSocketPeer) delete cSocketPeer;
    cSocketPeer = nullptr;
    
    // if we ain't got a URL then we are already finished
    if (!sURLPeer.empty()) sURLPeer = fix_url(sURLPeer);
    if (sURLPeer.empty()) return true;
    
    try {
        
        // create the ZMQ socket
        cSocketPeer = new zmq::socket_t(g_cInit.zmq_ctx(), ZMQ_DEALER);
        
#if (ZMQ_VERSION_MAJOR == 3)
        // set HWM to 1000
        int nHighWaterMark = 1000;
        cSocketPeer->setsockopt(ZMQ_RCVHWM, &nHighWaterMark, sizeof(nHighWaterMark));
        cSocketPeer->setsockopt(ZMQ_SNDHWM, &nHighWaterMark, sizeof(nHighWaterMark));
#else
        // set HWM to 1000
        uint64_t nHighWaterMark = 1000;
        cSocketPeer->setsockopt(ZMQ_HWM, &nHighWaterMark, sizeof(nHighWaterMark));
#endif        
        
        // set send/recv time out
        cSocketPeer->setsockopt(ZMQ_RCVTIMEO, &nTimeoutNetwork, sizeof(nTimeoutNetwork));
        cSocketPeer->setsockopt(ZMQ_SNDTIMEO, &nTimeoutNetwork, sizeof(nTimeoutNetwork));
        
        // connect
        cSocketPeer->connect(sURLPeer.c_str());
        
        // debug
        if (qkd::utility::debug::enabled()) qkd::utility::debug() << "connected to '" << sURLPeer << "'";
    }
    catch (zmq::error_t & cZMQError) {
        
        // fail
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to connect to url: " << sURLPeer << " error: " << cZMQError.what();
        
        if (cSocketPeer) delete cSocketPeer;
        cSocketPeer = nullptr;
        
        return false;
    }

    return true;
}


/**
 * setup pipe IN
 * 
 * @return  true, for success
 */
bool module::module_internal::setup_pipe_in() {
    
    // do not change URL settings yet
    std::lock_guard<std::mutex> cLock(cURLMutex);
    
    // from now on we have tried to setup pipe-in
    bSetupPipeIn = false;
    
    // reset pipe in stuff
    if (cSocketPipeIn) delete cSocketPipeIn;
    cSocketPipeIn = nullptr;
    bPipeInStdin = false;
    bPipeInVoid = true;

    // if we ain't got an URL then we are already finished
    if (sURLPipeIn.empty()) return true;
    
    // check URL
    QUrl cURLPipeIn(QString::fromStdString(sURLPipeIn));
    if (cURLPipeIn.scheme() == "stdout") {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "input pipe stream can't be 'stdout'";
        return false;
    }
    
    // stdin://
    if (cURLPipeIn.scheme() == "stdin") {
        if (qkd::utility::debug::enabled()) qkd::utility::debug() << "input pipe stream set to 'stdin://'";
        bPipeInStdin = true;
        bPipeInVoid = false;
        return true;
    }
    
    // fix ipc:// URLs in advance
    if (cURLPipeIn.scheme() == "ipc") {
        
        // pick the correct IPC path
        boost::filesystem::path cIPC(cURLPipeIn.path().toStdString());
        if (cIPC.empty()) cIPC = create_ipc_in();
        if (cIPC.empty()) {
            qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to create input IPC for '" << sURLPipeIn << "'";
            return false;
        }
        
        // remember fixed URL
        sURLPipeIn = fix_url_ipc("ipc://" + cIPC.string());
        
        // reread url
        cURLPipeIn = QUrl(QString::fromStdString(sURLPipeIn));
    }
        
    // ipc:// or tcp://
    if ((cURLPipeIn.scheme() == "ipc") || (cURLPipeIn.scheme() == "tcp")) {
        
        bPipeInStdin = false;
        bPipeInVoid = false;
        
        try {


            // create the ZMQ socket
            cSocketPipeIn = new zmq::socket_t(g_cInit.zmq_ctx(), ZMQ_PULL);
            
#if (ZMQ_VERSION_MAJOR == 3)
            
            // set HighWaterMark to 1000
            int nHighWaterMark = 1000;
            cSocketPipeIn->setsockopt(ZMQ_RCVHWM, &nHighWaterMark, sizeof(nHighWaterMark));
#else
            // set HighWaterMark to 1000
            uint64_t nHighWaterMark = 1000;
            cSocketPipeIn->setsockopt(ZMQ_HWM, &nHighWaterMark, sizeof(nHighWaterMark));
#endif        
            
            // set recv time out
            cSocketPipeIn->setsockopt(ZMQ_RCVTIMEO, &nTimeoutPipe, sizeof(nTimeoutPipe));
            
            // warn if we use a "*" or empty host here
            bool bAmbiguousHost = (cURLPipeIn.scheme() == "tcp") && ((cURLPipeIn.host().isEmpty()) || (cURLPipeIn.host() == "*") || (cURLPipeIn.host() == "0.0.0.0"));
            if (bAmbiguousHost) qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "warning: pipe-in URL '" << sURLPipeIn << "' contains ambiguous host address - this may fail!";
        
            // bind!
            cSocketPipeIn->bind(sURLPipeIn.c_str());
            
            // debug
            if (qkd::utility::debug::enabled()) qkd::utility::debug() << "input pipe stream set to '" << sURLPipeIn << "'";
            
        }
        catch (zmq::error_t & cZMQError) {
            
            // fail
            qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to setup input with url: " << sURLPipeIn << " error: " << cZMQError.what(); 
            
            if (cSocketPipeIn) delete cSocketPipeIn;
            cSocketPipeIn = nullptr;
            
            return false;
        }

        return true;
    }
    
    // we do not know how to handle this type of URL
    qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "input pipe url scheme not recognized: " << cURLPipeIn.scheme().toStdString();
    
    return false;
}


/**
 * setup pipe OUT
 * 
 * @return  true, for success
 */
bool module::module_internal::setup_pipe_out() {

    // do not change URL settings yet
    std::lock_guard<std::mutex> cLock(cURLMutex);
    
    // from now on we have tried to setup pipe-out
    bSetupPipeOut = false;
    
    // reset pipe out stuff
    if (cSocketPipeOut) delete cSocketPipeOut;
    cSocketPipeOut = nullptr;
    bPipeOutStdout = false;
    bPipeOutVoid = true;
    
    // if we ain't got a URL then we are already finished
    if (sURLPipeOut.empty()) return true;
    
    // check URL scheme
    QUrl cURLPipeOut(QString::fromStdString(sURLPipeOut));
    if (cURLPipeOut.scheme() == "stdin") {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "output pipe stream can't be 'stdin'";
        return false;
    }
    
    // stdout://
    if (cURLPipeOut.scheme() == "stdout") {
        if (qkd::utility::debug::enabled()) qkd::utility::debug() << "output pipe stream set to 'stdout://'";
        bPipeOutStdout = true;
        bPipeOutVoid = false;
        return true;
    }
    
    // fix ipc:// URL strings in advance
    if (cURLPipeOut.scheme() == "ipc") {
        
        // pick the correct IPC path
        boost::filesystem::path cIPC(cURLPipeOut.path().toStdString());
        if (cIPC.empty()) cIPC = create_ipc_out();
        if (cIPC.empty()) {
            qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to create output IPC for '" << sURLPipeOut  << "'";
            return false;
        }
        
        // remember fixed URL
        sURLPipeOut = fix_url_ipc("ipc://" + cIPC.string());
        
        // reread url
        cURLPipeOut = QUrl(QString::fromStdString(sURLPipeOut));
    }
        
    // ipc:// or tcp://
    if ((cURLPipeOut.scheme() == "ipc") || (cURLPipeOut.scheme() == "tcp")) {
        
        bPipeOutStdout = false;
        bPipeOutVoid = false;
        
        try {
            
            // create the ZMQ socket
            cSocketPipeOut = new zmq::socket_t(g_cInit.zmq_ctx(), ZMQ_PUSH);

#if (ZMQ_VERSION_MAJOR == 3)
            // set HWM to 1000
            int nHighWaterMark = 1000;
            cSocketPipeOut->setsockopt(ZMQ_SNDHWM, &nHighWaterMark, sizeof(nHighWaterMark));
#else
            // set HWM to 1000
            uint64_t nHighWaterMark = 1000;
            cSocketPipeOut->setsockopt(ZMQ_HWM, &nHighWaterMark, sizeof(nHighWaterMark));
#endif        
            
            // set recv time out
            cSocketPipeOut->setsockopt(ZMQ_SNDTIMEO, &nTimeoutPipe, sizeof(nTimeoutPipe));
            
            // warn if we use a "*" or empty host here
            bool bAmbiguousHost = (cURLPipeOut.scheme() == "tcp") && ((cURLPipeOut.host().isEmpty()) || (cURLPipeOut.host() == "*") || (cURLPipeOut.host() == "0.0.0.0"));
            if (bAmbiguousHost) qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "warning: pipe-out URL '" << sURLPipeOut << "' contains ambiguous host address - this may fail!";
            
            // connect!
            cSocketPipeOut->connect(sURLPipeOut.c_str());
            
            // debug
            if (qkd::utility::debug::enabled()) qkd::utility::debug() << "output pipe stream set to '" << sURLPipeOut << "'";
            
        }
        catch (zmq::error_t & cZMQError) {
            
            // fail
            qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to setup output with url: " << sURLPipeOut << " error: " << cZMQError.what(); 
            
            if (cSocketPipeOut) delete cSocketPipeOut;
            cSocketPipeOut = nullptr;
            
            return false;
        }

        return true;
    }
    
    // we do not know how to handle this type of URL
    qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "output pipe url scheme not recognized: " << cURLPipeOut.scheme().toStdString();
    
    return false;
}


/**
 * wait for state change
 * 
 * this method waits for any state change caused by another
 * thread but the working one
 * 
 * This method returns if we have a new state but eWorkingState
 * 
 * @param   eWorkingState       current working state
 * @return  the new module state
 */
module_state module::module_internal::wait_for_state_change(module_state eWorkingState) const {
    std::unique_lock<std::mutex> cLock(cStateMutex);
    while (eWorkingState == eState) cStateCondition.wait(cLock);
    return eState;
}



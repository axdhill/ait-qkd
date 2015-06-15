/* 
 * module.cpp
 * 
 * QKD module implementation
 *
 * Autor: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
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


// ------------------------------------------------------------
// incs

#include "config.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <fstream>
#include <iostream>
#include <map>
#include <queue>
#include <sstream>
#include <thread>

#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <boost/program_options/detail/config_file.hpp>

#include <signal.h>

// time system headers
#ifdef HAVE_SYS_TIME_H
#   include <sys/time.h>
#endif

#ifdef HAVE_SYS_TIMES_H
#   include <sys/times.h>
#endif

#ifdef HAVE_TIME_H
#   include <time.h>
#endif

// 0MQ
#ifdef HAVE_ZMQ_H
#   include <zmq.h>
#   ifdef __cplusplus
#       if (ZMQ_VERSION_MAJOR == 3)
#           include "lib/utility/zmq.hpp"
#       else
#           include <zmq.hpp>
#       endif
#   endif
#endif

// Qt
#include <QtCore/QUrl>
#include <QtDBus/QDBusConnection>

// ait
#include <qkd/crypto/context.h>
#include <qkd/crypto/engine.h>
#include <qkd/module/module.h>
#include <qkd/utility/dbus.h>
#include <qkd/utility/debug.h>
#include <qkd/utility/syslog.h>

// DBus integration
#include "module_dbus.h"

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


/**
 * the module pimpl
 */
class qkd::module::module::module_data {
    
public:

    
    /**
     * ctor
     */
    module_data(std::string sId) : sId(sId), nStartTimeStamp(0) { 
        
        // default values
        
        eRole = module_role::ROLE_ALICE;
        nTimeoutNetwork = 2500;
        nTimeoutPipe = 2500;
        eType = module_type::TYPE_OTHER;
        
        cRandom = qkd::utility::random_source::source();
        
        // indicate to setup the connections
        bSetupListen = true;
        bSetupPeer = true;
        bSetupPipeIn = true;
        bSetupPipeOut = true;
        
        bPipeInStdin = true;
        bPipeInVoid = false;
        bPipeOutStdout = true;
        bPipeOutVoid = false;
        
        sURLPipeIn = "stdin://";
        sURLPipeOut = "stdout://";
        
        cSocketListener = nullptr;
        cSocketPeer = nullptr;
        cSocketPipeIn = nullptr;
        cSocketPipeOut = nullptr;
        
        bSynchronizeKeys = true;
        nSynchronizeTTL = 10;
        
        cLastProcessedKey = std::chrono::system_clock::now() - std::chrono::hours(1);
        
        cModuleBirth = std::chrono::high_resolution_clock::now();
        
        bProcessing = false;

        bDebugMessageFlow = false;

        cStash.nLastInSyncKeyPicked = 0;
    };
    
    
    /**
     * dtor
     */
    ~module_data() {
        
        // clean up
        if (cSocketListener != nullptr) delete cSocketListener;
        cSocketListener = nullptr;
        if (cSocketPeer != nullptr) delete cSocketPeer;
        cSocketPeer = nullptr;
        if (cSocketPipeIn != nullptr) delete cSocketPipeIn;
        cSocketPipeIn = nullptr;
        if (cSocketPipeOut != nullptr) delete cSocketPipeOut;
        cSocketPipeOut = nullptr;
    };
    
    module_stat cStat;                          /**< the module statistic */
    
    std::string sId;                            /**< the id of the module */
    std::string sDescription;                   /**< the description of the module */
    std::string sOrganisation;                  /**< the organisation/creator of the module */
    std::string sPipeline;                      /**< the pipeline id this module is assigned */
    std::string sHint;                          /**< the module's hint */
    qkd::utility::random cRandom;               /**< random number generaror */
    std::string sRandomUrl;                     /**< random number source URL */
    module_role eRole;                          /**< role of the module */
    unsigned long nStartTimeStamp;              /**< init UNIX epoch: time of birth */
    int nTimeoutNetwork;                        /**< timeout in milliseconds for network send/recv timeout */
    int nTimeoutPipe;                           /**< timeout in milliseconds to wait after a failed read */
    module_type eType;                          /**< the type of the module */

    std::atomic<uint64_t> nTerminateAfter;      /**< termination counter */
    
    std::string sDBusObjectPath;                /**< the DBus object path */

    std::mutex cURLMutex;                       /**< sync change on URLs */
        
    std::string sURLListen;                     /**< listen URL for peer serving */
    std::string sURLPeer;                       /**< peer URL for connection  */
    std::string sURLPipeIn;                     /**< URL for pipe in serving */
    std::string sURLPipeOut;                    /**< URL for pipe out */
    
    std::atomic<bool> bSetupListen;             /**< setup a new listen address flag */
    std::atomic<bool> bSetupPeer;               /**< setup a new peer connect flag */
    std::atomic<bool> bSetupPipeIn;             /**< setup a new pipe in flag */
    std::atomic<bool> bSetupPipeOut;            /**< setup a new pipe out flag */
    
    bool bPipeInStdin;                          /**< pipe in is stdin:// flag */
    bool bPipeInVoid;                           /**< pipe in is void flag */
    bool bPipeOutStdout;                        /**< pipe out is stdout:// flag */
    bool bPipeOutVoid;                          /**< pipe out is void flag */
    
    zmq::socket_t * cSocketListener;            /**< listener socket */
    zmq::socket_t * cSocketPeer;                /**< connection to peer */
    zmq::socket_t * cSocketPipeIn;              /**< incoming 0MQ socket of the pipe */
    zmq::socket_t * cSocketPipeOut;             /**< outgoing 0MQ socket of the pipe */
    
    std::chrono::high_resolution_clock::time_point cModuleBirth;        /**< timestamp of module birth */
    
    std::thread cModuleThread;                  /**< the real module worker */
    
    std::atomic<bool> bProcessing;              /**< processing flag */

    std::atomic<bool> bDebugMessageFlow;        /**< debug message flow for send and recv packages */


    /**
     * message queues for different messages of different type
     */
    std::map<qkd::module::message_type, std::queue<qkd::module::message>> cMessageQueues;
    
    
    /**
     * this is holds the information for a single stashed key
     */
    typedef struct {
        
        qkd::key::key cKey;                                 /**< the key which is currently not present within the peer module */
        std::chrono::system_clock::time_point cStashed;     /**< time point of stashing */
        bool bValid;                                        /**< valid during current round */
        
        /**
         * age of the stashed key in seconds
         */
        inline uint64_t age() const { 
            return (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - cStashed)).count(); 
        };
        
    } stashed_key;
    
    
    /**
     * our stash of keys been sync ... or about to get in sync
     */
    struct {
    
        std::map<qkd::key::key_id, stashed_key> cInSync;        /**< keys we now are present on the peer side: ready to process */
        std::map<qkd::key::key_id, stashed_key> cOutOfSync;     /**< keys we did receive from a previous module but are not present on the remote module */

        qkd::key::key_id nLastInSyncKeyPicked;                  /**< the last key picked for in sync */

        /**
         * return next in sync key
         * 
         * @return  iterator to next in sync key
         */
        std::map<qkd::key::key_id, stashed_key>::iterator next_in_sync() {
            if (cInSync.size() == 0) return cInSync.end();
            if (cInSync.size() == 1) return cInSync.begin();
            auto iter = cInSync.lower_bound(nLastInSyncKeyPicked);
            if (iter == cInSync.end()) return cInSync.begin();
            return iter;
        }
        
    } cStash;
    
    
    std::atomic<bool> bSynchronizeKeys;         /**< synchronize key ids flag */
    std::atomic<uint64_t> nSynchronizeTTL;      /**< TTL for new not in-sync keys */
    
    std::chrono::system_clock::time_point cLastProcessedKey;    /**< timestamp of last processed key */
    
    
    /**
     * create an IPC incoming path
     */
    boost::filesystem::path create_ipc_in() const;
    
    
    /**
     * create an IPC outgoing path
     */
    boost::filesystem::path create_ipc_out() const;
    
    
    /**
     * connect to remote instance
     * 
     * @param   sPeerURL        the remote instance URL
     */
    void connect(std::string sPeerURL);
    
    
    /**
     * dump a message to stderr
     *
     * @param   bSent       message has been sent
     * @param   cMessage    message itself
     */
    void debug_message(bool bSent, qkd::module::message const & cMessage);


    /**
     * deduce a correct, proper URL from a would-be URL
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
     * get the current module state
     * 
     * @return  the current module state
     */
    module_state get_state() const;
    
    
    /**
     * cleans any resources left
     */
    void release();
    
    
    /**
     * set a new module state
     * 
     * the working thread will be notified (if waiting)
     * 
     * @param   eNewState       the new module state
     */
    void set_state(module_state eNewState);
    
    
    /**
     * runs all the setup code for the module worker thread
     * 
     * @return  true, if all is laid out properly
     */
    bool setup();

    
    /**
     * setup listen
     * 
     * @return  true, for success
     */
    bool setup_listen();
    

    /**
     * setup peer connection
     * 
     * @return  true, for success
     */
    bool setup_peer();
    

    /**
     * setup pipe IN
     * 
     * @return  true, for success
     */
    bool setup_pipe_in();
    

    /**
     * setup pipe OUT
     * 
     * @return  true, for success
     */
    bool setup_pipe_out();
    
    
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
    module_state wait_for_state_change(module_state eWorkingState) const;


private:
    
    module_state eState;                                /**< the state of the module */
    mutable std::mutex cStateMutex;                     /**< state modification mutex */
    mutable std::condition_variable cStateCondition;    /**< state modification condition */
    
};


// ------------------------------------------------------------
// vars


/**
 * create a static instance of the initilizer
 */
static module_init g_cInit;


// ------------------------------------------------------------
// fwd.

void memory_delete(void * cData, void * cHint);


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
 * connect to remote instance
 * 
 * @param   sPeerURL        the remote instance URL
 */
void module::module_data::connect(std::string sPeerURL) {
    this->sURLPeer = sPeerURL;
}


/**
 * dump a message to stderr
 *
 * @param   bSent       message has been sent
 * @param   cMessage    message itself
 */
void module::module_data::debug_message(bool bSent, qkd::module::message const & cMessage) {

    if (!bDebugMessageFlow) return;
    if (bSent) {
        qkd::utility::debug() << "<MOD-SENT>" << cMessage.string();
    }
    else {
        qkd::utility::debug() << "<MOD-RECV>" << cMessage.string();
    }
 }


/**
 * create an IPC incoming path
 */
boost::filesystem::path module::module_data::create_ipc_in() const {
    
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
boost::filesystem::path module::module_data::create_ipc_out() const {
    
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
 * deduce a correct, proper URL from a would-be URL
 * 
 * @param   sURL        an url
 * @return  a good, real, usable url (or empty() in case of failure)
 */
std::string module::module_data::fix_url(std::string const & sURL) {

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
std::string module::module_data::fix_url_ipc(std::string const & sURL) {

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
std::string module::module_data::fix_url_tcp(std::string const & sURL) {

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
module_state module::module_data::get_state() const {
    std::unique_lock<std::mutex> cLock(cStateMutex);
    return eState;
}


/**
 * clean any resources left
 */
void module::module_data::release() {
    
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
void module::module_data::set_state(module_state eNewState) {
    
    std::unique_lock<std::mutex> cLock(cStateMutex);
    eState = eNewState;
    cStateCondition.notify_all();
}


/**
 * runs all the setup code for the module worker thread
 * 
 * @return  true, if all is laid out properly
 */
bool module::module_data::setup() {
    
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
bool module::module_data::setup_listen() {

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
bool module::module_data::setup_peer() {

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
bool module::module_data::setup_pipe_in() {
    
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
bool module::module_data::setup_pipe_out() {

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
module_state module::module_data::wait_for_state_change(module_state eWorkingState) const {
    std::unique_lock<std::mutex> cLock(cStateMutex);
    while (eWorkingState == eState) cStateCondition.wait(cLock);
    return eState;
}


/**
 * ctor
 * 
 * @param   sId             identification of the module
 * @param   eType           type of the module
 * @param   sDescription    description of the module
 * @param   sOrganisation   organisation (vendor) of module
 */
module::module(std::string sId, module_type eType, std::string sDescription, std::string sOrganisation) : QObject() {
    
    d = boost::shared_ptr<qkd::module::module::module_data>(new qkd::module::module::module_data(sId));
    
    // remember our birthday
    struct timeval cTV;
    struct timezone cTZ;
    gettimeofday(&cTV, &cTZ);
    d->nStartTimeStamp = cTV.tv_sec;
    
    // basic init values
    d->eType = eType;
    d->sDescription = sDescription;
    d->sOrganisation = sOrganisation;
    d->set_state(module_state::STATE_NEW);
    
    set_pipeline("default");
    set_synchronize_keys(true);
    set_synchronize_ttl(10);
    set_url_pipe_in("stdin://");
    set_url_pipe_out("stdout://");
    
    // init DBus proxy
    new ModuleAdaptor(this);
    
    // register ourself on the DBus
    QTimer::singleShot(0, this, SLOT(init()));
}


/**
 * dtor
 */
module::~module() {}


/**
 * accept a key for processing
 * 
 * each time a key is ought to be processed by a module, this
 * method is called. if this method returns false the key is
 * discarded
 * 
 * The default implementation discards DISCLOSED keys.
 * 
 * @param   cKey            the key to check
 * @return  true, if the key should be processed by this module
 */
bool module::accept(qkd::key::key const & cKey) const {
    
    // check for acceptance
    if (cKey.meta().eKeyState == qkd::key::key_state::KEY_STATE_DISCLOSED) {
        qkd::utility::syslog::info() << "key #" << cKey.id() << " has state: DISCLOSED. processing canceled";
        return false;
    }
    
    return true;
}


/**
 * apply the loaded key value map to the module
 * 
 * @param   sURL            URL of config file loaded
 * @param   cConfig         map of key --> value
 */
void module::apply_config(UNUSED std::string const & sURL, UNUSED qkd::utility::properties const & cConfig) {
    
    // the default module does not use any
    // configuration key->value pairs
    // overwrite this method in derived classed
    // to use configuration files correctly
}


/**
 * apply known config key
 * 
 * standard module config keys are:
 * 
 *      module.ID.alice.url_peer
 *      module.ID.alice.url_pipe_in
 *      module.ID.alice.url_pipe_out
 *      module.ID.bob.url_listen
 *      module.ID.bob.url_pipe_in
 *      module.ID.bob.url_pipe_out
 *      module.ID.pipeline
 *      module.ID.random_url
 *      module.ID.synchronize_keys
 *      module.ID.synchronize_ttl
 *      module.ID.timeout_network
 *      module.ID.timeout_pipe
 * 
 * where ID is the module id as been resulted by the id() call.
 * 
 * E.g.
 * 
 *          module.bb84.alice.url_pipe_out = ipc:///tmp/bb84.out
 * 
 *      specifies that the BB84 qkd module should use the
 *      IPC (UNIX domain socket) address of /tmp/bb84.out as its
 *      key output sink within the pipeline.
 * 
 * Hence, the role MUST be set in advance. To let this method
 * decide weather to apply alice's or bob's keys.
 * 
 * @param   sKey            the key
 * @param   sValue          the value
 * @return  true, if the value is one of the standard and has been parsed and applied
 */
bool module::apply_standard_config(std::string const & sKey, std::string const & sValue) {
    
    if (!is_standard_config_key(sKey)) return false;
    
    std::string sSubKey = sKey.substr(config_prefix().size());

    if (sSubKey == "alice.url_peer") {
        if (is_alice()) {
            set_url_peer(QString::fromStdString(sValue));
            return true;
        }
    }
    else
    if (sSubKey == "alice.url_pipe_in") {
        if (is_alice()) {
            set_url_pipe_in(QString::fromStdString(sValue));
            return true;
        }
    }
    else
    if (sSubKey == "alice.url_pipe_out") {
        if (is_alice()) {
            set_url_pipe_out(QString::fromStdString(sValue));
            return true;
        }
    }
    else 
    if (sSubKey == "bob.url_listen") {
        if (is_bob()) {
            set_url_listen(QString::fromStdString(sValue));
            return true;
        }
    }
    else 
    if (sSubKey == "bob.url_pipe_in") {
        if (is_bob()) {
            set_url_pipe_in(QString::fromStdString(sValue));
            return true;
        }
    }
    else 
    if (sSubKey == "bob.url_pipe_out") {
        if (is_bob()) {
            set_url_pipe_out(QString::fromStdString(sValue));
            return true;
        }
    }
    else
    if (sSubKey == "pipeline") {
        set_pipeline(QString::fromStdString(sValue));
        return true;
    }
    else
    if (sSubKey == "random_url") {
        set_random_url(QString::fromStdString(sValue));
        return true;
    }
    else
    if (sSubKey == "synchronize_keys") {
        set_synchronize_keys(!((sValue == "0") || (sValue == "no") || (sValue == "off") || (sValue == "false")));
        return true;
    }
    else
    if (sSubKey == "synchronize_ttl") {
        set_synchronize_ttl(std::stoll(sValue));
        return true;
    }
    else
    if (sSubKey == "timeout_network") {
        set_timeout_network(std::stoll(sValue));
        return true;
    }
    else
    if (sSubKey == "timeout_pipe") {
        set_timeout_pipe(std::stoll(sValue));
        return true;
    }
    
    // here it is a known key but not applicable.
    return true;
}


/**
 * most exact date of module birth
 * 
 * @return  timepoint of birth as exact as possible
 */
std::chrono::high_resolution_clock::time_point module::birth() const {
    return d->cModuleBirth;
}


/**
 * configures the module
 * 
 * The given URL has to point to a so-called INI-file.
 * 
 * e.g.     configure("file:///etc/qkd/qkd.conf")
 * 
 * The given file URL is parsed and the found configuration
 * is handed out to apply_config()
 *
 * Oppossed to the D-Bus method slot of the same name
 * this method calls exit(1) if the configuration URL
 * failed to load.
 * 
 * @param   sConfigURL      configuration file to load
 * @param   bRequired       if true, on failure calls exit(1)
 */
bool module::configure(QString sConfigURL, bool bRequired) {

    // get the configuration into memory
    QUrl cConfigURL(sConfigURL);
    
    // if we don't have a scheme assume a file, absolute or relative
    if (cConfigURL.scheme().isEmpty()) {
        
        // find file
        boost::filesystem::path cPath = qkd::utility::environment::find_path(sConfigURL.toStdString());
        if (cPath.empty()) {

            std::stringstream ss;
            ss << "failed to load module configuration from: '" << sConfigURL.toStdString() << "': unknown scheme '" << cConfigURL.scheme().toStdString() << "' or file not found";
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << ss.str();

            // this is a failure
            if (bRequired) {
                qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": as configuration is required, this is futile --> aborted"; 
                std::exit(1);
            }
            return false;
        }
        
        // found: create URL
        cConfigURL = QUrl(QString("file://") + QString::fromStdString(cPath.string()));
    }

    // "file://"
    if (cConfigURL.scheme() != "file") {
        std::stringstream ss;
        ss << "failed to load module configuration from: '" << sConfigURL.toStdString() << "': unknown scheme '" << cConfigURL.scheme().toStdString() << "'";
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << ss.str();

        // this is a failure
        if (bRequired) {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": as configuration is required, this is futile --> aborted"; 
            std::exit(1);
        }
        return false;
    }
    
    // debug
    qkd::utility::debug() << "loading configuration from: " << cConfigURL.toString().toStdString();
    
    // open file
    std::string sFile = cConfigURL.toLocalFile().toStdString();
    std::ifstream cConfigFile(sFile);
    if (!cConfigFile.is_open()) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to open configuration '" << sFile << "'";

        // this is a failure
        if (bRequired) {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": as configuration is required, this is futile --> aborted"; 
            std::exit(1);
        }
        return false;
    }
    
    // read in the options
    std::set<std::string> cOptions;
    cOptions.insert("*");
    
    // this will take all options found
    qkd::utility::properties cConfig;
    
    try {
        
        // walk over all config details
        boost::program_options::detail::config_file_iterator cConfigIter(cConfigFile, cOptions);
        boost::program_options::detail::config_file_iterator cEOF;
        
        // collect all found options
        for (; cConfigIter != cEOF; cConfigIter++) {
            
            // get the next option
            boost::program_options::option cOption = *cConfigIter;
            cConfig[cOption.string_key] = cOption.value[0];
        }
        
        // apply standard config
        std::string sConfigPrefix = config_prefix();
        for (auto const & cEntry : cConfig) {
            
            // grab any key which is intended for us
            if (cEntry.first.substr(0, sConfigPrefix.size()) != sConfigPrefix) continue;
            
            // check for any standard config option
            apply_standard_config(cEntry.first, cEntry.second);
        }        
        
        // apply found config
        apply_config(sConfigURL.toStdString(), cConfig);
        
    }
    catch (boost::program_options::invalid_syntax const & cErrInvalidSyntax) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to parse config file: " << sFile << " invalid syntax at: '" << cErrInvalidSyntax.tokens() << "'";
    }
    catch (std::exception const & cException) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to parse config file: " << sFile << " exception: " << cException.what();
    }

    return true;
}


/**
 * configures the module
 * 
 * The given URL has to point to a so-called INI-file.
 * 
 * e.g.     configure("file:///etc/qkd/qkd.conf")
 * 
 * The given file URL is parsed and the found configuration
 * is handed out to apply_config()
 *
 * This is a call to this->configuration(sConfigURL, false);
 * 
 * @param   sConfigURL      configuration file to load
 */
void module::configure(QString sConfigURL) {
    configure(sConfigURL, false);
}


/**
 * check if message flow particles are printed on stderr
 * 
 * @return  true, if debug messages of communication are pasted on stderr
 */
bool module::debug_message_flow() const {
    return d->bDebugMessageFlow;
}


/**
 * this is the start method call
 * 
 * (reason it is not public: it would be visible 
 * at DBus, which is currently not an option)
 */
void module::delayed_start() {
    
    // ensure all things have been done before
    QCoreApplication::processEvents(QEventLoop::AllEvents);
    
    // ensure the module has been initialized properly
    init();

    // start the module thread
    run();
    
    // wait until the thread signals ready (by setting the state to paused)
    qkd::module::module_state eState = qkd::module::module_state::STATE_NEW;
    while (eState == qkd::module::module_state::STATE_NEW) eState = wait_for_state_change(eState);
    
    // set to running if paused
    if (eState == qkd::module::module_state::STATE_READY) resume();
}


/**
 * return the description of the module
 * 
 * @return  the description of the module
 */
QString module::description() const {
    return QString::fromStdString(d->sDescription);
}


/**
 * get the current state
 * 
 * retrieves the current state
 * this is exactly like state() but with the correct
 * type for convenience
 * 
 * @return  the new module state
 */
module_state module::get_state() const {
    return d->get_state();
}


/**
 * return the module's hint
 * 
 * @return  the hint to this module instance
 */
QString module::hint() const {
    return QString::fromStdString(d->sHint);
}


/**
 * return the id of the module
 * 
 * @return  the id of the module
 */
QString module::id() const {
    return QString::fromStdString(d->sId);
}


/**
 * initialize the module
 */
void module::init() {
    
    // do not init twice
    static bool bInitialized = false;
    if (bInitialized) return;
    bInitialized = true;
    
    // run initialization code
    register_dbus();
}


/**
 * this methods interrupts the worker thread
 * 
 * this is usefull if you find the worker thread been
 * blocked by some I/O operation (e.g. send/recv) and
 * want to abort that action or need other steps to be
 * undertaken.
 * 
 * if the worker thread is blocked by a send/recv then
 * the method returns with no data read or sent and with
 * the return value of false.
 */
void module::interrupt_worker() {
    
    // do we have a thread at all? 
    if (d->cModuleThread.get_id() == std::thread::id()) return;
        
    // interrupt worker thread
    sigval cSignalValue = { 0 };
    pthread_sigqueue(d->cModuleThread.native_handle(), SIGINT, cSignalValue);
    pthread_yield();
    pthread_kill(d->cModuleThread.native_handle(), SIGCHLD);
}


/**
 * test if the given key denotes a standard config key
 * 
 * @param   sKey            a module key
 * @return  true, if the the given string does comply to a standard module config key
 */
bool module::is_standard_config_key(std::string const & sKey) const {
    
    if (!is_config_key(sKey)) return false;
    
    std::string sSubKey = sKey.substr(config_prefix().size());

    if (sSubKey == "alice.url_peer") return true;
    if (sSubKey == "alice.url_pipe_in") return true;
    if (sSubKey == "alice.url_pipe_out")  return true;
    if (sSubKey == "bob.url_listen") return true;
    if (sSubKey == "bob.url_pipe_in") return true;
    if (sSubKey == "bob.url_pipe_out") return true;
    if (sSubKey == "pipeline") return true;
    if (sSubKey == "random_url") return true;
    if (sSubKey == "synchronize_keys") return true;
    if (sSubKey == "synchronize_ttl") return true;
    if (sSubKey == "timeout_network") return true;
    if (sSubKey == "timeout_pipe") return true;
    
    return false;
}


/**
 * waits until the module's worker thread finished
 * 
 * The current thread waits until the module worker thread
 * has finished. 
 * 
 * Hence, when this is the main (GUI?) thread of an application, 
 * then all actions are blocked and frozen. This means also
 * that no DBus action is available.
 * 
 * So do this only on program exit.
 */
void module::join() const {
    
    // don't join a not-started thread
    if (d->cModuleThread.get_id() == std::thread::id()) {
        if (qkd::utility::debug::enabled()) qkd::utility::debug() << "module thread not running";
        return;
    }
    
    // don't join ourselves
    if (d->cModuleThread.get_id() == std::this_thread::get_id()) {
        if (qkd::utility::debug::enabled()) qkd::utility::debug() << "module thread won't join itself";
        return;
    }
    
    // wait until the module thread finished
    d->cModuleThread.join();
}


/**
 * return the organisation/creator of the module
 * 
 * @return  the organisation/creator of the module
 */
QString module::organisation() const {
    return QString::fromStdString(d->sOrganisation);
}


/**
 * pauses current processing
 */
void module::pause() {
    if (d->get_state() != module_state::STATE_RUNNING) return;
    d->set_state(module_state::STATE_READY);
    emit paused();
}


/**
 * get the pipeline id this module is assigned
 *
 * @return  the pipeline id this module is assigned
 */
QString module::pipeline() const {
    return QString::fromStdString(d->sPipeline);
}


/**
 * check if the module is currently processing a key    
 * 
 * @return  true, if we currently processing a key
 */
bool module::processing() const {
    return d->bProcessing;
}


/**
 * get the internally used random number source
 * 
 * @return  the random number source to be used by the module
 */
qkd::utility::random & module::random() {
    return d->cRandom;
}


/**
 * get the url of the random value source
 *
 * @return  the url of the random value source
 */
QString module::random_url() const {
    return QString::fromStdString(d->sRandomUrl);
}


/**
 * get the next key from the previous module
 * 
 * This method is called inside of work(). Call this 
 * method inside of process() if you know _exactly_ what
 * you are doing.
 * 
 * You should not need to call this directly.
 * 
 * @param   cKey        this will receive the next key
 * @return  true, if reading was successful
 */
bool module::read(qkd::key::key & cKey) {
    
    // reset
    cKey = qkd::key::key::null();
    
    // setup pipin if necessary
    if (d->bSetupPipeIn) d->setup_pipe_in();
    
    // check if we do have an input at all
    if (d->bPipeInVoid) return true;
    
    // read from stdin:// ?
    if (d->bPipeInStdin) {
        std::cin >> cKey;
    }
    else if (d->cSocketPipeIn) {
        
        // read from ZMQ
        zmq::message_t cZMQMessage;
        try {
            if (d->cSocketPipeIn->recv(&cZMQMessage)) {
                qkd::utility::buffer cData = qkd::utility::buffer(qkd::utility::memory::wrap((unsigned char *)cZMQMessage.data(), cZMQMessage.size()));
                cData >> cKey;
            }
        }
        catch (UNUSED zmq::error_t & cZMQError) {}
    }
    
    // check for successful read
    if (cKey == qkd::key::key::null()) {
        
        // failed to read ... sleep some timeslice and yield execution to another
        rest();
        return false;
    }
    
    // collect key data
    std::lock_guard<std::recursive_mutex> cLock(d->cStat.cMutex);
    d->cStat.nKeysIncoming++;
    d->cStat.nKeyBitsIncoming += cKey.size() * 8;
    d->cStat.nDisclosedBitsIncoming += cKey.meta().nDisclosedBits;
    d->cStat.cKeysIncomingRate << d->cStat.nKeysIncoming;
    d->cStat.cKeyBitsIncomingRate << d->cStat.nKeyBitsIncoming;
    d->cStat.cDisclosedBitsIncomingRate << d->cStat.nDisclosedBitsIncoming;

    // correct timestamp (if needed)
    cKey.meta().cTimestampRead = std::chrono::high_resolution_clock::now();

    // state semething if debug is on
    if (qkd::utility::debug::enabled()) {
        
        // pretty printing for debug
        // if not needed, then performance is wasted here
        
        boost::format cLineFormater = boost::format("key-PULL [%015ums] id: %010u bits: %010u err: %6.4f dis: %010u crc: %08x state: %-13s");
        
        auto cTimePoint = std::chrono::duration_cast<std::chrono::milliseconds>(age());
        cLineFormater % cTimePoint.count();
        cLineFormater % cKey.id();
        cLineFormater % (cKey.size() * 8);
        cLineFormater % cKey.meta().nErrorRate;
        cLineFormater % cKey.meta().nDisclosedBits;
        
        // CRC32 checksum
        cLineFormater % cKey.data().crc32();
        
        // key state
        cLineFormater % cKey.state_string();
        
        qkd::utility::debug() << cLineFormater.str();
    }

    return true;
}


/**
 * read a message from the peer module
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
 * Internally the recv_internal method is called and the actual receive
 * is performed. 
 * 
 * @param   cMessage            this will receive the message
 * @param   cAuthContext        the authentication context involved
 * @param   eType               message type to receive
 * @param   nTimeOut            timeout in ms
 * @return  true, if we have receuived a message
 */
bool module::recv(qkd::module::message & cMessage, qkd::crypto::crypto_context & cAuthContext, qkd::module::message_type eType, int nTimeOut) throw (std::runtime_error) {

    // stopwatch: start...
    bool bReceived = false;
    auto cStartOfRecv = std::chrono::high_resolution_clock::now();

    // ensure there is at least an empty message queue for this message type
    if (d->cMessageQueues.find(eType) == d->cMessageQueues.end()) d->cMessageQueues[eType] = std::queue<qkd::module::message>();

    // pick first item if in message queue already
    if (!d->cMessageQueues[eType].empty()) {
        cMessage = d->cMessageQueues[eType].front();
        d->cMessageQueues[eType].pop();
        qkd::utility::debug() << "message for type " << static_cast<uint32_t>(eType) << " already in message queue - popped from queue.";
    }
    else {

        // receive message and push them into queue
        // until a) correct type received or b) timeout

        do {

            // call real receive
            bReceived = recv_internal(cMessage, nTimeOut);
            if (!bReceived) return false;

            // check message type
            if (cMessage.type() != eType) {

                // not correct type: push into queue for later dispatch
                if (d->cMessageQueues.find(cMessage.type()) == d->cMessageQueues.end()) {
                    d->cMessageQueues[cMessage.type()] = std::queue<qkd::module::message>();
                }
                d->cMessageQueues[cMessage.type()].push(cMessage);
                qkd::utility::debug() << "received a QKD message for message type " << static_cast<uint32_t>(cMessage.type()) << " when expecting " << static_cast<uint32_t>(eType) << " - pushed into queue for later dispatch.";
                bReceived = false;

                // check for exceeded timeout value
                if (nTimeOut >= 0) {

                    auto cNow = std::chrono::high_resolution_clock::now();
                    if (std::chrono::duration_cast<std::chrono::milliseconds>(cNow - cStartOfRecv).count() > nTimeOut) {

                        // timeout over: failed to get proper message from peer! =(
                        // clear message (remove memory artifacts) and exit
                        memset(&(cMessage.m_cHeader), 0, sizeof(cMessage.m_cHeader));
                        cMessage.data() = qkd::utility::memory(0);
                        return false;
                    }
                }
            }

        } while (!bReceived); 
    }
        
    // add message content to crypto context
    cAuthContext << cMessage.data();
    cMessage.data().set_position(0);

    return bReceived;
}


/**
 * read a message from the peer module internal private version
 *
 * this is called by the protected recv method and stuffs the received
 * messages into queues depending on their message type.
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
 * @return  true, if we have receuived a message
 */
bool module::recv_internal(qkd::module::message & cMessage, int nTimeOut) throw (std::runtime_error) {
    
    bool bIsAlice = is_alice();
    bool bIsBob = is_bob();

    // setup peer connection if necessary
    if (bIsAlice && d->bSetupPeer) d->setup_peer();
    if (bIsBob && d->bSetupListen) d->setup_listen();
        
    // check if we do have a connection
    if (bIsAlice && (d->cSocketPeer == nullptr)) throw std::runtime_error("no connection to peer");
    if (bIsBob && (d->cSocketListener == nullptr)) throw std::runtime_error("not accepting connection");
    
    // read from ZMQ
    bool bRecv = false;
    try {
        
        zmq::socket_t * cSocket = nullptr;
        if (bIsAlice) cSocket = d->cSocketPeer;
        if (bIsBob) cSocket = d->cSocketListener;
        
        // do we have a channel?
        if (!cSocket) {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to decide which channel to use for receive";
            throw std::runtime_error("failed to decide which channel to use for recv");
        }
        
        // adjust timeout
        cSocket->setsockopt(ZMQ_RCVTIMEO, &nTimeOut, sizeof(nTimeOut));
        
        // revceive: header    
        zmq::message_t cZMQHeader;
        bRecv = (cSocket->recv(&cZMQHeader));
        if (bRecv) {
            
#if (ZMQ_VERSION_MAJOR == 3)
            int bMoreData = 0;
#else
            int64_t bMoreData = 0;
#endif            
            size_t nSizeOfOption = sizeof(bMoreData);
            cSocket->getsockopt(ZMQ_RCVMORE, &bMoreData, &nSizeOfOption);
            
            // check
            if ((bMoreData == 0) || (cZMQHeader.size() != sizeof(cMessage.m_cHeader))) {
                throw std::runtime_error("received invalid message header");
            }
            
            // get header data
            memcpy(&(cMessage.m_cHeader), cZMQHeader.data(), sizeof(cMessage.m_cHeader));
            
            // receive: data
            zmq::message_t cZMQData;
            bRecv = (cSocket->recv(&cZMQData));
            if (bRecv) {
                cMessage.data().resize(cZMQData.size());
                memcpy(cMessage.data().get(), cZMQData.data(), cZMQData.size());
                cMessage.data().set_position(0);
            }
        }
    }
    catch (zmq::error_t & cZMQError) {
        throw std::runtime_error(cZMQError.what());
    }
    
    // something wrong?
    if (is_dying_state() || !bRecv) return false;
    
    // record action
    cMessage.m_cTimeStamp = std::chrono::high_resolution_clock::now();
    
    d->debug_message(false, cMessage);
  
    return true;
}


/**
 * process a received synchronize message
 * 
 * @param   cMessage            the message received
 */
void module::recv_synchronize(qkd::module::message & cMessage) throw (std::runtime_error) {
    
    if (cMessage.type() != qkd::module::message_type::MESSAGE_TYPE_KEY_SYNC) {
        throw std::runtime_error("accidently tried to sync keys based on a non-sync message");
    }

    for (auto & cStashedKey : d->cStash.cInSync) {
        cStashedKey.second.bValid = false;
    }

    // the sync message consist of two lists: in-sync and out-of-sync key ids
    // however, we perform the very same action on both of them
    for (int i = 0; i < 2; ++i) {

        uint64_t nPeerInSyncKeys;
        cMessage.data() >> nPeerInSyncKeys;
        for (uint64_t i = 0; i < nPeerInSyncKeys; i++) {
            
            qkd::key::key_id nPeerKeyId;
            cMessage.data() >> nPeerKeyId;

            auto cStashIter = d->cStash.cInSync.find(nPeerKeyId);
            if (cStashIter != d->cStash.cInSync.end()) {
                (*cStashIter).second.bValid = true;
            }
            cStashIter = d->cStash.cOutOfSync.find(nPeerKeyId);
            if (cStashIter != d->cStash.cOutOfSync.end()) {
                auto p = d->cStash.cInSync.emplace((*cStashIter).first, (*cStashIter).second);
                if (!p.second) {
                    throw std::runtime_error("failed to move out-of-sync key to in-sync key stash");
                }
                (*p.first).second.bValid = true;
                d->cStash.cOutOfSync.erase(cStashIter);
            }
        }
    }
   
    std::list<qkd::key::key_id> cToDelete;
    for (auto & cStashedKey : d->cStash.cInSync) {
        if (!cStashedKey.second.bValid) {
            cToDelete.push_back(cStashedKey.second.cKey.id());
        }
    }
    for (auto nKeyId: cToDelete) d->cStash.cInSync.erase(nKeyId);
    cToDelete.clear();
    for (auto & cStashedKey : d->cStash.cOutOfSync) {
        if (cStashedKey.second.age() > synchronize_ttl()) {
            cToDelete.push_back(cStashedKey.second.cKey.id());
        }
    }
    for (auto nKeyId: cToDelete) d->cStash.cOutOfSync.erase(nKeyId);

    if (d->cStash.cInSync.size() <= 1) d->cStash.nLastInSyncKeyPicked = 0;
    
    if (qkd::utility::debug::enabled()) {

        std::stringstream ssInSyncKeys;
        bool bInSyncKeyFirst = true;
        for (auto const & cStashedKey : d->cStash.cInSync) {
            if (!bInSyncKeyFirst) ssInSyncKeys << ", ";
            ssInSyncKeys << cStashedKey.second.cKey.id();
            bInSyncKeyFirst = false;
        }

        std::stringstream ssOutOfSyncKeys;
        bool bOutOfSyncKeyFirst = true;
        for (auto const & cStashedKey : d->cStash.cOutOfSync) {
            if (!bOutOfSyncKeyFirst) ssOutOfSyncKeys << ", ";
            ssOutOfSyncKeys << cStashedKey.second.cKey.id();
            bOutOfSyncKeyFirst = false;
        }

        qkd::utility::debug() <<
            "key-SYNC " <<
            "in-sync=<" << ssInSyncKeys.str() << "> " << 
            "out-sync=<" << ssOutOfSyncKeys.str() << ">";
    }
}


/**
 * register this object on the DBus
 * 
 * this method calls service_name() and registers
 * the service as /Module on the DBus
 */
void module::register_dbus() {
    
    // syslog
    qkd::utility::syslog::info() << "connecting to DBus:" << getenv("DBUS_SESSION_BUS_ADDRESS");
    
    // get DBus
    QDBusConnection cDBus = qkd::utility::dbus::qkd_dbus();
    
    // try to register on DBus
    QString sServiceName = service_name();
    if (!cDBus.registerService(sServiceName)) {
        QString sMessage = QString("failed to register DBus service \"") + sServiceName + "\""; 
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << sMessage.toStdString();
    }
    
    // syslog
    qkd::utility::syslog::info() << "connected to DBus:" << getenv("DBUS_SESSION_BUS_ADDRESS") << " as \"" << sServiceName.toStdString() << "\"";

    // register Object on DBus
    if (!cDBus.registerObject("/Module", this)) {
        QString sMessage = QString("failed to register DBus object /Module");
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << sMessage.toStdString();
    }
    else {
        qkd::utility::syslog::info() << "module registered on DBus as /Module";
    }
}


/**
 * rest timeout() milliseconds for a next communication try
 */
void module::rest() const {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}


/**
 * resumes processing (if paused)
 */
void module::resume() {
    if (d->get_state() != module_state::STATE_READY) return;
    d->set_state(module_state::STATE_RUNNING);
    emit resumed();
}


/**
 * return the role of the module
 * 
 * @return  the role as integer
 */
qulonglong module::role() const {
    return (qulonglong)d->eRole;
}


/**
 * return the role name description for a given role
 * 
 * @param   eRole           the role in question
 * @return  the human readable role name 
 */
QString module::role_name(module_role eRole) {
    
    QString res;
    
    switch (eRole) {
    case module_role::ROLE_ALICE: return "alice";
    case module_role::ROLE_BOB: return "bob";
    }
    
    return "unkown role";
}


/**
 * starts the module
 * 
 * This internally creates a new thread of execution which will finally
 * invoke the "work" function, which has to be overwritten with actual code.
 */
void module::run() {

    // don't start if we are running allready
    if (d->cModuleThread.get_id() != std::thread::id()) {
        if (qkd::utility::debug::enabled()) qkd::utility::debug() << "module thread already running";
        return;
    }
    
    // force state change
    d->set_state(module_state::STATE_NEW);
    
    qkd::utility::debug() << "run module: " << 
            "in='" << d->sURLPipeIn << "' " <<
            "out='" << d->sURLPipeOut << "' " << 
            "listen='" << d->sURLListen << "' " << 
            "peer='" << d->sURLPeer << "'";
    
    // launch work thread!
    d->cModuleThread = std::thread([this]{ thread(); });
}


/**
 * send a message to the peer module
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
 * @param   cMessage            the message to send
 * @param   cAuthContext        the authentication context involved
 * @param   nTimeOut            timeout in ms
 */
void module::send(qkd::module::message & cMessage, qkd::crypto::crypto_context & cAuthContext, int nTimeOut) throw (std::runtime_error) {
    
    bool bIsAlice = is_alice();
    bool bIsBob = is_bob();

    // setup peer connection if necessary
    if (bIsAlice && d->bSetupPeer) d->setup_peer();
    if (bIsBob && d->bSetupListen) d->setup_listen();
        
    // check if we do have a connection
    if (bIsAlice && (d->cSocketPeer == nullptr)) throw std::runtime_error("no connection to peer");
    if (bIsBob && (d->cSocketListener == nullptr)) throw std::runtime_error("not accepting connection");
    
    // try to send some data
    try {
        
        zmq::socket_t * cSocket = nullptr;
        if (bIsAlice) cSocket = d->cSocketPeer;
        if (bIsBob) cSocket = d->cSocketListener;
        
        // do we have a channel?
        if (!cSocket) {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to decide which channel to use for send";
            throw std::runtime_error("failed to decide which channel to use for send");
        }
        
        // adjust timeout
        cSocket->setsockopt(ZMQ_RCVTIMEO, &nTimeOut, sizeof(nTimeOut));
        
        // record action
        cMessage.m_cHeader.nId = htobe32(++qkd::module::message::m_nLastId);
        cMessage.m_cTimeStamp = std::chrono::high_resolution_clock::now();
        
        d->debug_message(true, cMessage);
        
        // send!
        zmq::message_t cZMQHeader(sizeof(cMessage.m_cHeader));
        memcpy(cZMQHeader.data(), &(cMessage.m_cHeader), sizeof(cMessage.m_cHeader));
        cSocket->send(cZMQHeader, ZMQ_SNDMORE);
        
        // make a shallow copy of the message memory
        // this should increas the reference count and thus
        // lets the memory delete later (within memory_delete)
        // this should avoid a memcpy as much as possible
        qkd::utility::memory * cData = new qkd::utility::memory(cMessage.data());
        zmq::message_t cZMQData(cData->get(), cData->size(), memory_delete, cData);
        cSocket->send(cZMQData);
    }
    catch (zmq::error_t & cZMQError) {
        throw std::runtime_error(cZMQError.what());
    }
    
    // add message content to crypto context
    cAuthContext << cMessage.data();
    
    // clear the message
    cMessage = qkd::module::message();    
}


/**
 * return the DBus service name
 * 
 * This returns "at.ac.ait.qkd.module." + id() + PID.
 * If you want to use a different service name, then 
 * overwrite this method in sub classes
 * 
 * @return  the DBus service name this module to register
 */
QString module::service_name() const {
    
    std::stringstream ss;
    ss << id().toStdString() << "-" << process_id();
    
    // check service name
    if (!qkd::utility::dbus::valid_service_name_particle(ss.str())) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "DBus service name 'at.ac.ait.qkd.module." << ss.str() << "' is not valid - impossible to register on DBus";
    }
    
    // try anyway to connect to DBus
    return QString("at.ac.ait.qkd.module.") + QString::fromStdString(ss.str());
}


/**
 * set the debug message particle flow flag
 * 
 * @param   bDebug      new debug value for message particles
 */
void module::set_debug_message_flow(bool bDebug) {
    d->bDebugMessageFlow = bDebug;
}


/**
 * set the module's hint
 * 
 * @param   sHint       the new hint to this module instance
 */
void module::set_hint(QString sHint) {
    d->sHint = sHint.toStdString();
}


/**
 * set the new pipeline id this module is assigned
 *
 * @param   sPipeline   the new pipeline id this module is assigned
 */
void module::set_pipeline(QString sPipeline) {
    
    // check module state
    if (is_working_state()) {
        
        // warn user: the module is already up and working
        // changing the pipeline should have been done earlier
        // this may cause problems ...
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "warning: setting pipeline in working state.";
    }
    
    d->sPipeline = sPipeline.toStdString();
}


/**
 * set the url of the random value source
 *
 * @param   sRandomUrl      the new url of the random value source
 */
void module::set_random_url(QString sRandomUrl) {
    
    try {
        
        // the next line may fail, when an invalid URL is specified
        qkd::utility::random r = qkd::utility::random_source::create(sRandomUrl.toStdString());
        
        // good. apply!
        d->cRandom = r;
        d->sRandomUrl = sRandomUrl.toStdString();
        
        // syslog
        QString sMessage = QString("new random source: \"%1\"").arg(sRandomUrl);
        qkd::utility::syslog::info() << sMessage.toStdString();
    }
    catch (...) {
        
        // syslog
        QString sMessage = QString("failed to set new random source: \"%1\"").arg(sRandomUrl);
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << sMessage.toStdString();
    }
}


/**
 * set the role
 * 
 * @param   nRole           the new role
 */
void module::set_role(qulonglong nRole) {
    
    if (nRole == (uint8_t)module_role::ROLE_ALICE) d->eRole = module_role::ROLE_ALICE;
    else
    if (nRole == (uint8_t)module_role::ROLE_BOB) d->eRole = module_role::ROLE_BOB;
    else {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "refusing to set role to " << nRole << " - unknown role id.";
    }
}


/**
 * set the synchronize key ids flag
 * 
 * @param   bSynchronize    the new synchronize key id flag
 */
void module::set_synchronize_keys(bool bSynchronize) {
    d->bSynchronizeKeys = bSynchronize;
}


/**
 * set the synchronize TTL for not in-sync keys
 * 
 * @param   nTTL            the new synchronize TTL in seconds
 */
void module::set_synchronize_ttl(qulonglong nTTL) {
    d->nSynchronizeTTL = nTTL;
}


/**
 * set the number of keys left before terminating (0 --> do not terminate) 
 *
 * The idea is to have a per module counter which decreases
 * when processing a key (no matter if successfull or not).
 * This comes handy when testing a pipeline and one wants the
 * pipeline after some amount of keys done and terminated.
 *
 * If this number is already 0 the module is not terminated. 
 * Otherwise if the number _reaches_ 0 the module terminates.
 *
 * @param   nTerminateAfter     the new number of keys left to process before terminating (0 == don't terminate)
 */    
void module::set_terminate_after(qulonglong nTerminateAfter) {
    d->nTerminateAfter = nTerminateAfter;
}


/**
 * set the number of milliseconds for network send/recv timeout
 * 
 * @param   nTimeout        the new number of milliseconds for network send/recv timeout
 */
void module::set_timeout_network(qlonglong nTimeout) {
    
    std::lock_guard<std::mutex> cLock(d->cURLMutex);
    d->nTimeoutNetwork = nTimeout;
    if (d->cSocketListener) {
        d->cSocketListener->setsockopt(ZMQ_RCVTIMEO, &d->nTimeoutNetwork, sizeof(d->nTimeoutNetwork));
        d->cSocketListener->setsockopt(ZMQ_SNDTIMEO, &d->nTimeoutNetwork, sizeof(d->nTimeoutNetwork));
    }
    if (d->cSocketPeer) {
        d->cSocketPeer->setsockopt(ZMQ_RCVTIMEO, &d->nTimeoutNetwork, sizeof(d->nTimeoutNetwork));
        d->cSocketPeer->setsockopt(ZMQ_SNDTIMEO, &d->nTimeoutNetwork, sizeof(d->nTimeoutNetwork));
    }
}


/**
 * set the number of milliseconds after a failed read
 * 
 * @param   nTimeout        the new number of milliseconds to wait after a failed read
 */
void module::set_timeout_pipe(qlonglong nTimeout) {
    
    std::lock_guard<std::mutex> cLock(d->cURLMutex);
    d->nTimeoutPipe = nTimeout;
    if (d->cSocketPipeIn) {
        d->cSocketPipeIn->setsockopt(ZMQ_RCVTIMEO, &d->nTimeoutPipe, sizeof(d->nTimeoutPipe));
        d->cSocketPipeIn->setsockopt(ZMQ_SNDTIMEO, &d->nTimeoutPipe, sizeof(d->nTimeoutPipe));
    }
    if (d->cSocketPipeOut) {
        d->cSocketPipeOut->setsockopt(ZMQ_RCVTIMEO, &d->nTimeoutPipe, sizeof(d->nTimeoutPipe));
        d->cSocketPipeOut->setsockopt(ZMQ_SNDTIMEO, &d->nTimeoutPipe, sizeof(d->nTimeoutPipe));
    }
    
    d->nTimeoutPipe = nTimeout;
}


/**
 * convenience method to set all URLs at once
 * 
 * @param   sURLPipeIn          pipe in URL
 * @param   sURLPipeOut         pipe out URL
 * @param   sURLListen          listen URL
 * @param   sURLPeer            peer URL
 */
void module::set_urls(QString sURLPipeIn, QString sURLPipeOut, QString sURLListen, QString sURLPeer) {
    set_url_pipe_in(sURLPipeIn);
    set_url_pipe_out(sURLPipeOut);
    set_url_listen(sURLListen);
    set_url_peer(sURLPeer);
}


/**
 * sets a new LISTEN URL
 *
 * @param   sURL        the new LISTEN URL
 */
void module::set_url_listen(QString sURL) {
    std::lock_guard<std::mutex> cLock(d->cURLMutex);
    d->sURLListen = sURL.toStdString();
    d->bSetupListen = true;
}


/**
 * sets a new PEER URL
 *
 * @param   sURL        the new PEER URL
 */
void module::set_url_peer(QString sURL) {
    std::lock_guard<std::mutex> cLock(d->cURLMutex);
    d->sURLPeer = sURL.toStdString();
    d->bSetupPeer = true;
}


/**
 * sets a new pipeline INCOMING URL
 *
 * @param   sURL        the new pipe in URL
 */
void module::set_url_pipe_in(QString sURL) {
    std::lock_guard<std::mutex> cLock(d->cURLMutex);
    d->sURLPipeIn = sURL.toStdString();
    d->bSetupPipeIn = true;
}


/**
 * sets a new pipeline OUTGOING URL
 *
 * @param   sURL        the new pipe out URL
 */
void module::set_url_pipe_out(QString sURL) {
    std::lock_guard<std::mutex> cLock(d->cURLMutex);
    d->sURLPipeOut = sURL.toStdString();
    d->bSetupPipeOut = true;
}


/**
 * finished work on a key for at least 1 sec ago
 * 
 * @return  stalled flag
 */
bool module::stalled() const {
    if (processing()) return false;
    std::chrono::system_clock::time_point cNow = std::chrono::system_clock::now();
    return (std::chrono::duration_cast<std::chrono::milliseconds>(cNow - d->cLastProcessedKey)).count() > 1000;
    
}


/**
 * runs and resumes the module as soon as possible
 * 
 * this is a helper function, which calls run()
 * with all URLs set to "" and then calls resume()
 * to immediately start the module.
 * 
 * This is done by the main thread later on, be
 * sure to have a running QCoreApplication.
 */
void module::start_later() {
    QTimer::singleShot(0, this, SLOT(delayed_start()));
}


/**
 * UNIX epoch timestamp of launch
 * 
 * Seconds since 1/1/1970 when this module
 * has been launched.
 * 
 * @return  UNIX epoch timestamp of key-store launch
 */
unsigned long module::start_time() const {
    return d->nStartTimeStamp;
}

    
/**
 * get the module statistic
 * 
 * @return  the module statistics
 */
module::module_stat & module::statistics() {
    return d->cStat;
}


/**
 * get the module statistic
 * 
 * @return  the module statistics
 */
module::module_stat const & module::statistics() const {
    return d->cStat;
}


/**
 * return the state of the module
 * 
 * @return  the state as integer
 */
qulonglong module::state() const {
    return (uint8_t)d->get_state();
}


/**
 * return the state name description for a given state
 * 
 * @param   eState      the module state of the module questioned
 * @return  the human readable module state name 
 */
QString module::state_name(module_state eState) {
    
    QString res;
    
    switch (eState) {
    case module_state::STATE_NEW: return "new";
    case module_state::STATE_READY: return "ready";
    case module_state::STATE_RUNNING: return "running";
    case module_state::STATE_TERMINATING: return "terminating";
    case module_state::STATE_TERMINATED: return "terminated";
    }
    
    return "unkown state";
}


/**
 * ensure we have the same keys on both sides to process
 */
void module::synchronize() {
    
    if (!is_synchronizing()) return;

    if (qkd::utility::debug::enabled()) qkd::utility::debug() << "synchronizing keys...";

    // TODO: we do not have authenticity when synchronizing keys ... is this a problem? o.O
    static qkd::crypto::crypto_context cNullContxt = qkd::crypto::engine::create("null");
    
    qkd::module::message cMessage;
    cMessage.m_cHeader.eType = qkd::module::message_type::MESSAGE_TYPE_KEY_SYNC;
    cMessage.data() << d->cStash.cInSync.size();
    for (auto const & cStashedKey : d->cStash.cInSync) cMessage.data() << cStashedKey.first;
    cMessage.data() << d->cStash.cOutOfSync.size();
    for (auto const & cStashedKey : d->cStash.cOutOfSync) cMessage.data() << cStashedKey.first;
    
    try {
        send(cMessage, cNullContxt, timeout_network());
    }
    catch (std::runtime_error & cException) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to send list of stashed keys to peer: " << cException.what();
        return;
    }
    
    try {
        recv(cMessage, cNullContxt, qkd::module::message_type::MESSAGE_TYPE_KEY_SYNC, timeout_network());
        recv_synchronize(cMessage);
    }
    catch (std::runtime_error & cException) {}
}


/**
 * get the synchronize key ids flag
 * 
 * @return  the synchronize key id flag
 */
bool module::synchronize_keys() const {
    return d->bSynchronizeKeys;
}


/**
 * get the synchronize TTL for not in-sync keys
 * 
 * @return  the synchronize TTL in seconds
 */
qulonglong module::synchronize_ttl() const {
    return d->nSynchronizeTTL;
}


/**
 * stops the module
 * 
 * This is a gracefull shutdown
 */
void module::terminate() {

    if (d->get_state() == module_state::STATE_TERMINATING) return;
    if (d->get_state() == module_state::STATE_TERMINATED) return;

    // do we have a thread at all? 
    if (d->cModuleThread.get_id() == std::thread::id()) {
        
        // no worker thread present: wind down ourselves here
        d->release();
    }
    else {
        
        // signal worker thread to terminate ...
        d->set_state(module_state::STATE_TERMINATING);
        
        // interrupt worker thread
        interrupt_worker();
    }
    
    // tell anyone we have terminated
    emit terminated();
}


/**
 * this is the entry point of the main thread worker
 */
void module::thread() {

    // setup all stuff
    if (!d->setup()) {
        d->release();
        emit terminated();
        return;
    }
    
    // debug to the user
    qkd::utility::debug() << "module setup done - entering ready state";
    
    // work!
    d->set_state(module_state::STATE_READY);
    emit ready();
    work();
    
    // debug to the user
    qkd::utility::debug() << "module work done - winding down module";
    
    // wind down
    d->release();
    emit terminated();
}


/**
 * number of keys left before terminating (0 --> do not terminate) 
 *
 * The idea is to have a per module counter which decreases
 * when processing a key (no matter if successfull or not).
 * This comes handy when testing a pipeline and one wants the
 * pipeline after some amount of keys done and terminated.
 *
 * If this number is already 0 the module is not terminated. 
 * Otherwise if the number _reaches_ 0 the module terminates.
 *
 * @return  the number of keys left to process before terminating
 */    
qulonglong module::terminate_after() const {
    return d->nTerminateAfter;
}


/**
 * return the number of milliseconds for network send/recv timeout
 * 
 * @return  the number of milliseconds for network send/recv timeout
 */
qlonglong module::timeout_network() const {
    std::lock_guard<std::mutex> cLock(d->cURLMutex);
    return d->nTimeoutNetwork;
}


/**
 * return the number of milliseconds after a failed read
 * 
 * @return  the number of milliseconds to wait after a failed read
 */
qlonglong module::timeout_pipe() const {
    std::lock_guard<std::mutex> cLock(d->cURLMutex);
    return d->nTimeoutPipe;
}


/**
 * return the type of the module
 * 
 * @return  the module-type as integer
 */
qulonglong module::type() const {
    return (qulonglong)d->eType;
}


/**
 * return the type name description for a given type
 * 
 * @param   eType           the type of the module questioned
 * @return  the human readable module-type name 
 */
QString module::type_name(module_type eType) {
    
    QString res;
    
    switch (eType) {
    case module_type::TYPE_PRESIFTING: return "presifting";
    case module_type::TYPE_SIFTING: return "sifting";
    case module_type::TYPE_ERROR_ESTIMATION: return "error estimation";
    case module_type::TYPE_ERROR_CORRECTION: return "error correction";
    case module_type::TYPE_CONFIRMATION: return "confirmation";
    case module_type::TYPE_PRIVACY_AMPLIFICATION: return "privacy amplification";
    case module_type::TYPE_KEYSTORE: return "q3p keystore";
    case module_type::TYPE_OTHER: return "other";
    }
    
    return "unkown type";
}


/**
 * return the URL for peer (serving endpoint)
 * 
 * @return  the URL for peer (serving endpoint)
 */
QString module::url_listen() const {
    return QString::fromStdString(d->sURLListen);
}


/**
 * return the URL of the peer connection (where this module connected to)
 * 
 * @return  the URL of the peer connection (where this module connected to)
 */
QString module::url_peer() const {
    return QString::fromStdString(d->sURLPeer);
}


/**
 * return the URL of incoming Pipe (serving endpoint)
 * 
 * @return  the URL of incoming Pipe (serving endpoint)
 */
QString module::url_pipe_in() const {
    return QString::fromStdString(d->sURLPipeIn);
}


/**
 * return the URL of outgoing Pipe
 * 
 * @return  the URL of outgoing Pipe
 */
QString module::url_pipe_out() const {
    return QString::fromStdString(d->sURLPipeOut);
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
module_state module::wait_for_state_change(module_state eWorkingState) const {
    return d->wait_for_state_change(eWorkingState);
}


/**
 * this is the real work function
 * 
 * This is called indirectly by run().
 * 
 * This contains the main worker loop, which is sketched (roughly!) here:
 * 
 * 1) as long we are READY: wait
 * 2) exit if not RUNNING
 * 3) get a key (if input Pipe has been specified)
 * 4) invoke process()
 * 5) write key (if process return true)
 * 6) return to 1
 * 
 * You may overwritte this method. But this changes module operation
 * dramatically.
 */
void module::work() {

    qkd::module::module_state eState = qkd::module::module_state::STATE_NEW;
    
    // loop until told otherwise
    do {
        
        d->bProcessing = false;
        
        // get the module state
        eState = get_state();
        while (eState == qkd::module::module_state::STATE_READY) eState = wait_for_state_change(eState);
        if (eState != qkd::module::module_state::STATE_RUNNING) break;
        
        qkd::key::key cKey;
        if (d->cStash.cInSync.size() > 0) {
            
            auto cStashIter = d->cStash.next_in_sync();
            if (cStashIter == d->cStash.cInSync.end()) {
                throw std::runtime_error("failed to pick next key in sync though stash is not empty");
            }
            cKey = (*cStashIter).second.cKey;
            d->cStash.cInSync.erase(cStashIter);
            d->cStash.nLastInSyncKeyPicked = cKey.id();
        }
        else {
            
            // ... from previous module
            if (!read(cKey)) {
                
                // failed read
                if (qkd::utility::debug::enabled()) qkd::utility::debug() << "failed to read key from previous module in pipe";
                
                // maybe we synchonize?
                synchronize();
                
                continue;
            }
            
            // evaluate if the key is accepted by this module
            if (!accept(cKey)) continue;
            
            // check module state again
            eState = get_state();
            while (eState == qkd::module::module_state::STATE_READY) eState = wait_for_state_change(eState);
            if (eState != qkd::module::module_state::STATE_RUNNING) break;
            
            // synchronize key if necessary
            if (is_synchronizing()) {
                
                // place key in "to-be-synced" area
                d->cStash.cOutOfSync[cKey.id()].cKey = cKey;
                d->cStash.cOutOfSync[cKey.id()].cStashed = std::chrono::system_clock::now();
                
                // sync keys
                synchronize();
                
                // reenter loop
                continue;
            }
        }
        
        // extract crypto contexts
        qkd::crypto::crypto_context cIncomingContext = qkd::crypto::engine::create("null");
        qkd::crypto::crypto_context cOutgoingContext = qkd::crypto::engine::create("null");
        
        // create incoming context
        try {
            if (!cKey.meta().sCryptoSchemeIncoming.empty()) {
                qkd::crypto::scheme cScheme(cKey.meta().sCryptoSchemeIncoming);
                cIncomingContext = qkd::crypto::engine::create(cScheme);
            }
        }
        catch (...) {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to create incoming crypto context for key";
        }
        
        // create outgoing context
        try {
            if (!cKey.meta().sCryptoSchemeOutgoing.empty()) {
                qkd::crypto::scheme cScheme(cKey.meta().sCryptoSchemeOutgoing);
                cOutgoingContext = qkd::crypto::engine::create(cScheme);
            }
        }
        catch (...) {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to create outgoing crypto context for key";
        }

        // work on key
        d->bProcessing = true;
        bool bForwardKey = process(cKey, cIncomingContext, cOutgoingContext);
        d->cLastProcessedKey = std::chrono::system_clock::now();
        d->bProcessing = false;
        
        // check module state again
        eState = get_state();
        while (eState == qkd::module::module_state::STATE_READY) eState = wait_for_state_change(eState);
        if (eState != qkd::module::module_state::STATE_RUNNING) break;
        
        // pass key to next module
        if (bForwardKey) {
            
            // pack the crypto context into the key
            cKey.meta().sCryptoSchemeIncoming = cIncomingContext->scheme().str();
            cKey.meta().sCryptoSchemeOutgoing = cOutgoingContext->scheme().str();
            if (cKey.meta().sCryptoSchemeIncoming == "null") cKey.meta().sCryptoSchemeIncoming = "";
            if (cKey.meta().sCryptoSchemeOutgoing == "null") cKey.meta().sCryptoSchemeOutgoing = "";

            // out with the key
            if (!write(cKey)) {
                if (qkd::utility::debug::enabled()) qkd::utility::debug() << "failed to write key to next module in pipe.";
            }
        }

        // check if we should terminate now
        if (d->nTerminateAfter != 0) {

            d->nTerminateAfter--;
            if (d->nTerminateAfter == 0) {
                terminate();
            }
        }
        
    } while (is_working_state(eState));
    
    // fix processing state (in case we missed that above)
    d->bProcessing = false;
}


/**
 * push the key to the next module
 * 
 * This method is called inside of work(). Call this 
 * method inside of process() if you know _exactly_ what
 * you are doing.
 * 
 * You should not need to call this directly. It get's called
 * if process() returns "true".
 * 
 * @param   cKey        key to pass to the next module
 * @return  true, if writing was successful
 */
bool module::write(qkd::key::key const & cKey) {

    // setup pipout if necessary
    if (d->bSetupPipeOut) d->setup_pipe_out();
    
    // check if we do have an output
    if (d->bPipeOutVoid) return true;
    
    // failed flag
    bool bFailed = true;
    
    // write to stdout:// ?
    if (d->bPipeOutStdout) {
        std::cout << cKey;
        bFailed = false;
    }
    else if (d->cSocketPipeOut) {
        
        // create a memory to hold key data
        qkd::utility::buffer cBuffer;
        cBuffer << cKey;

        // write to zeroMQ
        zmq::message_t cZMQMessage(cBuffer.size());
        memcpy(cZMQMessage.data(), cBuffer.get(), cBuffer.size());
         
        // get out with it!
        try {
            if (d->cSocketPipeOut->send(cZMQMessage)) bFailed = false;
        }
        catch (UNUSED zmq::error_t & cZMQError) {}
    }
    
    // check for successful write: key != null()
    if (bFailed) {
        
        // failed to write :(
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to send key to next module - key-id: " << cKey.id();
        return false;
    }

    // collect key data
    std::lock_guard<std::recursive_mutex> cLock(d->cStat.cMutex);
    d->cStat.nKeysOutgoing++;
    d->cStat.nKeyBitsOutgoing += cKey.size() * 8;
    d->cStat.nDisclosedBitsOutgoing += cKey.meta().nDisclosedBits;
    d->cStat.cKeysOutgoingRate << d->cStat.nKeysOutgoing;
    d->cStat.cKeyBitsOutgoingRate << d->cStat.nKeyBitsOutgoing;
    d->cStat.cDisclosedBitsOutgoingRate << d->cStat.nDisclosedBitsOutgoing;
    
    // state semething if debug is on
    if (qkd::utility::debug::enabled()) {
        
        // pretty printing for debug
        // if not needed, then performance is wasted here
        
        boost::format cLineFormater = boost::format("key-PUSH [%015ums] id: %010u bits: %010u err: %6.4f dis: %010u crc: %08x state: %-13s dur: %012u ns (%06u ms)");

        auto cTimePoint = std::chrono::duration_cast<std::chrono::milliseconds>(age());
        cLineFormater % cTimePoint.count();
        cLineFormater % cKey.id();
        cLineFormater % (cKey.size() * 8);
        cLineFormater % cKey.meta().nErrorRate;
        cLineFormater % cKey.meta().nDisclosedBits;
        
        // CRC32 checksum
        cLineFormater % cKey.data().crc32();
        
        // key state
        cLineFormater % cKey.state_string();
        
        // get the number of nanoseconds the key has 
        // dwelled in this process
        auto cNanoSeconds =  std::chrono::duration_cast<std::chrono::nanoseconds>(cKey.dwell());
        cLineFormater % cNanoSeconds.count();
        cLineFormater % (uint64_t)(std::floor(cNanoSeconds.count() / 1000000.0 + 0.5));
        
        qkd::utility::debug() << cLineFormater.str();
    }
    
    return true;
}


/**
 * delete a buffer function
 * needed for ZMQ delayed deletion of queued messges
 * 
 * This function assumes that the object referenced
 * by cHint is a qkd::utility::buffer instance
 * created with "new".
 * 
 * @param   cData           the data sent
 * @param   cHint           the buffer object itself
 */
void memory_delete(UNUSED void * cData, void * cHint) {
    qkd::utility::memory * cMemory = static_cast<qkd::utility::memory *>(cHint);
    if (cMemory != nullptr) delete cMemory;
}


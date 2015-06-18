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

#include <fstream>

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

// Qt
//#include <QtCore/QUrl>
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

#include "module_internal.h"


using namespace qkd::module;


// ------------------------------------------------------------
// fwd.

void memory_delete(void * cData, void * cHint);


// ------------------------------------------------------------
// code


/**
 * ctor
 * 
 * @param   sId             identification of the module
 * @param   eType           type of the module
 * @param   sDescription    description of the module
 * @param   sOrganisation   organisation (vendor) of module
 */
module::module(std::string sId, module_type eType, std::string sDescription, std::string sOrganisation) : QObject() {
    
    d = boost::shared_ptr<qkd::module::module::module_internal>(new qkd::module::module::module_internal(this, sId));
    
    struct timeval cTV;
    struct timezone cTZ;
    gettimeofday(&cTV, &cTZ);
    d->nStartTimeStamp = cTV.tv_sec;
    
    d->eType = eType;
    d->sDescription = sDescription;
    d->sOrganisation = sOrganisation;
    d->set_state(module_state::STATE_NEW);
    
    set_pipeline("default");
    set_synchronize_keys(true);
    set_synchronize_ttl(10);
    set_url_pipe_in("stdin://");
    set_url_pipe_out("stdout://");
    
    new ModuleAdaptor(this);
    
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

    QUrl cConfigURL(sConfigURL);
    
    // if we don't have a scheme assume a file, absolute or relative
    if (cConfigURL.scheme().isEmpty()) {
        
        boost::filesystem::path cPath = qkd::utility::environment::find_path(sConfigURL.toStdString());
        if (cPath.empty()) {

            std::stringstream ss;
            ss << "failed to load module configuration from: '"
                    << sConfigURL.toStdString() 
                    << "': unknown scheme '" 
                    << cConfigURL.scheme().toStdString() 
                    << "' or file not found";
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << ss.str();

            if (bRequired) {
                qkd::utility::syslog::warning() << __FILENAME__ 
                        << '@' 
                        << __LINE__ 
                        << ": as configuration is required, this is futile --> aborted"; 
                std::exit(1);
            }
            return false;
        }
        
        cConfigURL = QUrl(QString("file://") + QString::fromStdString(cPath.string()));
    }

    // "file://"
    if (cConfigURL.scheme() != "file") {

        std::stringstream ss;
        ss << "failed to load module configuration from: '" 
                << sConfigURL.toStdString() 
                << "': unknown scheme '" 
                << cConfigURL.scheme().toStdString() 
                << "'";
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << ss.str();

        if (bRequired) {
            qkd::utility::syslog::warning() << __FILENAME__ 
                    << '@' 
                    << __LINE__ 
                    << ": as configuration is required, this is futile --> aborted"; 
            std::exit(1);
        }
        return false;
    }
    
    qkd::utility::debug() << "loading configuration from: " << cConfigURL.toString().toStdString();
    
    std::string sFile = cConfigURL.toLocalFile().toStdString();
    std::ifstream cConfigFile(sFile);
    if (!cConfigFile.is_open()) {
        qkd::utility::syslog::warning() << __FILENAME__ 
                << '@' 
                << __LINE__ 
                << ": " 
                << "failed to open configuration '" 
                << sFile 
                << "'";

        if (bRequired) {
            qkd::utility::syslog::warning() << __FILENAME__ 
                    << '@' 
                    << __LINE__ 
                    << ": as configuration is required, this is futile --> aborted"; 
            std::exit(1);
        }
        return false;
    }
    
    std::set<std::string> cOptions;
    cOptions.insert("*");
    
    qkd::utility::properties cConfig;
    try {
        
        boost::program_options::detail::config_file_iterator cConfigIter(cConfigFile, cOptions);
        boost::program_options::detail::config_file_iterator cEOF;
        
        for (; cConfigIter != cEOF; cConfigIter++) {
            boost::program_options::option cOption = *cConfigIter;
            cConfig[cOption.string_key] = cOption.value[0];
        }
        
        std::string sConfigPrefix = config_prefix();
        for (auto const & cEntry : cConfig) {
            
            if (cEntry.first.substr(0, sConfigPrefix.size()) != sConfigPrefix) continue;
            apply_standard_config(cEntry.first, cEntry.second);
        }        
        
        apply_config(sConfigURL.toStdString(), cConfig);
        
    }
    catch (boost::program_options::invalid_syntax const & cErrInvalidSyntax) {
        qkd::utility::syslog::crit() << __FILENAME__ 
                << '@' 
                << __LINE__ 
                << ": " 
                << "failed to parse config file: " 
                << sFile 
                << " invalid syntax at: '" 
                << cErrInvalidSyntax.tokens() 
                << "'";
    }
    catch (std::exception const & cException) {
        qkd::utility::syslog::crit() << __FILENAME__ 
                << '@' 
                << __LINE__ 
                << ": " 
                << "failed to parse config file: " 
                << sFile 
                << " exception: " 
                << cException.what();
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
    
    QCoreApplication::processEvents(QEventLoop::AllEvents);
    
    init();
    run();
    
    qkd::module::module_state eState = qkd::module::module_state::STATE_NEW;
    while (eState == qkd::module::module_state::STATE_NEW) eState = wait_for_state_change(eState);
    
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
    
    // avoid double init
    static bool bInitialized = false;
    if (bInitialized) return;
    bInitialized = true;
    
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
    if (d->cModuleThread.get_id() == std::this_thread::get_id()) return;
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
    
    if (d->cModuleThread.get_id() == std::thread::id()) {
        if (qkd::utility::debug::enabled()) qkd::utility::debug() << "module thread not running";
        return;
    }
    
    if (d->cModuleThread.get_id() == std::this_thread::get_id()) {
        if (qkd::utility::debug::enabled()) qkd::utility::debug() << "module thread won't join itself";
        return;
    }
    
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
    
    cKey = qkd::key::key::null();
    
    if (d->bSetupPipeIn) d->setup_pipe_in();
    if (d->bPipeInVoid) return true;
    
    if (d->bPipeInStdin) {
        std::cin >> cKey;
    }
    else if (d->cSocketPipeIn) {
        
        zmq_msg_t cZMQMessage;
        assert(zmq_msg_init(&cZMQMessage) == 0);
        int nRead = zmq_msg_recv(&cZMQMessage, d->cSocketPipeIn, 0);
        if (nRead == -1) {
            std::stringstream ss;
            ss << "failed reading key: " << strerror(zmq_errno());
            zmq_msg_close(&cZMQMessage);
            throw std::runtime_error(ss.str());
        }

        qkd::utility::buffer cData = qkd::utility::buffer(
                qkd::utility::memory::wrap((unsigned char *)zmq_msg_data(&cZMQMessage), zmq_msg_size(&cZMQMessage)));
        cData >> cKey;

        zmq_msg_close(&cZMQMessage);
    }
    
    if (cKey == qkd::key::key::null()) {
        rest();
        return false;
    }
    
    d->add_stats_incoming(cKey);
    cKey.meta().cTimestampRead = std::chrono::high_resolution_clock::now();
    if (qkd::utility::debug::enabled()) d->debug_key_pull(cKey);

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
bool module::recv(qkd::module::message & cMessage, 
        qkd::crypto::crypto_context & cAuthContext, 
        qkd::module::message_type eType, 
        int nTimeOut) throw (std::runtime_error) {

    bool bReceived = false;
    auto cStartOfRecv = std::chrono::high_resolution_clock::now();

    // ensure there is at least an empty message queue for this message type
    if (d->cMessageQueues.find(eType) == d->cMessageQueues.end()) {
        d->cMessageQueues[eType] = std::queue<qkd::module::message>();
    }

    if (!d->cMessageQueues[eType].empty()) {
        cMessage = d->cMessageQueues[eType].front();
        d->cMessageQueues[eType].pop();
        qkd::utility::debug() << "message for type " 
                << static_cast<uint32_t>(eType) 
                << " already in message queue - popped from queue.";
    }
    else {

        // receive message and push them into queue
        // until a) correct type received or b) timeout

        do {

            bReceived = recv_internal(cMessage, nTimeOut);
            if (!bReceived) return false;

            if (cMessage.type() != eType) {

                if (d->cMessageQueues.find(cMessage.type()) == d->cMessageQueues.end()) {
                    d->cMessageQueues[cMessage.type()] = std::queue<qkd::module::message>();
                }
                d->cMessageQueues[cMessage.type()].push(cMessage);
                qkd::utility::debug() << "received a QKD message for message type " 
                        << static_cast<uint32_t>(cMessage.type()) 
                        << " when expecting " 
                        << static_cast<uint32_t>(eType) 
                        << " - pushed into queue for later dispatch.";
                bReceived = false;

                if (nTimeOut >= 0) {

                    auto cNow = std::chrono::high_resolution_clock::now();
                    auto nPassed = std::chrono::duration_cast<std::chrono::milliseconds>(cNow - cStartOfRecv).count();
                    if (nPassed > nTimeOut) {

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

    if (bIsAlice && d->bSetupPeer) d->setup_peer();
    if (bIsBob && d->bSetupListen) d->setup_listen();
        
    if (bIsAlice && (d->cSocketPeer == nullptr)) throw std::runtime_error("no connection to peer");
    if (bIsBob && (d->cSocketListener == nullptr)) throw std::runtime_error("not accepting connection");
    
    void * cSocket = nullptr;
    if (bIsAlice) cSocket = d->cSocketPeer;
    if (bIsBob) cSocket = d->cSocketListener;
    
    if (!cSocket) {
        qkd::utility::syslog::warning() << __FILENAME__ 
                << '@' 
                << __LINE__ 
                << ": " 
                << "failed to decide which channel to use for receive";
        throw std::runtime_error("failed to decide which channel to use for recv");
    }
    
    // TODO: setting timeout may be only valid before socket connet/bind
    if (zmq_setsockopt(cSocket, ZMQ_RCVTIMEO, &nTimeOut, sizeof(nTimeOut)) == -1) {
        std::stringstream ss;
        ss << "failed to set timeout on socket: " << strerror(zmq_errno());
        throw std::runtime_error(ss.str());
    }
    
    zmq_msg_t cZMQHeader;
    assert(zmq_msg_init(&cZMQHeader) == 0);
    int nReadHeader = zmq_msg_recv(&cZMQHeader, cSocket, 0);
    if (nReadHeader == -1) {
        std::stringstream ss;
        ss << "failed reading message header from peer: " << strerror(zmq_errno());
        zmq_msg_close(&cZMQHeader);
        throw std::runtime_error(ss.str());
    }
    if (!zmq_msg_more(&cZMQHeader) || (zmq_msg_size(&cZMQHeader) != sizeof(cMessage.m_cHeader))) {
            throw std::runtime_error("received invalid message header");
    }

    memcpy(&(cMessage.m_cHeader), (unsigned char *)zmq_msg_data(&cZMQHeader), sizeof(cMessage.m_cHeader));
    zmq_msg_close(&cZMQHeader);

    zmq_msg_t cZMQData;
    assert(zmq_msg_init(&cZMQData) == 0);
    int nReadData = zmq_msg_recv(&cZMQData, cSocket, 0);
    if (nReadData == -1) {
        std::stringstream ss;
        ss << "failed reading message data from peer: " << strerror(zmq_errno());
        zmq_msg_close(&cZMQData);
        throw std::runtime_error(ss.str());
    }

    cMessage.data().resize(zmq_msg_size(&cZMQData));
    memcpy(cMessage.data().get(), zmq_msg_data(&cZMQData), zmq_msg_size(&cZMQData));
    cMessage.data().set_position(0);
    zmq_msg_close(&cZMQData);

    if (is_dying_state()) return false;

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
    
    qkd::utility::syslog::info() << "connecting to DBus:" << getenv("DBUS_SESSION_BUS_ADDRESS");
    
    QDBusConnection cDBus = qkd::utility::dbus::qkd_dbus();
    QString sServiceName = service_name();
    if (!cDBus.registerService(sServiceName)) {
        QString sMessage = QString("failed to register DBus service \"") + sServiceName + "\""; 
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << sMessage.toStdString();
    }
    
    qkd::utility::syslog::info() << "connected to DBus:" 
            << getenv("DBUS_SESSION_BUS_ADDRESS") 
            << " as \"" 
            << sServiceName.toStdString() 
            << "\"";

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
    case module_role::ROLE_ALICE: 
        return "alice";
    case module_role::ROLE_BOB: 
        return "bob";
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

    if (d->cModuleThread.get_id() != std::thread::id()) {
        if (qkd::utility::debug::enabled()) qkd::utility::debug() << "module thread already running";
        return;
    }
    
    d->set_state(module_state::STATE_NEW);
    
    qkd::utility::debug() << "run module: " << 
            "in='" << d->sURLPipeIn << "' " <<
            "out='" << d->sURLPipeOut << "' " << 
            "listen='" << d->sURLListen << "' " << 
            "peer='" << d->sURLPeer << "'";
    
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
void module::send(qkd::module::message & cMessage, 
        qkd::crypto::crypto_context & cAuthContext, 
        int nTimeOut) throw (std::runtime_error) {
    
    bool bIsAlice = is_alice();
    bool bIsBob = is_bob();

    if (bIsAlice && d->bSetupPeer) d->setup_peer();
    if (bIsBob && d->bSetupListen) d->setup_listen();
        
    if (bIsAlice && (d->cSocketPeer == nullptr)) throw std::runtime_error("no connection to peer");
    if (bIsBob && (d->cSocketListener == nullptr)) throw std::runtime_error("not accepting connection");
   
    void * cSocket = nullptr;
    if (bIsAlice) cSocket = d->cSocketPeer;
    if (bIsBob) cSocket = d->cSocketListener;
    
    if (!cSocket) {
        qkd::utility::syslog::warning() << __FILENAME__ 
                << '@' 
                << __LINE__ 
                << ": " 
                << "failed to decide which channel to use for send";
        throw std::runtime_error("failed to decide which channel to use for send");
    }

    // TODO: setting timeout may be only valid before socket connet/bind
    if (zmq_setsockopt(cSocket, ZMQ_SNDTIMEO, &nTimeOut, sizeof(nTimeOut)) == -1) {
        std::stringstream ss;
        ss << "failed to set timeout on socket: " << strerror(zmq_errno());
        throw std::runtime_error(ss.str());
    }
  
    cMessage.m_cHeader.nId = htobe32(++qkd::module::message::m_nLastId);
    cMessage.m_cTimeStamp = std::chrono::high_resolution_clock::now();
    d->debug_message(true, cMessage);

    int nSentHeader = zmq_send(cSocket, &(cMessage.m_cHeader), sizeof(cMessage.m_cHeader), ZMQ_SNDMORE);
    if (nSentHeader == -1) {
        std::stringstream ss;
        ss << "failed sending message header to peer: " << strerror(zmq_errno());
        throw std::runtime_error(ss.str());
    }

    int nSentData = zmq_send(cSocket, cMessage.data().get(), cMessage.data().size(), 0);
    if (nSentData == -1) {
        std::stringstream ss;
        ss << "failed sending message data to peer: " << strerror(zmq_errno());
        throw std::runtime_error(ss.str());
    }

    cAuthContext << cMessage.data();
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
    
    if (!qkd::utility::dbus::valid_service_name_particle(ss.str())) {
        qkd::utility::syslog::crit() << __FILENAME__ 
                << '@' 
                << __LINE__ 
                << ": " 
                << "DBus service name 'at.ac.ait.qkd.module." 
                << ss.str() 
                << "' is not valid - impossible to register on DBus";
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
    
    if (is_working_state()) {
        
        // warn user: the module is already up and working
        // changing the pipeline should have been done earlier
        // this may cause problems ...
        qkd::utility::syslog::warning() << __FILENAME__ 
                << '@' 
                << __LINE__ 
                << ": " 
                << "warning: setting pipeline in working state.";
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
        
        qkd::utility::random r = qkd::utility::random_source::create(sRandomUrl.toStdString());
        
        d->cRandom = r;
        d->sRandomUrl = sRandomUrl.toStdString();
        
        QString sMessage = QString("new random source: \"%1\"").arg(sRandomUrl);
        qkd::utility::syslog::info() << sMessage.toStdString();
    }
    catch (...) {
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
    
    if (nRole == (uint8_t)module_role::ROLE_ALICE) {
        d->eRole = module_role::ROLE_ALICE;
    }
    else
    if (nRole == (uint8_t)module_role::ROLE_BOB) {
        d->eRole = module_role::ROLE_BOB;
    }
    else {
        qkd::utility::syslog::warning() << __FILENAME__ 
                << '@' 
                << __LINE__ 
                << ": " 
                << "refusing to set role to " 
                << nRole 
                << " - unknown role id.";
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
    
    // TODO: this may not work on already opened sockets
    std::lock_guard<std::mutex> cLock(d->cURLMutex);
    d->nTimeoutNetwork = nTimeout;
    if (d->cSocketListener) {
        if (zmq_setsockopt(d->cSocketListener, ZMQ_RCVTIMEO, &d->nTimeoutNetwork, sizeof(d->nTimeoutNetwork)) == -1) {
            std::stringstream ss;
            ss << "failed to set timeout on socket: " << strerror(zmq_errno());
            throw std::runtime_error(ss.str());
        }
        if (zmq_setsockopt(d->cSocketListener, ZMQ_SNDTIMEO, &d->nTimeoutNetwork, sizeof(d->nTimeoutNetwork)) == -1) {
            std::stringstream ss;
            ss << "failed to set timeout on socket: " << strerror(zmq_errno());
            throw std::runtime_error(ss.str());
        }
    }
    if (d->cSocketPeer) {
        if (zmq_setsockopt(d->cSocketPeer, ZMQ_RCVTIMEO, &d->nTimeoutNetwork, sizeof(d->nTimeoutNetwork)) == -1) {
            std::stringstream ss;
            ss << "failed to set timeout on socket: " << strerror(zmq_errno());
            throw std::runtime_error(ss.str());
        }
        if (zmq_setsockopt(d->cSocketPeer, ZMQ_SNDTIMEO, &d->nTimeoutNetwork, sizeof(d->nTimeoutNetwork)) == -1) {
            std::stringstream ss;
            ss << "failed to set timeout on socket: " << strerror(zmq_errno());
            throw std::runtime_error(ss.str());
        }
    }
}


/**
 * set the number of milliseconds after a failed read
 * 
 * @param   nTimeout        the new number of milliseconds to wait after a failed read
 */
void module::set_timeout_pipe(qlonglong nTimeout) {
    
    // TODO: this may not work on already opened sockets
    std::lock_guard<std::mutex> cLock(d->cURLMutex);
    d->nTimeoutPipe = nTimeout;
    if (d->cSocketPipeIn) {
        if (zmq_setsockopt(d->cSocketPipeIn, ZMQ_RCVTIMEO, &d->nTimeoutPipe, sizeof(d->nTimeoutPipe)) == -1) {
            std::stringstream ss;
            ss << "failed to set timeout on socket: " << strerror(zmq_errno());
            throw std::runtime_error(ss.str());
        }
    }
    if (d->cSocketPipeOut) {
        if (zmq_setsockopt(d->cSocketPipeOut, ZMQ_SNDTIMEO, &d->nTimeoutPipe, sizeof(d->nTimeoutPipe)) == -1) {
            std::stringstream ss;
            ss << "failed to set timeout on socket: " << strerror(zmq_errno());
            throw std::runtime_error(ss.str());
        }
    }
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
        qkd::utility::syslog::warning() << __FILENAME__ 
                << '@' 
                << __LINE__ 
                << ": " 
                << "failed to send list of stashed keys to peer: " 
                << cException.what();
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

    qkd::utility::debug() << "terminate call received";

    if (d->get_state() == module_state::STATE_TERMINATING) return;
    if (d->get_state() == module_state::STATE_TERMINATED) return;

    if (d->cModuleThread.get_id() == std::this_thread::get_id()) {
        d->release();
    }
    else {
        
        d->set_state(module_state::STATE_TERMINATING);
        interrupt_worker();
    }
}


/**
 * this is the entry point of the main thread worker
 */
void module::thread() {

    if (!d->setup()) {
        qkd::utility::syslog::crit() << __FILENAME__ 
                << '@' 
                << __LINE__ 
                << ": " 
                << "unable to setup module thread: terminating";
        d->release();
        emit terminated();
        return;
    }
    
    qkd::utility::debug() << "module setup done - entering ready state";
    d->debug_config();
    d->set_state(module_state::STATE_READY);
    emit ready();
    work();
    qkd::utility::debug() << "module work done - winding down module";
    
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

    qkd::utility::debug() << "working on icoming keys started";
    
    do {
        
        d->bProcessing = false;
        
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
            qkd::utility::debug() << "scheduled key " << cKey.id() << " from in-sync stash for next process";
        }
        else {
            
            if (!read(cKey)) {
                if (qkd::utility::debug::enabled()) {
                    qkd::utility::debug() << "failed to read key from previous module in pipe";
                }
                synchronize();
                continue;
            }
            
            if (!accept(cKey)) {
                if (qkd::utility::debug::enabled()) {
                    qkd::utility::debug() << "key " << cKey.id() << " is not accepted by this module";
                }
                continue;
            }
            
            eState = get_state();
            while (eState == qkd::module::module_state::STATE_READY) eState = wait_for_state_change(eState);
            if (eState != qkd::module::module_state::STATE_RUNNING) break;
            
            if (is_synchronizing()) {
                
                d->cStash.cOutOfSync[cKey.id()].cKey = cKey;
                d->cStash.cOutOfSync[cKey.id()].cStashed = std::chrono::system_clock::now();
                synchronize();
                continue;
            }
        }
        
        qkd::crypto::crypto_context cIncomingContext = qkd::crypto::engine::create("null");
        qkd::crypto::crypto_context cOutgoingContext = qkd::crypto::engine::create("null");
        try {
            if (!cKey.meta().sCryptoSchemeIncoming.empty()) {
                qkd::crypto::scheme cScheme(cKey.meta().sCryptoSchemeIncoming);
                cIncomingContext = qkd::crypto::engine::create(cScheme);
            }
        }
        catch (...) {
            qkd::utility::syslog::warning() << __FILENAME__ 
                    << '@' 
                    << __LINE__ 
                    << ": " 
                    << "failed to create incoming crypto context for key";
        }
        try {
            if (!cKey.meta().sCryptoSchemeOutgoing.empty()) {
                qkd::crypto::scheme cScheme(cKey.meta().sCryptoSchemeOutgoing);
                cOutgoingContext = qkd::crypto::engine::create(cScheme);
            }
        }
        catch (...) {
            qkd::utility::syslog::warning() << __FILENAME__ 
                    << '@' 
                    << __LINE__ 
                    << ": " 
                    << "failed to create outgoing crypto context for key";
        }

        d->bProcessing = true;
        qkd::utility::debug() << "processing key " << cKey.id();
        bool bForwardKey = process(cKey, cIncomingContext, cOutgoingContext);
        d->cLastProcessedKey = std::chrono::system_clock::now();
        d->bProcessing = false;
        
        eState = get_state();
        while (eState == qkd::module::module_state::STATE_READY) eState = wait_for_state_change(eState);
        if (eState != qkd::module::module_state::STATE_RUNNING) break;
        
        if (bForwardKey) {
            
            cKey.meta().sCryptoSchemeIncoming = cIncomingContext->scheme().str();
            cKey.meta().sCryptoSchemeOutgoing = cOutgoingContext->scheme().str();
            if (cKey.meta().sCryptoSchemeIncoming == "null") cKey.meta().sCryptoSchemeIncoming = "";
            if (cKey.meta().sCryptoSchemeOutgoing == "null") cKey.meta().sCryptoSchemeOutgoing = "";

            if (!write(cKey)) {
                if (qkd::utility::debug::enabled()) {
                    qkd::utility::debug() << "failed to write key to next module in pipe.";
                }
            }
        }

        if (d->nTerminateAfter != 0) {

            d->nTerminateAfter--;
            if (d->nTerminateAfter == 0) {
                terminate();
            }
        }
        
    } while (is_working_state(eState));
    
    qkd::utility::debug() << "working on incoming keys suspended";

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

    if (d->bSetupPipeOut) d->setup_pipe_out();
    if (d->bPipeOutVoid) return true;
    
    bool bFailed = true;
    
    if (d->bPipeOutStdout) {
        std::cout << cKey;
        bFailed = false;
    }
    else if (d->cSocketPipeOut) {
        
        qkd::utility::buffer cBuffer;
        cBuffer << cKey;

        int nWritten = zmq_send(d->cSocketPipeOut, cBuffer.get(), cBuffer.size(), 0);
        if (nWritten == -1) {
            std::stringstream ss;
            ss << "failed writing key to next module: " << strerror(zmq_errno());
            throw std::runtime_error(ss.str());
        }
    }
    
    if (bFailed) {
        qkd::utility::syslog::warning() << __FILENAME__ 
                << '@' 
                << __LINE__ 
                << ": " 
                << "failed to send key to next module - key-id: " 
                << cKey.id();
        return false;
    }

    d->add_stats_outgoing(cKey);
    
    if (qkd::utility::debug::enabled()) d->debug_key_push(cKey);
    
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


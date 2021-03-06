/* 
 * module.cpp
 * 
 * QKD module implementation
 *
 * Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *
 * Copyright (C) 2012-2016 AIT Austrian Institute of Technology
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
#include <QtDBus/QDBusConnection>

// ait
#include <qkd/crypto/context.h>
#include <qkd/crypto/engine.h>
#include <qkd/exception/connection_error.h>
#include <qkd/exception/protocol_error.h>
#include <qkd/utility/dbus.h>
#include <qkd/utility/debug.h>
#include <qkd/utility/syslog.h>

#include <qkd/module/module.h>

// DBus integration
#include "module_dbus.h"

#include "module_internal.h"

using namespace qkd::module;


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
    
    d = std::shared_ptr<qkd::module::module::module_internal>(new qkd::module::module::module_internal(this, sId));
    
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
    
    init();
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
    if (sSubKey == "terminate_after") {
        set_terminate_after(std::stoll(sValue));
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
 * return the connection object associated with a connection type
 * 
 * @param   eType           the connection type
 * @return  the connection associated with this type
 */
qkd::module::connection const & qkd::module::module::connection(qkd::module::connection_type eType) const {
    
    switch (eType) {
        
    case qkd::module::connection_type::LISTEN:
        return *(d->cConListen);
        
    case qkd::module::connection_type::PEER:
        return *(d->cConPeer);
        
    case qkd::module::connection_type::PIPE_IN:
        return *(d->cConPipeIn);
        
    case qkd::module::connection_type::PIPE_OUT:
        return *(d->cConPipeOut);
        
    }
    
    throw qkd::exception::connection_error("no connection known for this connection type");
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
 * Opposed to the D-Bus method slot of the same name,
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
                qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ 
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
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ 
                    << ": as configuration is required, this is futile --> aborted"; 
            std::exit(1);
        }
        return false;
    }
    
    qkd::utility::debug() << "loading configuration from: " << cConfigURL.toString().toStdString();
    
    std::string sFile = cConfigURL.toLocalFile().toStdString();
    std::ifstream cConfigFile(sFile);
    if (!cConfigFile.is_open()) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ 
                << ": failed to open configuration '" << sFile << "'";

        if (bRequired) {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ 
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
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ 
                << ": failed to parse config file: " << sFile 
                << " invalid syntax at: '" << cErrInvalidSyntax.tokens() << "'";
    }
    catch (std::exception const & cException) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ 
                << ": failed to parse config file: " << sFile 
                << " exception: "    << cException.what();
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
 * check if we should print key sync issues on stderr.
 *
 * @return true, iff key sync debug messages are printed on stderr
 */
bool module::debug_key_sync() const {
    return d->bDebugKeySync;
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
 * finished work on a key for at least 1 sec ago
 * 
 * @return  idle flag
 */
bool module::idle() const {
    if (processing() && (get_state() == qkd::module::module_state::STATE_RUNNING)) return false;
    std::chrono::system_clock::time_point cNow = std::chrono::system_clock::now();
    bool bIdle = (std::chrono::duration_cast<std::chrono::milliseconds>(cNow - d->cLastProcessedKey)).count() > 1000;
    return bIdle;
    
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
 * this is useful if you find the worker thread been
 * blocked by some I/O operation (e.g. send/recv) and
 * want to abort that action or need other steps to be
 * undertaken.
 * 
 * if the worker thread is blocked by a send/recv then
 * the method returns with no data read or sent and with
 * the return value of false.
 */
void module::interrupt_worker() {
    if (d->cModuleThread.get_id() == std::thread::id()) return;
    if (d->cModuleThread.get_id() == std::this_thread::get_id()) return;
    pthread_kill(d->cModuleThread.native_handle(), SIGINT);
    std::this_thread::yield();
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
    if (sSubKey == "terminate_after") return true;
    
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
 * this is the real module's working method on a single key
 * 
 * You have to overwrite this to work on a single key.
 * 
 * This method is called by work() for a new key. If the input pipe
 * has been set to void ("") then the input key is always a NULL key
 * and the crypto contexts are "null".
 * 
 * This method is ought to process the new incoming key and to perform
 * actions on this key. The given parameter "cKey" is passed by reference
 * and should contain the modified key for the next module in the pipe.
 * 
 * If the modified key is to be forwarded to the next module, then this
 * method has to return "true".
 * 
 * @param   cKey                    the key just read from the input pipe
 * @param   cIncomingContext        incoming crypto context
 * @param   cOutgoingContext        outgoing crypto context
 * @return  true, if the key is to be pushed to the output pipe
 */
bool module::process(UNUSED qkd::key::key & cKey, 
        UNUSED qkd::crypto::crypto_context & cIncomingContext, 
        UNUSED qkd::crypto::crypto_context & cOutgoingContext) {
    
    qkd::utility::debug() << "implementation for process() missing - please check for module code.";
    return false;
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
    if (!d->cConPipeIn->read_key(cKey)) {
        rest();
        return false;
    }
    
    // do not add stat if we received on a void connection
    if (d->cConPipeIn->is_void()) return true;
    
    d->add_stats_incoming(cKey);
    cKey.meta().cTimestampRead = std::chrono::high_resolution_clock::now();
    if (qkd::utility::debug::enabled()) d->debug_key_pull(cKey);

    return true;
}


/**
 * read a message from the peer module
 * 
 * this call is blocking
 * 
 * The given message object will be deleted with delete before assigning new values.
 * Therefore if message receive has been successful the message is not NULL
 * 
 * This call waits explicitly for the next message been of type eType. If this
 * is NOT the case a exception is thrown.
 *
 * Internally the recv_internal method is called and the actual receive
 * is performed. 
 * 
 * @deprecated
 * @param   cMessage            this will receive the message
 * @param   cAuthContext        the authentication context involved
 * @param   eType               message type to receive
 * @return  true, if we have received a message
 */
bool module::recv(qkd::module::message & cMessage, 
        qkd::crypto::crypto_context & cAuthContext, 
        qkd::module::message_type eType) {

    return recv(0, cMessage, cAuthContext, eType);
}


/**
 * read a message from the peer module
 * 
 * this call is blocking
 * 
 * Every message's data recveived from the peer must be associated with the current key
 * we are working on. Therefore the given key id will be compared with the message's key id.
 * On mismatch a qkd::exception::protocol_error will be thrown.
 * 
 * The given message object will be deleted with delete before assigning new values.
 * Therefore if message receive has been successful the message is not NULL
 * 
 * This call waits explicitly for the next message been of type eType. If this
 * is NOT the case a exception is thrown.
 * 
 * Internally the recv_internal method is called and the actual receive
 * is performed. 
 * 
 * @param   nKeyId              the key id we are currently working on
 * @param   cMessage            this will receive the message
 * @param   cAuthContext        the authentication context involved
 * @param   eType               message type to receive
 * @return  true, if we have received a message, false else
 */
bool module::recv(qkd::key::key_id nKeyId,
                  qkd::module::message & cMessage, 
                  qkd::crypto::crypto_context & cAuthContext, 
                  qkd::module::message_type eType) {
    
    qkd::module::connection * cCon = nullptr;
    if (is_alice()) cCon = d->cConPeer;
    if (is_bob()) cCon = d->cConListen;
    if (!cCon) {
        throw std::logic_error("Cannot determine where to receive from (nor alice neither bob).");
    }
    
    if (!cCon->recv_message(cMessage)) {
        return false;
    }
    if (is_dying_state()) return false;

    cMessage.m_cTimeStamp = std::chrono::high_resolution_clock::now();
    d->debug_message(false, cMessage);
    
    if (eType == cMessage.type()) {
        
        if (nKeyId != cMessage.key_id()) {
            throw qkd::exception::protocol_error("Key id mismatch in received message, we might operate on different keys");
        }
        
        cAuthContext << cMessage.data();
        cMessage.data().set_position(0);
        return true;
    }
    
    qkd::utility::debug() << "Received a QKD message for message type " 
            << static_cast<uint32_t>(cMessage.type()) 
            << " when expecting " 
            << static_cast<uint32_t>(eType);    
            
    // waited for different message type ... %(
    if ((eType == qkd::module::message_type::MESSAGE_TYPE_DATA) && (cMessage.type() == qkd::module::message_type::MESSAGE_TYPE_KEY_SYNC)) {
        
        // waited for data but received sync:
        // our module worker wants some data, 
        // but our peer sent us a sync
        try {
            d->cStash->recv(cMessage);
        }
        catch (...) {}
    }
    
    return false;
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
    
    qkd::utility::syslog::info() << "connected to DBus:" << getenv("DBUS_SESSION_BUS_ADDRESS") 
            << " as \"" << sServiceName.toStdString() << "\"";

    if (!cDBus.registerObject("/Module", this)) {
        QString sMessage = QString("failed to register DBus object /Module");
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << sMessage.toStdString();
    }
    else {
        qkd::utility::syslog::info() << "module registered on DBus as /Module";
    }
}


/**
 * rest 50 milliseconds for a next communication try
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
            "in='" << d->cConPipeIn->urls_string() << "' " <<
            "out='" << d->cConPipeOut->urls_string() << "' " << 
            "listen='" << d->cConListen->urls_string() << "' " << 
            "peer='" << d->cConPeer->urls_string() << "'";
    
    d->cModuleThread = std::thread([this]{ thread(); });
}


/**
 * send a message to the peer module
 * 
 * this call is blocking
 * 
 * Note: this function takes ownership of the message's data sent! 
 * Afterwards the message's data will be void
 * 
 * Sending might fail on interrupt.
 *
 * @deprecated
 * @param   cMessage            the message to send
 * @param   cAuthContext        the authentication context involved
 * @returns true, if successfully sent
 */
bool module::send(qkd::module::message & cMessage, qkd::crypto::crypto_context & cAuthContext, int nPath) {
    return send(0, cMessage, cAuthContext, nPath);
}


/**
 * send a message to the peer module
 * 
 * this call is blocking
 * 
 * Every message sent to the remote peer module requires a key id. This
 * is necessary in order to ensure that the data of messages received are 
 * dealing with the very same key the module is currently working upon.
 * 
 * Note: this function takes ownership of the message's data sent! 
 * Afterwards the message's data will be void
 *
 * Sending might fail on interrupt.
 * 
 * The path index holds the number of the path to choose. 
 * On -1 the next suitable path(s) are taken.
 * 
 * @param   nKeyId              the key id the message is bound to
 * @param   cMessage            the message to send
 * @param   cAuthContext        the authentication context involved
 * @param   nPath               path index to send
 * @returns true, if successfully sent
 */
bool module::send(qkd::key::key_id nKeyId, 
                  qkd::module::message & cMessage, 
                  qkd::crypto::crypto_context & cAuthContext, 
                  int nPath) {
    
    qkd::module::connection * cCon = nullptr;
    if (is_alice()) cCon = d->cConPeer;
    if (is_bob()) cCon = d->cConListen;
    
    cMessage.key_id() = nKeyId;
    if (!cCon->send_message(cMessage, nPath)) return false;
    d->debug_message(true, cMessage);
    
    cAuthContext << cMessage.data();
    cMessage = qkd::module::message();    

    return true;
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
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ 
                << ": DBus service name 'at.ac.ait.qkd.module." 
                << ss.str() << "' is not valid - impossible to register on DBus";
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
 * set the debug key sync flag
 *
 * @param   bDebug      new debug value for key syncs
 */
void module::set_debug_key_sync(bool bDebug) {
    d->bDebugKeySync = bDebug;
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
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ 
                << ": warning: setting pipeline in working state.";
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
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ 
                << ": refusing to set role to " << nRole << " - unknown role id.";
    }
}


/**
 * set the synchronize key ids flag
 * 
 * @param   bSynchronize    the new synchronize key id flag
 */
void module::set_synchronize_keys(bool bSynchronize) {
    d->cStash->m_bSynchronize = bSynchronize;
}


/**
 * set the synchronize TTL for not in-sync keys
 * 
 * @param   nTTL            the new synchronize TTL in seconds
 */
void module::set_synchronize_ttl(qulonglong nTTL) {
    d->cStash->m_nTTL = nTTL;
}


/**
 * set the number of keys left before terminating (0 --> do not terminate) 
 *
 * The idea is to have a per module counter which decreases
 * when processing a key (no matter if successful or not).
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
    d->cConListen->clear();
    std::string s = sURL.toStdString();
    if (!s.empty()) {
        for (auto & u : connection::split_urls(s)) {
            d->cConListen->add(u, 1, id().toStdString(), "listen");
        }
    }
    else d->cConListen->add("");
}


/**
 * sets a new PEER URL
 *
 * @param   sURL        the new PEER URL
 */
void module::set_url_peer(QString sURL) {
    std::lock_guard<std::mutex> cLock(d->cURLMutex);
    d->cConPeer->clear();
    std::string s = sURL.toStdString();
    if (!s.empty()) {
        for (auto & u : connection::split_urls(s)) {
            d->cConPeer->add(u, 1, id().toStdString(), "listen");
        }
    }
    else d->cConPeer->add("");
}


/**
 * sets a new pipeline INCOMING URL
 *
 * @param   sURL        the new pipe in URL
 */
void module::set_url_pipe_in(QString sURL) {
    std::lock_guard<std::mutex> cLock(d->cURLMutex);
    d->cConPipeIn->clear();
    std::string s = sURL.toStdString();
    if (!s.empty()) {
        for (auto & u : connection::split_urls(s)) {
            d->cConPipeIn->add(u, 10, id().toStdString(), "in");
        }
    }
    else d->cConPipeIn->add("");
}


/**
 * sets a new pipeline OUTGOING URL
 *
 * @param   sURL        the new pipe out URL
 */
void module::set_url_pipe_out(QString sURL) {
    std::lock_guard<std::mutex> cLock(d->cURLMutex);
    d->cConPipeOut->clear();
    std::string s = sURL.toStdString();
    if (!s.empty()) {
        for (auto & u : connection::split_urls(s)) {
            d->cConPipeOut->add(u, 10, id().toStdString(), "out");
        }
    }
    else d->cConPipeOut->add("");
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
    d->cStash->sync();
}


/**
 * get the synchronize key ids flag
 * 
 * @return  the synchronize key id flag
 */
bool module::synchronize_keys() const {
    return d->cStash->m_bSynchronize;
}


/**
 * get the synchronize TTL for not in-sync keys
 * 
 * @return  the synchronize TTL in seconds
 */
qulonglong module::synchronize_ttl() const {
    return d->cStash->m_nTTL;
}


/**
 * stops the module
 * 
 * This is a graceful shutdown
 */
void module::terminate() {

    qkd::utility::debug() << "terminate call received" 
            << " (currently key processing=" << (processing() ? "true" : "false")
            << " , current idle=" << (idle() ? "true" : "false")
            << " , current state=" << state_name(get_state()).toStdString()
            << ")";

    if (d->get_state() == module_state::STATE_TERMINATING) return;
    if (d->get_state() == module_state::STATE_TERMINATED) return;

    if (d->cModuleThread.get_id() == std::this_thread::get_id()) {
        d->release();
    }
    else {
        d->set_state(module_state::STATE_TERMINATING);
        interrupt_worker();
        join();
    }
}


/**
 * this is the entry point of the main thread worker
 */
void module::thread() {

    qkd::utility::debug() << "entering ready state";
    d->debug_config();
    d->set_state(module_state::STATE_READY);
    emit ready();
    work();
    qkd::utility::debug() << "winding down module";
    
    d->release();
    emit terminated();
}


/**
 * number of keys left before terminating (0 --> do not terminate) 
 *
 * The idea is to have a per module counter which decreases
 * when processing a key (no matter if successful or not).
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
    return QString::fromStdString(d->cConListen->urls_string());
}


/**
 * return the URL of the peer connection (where this module connected to)
 * 
 * @return  the URL of the peer connection (where this module connected to)
 */
QString module::url_peer() const {
    return QString::fromStdString(d->cConPeer->urls_string());
}


/**
 * return the URL of incoming Pipe (serving endpoint)
 * 
 * @return  the URL of incoming Pipe (serving endpoint)
 */
QString module::url_pipe_in() const {
    return QString::fromStdString(d->cConPipeIn->urls_string());
}


/**
 * return the URL of outgoing Pipe
 * 
 * @return  the URL of outgoing Pipe
 */
QString module::url_pipe_out() const {
    return QString::fromStdString(d->cConPipeOut->urls_string());
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
 * You may overwrite this method. But this changes module operation
 * dramatically.
 */
void module::work() {

    qkd::module::module_state eState = qkd::module::module_state::STATE_NEW;

    qkd::utility::debug() << "working on incoming keys started";
    
    // main worker loop: get key, create context, process, forward, and check for termination
    do {
        
        d->bProcessing = false;
        
        eState = get_state();
        while (eState == qkd::module::module_state::STATE_READY) eState = wait_for_state_change(eState);
        if (eState != qkd::module::module_state::STATE_RUNNING) break;

        // get a key
        qkd::key::key cKey = qkd::key::key::null();
        if (is_synchronizing()) {
            try {
                synchronize();
                cKey = d->cStash->pick();
            }
            catch (std::exception const & e) {
                qkd::utility::debug() << "Caugth exception while key-sync: " << e.what();
                cKey = qkd::key::key::null();
            }
        }
        if (!cKey.is_null()) {
            qkd::utility::debug() << "key #" << cKey.id() << " is present at peer - picked";
        }
        else {
            
            if (!read(cKey)) {

                if (get_state() != qkd::module::module_state::STATE_RUNNING) break;
                continue;
            }
            
            if (!accept(cKey)) {
                qkd::utility::debug() << "key " << cKey.id() << " is not accepted by this module";
                continue;
            }
            
            eState = get_state();
            while (eState == qkd::module::module_state::STATE_READY) eState = wait_for_state_change(eState);
            if (eState != qkd::module::module_state::STATE_RUNNING) break;
            
            if (is_synchronizing()) {
                d->cStash->push(cKey);
                continue;
            }
        }
        
        d->bProcessing = true;
        
        // create crypto context for retrieved key
        qkd::crypto::crypto_context cIncomingContext = qkd::crypto::context::null_context();
        qkd::crypto::crypto_context cOutgoingContext = qkd::crypto::context::null_context();
        try {
            if (!cKey.meta().sCryptoSchemeIncoming.empty()) {
                qkd::crypto::scheme cScheme(cKey.meta().sCryptoSchemeIncoming);
                cIncomingContext = qkd::crypto::engine::create(cScheme);
            }
        }
        catch (...) {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ 
                    << ": failed to create incoming crypto context for key";
        }
        try {
            if (!cKey.meta().sCryptoSchemeOutgoing.empty()) {
                qkd::crypto::scheme cScheme(cKey.meta().sCryptoSchemeOutgoing);
                cOutgoingContext = qkd::crypto::engine::create(cScheme);
            }
        }
        catch (...) {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ 
                    << ": failed to create outgoing crypto context for key";
        }

        // call the module working method
        workload cWorkload = { qkd::module::work(cKey, cIncomingContext, cOutgoingContext) };
        process(cWorkload);
        d->cLastProcessedKey = std::chrono::system_clock::now();
        
        eState = get_state();
        while (eState == qkd::module::module_state::STATE_READY) eState = wait_for_state_change(eState);
        if (eState != qkd::module::module_state::STATE_RUNNING) break;
        
        // forward all keys processed
        for (auto & w : cWorkload) {
            if (w.bForward) {
                
                w.cKey.meta().sCryptoSchemeIncoming = w.cIncomingContext->scheme().str();
                w.cKey.meta().sCryptoSchemeOutgoing = w.cOutgoingContext->scheme().str();
                if (w.cKey.meta().sCryptoSchemeIncoming == "null") w.cKey.meta().sCryptoSchemeIncoming = "";
                if (w.cKey.meta().sCryptoSchemeOutgoing == "null") w.cKey.meta().sCryptoSchemeOutgoing = "";

                bool bWrittenToNextModule = false;
                do {

                    // the write might fail for EINTR or EAGAIN --> wait or break processing loop
                    // other errors are turned into severe exception
                    bWrittenToNextModule = write(w.cKey, w.nPath);
                    if (!bWrittenToNextModule ) {
                        if (get_state() != qkd::module::module_state::STATE_RUNNING) break;
                        qkd::utility::debug() << "failed to write key to next module in pipe.";
                        std::this_thread::yield();
                    }
                    else {
                        d->cLastProcessedKey = std::chrono::system_clock::now();
                    }

                } while (!bWrittenToNextModule);

                if (get_state() != qkd::module::module_state::STATE_RUNNING) break;
            }
        }

        d->bProcessing = false;
        d->cLastProcessedKey = std::chrono::system_clock::now();
        
        // check for exit
        if (d->nTerminateAfter != 0) {

            d->nTerminateAfter--;
            if (d->nTerminateAfter == 0) {
                qkd::utility::debug() << "reached maximum number of keys to process - winding down";
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
 * nPath holds the path index of the PIPE_OUT connection to
 * write. If nPath == 1 then the framework picks the next
 * suitable path.
 * 
 * You should not need to call this directly. It get's called
 * if process() returns "true".
 * 
 * @param   cKey        key to pass to the next module
 * @return  true, if writing was successful
 */
bool module::write(qkd::key::key const & cKey, int nPath) {

    if (!d->cConPipeOut->write_key(cKey, nPath)) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ 
                << ": failed to send key to next module - key-id: " << cKey.id();
        return false;
    }
    
    d->add_stats_outgoing(cKey);
    if (qkd::utility::debug::enabled()) d->debug_key_push(cKey);
    
    return true;
}


/*
 * engine.cpp
 * 
 * implementation of the Q3P engine
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

#include <iostream>
#include <thread>

// Qt
#include <QtCore/QTimer>
#include <QtDBus/QDBusConnection>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>

// ait
#include <qkd/key/key.h>
#include <qkd/key/key_ring.h>
#include <qkd/crypto/association.h>
#include <qkd/crypto/engine.h>
#include <qkd/q3p/engine.h>
#include <qkd/q3p/mq.h>
#include <qkd/q3p/nic.h>
#include <qkd/utility/dbus.h>
#include <qkd/utility/debug.h>
#include <qkd/utility/syslog.h>

#include "socket_error_strings.h"

#include "protocol/data.h"
#include "protocol/handshake.h"
#include "protocol/load.h"
#include "protocol/load_request.h"
#include "protocol/store.h"

// DBus integration
#include "db_dbus.h"
#include "engine_dbus.h"
#include "mq_dbus.h"
#include "nic_dbus.h"

using namespace qkd::q3p;


// ------------------------------------------------------------
// defs

#define MODULE_DESCRIPTION      "This is the qkd-keystore QKD Module."
#define MODULE_ORGANISATION     "(C)opyright 2012-2016 AIT Austrian Institute of Technology, http://www.ait.ac.at"


// ------------------------------------------------------------
// decl


/**
 * the engine pimpl
 */
class qkd::q3p::engine_instance::engine_data {
    
    
public:
    
    
    /**
     * ctor
     * 
     * @param   cDBus       the DBus connection to use
     */
    engine_data(QDBusConnection cDBus) : m_cDBus(cDBus), m_bReconnect(false), m_nPeerPort(0) {
        
        m_cAssociationDefinition.sAuthenticationIncoming = "evhash-96";
        m_cAssociationDefinition.sAuthenticationOutgoing = "evhash-96";
        m_cAssociationDefinition.sEncryptionIncoming = "xor";
        m_cAssociationDefinition.sEncryptionOutgoing = "xor";
        
        m_bMaster = false;
        m_bSlave = false;
        
        m_eLinkState = engine_state::ENGINE_INIT;
        
        m_bConnected = false;
        
        m_cServer = nullptr;
        m_cSocket = nullptr;
        
        m_cProtocol.cData = nullptr;
        m_cProtocol.cHandshake = nullptr;
        m_cProtocol.cLoad = nullptr;
        m_cProtocol.cLoadRequest = nullptr;
        m_cProtocol.cStore = nullptr;
        
        m_nChannelId = 0;
        
        m_cTimer = nullptr;
    };
    
    QDBusConnection m_cDBus;                        /**< the DBus connection used */
    
    QString m_sLinkId;                              /**< engine id */
    QString m_sNode;                                /**< node id */
    QString m_sDBusObjectPath;                      /**< object path on DBus */
    
    bool m_bMaster;                                 /**< master flag */
    bool m_bSlave;                                  /**< slave flag */
    
    engine_state m_eLinkState;                      /**< engine state */
    
    bool m_bConnected;                              /**< connected flag */
    QTcpServer * m_cServer;                         /**< listen server */
    QAbstractSocket * m_cSocket;                    /**< connection to peer */
    QByteArray m_cRecvBuffer;                       /**< the data received so far but not yet managed */
    
    bool m_bReconnect;                              /**< flag to reconnect */
    QHostAddress m_cPeerAddress;                    /**< peer address we connected to */
    uint32_t m_nPeerPort;                           /**< peer port we are connected to */

    /**
     * Q3P protocol instances
     */
    struct {
        
        protocol::protocol * cData;                 /**< data protocol */
        protocol::protocol * cHandshake;            /**< handshake protocol */
        protocol::protocol * cLoad;                 /**< load protocol */
        protocol::protocol * cLoadRequest;          /**< load-request protocol */
        protocol::protocol * cStore;                /**< store protocol */
        
    } m_cProtocol;
    
    qkd::q3p::mq m_cMQ;                             /**< the message queue */
    qkd::q3p::nic m_cNIC;                           /**< network interface "card" */
    std::string m_sNICIP4Local;                     /**< local IP4 address */
    std::string m_sNICIP4Remote;                    /**< remote IP4 address */
    
    
    // TODO: Stefan
    //qkd::q3p::ipsec m_cIPSec;                       /**< IPSec implementation */
    
    key_db m_cCommonStoreDB;                        /**< the Common Store Key DB */
    key_db m_cIncomingDB;                           /**< the Incoming Key DB */
    key_db m_cOutgoingDB;                           /**< the Outgoing Key DB */
    key_db m_cApplicationDB;                        /**< the Application Key DB */
    
    
    /**
     * the (next) crypto association 
     */
    qkd::crypto::association::association_definition m_cAssociationDefinition;
    
    uint16_t m_nChannelId;                          /**< current channel ID */
    
    /**
     * current active channels 
     */
    std::map<uint16_t, qkd::q3p::channel> m_cChannelMap;    
    
    qkd::key::key m_cInitialSecret;                 /**< the initial secret */
    
    QTimer * m_cTimer;                              /**< timer for protocol checks */

};


// ------------------------------------------------------------
// vars


/**
 * the known engines
 */
static engine_map g_cEngines;


// ------------------------------------------------------------
// code


/**
 * ctor
 * 
 * @param   sNode       id of node
 * @param   sId         id of the engine
 */
engine_instance::engine_instance(QString const & sNode, QString const & sId) : qkd::module::module("keystore", qkd::module::module_type::TYPE_KEYSTORE, MODULE_DESCRIPTION, MODULE_ORGANISATION) {
    
    d = std::shared_ptr<qkd::q3p::engine_instance::engine_data>(new qkd::q3p::engine_instance::engine_data(qkd::utility::dbus::qkd_dbus()));

    d->m_sNode = sNode;
    d->m_sLinkId = sId;
    d->m_sDBusObjectPath = QString("/Link/") + sId;
    d->m_bReconnect = false;
    
    module::set_url_listen("");
    module::set_url_peer("");
    module::set_url_pipe_in("");
    module::set_url_pipe_out("");
    module::set_synchronize_keys(false);
    module::set_synchronize_ttl(0);
    
    setup_buffers();
    
    // setup timer
    d->m_cTimer = new QTimer(this);
    d->m_cTimer->setInterval(250);
    QObject::connect(d->m_cTimer, SIGNAL(timeout()), this, SLOT(q3p_timeout()));
    
    // register Object on DBus
    new LinkAdaptor(this);
    bool bSuccess = d->m_cDBus.registerObject(d->m_sDBusObjectPath, this);
    if (!bSuccess) {
        QString sMessage = QString("Failed to register DBus object \"") + d->m_sDBusObjectPath + "\"";
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << sMessage.toStdString();
    }
    else {
        qkd::utility::syslog::info() << "link registered on DBus as \"" << d->m_sDBusObjectPath.toStdString() << "\"";
    }
    
    // start timer
    d->m_cTimer->start();
}


/**
 * dtor
 */
engine_instance::~engine_instance() {
}


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
bool engine_instance::accept(qkd::key::key const & cKey) const {
    
    if (cKey.meta().eKeyState == qkd::key::key_state::KEY_STATE_DISCLOSED) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "received key #" << cKey.id() << " has state: DISCLOSED. unacceptable. discarded.";
        return false;
    }
    
    // if the key has not been authenticated, then we should state a warning
    if (cKey.meta().eKeyState != qkd::key::key_state::KEY_STATE_AUTHENTICATED) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "received key #" << cKey.id() << " not in state: AUTHENTICATED. warning: unauthenticated keys bear a security risk!";
        return false;
    }
    
    return true;
}


/**
 * request keys from the application buffer
 * 
 * the given key material is extracted from the application buffers
 * on both sides based on the given application identifier and size.
 * 
 * The method returns successfully if the requested key material has
 * been placed into the cKeys object on both sides (local and remote).
 * 
 * the number of bytes requested must be a multiple of the application
 * buffer key quantum (usually 32 bits == 4 bytes).
 * 
 * the method fails:
 *  - if we lack a peer and/or if we have insufficient key material left in the application buffer
 *  - if the peer didn't acquire keys within the timeout
 * 
 * @param   cKeys           this will receive the key material (any previous content will be zapped)
 * @param   nAppId          the application id: this identifies the request on both sides
 * @param   nBytes          number of bytes requested
 * @param   nTimeout        timeout in millisecond to wait for peer to acquire as well
 * @return  true, if successful
 */
bool engine_instance::acquire_keys(UNUSED qkd::key::key_ring & cKeys, UNUSED uint64_t nAppId, UNUSED uint64_t nBytes, UNUSED std::chrono::milliseconds nTimeout) {

    // TODO; code me
    
    return false;
}


/**
 * access to the current application buffer
 * 
 * @return  the current application buffer
 */
key_db & engine_instance::application_buffer() {
    return d->m_cApplicationDB;
}


/**
 * access to the current application buffer
 * 
 * @return  the current application buffer
 */
key_db const & engine_instance::application_buffer() const {
    return d->m_cApplicationDB;
}


/**
 * the current (next) authentication scheme for incoming messages
 * 
 * @return  the current (next) authentication scheme for incoming
 */
QString engine_instance::authentication_scheme_incoming() const {
    return QString::fromStdString(d->m_cAssociationDefinition.sAuthenticationIncoming);
}


/**
 * the current (next) authentication scheme for outgoing messages
 * 
 * @return  the current (next) authentication scheme for outgoing messages
 */
QString engine_instance::authentication_scheme_outgoing() const {
    return QString::fromStdString(d->m_cAssociationDefinition.sAuthenticationOutgoing);
}


/**
 * calculate new state value
 */
void engine_instance::calculate_state() {
    
    engine_state eNewState = engine_state::ENGINE_INIT;
    engine_state eOldState = d->m_eLinkState;
    
    if (db_opened()) {
        
        eNewState = engine_state::ENGINE_OPEN;
        if (d->m_cServer != nullptr) {
            
            eNewState = engine_state::ENGINE_CONNECTING;
            if (d->m_cProtocol.cHandshake) {
                eNewState = engine_state::ENGINE_HANDSHAKE;
            }
            else if (connected()) {
                eNewState = engine_state::ENGINE_CONNECTED;
            }
        }
    }
    
    if (eNewState == eOldState) return;

    d->m_eLinkState = eNewState;
    emit state_changed((unsigned int)d->m_eLinkState);
}


/**
 * get a channel
 * 
 * if the nChannelId is 0 the current channel is fetched
 * 
 * @param   nChannelId      if of the channel
 * @return  a known channel channel
 */
qkd::q3p::channel & engine_instance::channel(uint16_t nChannelId) {
    
    // decide which channel to fetch
    if (!nChannelId) nChannelId = d->m_nChannelId;
    
    static qkd::q3p::channel cNullChannel;
    if (nChannelId == 0) return cNullChannel;
    if (d->m_cChannelMap.find(nChannelId) == d->m_cChannelMap.end()) return cNullChannel;
    
    return d->m_cChannelMap[nChannelId];
}


/**
 * get a channel
 * 
 * if the nChannelId is 0 the current channel is fetched
 * 
 * @param   nChannelId      if of the channel
 * @return  a known channel channel
 */
qkd::q3p::channel const & engine_instance::channel(uint16_t nChannelId) const {
    
    // decide which channel to fetch
    if (!nChannelId) nChannelId = d->m_nChannelId;
    
    static qkd::q3p::channel cNullChannel;
    if (nChannelId == 0) return cNullChannel;
    if (d->m_cChannelMap.find(nChannelId) == d->m_cChannelMap.end()) return cNullChannel;
    
    return d->m_cChannelMap[nChannelId];
}


/**
 * returns a string describing the current charge states of the buffers
 * 
 * this is for debugging. The string has the form
 *      < < 'C',count, amount >,< 'I',count, amount >,< 'O',count, amount >,< 'A',count, amount > >
 *
 * having the current charge ("count") and the total ("amount") of the
 *      common store: "C"
 *      incoming buffer: "I"
 *      outgoing buffer: "O"
 *      application buffer: "A"
 * 
 * this is for debugging
 * 
 * @return  a string describing the current buffer states
 */
std::string engine_instance::charge_string() const {
    
    if (!connected()) return std::string("<not connected>");
    
    std::stringstream ss;
    ss << "<";
    
    ss << "<C: " << common_store()->count() << "/" << common_store()->amount() << ">, ";
    ss << "<I: " << incoming_buffer()->count() << "/" << incoming_buffer()->amount() << ">, ";
    ss << "<O: " << outgoing_buffer()->count() << "/" << outgoing_buffer()->amount() << ">, ";
    ss << "<A: " << application_buffer()->count() << "/" << application_buffer()->amount() << ">";
    
    ss << ">";
    
    return ss.str();
}


/**
 * closes an engine
 */
void engine_instance::close() {
    
    // wind down module thread
    interrupt_worker();
    std::this_thread::yield();
    terminate();
    
    // disconnect link peer
    disconnect();
    
    // unmount database
    close_db();

    // unregister ourselves
    auto iter = g_cEngines.find(d->m_sLinkId);
    if (iter != g_cEngines.end()) unregister_engine((*iter).second);
}


/**
 * closes all known engines
 */
void engine_instance::close_all() {
    
    // kick the head of the container until empty
    while (g_cEngines.size()) {
        engine_map::iterator iter = g_cEngines.begin();
        (*iter).second->close();
    }
}


/**
 * closes an opened key-DB
 */
void engine_instance::close_db() {
    
    // if we are connected: disconnect first
    if (connected()) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "won't close database while connected";
        return;
    }
    
    // only close what is open
    if (db_opened()) {
        
        QString sDBUrl = d->m_cCommonStoreDB->url();
        d->m_cCommonStoreDB->close();
        d->m_cCommonStoreDB = std::shared_ptr<db>();
        qkd::utility::syslog::info() << "database " << sDBUrl.toStdString() << " closed";
        
        // unregister object from DBus
        QString sCommonStoreObjectPath = d->m_sDBusObjectPath + "/CommonStore";
        d->m_cDBus.unregisterObject(sCommonStoreObjectPath, QDBusConnection::UnregisterTree);
        
        // tell environment
        emit db_closed(sDBUrl);
    }
    
    // state switch?
    calculate_state();
}


/**
 * access to the common store
 * 
 * @return  the common store
 */
key_db & engine_instance::common_store() {
    return d->m_cCommonStoreDB;
}


/**
 * access to the common store
 * 
 * @return  the common store
 */
key_db const & engine_instance::common_store() const {
    return d->m_cCommonStoreDB;
}


/**
 * configure the IPSec connection
 * 
 * @param   sIPSecConfiguration         IPSec Configuration string
 */
void engine_instance::configure_ipsec(QString sIPSecConfiguration) {
    
    // TODO: Stefan
    std::cerr << "engine_instance::configure_ipsec(): Stefan: IPSec configuration is: " << sIPSecConfiguration.toStdString() << std::endl;
}


/**
 * connect to a peer engine
 * 
 * The peer URI has the form
 * 
 *      scheme://address:port
 * 
 * e.g.
 * 
 *      tcp://127.0.0.1:10000
 * 
 * @param   sURI            the URI of the peer key-store
 * @param   cSecret         initial secret
 */
void engine_instance::connect(QString sURI, QByteArray cSecret) {
    
    // sanity checks
    if (sURI.length() == 0) return;
    
    // parse URI
    QUrl cURL(sURI);
    if (cURL.scheme() != "tcp") {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to connect: unknown scheme in URI: '" << sURI.toStdString() << "'";
        return;
    }
    
    unsigned int nPort = 0;
    if (cURL.port() == -1) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to connect: no port given";
        return;
    }
    nPort = (unsigned int)cURL.port();

    QString sAddress = cURL.host();
    if (sAddress.length() == 0) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to connect: no host given";
        return;
    }
    
    // turn any (possible) hostname into an IP address
    std::set<std::string> cAddressesForHost = qkd::utility::environment::host_lookup(sAddress.toStdString());
    if (cAddressesForHost.empty()) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to connect: unable to get IP address for hostname: " << sAddress.toStdString();
        return;
    }
    
    // pick the first
    sAddress = QString::fromStdString(*cAddressesForHost.begin());
    
    // we don't proceed if we ain't got a DB open
    if (!db_opened()) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "won't connect to peer without an opened database";
        return;
    }

    // don't proceed if we ain't got enough keys
    if (d->m_cCommonStoreDB->count() < MIN_KEYS_IN_DB) {
        QString sMessage = QString("insufficient keys in database (minimum is %1): inject keys first in order to connect").arg(MIN_KEYS_IN_DB);
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << sMessage.toStdString();
        return;
    }
    
    // syslog
    if (d->m_cSocket) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "connection live or connection attempt ongoing - refusing - disconnect first";
        return;
    }

    // check if we have enough shared secrets to start
    uint64_t nKeyConsumptionPerRound = qkd::crypto::association::key_consumption(d->m_cAssociationDefinition);
    if ((uint64_t)cSecret.size() < nKeyConsumptionPerRound) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "not enough shared secret bytes provided. needed min.: " << nKeyConsumptionPerRound << " bytes, provided: " << cSecret.size() << " bytes.";
        return;
    }
    d->m_cInitialSecret = qkd::key::key(0, qkd::utility::memory::duplicate((unsigned char * const)cSecret.data(), cSecret.size()));
    
    // set module role
    set_role((qulonglong)qkd::module::module_role::ROLE_ALICE);

    // create a new tcp socket listener
    d->m_cSocket = new QTcpSocket(this);
    QObject::connect(d->m_cSocket, SIGNAL(connected()), SLOT(socket_connected()));
    QObject::connect(d->m_cSocket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(socket_error(QAbstractSocket::SocketError)));
    QObject::connect(d->m_cSocket, SIGNAL(readyRead()), SLOT(socket_ready_read()));
    
    // syslog
    QString sMessage = QString("trying to connect peer key-store at \"tcp://%1:%2\"").arg(sAddress).arg(QString::number(nPort));
    qkd::utility::syslog::info() << sMessage.toStdString();
    
    // connect!
    d->m_cSocket->connectToHost(sAddress, nPort);
    
    // state switch
    calculate_state();
}


/**
 * check if we are connected with our peer
 * 
 * @return  true, if we are connected
 */
bool engine_instance::connected() const {
    return d->m_bConnected;
}


/**
 * this object MUST be created on the heap
 * 
 * @param   sNode       id of node
 * @param   sId         if of the new engine
 * @return  an instance on the heap
 * @throws  engine_already_registered
 * @throws  engine_invalid_id
 */
engine engine_instance::create(QString const & sNode, QString const & sId) {
    
    // check name
    if (!qkd::utility::dbus::valid_service_name_particle(sId.toStdString())) {
        throw std::invalid_argument("create engine with invalid id");
    }
    
    // try to register engine
    engine cEngine = std::shared_ptr<qkd::q3p::engine_instance>(new qkd::q3p::engine_instance(sNode, sId));
    if (!register_engine(cEngine)) {
        throw std::runtime_error("engine with this id already registered");
    }
    
    return cEngine;
}


/**
 * data protocol failed
 * 
 * @param   nReason     reason why it failed (protocol::protocol_error code)
 */
void engine_instance::data_failed(uint8_t nReason) {
    std::string sError = qkd::q3p::protocol::protocol::protocol_error_description((qkd::q3p::protocol::protocol_error)nReason).toStdString();
    qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "DATA protocol failed! Reason: " << nReason << " - " << sError;
}


/**
 * data protocol succeeded
 */
void engine_instance::data_success() {
}


/**
 * check if we have an opened Key-DB
 * 
 * @return  true, if we are operating on a key-db
 */
bool engine_instance::db_opened() const {
    return (d->m_cCommonStoreDB.get() != nullptr);
}


/**
 * wind down any connection
 */
void engine_instance::disconnect() {

    if (connected()) qkd::utility::syslog::info() << "disconnecting from peer";
    
    // this has been called by the user (this is a DBus method)
    // so cancel reconnection feature
    d->m_bReconnect = false;
    
    // stop module worker
    interrupt_worker();
    join();

    shutdown_nic();
    shutdown_mq();
    
    if (d->m_cProtocol.cData) d->m_cProtocol.cData->deleteLater();
    d->m_cProtocol.cData = nullptr;
    if (d->m_cProtocol.cLoad) d->m_cProtocol.cLoad->deleteLater();
    d->m_cProtocol.cLoad = nullptr;
    if (d->m_cProtocol.cLoadRequest) d->m_cProtocol.cLoadRequest->deleteLater();
    d->m_cProtocol.cLoadRequest = nullptr;
    if (d->m_cProtocol.cStore) d->m_cProtocol.cStore->deleteLater();
    d->m_cProtocol.cStore = nullptr;
    
    shutdown_buffers();
    
    if (d->m_cSocket) {
        d->m_cSocket->disconnectFromHost();
        delete d->m_cSocket;
        d->m_cSocket = nullptr;
    }
    
    shutdown_channels();
    
    d->m_cRecvBuffer = QByteArray();
    d->m_bConnected = false;
    
    emit connection_lost();
    calculate_state();
}


/**
 * the current (next) encryption scheme for incoming messages
 * 
 * @return  the current (next) encryption scheme for incoming messages
 */
QString engine_instance::encryption_scheme_incoming() const {
    return QString::fromStdString(d->m_cAssociationDefinition.sEncryptionIncoming);
}


/**
 * the current (next) encryption scheme for outgoing messages
 * 
 * @return  the current (next) encryption scheme for outgoing messages
 */
QString engine_instance::encryption_scheme_outgoing() const {
    return QString::fromStdString(d->m_cAssociationDefinition.sEncryptionOutgoing);
}


/**
 * list of known engines
 * 
 * @return  the list of known engine instance
 */
engine_map const & engine_instance::engines() {
    return g_cEngines;
}


/**
 * retrieves a certain engine
 * 
 * @param   sId         if of the engine
 * @return  an engine
 */
engine engine_instance::get(QString const & sId) {
    auto iter = g_cEngines.find(sId);
    return (*iter).second;
}


/**
 * handshake failed
 * 
 * @param   nReason     reason why it failed (protocol::protocol_error code)
 */
void engine_instance::handshake_failed(uint8_t nReason) {

    // syslog
    QString sError = qkd::q3p::protocol::protocol::protocol_error_description((qkd::q3p::protocol::protocol_error)nReason);
    QString sMessage = QString("handshake with peer failed, error %1 (%2)").arg(nReason).arg(sError);
    qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << sMessage.toStdString();
    
    d->m_cProtocol.cHandshake = nullptr;
    d->m_cSocket->disconnectFromHost();
    d->m_cSocket->deleteLater();
    d->m_cSocket = nullptr;
    d->m_bConnected = false;
    
    // tell the environment
    emit connection_lost();
    
    // state switch
    calculate_state();
}


/**
 * handshake succeeded
 */
void engine_instance::handshake_success() {
    
    // when the handshake succeeded we set up our internal buffers
    // and create the very first channel along its own dedicated
    // crypto association (having auth-I/O and encr-I/O contexts)
    //
    // This is the birth of a new fully working link.
    //

    // syslog
    QString sMessage = QString("handshake succeeded");
    qkd::utility::syslog::info() << sMessage.toStdString();
    
    // remember address and port
    QString sAddress;
    uint16_t nPort;
    if (is_alice()) {
        sAddress = d->m_cProtocol.cHandshake->socket()->peerAddress().toString();
        nPort = d->m_cProtocol.cHandshake->socket()->peerPort();
    }
    else {
        sAddress = d->m_cProtocol.cHandshake->socket()->localAddress().toString();
        nPort = d->m_cProtocol.cHandshake->socket()->localPort();
    }
    
    d->m_cProtocol.cHandshake->deleteLater();
    d->m_cProtocol.cHandshake = nullptr;
    d->m_bConnected = true;
    
    // setup our buffers
    setup_buffers();
    
    // prepare our first crypto association
    unsigned char * const cSecret = d->m_cInitialSecret.data().get();
    uint64_t nIndex = 0;
    
    // tweak crypto schemes: cut off any old key values (better safe than sorry)
    uint64_t nPos;
    if ((nPos = d->m_cAssociationDefinition.sAuthenticationIncoming.find(':')) != std::string::npos) d->m_cAssociationDefinition.sAuthenticationIncoming.resize(nPos);
    if ((nPos = d->m_cAssociationDefinition.sAuthenticationOutgoing.find(':')) != std::string::npos) d->m_cAssociationDefinition.sAuthenticationOutgoing.resize(nPos);
    if ((nPos = d->m_cAssociationDefinition.sEncryptionIncoming.find(':')) != std::string::npos) d->m_cAssociationDefinition.sEncryptionIncoming.resize(nPos);
    if ((nPos = d->m_cAssociationDefinition.sEncryptionOutgoing.find(':')) != std::string::npos) d->m_cAssociationDefinition.sEncryptionOutgoing.resize(nPos);
    
    // register what we need initially
    uint64_t nInitKeyAuthenticationIncoming = 0;
    uint64_t nInitKeyAuthenticationOutgoing = 0;
    uint64_t nInitKeyEncryptionIncoming = 0;
    uint64_t nInitKeyEncryptionOutgoing = 0;
    
    // fetch the proper init key sizes
    try {
        
        // dry run: without initial keys
        qkd::crypto::association cAssociation(d->m_cAssociationDefinition);
        
        nInitKeyAuthenticationIncoming = cAssociation.authentication().cIncoming->init_key_size();
        nInitKeyAuthenticationOutgoing = cAssociation.authentication().cOutgoing->init_key_size();
        nInitKeyEncryptionIncoming = cAssociation.encryption().cIncoming->init_key_size();
        nInitKeyEncryptionOutgoing = cAssociation.encryption().cOutgoing->init_key_size();
    }
    catch (...) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to setup initial crypto association";
        disconnect();
        return;
    }
    
    // now add initial keys to the association definition forming
    // a "ALGORITHM-VARIANT:INIT-KEY" crypto scheme string for each
    // crypto context we need (auth in/out and encr in/out)

    // fix schemes: authentication incoming
    if (nInitKeyAuthenticationIncoming != 0) {
        d->m_cAssociationDefinition.sAuthenticationIncoming += ":" + qkd::utility::memory::wrap(cSecret + nIndex, nInitKeyAuthenticationIncoming).as_hex();
        nIndex += nInitKeyAuthenticationIncoming;
    }
        
    // fix schemes: authentication outgoing
    if (nInitKeyAuthenticationOutgoing != 0) {
        d->m_cAssociationDefinition.sAuthenticationOutgoing += ":" + qkd::utility::memory::wrap(cSecret + nIndex, nInitKeyAuthenticationOutgoing).as_hex();
        nIndex += nInitKeyAuthenticationOutgoing;
    }
        
    // fix schemes: encryption incoming
    if (nInitKeyEncryptionIncoming != 0) {
        d->m_cAssociationDefinition.sEncryptionIncoming += ":" + qkd::utility::memory::wrap(cSecret + nIndex, nInitKeyEncryptionIncoming).as_hex();
        nIndex += nInitKeyEncryptionIncoming;
    }
        
    // fix schemes: encryption outgoing
    if (nInitKeyEncryptionOutgoing != 0) {
        d->m_cAssociationDefinition.sEncryptionOutgoing += ":" + qkd::utility::memory::wrap(cSecret + nIndex, nInitKeyEncryptionOutgoing).as_hex();
        nIndex += nInitKeyEncryptionOutgoing;
    }
    
    // swap if we are slave
    if (slave()) {
        std::swap(d->m_cAssociationDefinition.sAuthenticationIncoming, d->m_cAssociationDefinition.sAuthenticationOutgoing);
        std::swap(d->m_cAssociationDefinition.sEncryptionIncoming, d->m_cAssociationDefinition.sEncryptionOutgoing);
    }
    
    // dump the rest of the initial secret into the buffers
    // this will get into our in/out buffer
    // so the very first auth-tags are also generated with the
    // help of the initial shared secret
    uint64_t nRest = d->m_cInitialSecret.data().size() - nIndex;
    if (nRest % 2) nRest--;
    nRest /= 2;
    qkd::key::key_ring cKeyRing(nRest);
    cKeyRing << qkd::key::key(0, qkd::utility::memory::duplicate(cSecret + nIndex, d->m_cInitialSecret.data().size() - nIndex));
    
    // the key ring contains now at least 2 keys (3 if the former initial secret has been odd)
    // place the first in the Incoming on master and the second into Outgoing on master
    // at slave's side it is vice versa
    
    // set up a key ring buffer feeder
    qkd::key::key_ring cKeyBufferA(d->m_cIncomingDB->quantum());
    qkd::key::key_ring cKeyBufferB(d->m_cOutgoingDB->quantum());
    cKeyBufferA << cKeyRing.at(0);
    cKeyBufferB << cKeyRing.at(1);
    if (master()) {
        
        qkd::key::key_id nKeyId = 0;

        // dump the keys into the buffers (master version)
        for (uint64_t i = 0; i < cKeyBufferA.size(); i++) {
            nKeyId = d->m_cIncomingDB->insert(cKeyBufferA.at(i));
            d->m_cIncomingDB->set_real_sync(nKeyId);
            nKeyId = d->m_cOutgoingDB->insert(cKeyBufferB.at(i));
            d->m_cOutgoingDB->set_real_sync(nKeyId);
        }
    }
    else {
        
        qkd::key::key_id nKeyId = 0;
        
        // dump the keys into the buffers (slave version)
        for (uint64_t i = 0; i < cKeyBufferA.size(); i++) {
            nKeyId = d->m_cIncomingDB->insert(cKeyBufferB.at(i));
            d->m_cIncomingDB->set_real_sync(nKeyId);
            nKeyId = d->m_cOutgoingDB->insert(cKeyBufferA.at(i));
            d->m_cOutgoingDB->set_real_sync(nKeyId);
        }
    }
    
    // tell the world
    d->m_cIncomingDB->emit_charge_change(cKeyBufferA.size(), 0);
    d->m_cOutgoingDB->emit_charge_change(cKeyBufferB.size(), 0);
    qkd::utility::debug() << "current charges: " << charge_string();
    
    // setup the channel
    setup_channel();
    
    // create the protocol instances
    d->m_cProtocol.cData = new protocol::data(d->m_cSocket, this);
    d->m_cProtocol.cLoad = new protocol::load(d->m_cSocket, this);
    d->m_cProtocol.cLoadRequest = new protocol::load_request(d->m_cSocket, this);
    d->m_cProtocol.cStore = new protocol::store(d->m_cSocket, this);
    
    // connect slots
    QObject::connect(d->m_cProtocol.cData, SIGNAL(failed(uint8_t)), SLOT(data_failed(uint8_t)));
    QObject::connect(d->m_cProtocol.cData, SIGNAL(success()), SLOT(data_success()));
    QObject::connect(d->m_cProtocol.cLoad, SIGNAL(failed(uint8_t)), SLOT(load_failed(uint8_t)));
    QObject::connect(d->m_cProtocol.cLoad, SIGNAL(success()), SLOT(load_success()));
    QObject::connect(d->m_cProtocol.cLoadRequest, SIGNAL(failed(uint8_t)), SLOT(load_request_failed(uint8_t)));
    QObject::connect(d->m_cProtocol.cLoadRequest, SIGNAL(success()), SLOT(load_request_success()));
    QObject::connect(d->m_cProtocol.cStore, SIGNAL(failed(uint8_t)), SLOT(store_failed(uint8_t)));
    QObject::connect(d->m_cProtocol.cStore, SIGNAL(success()), SLOT(store_success()));
        
    // setup ipsec
    setup_ipsec();
    
    // setup nic
    setup_nic();
    
    // setup mq
    setup_mq();

    // ensure we have a pipe-in (default to TEMP_PATH/qkd/engine_id.link_id)
    QString sPipeIn = url_pipe_in();
    if (sPipeIn.isEmpty()) {
        std::stringstream ss;
        ss << "ipc://" << qkd::utility::environment::temp_path().string() << "/qkd/" << id().toStdString() << "." << link_id().toStdString();
        sPipeIn = QString::fromStdString(ss.str());
    }
    
    QString sConnect = QString("tcp://") + sAddress + ":" + QString::number(nPort + 1); 
    
    set_synchronize_keys(false);
    set_synchronize_ttl(0);
    set_url_pipe_in(sPipeIn);
    if (is_alice()) set_url_peer(sConnect);
    else set_url_listen(sConnect);
    start_later();
    
    calculate_state();
}


/**
 * access to the current incoming buffer
 * 
 * @return  the current incoming buffer
 */
key_db & engine_instance::incoming_buffer() {
    return d->m_cIncomingDB;
}


/**
 * access to the current incoming buffer
 * 
 * @return  the current incoming buffer
 */
key_db const & engine_instance::incoming_buffer() const {
    return d->m_cIncomingDB;
}


/**
 * insert a key into the DB (without peer interaction!)
 * 
 * @param   cSecretBits     the key to insert
 */
void engine_instance::inject(QByteArray cSecretBits) {
    
    qkd::utility::debug() << "injecting keys: " << cSecretBits.size() << " bytes";
    
    auto nStart = std::chrono::high_resolution_clock::now();
    
    if (!db_opened()) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "won't inject keys without an opened database";
        return;
    }
    
    if (connected()) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "won't inject keys while connected. disconnect first.";
        return;
    }
    
    qkd::key::key_ring cKeyRing(d->m_cCommonStoreDB->quantum());
    qkd::key::key cKey(0, qkd::utility::memory::duplicate((unsigned char *)cSecretBits.data(), cSecretBits.size()));
    cKeyRing << cKey;
    
    uint64_t nKeysInserted = 0;
    for (auto & cKey : cKeyRing) {
        
        if (cKey.size() == d->m_cCommonStoreDB->quantum()) {
            
            qkd::key::key_id nKeyId = d->m_cCommonStoreDB->insert(cKey);
            if (nKeyId != 0) {
                
                d->m_cCommonStoreDB->set_injected(nKeyId);
                d->m_cCommonStoreDB->set_real_sync(nKeyId);
                nKeysInserted++;
            }
            else {
                qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to injected key into database";
            }
        }
        else {
            
            if (qkd::utility::debug::enabled()) {
                QString sMessage = QString("dropping %1 bytes of key material - not a key quantum (%2 bytes)").arg(cKey.size()).arg(d->m_cCommonStoreDB->quantum());
                qkd::utility::debug() << sMessage.toStdString();
            }
        }
    }
        
    auto nStop = std::chrono::high_resolution_clock::now();
    auto nTimeDiff = std::chrono::duration_cast<std::chrono::milliseconds>(nStop - nStart);
    
    QString sMessage = QString("injected %1 keys into database in %2 millisec").arg(nKeysInserted).arg(nTimeDiff.count());
    qkd::utility::syslog::info() << sMessage.toStdString();
    
    d->m_cCommonStoreDB->emit_charge_change(nKeysInserted, 0);
}


/**
 * insert a key identified by an URL into the DB (without peer interaction!)
 * 
 * @param   sURL            the URL to insert
 */
void engine_instance::inject_url(QString sURL) {
    
    qkd::utility::debug() << "injecting keys from url: " << sURL.toStdString();
    
    QUrl cURL(sURL, QUrl::TolerantMode);
    if (cURL.isLocalFile()) {
        
        QString sFileName = cURL.toLocalFile();
        QFile cFile(sFileName);
        if (!cFile.open(QIODevice::ReadOnly)) {
            QString sMessage = QString("failed to open file \"%1\"").arg(sFileName);
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << sMessage.toStdString();
            return;
        }
        
        QByteArray cKeyData = cFile.readAll();
        inject(cKeyData);
    }
    else {
        QString sMessage = QString("failed to injected keys by url: \"%1\" - unknown scheme").arg(sURL);
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << sMessage.toStdString();
    }
}


/**
 * get the engine's id
 * 
 * @return  the id of the current engine
 */
QString const & engine_instance::link_id() const {
    return d->m_sLinkId;
}


/**
 * our local public address we are serving
 * 
 * @return  the address of our local server listening
 */
QString engine_instance::link_local() const {
    
    if (d->m_cServer == nullptr) return QString();

    QString sServerClass = d->m_cServer->metaObject()->className();
    QString sScheme;
    if (sServerClass == "QTcpServer") sScheme = "tcp://";
    else
    if (sServerClass == "QUdpServer") sScheme = "udp://";
    
    return QString("%1%2:%3").arg(sScheme).arg(d->m_cServer->serverAddress().toString()).arg(d->m_cServer->serverPort());
}


/**
 * the address of the connected peer key-store
 * 
 * @return  the address of the connected peer key-store
 */
QString engine_instance::link_peer() const {
    
    if (d->m_cSocket == nullptr) return QString();
    
    QString sSocketClass = d->m_cSocket->metaObject()->className();
    QString sScheme;
    if (sSocketClass == "QTcpSocket") sScheme = "tcp://";
    else
    if (sSocketClass == "QUdpSocket") sScheme = "udp://";
    
    return QString("%1%2:%3").arg(sScheme).arg(d->m_cSocket->peerAddress().toString()).arg(d->m_cSocket->peerPort());
}


/**
 * get the current key store state
 * 
 * @return  the current key store state
 */
unsigned int engine_instance::link_state() const {
    return (unsigned int)d->m_eLinkState;
}


/**
 * return a human readable key store state description
 * 
 * @return  a human readable key store state description
 */
QString engine_instance::link_state_description(unsigned int nState) {
    
    engine_state eState = (engine_state)nState;
    
    switch (eState) {
        
    case engine_state::ENGINE_INIT: return "initial";
    case engine_state::ENGINE_OPEN: return "database open";
    case engine_state::ENGINE_CONNECTING: return "connecting to peer";
    case engine_state::ENGINE_HANDSHAKE: return "handshaking with peer";
    case engine_state::ENGINE_CONNECTED: return "connected with peer";
    
    }
    
    return "unkown state";
}


/**
 * start listening on a specified address and port
 * 
 * The listen URI has the form
 * 
 *      scheme://address:port
 * 
 * e.g.
 * 
 *      tcp://127.0.0.1:10000
 * 
 * @param   sURI            the address to listen on (IP or hostname)
 * @param   cSecret         initial secret
 */
void engine_instance::listen(QString sURI, QByteArray cSecret) {
    
    qkd::utility::debug() << "start public listening on: " << sURI.toStdString();
    
    if (sURI.length() == 0) return;
    QUrl cURL(sURI);
    QString sScheme = cURL.scheme();
    if (sScheme != "tcp") {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to listen: unknown scheme in URI: '" << sURI.toStdString() << "'";
        return;
    }
    
    unsigned int nPort = 0;
    if (cURL.port() == -1) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to listen: no port given";
        return;
    }
    nPort = (unsigned int)cURL.port();

    QString sAddress = cURL.host();
    if (sAddress.isEmpty() || sAddress == "*") {
        
        // we ought to guess the IP ourselves ... -.-
        qkd::utility::nic cDefaultGatewayNic = qkd::utility::environment::default_gateway();
        if (cDefaultGatewayNic.sIPv4.empty()) {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to listen: can't deduce public IP to bind";
            return;
        }
        
        qkd::utility::syslog::info() << "provided '*' as host to listen on, picked IPv4: '" << cDefaultGatewayNic.sIPv4 << "' to bind";
        sAddress = QString::fromStdString(cDefaultGatewayNic.sIPv4);
    }
    
    std::set<std::string> cAddressesForHost = qkd::utility::environment::host_lookup(sAddress.toStdString());
    if (cAddressesForHost.empty()) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to listen: unable to get IP address for hostname: " << sAddress.toStdString();
        return;
    }
    sAddress = QString::fromStdString(*cAddressesForHost.begin());
    
    uint64_t nKeyConsumptionPerRound = qkd::crypto::association::key_consumption(d->m_cAssociationDefinition);
    if ((uint64_t)cSecret.size() < nKeyConsumptionPerRound) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "not enough shared secret bytes provided. needed min.: " << nKeyConsumptionPerRound << " bytes, provided: " << cSecret.size() << " bytes.";
        return;
    }
    d->m_cInitialSecret = qkd::key::key(0, qkd::utility::memory::duplicate((unsigned char * const)cSecret.data(), cSecret.size()));
    
    if (d->m_cServer) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "changing public socket listener";
        delete d->m_cServer;
        d->m_cServer = nullptr;
    }
    
    set_role((qulonglong)qkd::module::module_role::ROLE_BOB);
    
    d->m_cServer = new QTcpServer(this);
    QObject::connect(d->m_cServer, SIGNAL(newConnection()), SLOT(server_new()));
    
    if (!d->m_cServer->listen(QHostAddress(sAddress), nPort)) {
        QString sMessage = QString("failed to start listening on \"%1\"\nmaybe address already in use?").arg(sURI);
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << sMessage.toStdString();
        return;
    }
    
    sURI = sScheme + "://" + sAddress + ":" + QString::number(nPort);
    QString sMessage = QString("started listening on public address \"%1\"").arg(sURI);
    qkd::utility::syslog::info() << sMessage.toStdString();
    
    emit listening(sURI);
    
    calculate_state();
}


/**
 * load protocol failed
 * 
 * @param   nReason     reason why it failed (protocol::protocol_error code)
 */
void engine_instance::load_failed(uint8_t nReason) {
    std::string sError = qkd::q3p::protocol::protocol::protocol_error_description((qkd::q3p::protocol::protocol_error)nReason).toStdString();
    qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "LOAD protocol failed! Reason: " << nReason << " - " << sError;
}


/**
 * load protocol succeeded
 */
void engine_instance::load_success() {
}


/**
 * load request protocol failed
 * 
 * @param   nReason     reason why it failed (protocol::protocol_error code)
 */
void engine_instance::load_request_failed(uint8_t nReason) {
    std::string sError = qkd::q3p::protocol::protocol::protocol_error_description((qkd::q3p::protocol::protocol_error)nReason).toStdString();
    qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "LOAD-REQUEST protocol failed! Reason: " << nReason << " - " << sError;
}


/**
 * load request protocol succeeded
 */
void engine_instance::load_request_success() {
}


/**
 * check if we are the master keystore
 * 
 * @return  true, if we are the master key store
 */
bool engine_instance::master() const {
    return d->m_bMaster;
}


/**
 * get the message queue name
 * 
 * @return  the name of the message queue
 */
QString engine_instance::mq() const {
    if (d->m_cMQ.get() != NULL) return d->m_cMQ->name();
    return QString();
}


/**
 * get the network interface card name
 * 
 * @return  the name of the network interface
 */
QString engine_instance::nic() const {
    if (d->m_cNIC.get() != NULL) return d->m_cNIC->name();
    return QString();
}


/**
 * return the local IP4 NIC address
 * 
 * @return  the local IP4 NIC address
 */
std::string engine_instance::nic_ip4_local() const {
    return d->m_sNICIP4Local;
}


/**
 * return the remote IP4 NIC address
 * 
 * @return  the remote IP4 NIC address
 */
std::string engine_instance::nic_ip4_remote() const {
    return d->m_sNICIP4Remote;
}


/**
 * open (or create) the key store DB on the specified URL
 * 
 * @param   sURL        url defining the key-DB
 * @return  true, if successful
 */
void engine_instance::open_db(QString sURL) {
    
    auto nStart = std::chrono::high_resolution_clock::now();
    
    if (db_opened()) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "refusing to open key DB - already an instance open";
        return;
    }
    
    try {
        d->m_cCommonStoreDB = qkd::q3p::db::open(sURL);
    }
    catch (qkd::q3p::db::db_url_scheme_unknown & cDBUrlSchemeUnknown) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to open key DB - unknown URL scheme: \"" + sURL.toStdString() + "\"";
        return;
    }
    catch (qkd::q3p::db::db_init_error & cDBInitError) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to open key DB - init error: \"" + sURL.toStdString() + "\"";
        return;
    }
    
    uint64_t nKeyCount = d->m_cCommonStoreDB->count();

    auto nStop = std::chrono::high_resolution_clock::now();
    auto nTimeDiff = std::chrono::duration_cast<std::chrono::milliseconds>(nStop - nStart);

    new DatabaseAdaptor(d->m_cCommonStoreDB.get());
    QString sCommonStoreObjectPath = d->m_sDBusObjectPath + "/CommonStore";
    bool bSuccess = d->m_cDBus.registerObject(sCommonStoreObjectPath, d->m_cCommonStoreDB.get());
    if (!bSuccess) {
        QString sMessage = QString("Failed to register DBus object \"") + sCommonStoreObjectPath + "\"";
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << sMessage.toStdString();
    }
    else {
        qkd::utility::syslog::info() << "registered CommonStore DB on DBus as \"" << sCommonStoreObjectPath.toStdString() << "\"";
    }
    
    QString sMessage = QString("database opened in %1 millisec - %2 keys in database").arg(nTimeDiff.count()).arg(nKeyCount);
    qkd::utility::syslog::info() << sMessage.toStdString();
    
    emit db_opened(sURL);
    
    calculate_state();
}


/**
 * access to the current outgoing buffer
 * 
 * @return  the current outgoing buffer
 */
key_db & engine_instance::outgoing_buffer() {
    return d->m_cOutgoingDB;
}


/**
 * access to the current outgoing buffer
 * 
 * @return  the current outgoing buffer
 */
key_db const & engine_instance::outgoing_buffer() const {
    return d->m_cOutgoingDB;
}


/**
 * this is called whenever we have a key read from the qkd pipeline
 * 
 * @param   cKey                    the key just read from the input pipe
 * @param   cIncomingContext        incoming crypto context
 * @param   cOutgoingContext        outgoing crypto context
 * @return  true, if the key is to be pushed to the output pipe
 */
bool engine_instance::process(UNUSED qkd::key::key & cKey, UNUSED qkd::crypto::crypto_context & cIncomingContext, UNUSED qkd::crypto::crypto_context & cOutgoingContext) {
    return false;
}


/**
 * run a Q3P timer timeout
 * 
 * this is called periodically and triggers the
 * Q3P protocols if connected.
 * 
 * The protocols which are run:
 * 1. LOAD from the CommonStore keys into the buffers
 * 2. STORE from the PickupStores into the CommonStore
 * 
 * You may trigger this call also via DBus
 */
void engine_instance::q3p_timeout() {
    
    qkd::utility::debug() << "timeout: running Q3P LOAD and Q3P STORE or reconnect";
    
    if (connected()) {
    
        if (d->m_cProtocol.cLoad) d->m_cProtocol.cLoad->run();
        if (d->m_cProtocol.cLoadRequest) d->m_cProtocol.cLoadRequest->run();
        if (d->m_cProtocol.cStore) d->m_cProtocol.cStore->run();
        if (d->m_cMQ.get()) d->m_cMQ->produce();
    }
    else {
        
        if ((link_state() == (unsigned int)engine_state::ENGINE_OPEN) && (d->m_bReconnect)) {
            QByteArray cSecret((char const *)(d->m_cInitialSecret.data().get()), d->m_cInitialSecret.data().size());
            connect(QString("%1%2:%3").arg("tcp://").arg(d->m_cPeerAddress.toString()).arg(d->m_nPeerPort), cSecret);
        }
    }
}


/**
 * a bunch of data from the peer has been received: handle this!
 * 
 * @param   cData       the data to received
 */
void engine_instance::recv_data(qkd::utility::memory const & cData) {
    d->m_cNIC->write(cData);
}


/**
 * registers an engine
 * 
 * @param   cEngine     engine to register
 * @return  true, if the engine has been registered
 */
bool engine_instance::register_engine(engine cEngine) {
    
    engine_map::iterator cIter = g_cEngines.find(cEngine->link_id());
    if (cIter == g_cEngines.end()) {
        g_cEngines.insert(std::pair<QString, qkd::q3p::engine>(cEngine->link_id(), cEngine));
        return true;
    }
    
    return false;
}
    
    
/**
 * register this object on the DBus
 * 
 * we do this different then the standard modules
 */
void engine_instance::register_dbus() {
    // do not register an extra module instance here
}


/**
 * list all the remotely present modules
 * 
 * The return list is a series of string each one of
 * the format:
 * 
 *      ID;STATE;NODE;PIPELINE;HINT;URL_LISTEN;
 * 
 * All fields are separated with a semicolon ';'
 * 
 *      ID ............. The id of the module
 *      PID ............ The process ID of the module
 *      STATE .......... The current state of the module
 *      NODE ........... The id of this node been asked
 *      LINK ........... The id of this link been asked
 *      PIPELINE ....... The id of the pipeline the module
 *                       is currently in
 *      HINT ........... Any user supplied information to 
 *                       the module
 *      URL_LISTEN ..... The public available listen URL of
 *                       the module
 * @return  a list of running modules
 */
QStringList engine_instance::remote_modules() {

    QStringList cModules;
    
    // TODO: to be implemented
    
    return cModules;
}


/**
 * send a bunch of data to the peer
 * 
 * @param   cData       the data to send
 */
void engine_instance::send_data(qkd::utility::memory const & cData) {
    
    if (!d->m_bConnected) {
        qkd::utility::debug() << "refused to send data when not connected";
        return;
    }
    
    if (!d->m_cProtocol.cData) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "tried to send data (" << cData.size() << " bytes), I'm connected - but I lack a DATA protocol instance. This must not happen. This is a bug. Sorry";
        return;
    }

    qkd::q3p::message cMessage(true, true);
    cMessage << cData;
    
    // TODO: check for results: might be insufficient key material in the KS
    d->m_cProtocol.cData->send(cMessage);
}


/**
 * a peer key store connects
 */
void engine_instance::server_new() {
    
    qkd::utility::debug() << "peer connect!";
    
    if (!d->m_cServer) return;
    
    QTcpSocket * cConnection = d->m_cServer->nextPendingConnection();
    if (!cConnection) return;
    
    if (!db_opened()) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "won't connect to peer without an opened database";
        delete cConnection;
        cConnection = nullptr;
        return;
    }
    
    if (d->m_cCommonStoreDB->count() < MIN_KEYS_IN_DB) {
        QString sMessage = QString("insufficient keys in database (minimum is %1): inject keys first in order to connect").arg(MIN_KEYS_IN_DB);
        qkd::utility::syslog::info() << sMessage.toStdString();
        delete cConnection;
        cConnection = nullptr;
        return;
    }
    
    if (link_state() > (unsigned int)engine_state::ENGINE_CONNECTING) {
        QString sMessage = QString("connection attempt by \"%1:%2\" discarded: already connected or attempting to connect to peer").arg(cConnection->peerAddress().toString()).arg(cConnection->peerPort());
        qkd::utility::syslog::info() << sMessage.toStdString();
        delete cConnection;
        cConnection = nullptr;
        return;
    }
    
    // tweak crypto schemes: cut off any old key values (better safe than sorry)
    uint64_t nPos;
    if ((nPos = d->m_cAssociationDefinition.sAuthenticationIncoming.find(':')) != std::string::npos) d->m_cAssociationDefinition.sAuthenticationIncoming.resize(nPos);
    if ((nPos = d->m_cAssociationDefinition.sAuthenticationOutgoing.find(':')) != std::string::npos) d->m_cAssociationDefinition.sAuthenticationOutgoing.resize(nPos);
    if ((nPos = d->m_cAssociationDefinition.sEncryptionIncoming.find(':')) != std::string::npos) d->m_cAssociationDefinition.sEncryptionIncoming.resize(nPos);
    if ((nPos = d->m_cAssociationDefinition.sEncryptionOutgoing.find(':')) != std::string::npos) d->m_cAssociationDefinition.sEncryptionOutgoing.resize(nPos);
    
    QString sMessage = QString("connected by \"%1:%2\" - running handshake").arg(cConnection->peerAddress().toString()).arg(cConnection->peerPort());
    qkd::utility::syslog::info() << sMessage.toStdString();

    d->m_cSocket = cConnection;
    QObject::connect(d->m_cSocket, SIGNAL(connected()), SLOT(socket_connected()));
    QObject::connect(d->m_cSocket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(socket_error(QAbstractSocket::SocketError)));
    QObject::connect(d->m_cSocket, SIGNAL(readyRead()), SLOT(socket_ready_read()));
    
    d->m_cRecvBuffer = QByteArray();
    
    d->m_cProtocol.cHandshake = new protocol::handshake(d->m_cSocket, this);
    QObject::connect(d->m_cProtocol.cHandshake, SIGNAL(failed(uint8_t)), SLOT(handshake_failed(uint8_t)));
    QObject::connect(d->m_cProtocol.cHandshake, SIGNAL(success()), SLOT(handshake_success()));
    d->m_cProtocol.cHandshake->run();
    
    emit connection_established(link_peer());
    
    // if we are connected, then we wont force reconnection
    // on connection lost
    d->m_bReconnect = false;
    
    calculate_state();
}


/**
 * set a new authentication scheme for incoming
 * 
 * @param   sScheme         the new scheme
 * @throws  engine_invalid_scheme
 */
void engine_instance::set_authentication_scheme_incoming(QString const & sScheme) {
    if (!qkd::crypto::engine::valid_scheme(qkd::crypto::scheme(sScheme.toStdString()))) {
        throw std::invalid_argument("invalid authentication scheme for incoming data");
    }
    d->m_cAssociationDefinition.sAuthenticationIncoming = sScheme.toStdString();
}


/**
 * set a new authentication scheme for outgoing
 * 
 * @param   sScheme         the new scheme
 * @throws  engine_invalid_scheme
 */
void engine_instance::set_authentication_scheme_outgoing(QString const & sScheme) {
    if (!qkd::crypto::engine::valid_scheme(qkd::crypto::scheme(sScheme.toStdString()))) {
        throw std::invalid_argument("invalid authentication scheme for outgoing data");
    }
    d->m_cAssociationDefinition.sAuthenticationOutgoing = sScheme.toStdString();
}


/**
 * set a new encryption scheme for incoming
 * 
 * @param   sScheme         the new scheme
 * @throws  engine_invalid_scheme
 */
void engine_instance::set_encryption_context_name_incoming(QString const & sScheme) {
    if (!qkd::crypto::engine::valid_scheme(qkd::crypto::scheme(sScheme.toStdString()))) {
        throw std::invalid_argument("invalid encryption scheme for incoming data");
    }
    d->m_cAssociationDefinition.sEncryptionIncoming = sScheme.toStdString();
}


/**
 * set a new encryption scheme for outgoing
 * 
 * @param   sScheme         the new scheme
 * @throws  engine_invalid_scheme
 */
void engine_instance::set_encryption_context_name_outgoing(QString const & sScheme) {
    if (!qkd::crypto::engine::valid_scheme(qkd::crypto::scheme(sScheme.toStdString()))) {
        throw std::invalid_argument("invalid encryption scheme for outgoing data");
    }
    d->m_cAssociationDefinition.sEncryptionOutgoing = sScheme.toStdString();
}


/**
 * sets the master role on the keystore
 * 
 * This only works if the key store is not
 * connected. If the key store is connected
 * with its peer this method does nothing
 * 
 * @param   bMaster     the new master role flag
 */
void engine_instance::set_master(bool bMaster) {
    
    if (connected()) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "won't change master/slave relationship during connection. refusing change of role.";
        return;
    }
    
    d->m_bMaster = bMaster;
    d->m_bSlave = !bMaster;
    set_role((qulonglong)qkd::module::module_role::ROLE_ALICE);
    emit role_change(d->m_bMaster, d->m_bSlave);
}


/**
 * set the local IP4 NIC address
 * 
 * @param   sIP4    the new local IP4 NIC address
 */
void engine_instance::set_nic_ip4_local(std::string const & sIP4) {
    
    d->m_sNICIP4Local = sIP4;
    if (d->m_cNIC) {
        d->m_cNIC->set_ip4_local(QString::fromStdString(sIP4));
    }
}


/**
 * set the remote IP4 NIC address
 * 
 * @param   sIP4    the new remote IP4 NIC address
 */
void engine_instance::set_nic_ip4_remote(std::string const & sIP4) {
    
    d->m_sNICIP4Remote = sIP4;
    if (d->m_cNIC) {
        d->m_cNIC->set_ip4_remote(QString::fromStdString(sIP4));
    }
}


/**
 * sets the slave role on the keystore
 * 
 * This only works if the key store is not
 * connected. If the key store is connected
 * with its peer this method does nothing
 * 
 * @param   bSlave      the new slave role flag
 */
void engine_instance::set_slave(bool bSlave) {
    
    if (connected()) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "won't change master/slave relationship during connection. refusing change of role.";
        return;
    }
    
    d->m_bSlave = bSlave;
    d->m_bMaster = !bSlave;
    set_role((qulonglong)qkd::module::module_role::ROLE_BOB);
    emit role_change(d->m_bMaster, d->m_bSlave);
}


/**
 * init the internal buffers
 */
void engine_instance::setup_buffers() {
    
    qkd::utility::debug() << "setting up internal keystore buffers...";
    
    d->m_cIncomingDB = qkd::q3p::db::open("ram://");
    d->m_cOutgoingDB = qkd::q3p::db::open("ram://");
    d->m_cApplicationDB = qkd::q3p::db::open("ram://");
    
    new DatabaseAdaptor(d->m_cIncomingDB.get());
    QString sIncomingDBObjectPath = d->m_sDBusObjectPath + "/IncomingBuffer";
    bool bSuccess = d->m_cDBus.registerObject(sIncomingDBObjectPath, d->m_cIncomingDB.get());
    if (!bSuccess) {
        QString sMessage = QString("Failed to register DBus object \"") + sIncomingDBObjectPath + "\"";
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << sMessage.toStdString();
    }
    
    new DatabaseAdaptor(d->m_cOutgoingDB.get());
    QString sOutgoingDBObjectPath = d->m_sDBusObjectPath + "/OutgoingBuffer";
    bSuccess = d->m_cDBus.registerObject(sOutgoingDBObjectPath, d->m_cOutgoingDB.get());
    if (!bSuccess) {
        QString sMessage = QString("Failed to register DBus object \"") + sOutgoingDBObjectPath + "\"";
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << sMessage.toStdString();
    }
    
    new DatabaseAdaptor(d->m_cApplicationDB.get());
    QString sApplicationDBObjectPath = d->m_sDBusObjectPath + "/ApplicationBuffer";
    bSuccess = d->m_cDBus.registerObject(sApplicationDBObjectPath, d->m_cApplicationDB.get());
    if (!bSuccess) {
        QString sMessage = QString("Failed to register DBus object \"") + sApplicationDBObjectPath + "\"";
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << sMessage.toStdString();
    }
}
    
    
/**
 * init a new channel
 */
void engine_instance::setup_channel() {
    
    qkd::utility::debug() << "setting up new channel...";
    
    d->m_nChannelId++;
    if (d->m_nChannelId == 0) d->m_nChannelId = 1;
    
    if (d->m_cChannelMap.find(d->m_nChannelId) != d->m_cChannelMap.end()) {
        
        // woha! this channel already exists! why? should have been killed!
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "tried to create channel " << d->m_nChannelId << " but it already existed! This should not happen. This is a bug.";
        return;
    }
    
    try {
        d->m_cChannelMap.insert(std::pair<unsigned short, qkd::q3p::channel>(d->m_nChannelId, qkd::q3p::channel(d->m_nChannelId, this, qkd::crypto::association(d->m_cAssociationDefinition))));
    }
    catch (...) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "tried to create channel " << d->m_nChannelId << " but it already existed! This should not happen. This is a bug.";
        return;
    }
}
    
    
/**
 * init IPSec
 */
void engine_instance::setup_ipsec() {

    qkd::utility::debug() << "setting up IPSec...";
    
    // TODO: Stefan
    std::cerr << "engine_instance::setup_ipsec(): STEFAN: Bitte implement me." << std::endl;
}
    
    
/**
 * init a message queue
 */
void engine_instance::setup_mq() {
    
    qkd::utility::debug() << "setting up message queue...";
    
    d->m_cMQ = std::shared_ptr<qkd::q3p::mq_instance>(new qkd::q3p::mq_instance(this));
    
    new MqAdaptor(d->m_cMQ.get());
    QString sMQObjectPath = d->m_sDBusObjectPath + "/MQ";
    if (!d->m_cDBus.registerObject(sMQObjectPath, d->m_cMQ.get())) {
        QString sMessage = QString("Failed to register DBus object \"") + sMQObjectPath + "\"";
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << sMessage.toStdString();
    }
}
    
    
/**
 * init a network interface card (this creates the q3p_x_ network interface)
 */
void engine_instance::setup_nic() {
    
    qkd::utility::debug() << "setting up virtual NIC...";
    
    d->m_cNIC = std::shared_ptr<qkd::q3p::nic_instance>(new qkd::q3p::nic_instance(this));
    d->m_cNIC->set_ip4_local(QString::fromStdString(d->m_sNICIP4Local));
    d->m_cNIC->set_ip4_remote(QString::fromStdString(d->m_sNICIP4Remote));
    
    new NicAdaptor(d->m_cNIC.get());
    QString sNICObjectPath = d->m_sDBusObjectPath + "/NIC";
    if (!d->m_cDBus.registerObject(sNICObjectPath, d->m_cNIC.get())) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " 
                << "Failed to register DBus object \"" << sNICObjectPath.toStdString() << "\"";
    }
}
    
    
/**
 * shutdown buffers
 */
void engine_instance::shutdown_buffers() {
    
    qkd::utility::debug() << "shutting down internal keystore buffers...";
    
    d->m_cIncomingDB = qkd::q3p::db::open("ram://");
    d->m_cOutgoingDB = qkd::q3p::db::open("ram://");
    d->m_cApplicationDB = qkd::q3p::db::open("ram://");
}
    
    
/**
 * shutdown channels
 */
void engine_instance::shutdown_channels() {
    
    qkd::utility::debug() << "shutting down channels...";
    
    d->m_cChannelMap.clear();
    d->m_nChannelId = 0;
}
    
    
/**
 * shutdown ipsec
 */
void engine_instance::shutdown_ipsec() {
    
    qkd::utility::debug() << "shutting down IPSec...";
    
    // TODO: Stefan
    std::cerr << "engine_instance::shutdown_ipsec(): STEFAN: Bitte implement me." << std::endl;
}
    
    
/**
 * shutdown message queue
 */
void engine_instance::shutdown_mq() {
    qkd::utility::debug() << "shutting down message queue...";
    d->m_cMQ = std::shared_ptr<qkd::q3p::mq_instance>();
}
    
    
/**
 * shutdown nic
 */
void engine_instance::shutdown_nic() {
    qkd::utility::debug() << "shutting down virtual NIC...";
    d->m_cNIC = std::shared_ptr<qkd::q3p::nic_instance>();
}
    
    
/**
 * check if we are the slave keystore
 * 
 * @return  true, if we are the slave key store
 */
bool engine_instance::slave() const {
    return d->m_bSlave;
}


/**
 * we have a connection
 */
void engine_instance::socket_connected() {
    
    qkd::utility::debug() << "connected to peer...";
    
    if (!d->m_cSocket) return;
    
    // tweak crypto schemes: cut off any old key values (better safe than sorry)
    uint64_t nPos;
    if ((nPos = d->m_cAssociationDefinition.sAuthenticationIncoming.find(':')) != std::string::npos) d->m_cAssociationDefinition.sAuthenticationIncoming.resize(nPos);
    if ((nPos = d->m_cAssociationDefinition.sAuthenticationOutgoing.find(':')) != std::string::npos) d->m_cAssociationDefinition.sAuthenticationOutgoing.resize(nPos);
    if ((nPos = d->m_cAssociationDefinition.sEncryptionIncoming.find(':')) != std::string::npos) d->m_cAssociationDefinition.sEncryptionIncoming.resize(nPos);
    if ((nPos = d->m_cAssociationDefinition.sEncryptionOutgoing.find(':')) != std::string::npos) d->m_cAssociationDefinition.sEncryptionOutgoing.resize(nPos);
    
    QString sMessage = QString("connected to \"%1:%2\" - running handshake").arg(d->m_cSocket->peerAddress().toString()).arg(d->m_cSocket->peerPort());
    qkd::utility::syslog::info() << sMessage.toStdString();

    d->m_cRecvBuffer = QByteArray();
    
    d->m_cProtocol.cHandshake = new protocol::handshake(d->m_cSocket, this);
    QObject::connect(d->m_cProtocol.cHandshake, SIGNAL(failed(uint8_t)), SLOT(handshake_failed(uint8_t)));
    QObject::connect(d->m_cProtocol.cHandshake, SIGNAL(success()), SLOT(handshake_success()));
    d->m_cProtocol.cHandshake->run();
    
    emit connection_established(link_peer());
    
    // we made a connection
    // so we will reconnect if connection is lost
    d->m_bReconnect = true;
    d->m_cPeerAddress = d->m_cSocket->peerAddress();
    d->m_nPeerPort = d->m_cSocket->peerPort();
    
    calculate_state();
}


/**
 * we have an error on one of our connections
 * 
 * @param   eSocketError        the error
 */
void engine_instance::socket_error(QAbstractSocket::SocketError eSocketError) {
    
    QString sMessage;
    
    // deal with certain errors
    switch (eSocketError) {
    
    case QAbstractSocket::RemoteHostClosedError:
        sMessage = QString("connection closed by remote host");
        break;
        
    case QAbstractSocket::HostNotFoundError:
        sMessage = QString("connection failed: unknown host");
        break;
        
    default:
        sMessage = QString("connection error: %1 - %2").arg(eSocketError).arg(QString::fromStdString(socket_error_strings::str(eSocketError)));
    }
    
    qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << sMessage.toStdString();
    
    // stop module worker
    // TODO: better terminate() ?
    pause();

    shutdown_mq();
    shutdown_nic();
    shutdown_ipsec();
    
    if (d->m_cProtocol.cData) d->m_cProtocol.cData->deleteLater();
    d->m_cProtocol.cData = nullptr;
    if (d->m_cProtocol.cHandshake) d->m_cProtocol.cHandshake->deleteLater();
    d->m_cProtocol.cHandshake = nullptr;
    if (d->m_cProtocol.cLoad) d->m_cProtocol.cLoad->deleteLater();
    d->m_cProtocol.cLoad = nullptr;
    if (d->m_cProtocol.cLoadRequest) d->m_cProtocol.cLoadRequest->deleteLater();
    d->m_cProtocol.cLoadRequest = nullptr;
    if (d->m_cProtocol.cStore) d->m_cProtocol.cStore->deleteLater();
    d->m_cProtocol.cStore = nullptr;
    
    shutdown_buffers();
    
    shutdown_channels();
    
    d->m_cSocket = nullptr;
    d->m_bConnected = false;
    
    emit connection_lost();

    calculate_state();
}


/**
 * we have data available on the socket
 * 
 * this is the main single peer receive packet handler
 */
void engine_instance::socket_ready_read() {

    if (!d->m_cSocket) return;
    
    d->m_cRecvBuffer.append(d->m_cSocket->readAll());

    qkd::q3p::message cMessage;
    qkd::q3p::protocol::protocol_type eProtocol;
    qkd::q3p::protocol::protocol_error eError = qkd::q3p::protocol::protocol::recv(d->m_cRecvBuffer, cMessage, eProtocol);
    if (eError != qkd::q3p::protocol::protocol_error::PROTOCOL_ERROR_NO_ERROR) {
        
        // if we lack some additional data, we call this data parsing method soon again
        if (eError == qkd::q3p::protocol::protocol_error::PROTOCOL_ERROR_PENDING) QTimer::singleShot(250, this, SLOT(socket_ready_read()));
        return;
    }

    // if we still have some bytes left in the read buffer, than 
    // we got presumably more than 1 message: call parsing soon again
    if (d->m_cRecvBuffer.size()) QTimer::singleShot(0, this, SLOT(socket_ready_read()));
    
    qkd::utility::debug() << "<Q3P-RECV>" << cMessage.str();
    
    if (cMessage.channel_id()) {
        
        qkd::q3p::channel cChannel = channel(cMessage.channel_id());
        if (cChannel.id() == 0) {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "got message on channel: " << cMessage.channel_id() << " which is currently not configured or setup: message silently discarded.";
            return;
        }

        qkd::q3p::channel_error eChannelError = cChannel.decode(cMessage);
        if (eChannelError != qkd::q3p::channel_error::CHANNEL_ERROR_NO_ERROR) {
            qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " 
                    << "failed to decode message on channel #" << cChannel.id() 
                    << " decoding message returned: " << (unsigned int)eChannelError 
                    << " (" << qkd::q3p::channel::channel_error_description(eChannelError) << ")";
            return;
        }
    }
    else {
        
        // on channel 0 only handshake is allowed
        if (eProtocol != qkd::q3p::protocol::protocol_type::PROTOCOL_HANDSHAKE) {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "got message on channel 0 which is NOT related to HANDSHAKE protocol: message silently discarded.";
            return;
        }
    }
    
    switch (eProtocol) {
        
    case qkd::q3p::protocol::protocol_type::PROTOCOL_HANDSHAKE:
        
        // handshake
        if (!d->m_cProtocol.cHandshake) {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "got message for HANDSHAKE ... but I'm not ready for this right now.";
            return;
        }
        d->m_cProtocol.cHandshake->recv(cMessage);
        
        break;

    case qkd::q3p::protocol::protocol_type::PROTOCOL_LOAD:
        
        // load
        if (!d->m_cProtocol.cLoad) {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "got message for LOAD ... but I'm not ready for this right now.";
            return;
        }
        d->m_cProtocol.cLoad->recv(cMessage);

        break;

    case qkd::q3p::protocol::protocol_type::PROTOCOL_LOAD_REQUEST:
        
        // load-request
        if (!d->m_cProtocol.cLoadRequest) {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "got message for LOAD-REQUEST ... but I'm not ready for this right now.";
            return;
        }
        d->m_cProtocol.cLoadRequest->recv(cMessage);
        
        break;
        
    case qkd::q3p::protocol::protocol_type::PROTOCOL_STORE:
        
        // store
        if (!d->m_cProtocol.cStore) {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "got message for STORE ... but I'm not ready for this right now.";
            return;
        }
        d->m_cProtocol.cStore->recv(cMessage);
        
        break;

    case qkd::q3p::protocol::protocol_type::PROTOCOL_DATA:
        
        // data
        if (!d->m_cProtocol.cData) {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "got message for DATA ... but I'm not ready for this right now.";
            return;
        }
        d->m_cProtocol.cData->recv(cMessage);
        
        break;

        
    default:
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "got message on protocol " << qkd::q3p::protocol::protocol::protocol_id_name((uint8_t)eProtocol) << " but don't know what to do. this is a bug. Go tell Oliver.";
    }
}


/**
 * store protocol failed
 * 
 * @param   nReason     reason why it failed (protocol::protocol_error code)
 */
void engine_instance::store_failed(uint8_t nReason) {
    std::string sError = qkd::q3p::protocol::protocol::protocol_error_description((qkd::q3p::protocol::protocol_error)nReason).toStdString();
    qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "STORE protocol failed! Reason: " << nReason << " - " << sError;
}


/**
 * store protocol succeeded
 */
void engine_instance::store_success() {
}


/**
 * remove this engine registration
 * 
 * @param   cEngine     engine to unregister
 */
void engine_instance::unregister_engine(engine cEngine) {
    engine_map::iterator cIter = g_cEngines.find(cEngine->link_id());
    if (cIter != g_cEngines.end()) g_cEngines.erase(cIter);
}

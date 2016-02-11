/*
 * engine.h
 * 
 * the Q3P engine
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

 
#ifndef __QKD_Q3P_ENGINE_H_
#define __QKD_Q3P_ENGINE_H_


// ------------------------------------------------------------
// incs

#include <exception>
#include <memory>
#include <string>

#include <inttypes.h>

// Qt
#include <QtCore/QObject>
#include <QtDBus/QtDBus>
#include <QtNetwork/QAbstractSocket>

// ait
#include <qkd/crypto/context.h>
#include <qkd/module/module.h>
#include <qkd/q3p/channel.h>
#include <qkd/q3p/db.h>


// ------------------------------------------------------------
// defs


/**
 * the amount of minimum keys in the key DB before connecting
 */
#define MIN_KEYS_IN_DB      10


// ------------------------------------------------------------
// decls


namespace qkd {
    
namespace q3p {    

    
// fwd
class engine_instance;
typedef std::shared_ptr<engine_instance> engine;
typedef std::map<QString, qkd::q3p::engine> engine_map;


/**
 * the states of the internal key store
 */
enum class engine_state : uint8_t {
    
    ENGINE_INIT = 0,                /**< initial state */
    ENGINE_OPEN = 1,                /**< engine has opened the database */
    ENGINE_CONNECTING = 2,          /**< engine is currently connecting to its peer */
    ENGINE_HANDSHAKE = 3,           /**< engine is currently in the handshake phase with its peer */
    ENGINE_CONNECTED = 4            /**< engine is connected to its peer */
};


/**
 * This is the Q3P Engine - aka "Q3P KeyStore" - aka "Q3P Link" all synonyms.
 * 
 * A Q3P Engine / Q3P KeyStore / Q3P Link IS A special QKD module which 
 * terminates a QKD pipeline.
 * 
 * A Q3P engine (or "link") does all the Q3P work:
 * 
 *  - connect to other links (remote engines) instances
 *  - do message transformation for encryption and authentication
 *  - play master or slave
 *  - trigger the key-store protocols
 *  - manage the key store database
 *  - open up a message queue for keys (aka "Key Pump")
 *  - open up a NIC for point-to-point communication
 * 
 * 
 *          ---> This is the heart of Q3P. <---
 * 
 * 
 * All the Q3P engines need a parent node and an ID. This ID is used to register
 * an object under the parent node DBus service under the path "/Link/ID".
 * 
 * The offered DBus interface is
 * 
 *      DBus Interface: "at.ac.ait.q3p.link"
 * 
 * 
 * Properties of at.ac.ait.q3p.link
 * 
 *      -name-          -read/write-    -description-
 * 
 *      connected           R           flag for been connected with a peer instance
 * 
 *      db_opened           R           flag if the underlying key-DB has been opened
 * 
 *      link_id             R           ID of the engine/link instance
 * 
 *      link_local          R           The local public uri we are listening
 * 
 *      link_peer           R           When connected this is the address of the peer engine
 * 
 *      link_state          R           the current state ID of the q3p engine
 * 
 *      master              R/W         get/set master role. setting the master role only
 *                                      works, if the key store is NOT connected. Once the
 *                                      key store is connected, the roles are fixed.
 * 
 *      mq                  R           this is the name of message queue the Q3P link launches.
 * 
 *      nic                 R           this is the name of the network interface the Q3P link
 *                                      launches.
 * 
 * 
 * Methods of at.ac.ait.q3p.link
 * 
 *      -name-                      -description-
 * 
 *      close_db()                  closes the opened key DB
 *                                  one must be disconnected from the peer first
 * 
 *      connect()                   connects to a peer key-store which listens on a public port
 *                                  this can be done if the key DB has been opened and there
 *                                  is at some key therein
 *                              
 *                                  further: an initial secret is needed, sufficient enough
 *                                  for the first authentications
 * 
 *      disconnect()                wind down a living connection
 * 
 *      inject()                    inject some keybits into the DB. the injected keys will be placed
 *                                  at the very next free position in the key DB.
 *                                  HENCE 1:    there is no peer interaction here. Do this 
 *                                              ONLY to provide shared secrets. If you this 
 *                                              on both sides, you'll go out-of-sync.
 *                                              Works only on opened key DB and without connection.
 *                                  HENCE 2:    The key insert MUST be a multiple of quantum().
 *                                              Overflow will be discarded!
 * 
 *      inject_url()                this works as inject() but takes an resource identified by 
 *                                  an URL as key bits.
 *                                  HENCE:      Currently only file:// URLs are supported.
 * 
 *      link_state_description()    give a human readable description of a certain state number
 * 
 *      listen()                    starts listening on a public port
 * 
 *      open_db()                   opens a KeyDB specified with a given URL
 * 
 *      remote_modules()            list all the remotely present modules (which the capability
 *                                  to accept connections, that is. having URL_LISTEN to set to
 *                                  anything else but "").
 * 
 * 
 * The amount of keys managed by this KeyStore instance is specified by 
 * key_max - key_min. A single key has a fixed size of key_quantum in bytes.
 * Therefore the total amount of key material possible (in bytes) is
 * 
 *          K = (key_max - key_min) * key_quantum
 * 
 */
class engine_instance : public qkd::module::module {
    

    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "at.ac.ait.q3p.link")
    
    Q_PROPERTY(bool connected READ connected)                   /**< the engine's connected state */
    Q_PROPERTY(bool db_opened READ db_opened)                   /**< DB opened flag */
    Q_PROPERTY(QString link_id READ link_id)                    /**< the ID of this engine instance (aka Link) */
    Q_PROPERTY(QString link_local READ link_local)              /**< the local hostaddress to serve */
    Q_PROPERTY(QString link_peer READ link_peer)                /**< peer host address */
    Q_PROPERTY(unsigned int link_state READ link_state)         /**< the current engine's state */
    Q_PROPERTY(bool master READ master WRITE set_master)        /**< master role flag */
    Q_PROPERTY(QString mq READ mq)                              /**< message queue name */
    Q_PROPERTY(QString nic READ nic)                            /**< network interface card name */
    Q_PROPERTY(bool slave READ slave WRITE set_slave)           /**< slave role flag */

    
public:

    
    /**
     * dtor
     */
    virtual ~engine_instance();
    
    
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
    bool acquire_keys(qkd::key::key_ring & cKeys, uint64_t nAppId, uint64_t nBytes, std::chrono::milliseconds nTimeout);
    
    
    /**
     * access to the current application buffer
     * 
     * @return  the current application buffer
     */
    key_db & application_buffer();
    
    
    /**
     * access to the current application buffer
     * 
     * @return  the current application buffer
     */
    key_db const & application_buffer() const;
    
    
    /**
     * the current (next) authentication scheme for incoming messages
     * 
     * @return  the current (next) authentication scheme for incoming
     */
    QString authentication_scheme_incoming() const;
    
    
    /**
     * the current (next) authentication scheme for outgoing messages
     * 
     * @return  the current (next) authentication scheme for outgoing messages
     */
    QString authentication_scheme_outgoing() const;
    
    
    /**
     * get a channel
     * 
     * if the nChannelId is 0 the current channel is fetched
     * 
     * @param   nChannelId      if of the channel
     * @return  a known channel channel
     */
    qkd::q3p::channel & channel(uint16_t nChannelId = 0);
    
    
    /**
     * get a channel
     * 
     * if the nChannelId is 0 the current channel is fetched
     * 
     * @param   nChannelId      if of the channel
     * @return  a known channel channel
     */
    qkd::q3p::channel const & channel(uint16_t nChannelId = 0) const;
    
    
    /**
     * returns a string describing the current charge states of the buffers
     * 
     * this is for debugging. The stfin has the form
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
    std::string charge_string() const;
    
    
    /**
     * closes an engine
     */
    void close();
    
    
    /**
     * closes all known engines
     */
    static void close_all();
    
    
    /**
     * access to the common store
     * 
     * @return  the common store
     */
    key_db & common_store();
    
    
    /**
     * access to the common store
     * 
     * @return  the common store
     */
    key_db const & common_store() const;
    
    
    /**
     * configure the IPSec connection
     * 
     * @param   sIPSecConfiguration         IPSec Configuration string
     */
    void configure_ipsec(QString sIPSecConfiguration);
    
    
    /**
     * check if we are connected with our peer
     * 
     * @return  true, if we are connected
     */
    bool connected() const;
    
    
    /**
     * this object MUST be created on the heap
     * 
     * @param   sNode       id of node
     * @param   sId         if of the new engine
     * @return  an instance on the heap
     * @throws  engine_already_registered
     * @throws  engine_invalid_id
     */
    static engine create(QString const & sNode, QString const & sId);
    
    
    /**
     * check if we have an opened Key-DB
     * 
     * @return  true, if we are operating on a key-db
     */
    bool db_opened() const;
    
    
    /**
     * the current (next) encryption scheme for incoming messages
     * 
     * @return  the current (next) encryption scheme for incoming messages
     */
    QString encryption_scheme_incoming() const;
    
    
    /**
     * the current (next) encryption scheme for outgoing messages
     * 
     * @return  the current (next) encryption scheme for outgoing messages
     */
    QString encryption_scheme_outgoing() const;
    
    
    /**
     * list of known engines
     * 
     * @return  the list of known engine instance
     */
    static engine_map const & engines();
    
    
    /**
     * retrieves a certain engine
     * 
     * @param   sId         if of the engine
     * @return  an engine
     */
    static engine get(QString const & sId);
    
    
    /**
     * access to the current incoming buffer
     * 
     * @return  the current incoming buffer
     */
    key_db & incoming_buffer();
    
    
    /**
     * access to the current incoming buffer
     * 
     * @return  the current incoming buffer
     */
    key_db const & incoming_buffer() const;
    
    
    /**
     * get the engine's id
     * 
     * @return  the id of the current engine
     */
    QString const & link_id() const;
    
    
    /**
     * our local public address we are serving
     * 
     * @return  the address of our local server listening
     */
    QString link_local() const;
    
    
    /**
     * the address of the connected peer key-store
     * 
     * @return  the address of the connected peer key-store
     */
    QString link_peer() const;
    
    
    /**
     * get the current key store state
     * 
     * @return  the current key store state
     */
    unsigned int link_state() const;
    

    /**
     * check if we are the master keystore
     * 
     * @return  true, if we are the master key store
     */
    bool master() const;
    
    
    /**
     * get the message queue name
     * 
     * @return  the name of the message queue
     */
    QString mq() const;
    
    
    /**
     * get the network interface card name
     * 
     * @return  the name of the network interface
     */
    QString nic() const;
    
    
    /**
     * access to the current outgoing buffer
     * 
     * @return  the current outgoing buffer
     */
    key_db & outgoing_buffer();
    
    
    /**
     * access to the current outgoing buffer
     * 
     * @return  the current outgoing buffer
     */
    key_db const & outgoing_buffer() const;
    
    
    /**
     * a bunch of data from the peer has been received: handle this!
     * 
     * @param   cData       the data to received
     */
    void recv_data(qkd::utility::memory const & cData);
    
    
    /**
     * send a bunch of data to the peer
     * 
     * @param   cData       the data to send
     */
    void send_data(qkd::utility::memory const & cData);
    
    
    /**
     * set a new authentication scheme for incoming
     * 
     * @param   sScheme         the new scheme
     * @throws  engine_invalid_scheme
     */
    void set_authentication_scheme_incoming(QString const & sScheme);
    
    
    /**
     * set a new authentication scheme for outgoing
     * 
     * @param   sScheme         the new scheme
     * @throws  engine_invalid_scheme
     */
    void set_authentication_scheme_outgoing(QString const & sScheme);
    
    
    /**
     * set a new encryption scheme for incoming
     * 
     * @param   sScheme         the new scheme
     * @throws  engine_invalid_scheme
     */
    void set_encryption_context_name_incoming(QString const & sScheme);
    

    /**
     * set a new encryption scheme for outgoing
     * 
     * @param   sScheme         the new scheme
     * @throws  engine_invalid_scheme
     */
    void set_encryption_context_name_outgoing(QString const & sScheme);
    

    /**
     * sets the master role on the engine
     * 
     * This only works if the key store is not
     * connected. If the engine is connected
     * with its peer this method does nothing
     * 
     * @param   bMaster     the new master role flag
     */
    void set_master(bool bMaster);
    
    
    /**
     * sets the slave role on the engine
     * 
     * This only works if the engine is not
     * connected. If the engine is connected
     * with its peer this method does nothing
     * 
     * @param   bSlave      the new slave role flag
     */
    void set_slave(bool bSlave);
    
    
    /**
     * check if we are slave keystore
     * 
     * @return  true, if we are the slave key store
     */
    bool slave() const;
    
    
public slots:
    
    
    /**
     * closes an opened key-DB
     */
    void close_db();
    
    
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
    void connect(QString sURI, QByteArray cSecret);
    
    
    /**
     * wind down any connection
     */
    void disconnect();
    
    
    /**
     * insert a key into the DB (without peer interaction!)
     * 
     * @param   cSecretBits     the key to insert
     */
    void inject(QByteArray cSecretBits);
    
    
    /**
     * insert a key identified by an URL into the DB (without peer interaction!)
     * 
     * @param   sURL            the URL to insert
     */
    void inject_url(QString sURL);
    
    
    /**
     * return a human readable engine state description
     * 
     * 
     * @param   nState      the state value
     * @return  a human readable engine state description
     */
    QString link_state_description(unsigned int nState);
    

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
    void listen(QString sURI, QByteArray cSecret);
    
    
    /**
     * open (or create) the key store DB on the specified URL
     * 
     * Hence: this method triggers some actions and may take longer.
     *        Check with db_opened later if the opening was successful
     * 
     * @param   sURL        url defining the key-DB
     * @return  true, if successful
     */
    Q_NOREPLY void open_db(QString sURL);
    
    
    /**
     * run a Q3P timer timeout
     * 
     * this is called periodically and triggers the
     * Q3P protocols if connected.
     * 
     * The protocols which are run:
     * 1. LOAD (from the CommonStore keys into buffers)
     * 2. LOAD-REQUEST (from the CommonStore keys into a buffer)
     * 3. STORE (from the PickupStores into the CommonStore)
     * 
     * You may trigger this call also via DBus
     */
    Q_NOREPLY void q3p_timeout();
    
    
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
    QStringList remote_modules();


protected:
    
    
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
    virtual bool accept(qkd::key::key const & cKey) const;
    
    
    /**
     * register this object on the DBus
     * 
     * we do this different then the standard modules
     */
    virtual void register_dbus();
    
    
private slots:
    
    
    /**
     * data protocol failed
     * 
     * @param   nReason     reason why it failed (protocol::protocol_error code)
     */
    void data_failed(uint8_t nReason);
    
    
    /**
     * data protocol succeeded
     */
    void data_success();
    
    
    /**
     * handshake protocol failed
     * 
     * @param   nReason     reason why it failed (protocol::protocol_error code)
     */
    void handshake_failed(uint8_t nReason);
    
    
    /**
     * handshake protocol succeeded
     */
    void handshake_success();
    
    
    /**
     * load protocol failed
     * 
     * @param   nReason     reason why it failed (protocol::protocol_error code)
     */
    void load_failed(uint8_t nReason);
    
    
    /**
     * load protocol succeeded
     */
    void load_success();
    
    
    /**
     * load request protocol failed
     * 
     * @param   nReason     reason why it failed (protocol::protocol_error code)
     */
    void load_request_failed(uint8_t nReason);
    
    
    /**
     * load request protocol succeeded
     */
    void load_request_success();
    
    
    /**
     * a peer key store connects
     */
    void server_new();
    

    /**
     * we have a connection
     */
    void socket_connected();
    
    
    /**
     * we have an error on one of our connections
     * 
     * @param   eSocketError        the error
     */
    void socket_error(QAbstractSocket::SocketError eSocketError);
    
    
    /**
     * we have data available on the socket
     * 
     * this is the main single peer receive packet handler
     */
    void socket_ready_read();
    
    
    /**
     * store protocol failed
     * 
     * @param   nReason     reason why it failed (protocol::protocol_error code)
     */
    void store_failed(uint8_t nReason);
    
    
    /**
     * store protocol succeeded
     */
    void store_success();
    
    
signals:
    
    
    /**
     * connection to peer present
     * 
     * @param   sURI        peer's URI we are connected
     */
    void connection_established(QString sURI);
    
    
    /**
     * connection to peer lost
     */
    void connection_lost();
    
    
    /**
     * a DB has been closed
     * 
     * @param   sDBUrl      the URL of the DB closed
     */
    void db_closed(QString sDBUrl);
    
    
    /**
     * a DB has been opened
     * 
     * @param   sDBUrl      the URL of the DB opened
     */
    void db_opened(QString sDBUrl);
    
    
    /**
     * we started listening on a specify host address and port
     * 
     * @param   sURI        the URI we are listening on
     */
    void listening(QString sURI);
    
    
    /**
     * master/slave role has changed
     * 
     * @param   bMaster     true, if we are master
     * @param   bSlave      true, if we are slave
     */
    void role_change(bool bMaster, bool bSlave);
    
    
    /**
     * the engine's state changed
     * 
     * @param   nState      the new state
     */
    void state_changed(unsigned int nState);
    
    
private:

    
    /**
     * ctor
     * 
     * @param   sNode       id of node
     * @param   sId         id of the engine
     */
    explicit engine_instance(QString const & sNode, QString const & sId);
    
    
    /**
     * calculate new state value
     */
    void calculate_state();
    
    
    /**
     * this is called whenever we have a key read from the qkd pipeline
     * 
     * @param   cKey                    the key just read from the input pipe
     * @param   cIncomingContext        incoming crypto context
     * @param   cOutgoingContext        outgoing crypto context
     * @return  true, if the key is to be pushed to the output pipe
     */
    virtual bool process(qkd::key::key & cKey, qkd::crypto::crypto_context & cIncomingContext, qkd::crypto::crypto_context & cOutgoingContext);
    
    
    /**
     * registers an engine
     * 
     * @param   cEngine     engine to register
     * @return  true, if the engine has been registered
     */
    static bool register_engine(engine cEngine);
    
    
    /**
     * init the internal buffers
     */
    void setup_buffers();
        
        
    /**
     * init a new channel
     */
    void setup_channel();
        
        
    /**
     * init IPSec
     */
    void setup_ipsec();
        
        
    /**
     * init a message queue
     */
    void setup_mq();
        
        
    /**
     * init a network interface card (this creates the q3p_x_ network interface)
     */
    void setup_nic();
        
        
    /**
     * shutdown buffers
     */
    void shutdown_buffers();
        
        
    /**
     * shutdown channels
     */
    void shutdown_channels();
        
        
    /**
     * shutdown ipsec
     */
    void shutdown_ipsec();
        
        
    /**
     * shutdown message queue
     */
    void shutdown_mq();
        
        
    /**
     * shutdown nic
     */
    void shutdown_nic();
        
        
    /**
     * remove this engine registration
     * 
     * @param   cEngine     engine to unregister
     */
    static void unregister_engine(engine cEngine);
    

    // pimpl
    class engine_data;
    std::shared_ptr<engine_data> d;

};


}
    
}



#endif


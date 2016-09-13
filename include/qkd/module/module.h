/*
 * module.h
 * 
 * This is the base of all QKD modules
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

 
#ifndef __QKD_MODULE_MODULE_H_
#define __QKD_MODULE_MODULE_H_


// ------------------------------------------------------------
// incs

#include <chrono>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>

#include <inttypes.h>

#include <boost/property_tree/ptree.hpp>

// Qt
#include <QtCore/QObject>
#include <QtDBus/QtDBus>

// ait
#include <qkd/common_macros.h>
#include <qkd/key/key.h>
#include <qkd/module/communicator.h>
#include <qkd/module/connection.h>
#include <qkd/module/message.h>
#include <qkd/module/workload.h>
#include <qkd/utility/average.h>
#include <qkd/utility/debug.h>
#include <qkd/utility/environment.h>
#include <qkd/utility/properties.h>
#include <qkd/utility/random.h>


// ------------------------------------------------------------
// decls


namespace qkd {
    
namespace module {    
    
// fwd
class module_thread;    
   
    
/**
 * the module roles
 * 
 * The module roles serve these issues:
 * 
 * 1. Alice is always regarded as the initiator. That is:
 *    She is obligated to connect Bob, while the later waits
 *    for Alice to be connected.
 * 
 *    Alice wont listen and Bob wont connect. So if modules
 *    on a cross network communication link are set as Alice
 *    then both try to connect but noone listens. If both
 *    are set as Bob, then both are listening ... but noone
 *    is saying anything.
 * 
 *    This is important to keep in mind, when peer-to-peer
 *    (module-to-module) communication fails.
 * 
 * 2. This settings are also a hint for the module process
 *    implementation to evaluate to start the communication or
 *    to wait for an incoming call.
 * 
 *    Depending on the module nature, some modules may
 *    take advantage of this property.
 * 
 * The default value is set to "Alice".
 */
enum class module_role : uint8_t {
    
    ROLE_ALICE = 0,                 /**< the module should act as alice (protocol initiator) */
    ROLE_BOB = 1                    /**< the module should act as bob (protocol responder) */
};


/**
 * the module states
 * 
 * The initial state of a module is "NEW". 
 * 
 * From the NEW state the worker thread is launched which
 * is immediately set to READY.
 * 
 * Calling resume() will set the state to RUNNING, while
 * pause() switches back to PAUSED. This has (more or less)
 * immediate effect on the worker thread which refuses to run
 * if the module is not in RUNNING state.
 * 
 * Finally calling terminate() will set the module state
 * consecutively to TERMINATING, signaling the winding
 * down of the module and then TERMINATED, the final
 * module death.
 * 
 * So the state chart goes like this:
 * 
 *      [init] --> NEW
 *                  |
 *                  | (run)
 *                  |
 *                  v
 *               READY <-----------+
 *                  |               |
 *                  | (resume)      | (pause)
 *                  |               |
 *                  v               |          
 *               RUNNING -----------+
 *                  |
 *                  | (terminate)
 *                  |
 *                  v
 *             TERMINATING
 *                  |
 *                  | 
 *                  |
 *                  v
 *             TERMINATED
 * 
 * Why is there no "start"/"stop"? Well, I sense no need to.
 * Run the module as long as you want, pause/resume operation 
 * as needed, and when done kill the module (tenderly) by 
 * calling terminate().
 * 
 * What does a "start"/"stop" does extra?
 */
enum module_state : uint8_t {
    
    STATE_NEW = 0,                  /**< module has just been created */
    STATE_READY = 1,                /**< module is ready to run */
    STATE_RUNNING = 2,              /**< module is running */
    STATE_TERMINATING = 3,          /**< module is about to shut down */
    STATE_TERMINATED = 4            /**< module is has shut down */
};


/**
 * the module types
 * 
 * This roughly describes the module's nature and indicates
 * module's precedence in a pipeline.
 * 
 * First: PRESIFTING, which could be followed by SIFTING 
 * (e.g. BB84), ERROR_ESTIMATION and the ERROR_CORRECTION 
 * (e.g. Cascade). After ERROR_CORRECTION the pipe
 * should hold a CONFIRMATION and finally a PRIVACY_AMPLIFICATION.
 * 
 * A module marked as OTHER may either serve totally 
 * different purpose or be placed in any order in between.
 * 
 * However, no module or pipeline is forced to respect this
 * settings. This property serves as a administrative hint
 * when setting up pipelines or inspecting modules at runtime.
 */
enum class module_type : uint8_t {
    
    TYPE_PRESIFTING = 0,                /**< a presifting module */
    TYPE_SIFTING = 1,                   /**< a sifting module */
    TYPE_ERROR_ESTIMATION = 2,          /**< an error estimation module */
    TYPE_ERROR_CORRECTION = 3,          /**< an error correction module */
    TYPE_CONFIRMATION = 4,              /**< a confirmation module */
    TYPE_PRIVACY_AMPLIFICATION = 5,     /**< a privacy amplification module */
    TYPE_KEYSTORE = 6,                  /**< a keystore (final) module */
    TYPE_OTHER = 7                      /**< other type */
};


/**
 * This class is the base class of all QKD modules.
 * 
 * 
 *          THIS IS THE HEART OF QKD PROCESSING
 * 
 * 
 * Critical hint: if you experience problems and/or something
 * goes awry turn on the debug mode by call set_debug(true); and
 * inspect stderr of the module process. In most cases this will
 * give you a clue about what's wrong.
 * 
 * A QKD module requires a unique identifier. Also a module may be
 * assigned to a pipeline. The (id, pipeline) identifies a module
 * on a node (as a pipeline is ought to be assigned to a node). 
 * Based in this information a cross-network connection is made.
 * 
 * The life-cycle of a QKD Module is this (see states above):
 * 
 *      1. Creation
 *              A module essentially needs an id. Everything else
 *              is administrative sugar. 
 * 
 *              The id is based on the theme of the module. An id 
 *              of "bb84" means: this module does BB84 sifting. An 
 *              id of "cascade" means: this module does CASCADE error 
 *              correction.
 * 
 *      2. Pipeline
 *              In order to operate in a QKD pipeline (aka "Stack")
 *              it has to be assigned a pipeline to. The pair
 *              (id, pipeline) uniquely identifies a module.
 * 
 *              However, modules may run with an empty pipeline as
 *              well. But this is not recommended in an ideal scene.
 * 
 *      3. Hint
 *              A further distinction is by proving an arbitrary hint.
 * 
 *              This comes handy, if you want to distinguish between
 *              parallel instances of the same module at the same stage
 *              in the same pipeline. E.g. you may run 5 CASCADE modules
 *              in parallel. You may then identify each of them by
 *              adding a hint-string to each on, like "1", "2", "a", 
 *              "b", ... or full names like "blue", "red", ...
 * 
 *      4. Process-Id
 *              As every module is ought to be run in a process, they
 *              get each a process id assigned.
 * 
 *      Summary: to identify a module uniqu you have:
 * 
 *                  - < id, pipeline, hint [, process-id]> 
 *                  OR
 *                  - the DBus address
 * 
 * 
 *      5. Crypto-Schemes
 *              The module operates on an incoming and an outgoing
 *              crypto context. A crypto scheme now identifies a) the
 *              Crypto-Algorithm used, b) the Crypto-Algorithm variant
 *              and c) the init key (if any).
 * 
 *              Setting the crypto scheme for both (in and out) will
 *              start a new crypto context for key processing.
 * 
 *      6. Running the module
 *              provide the necessary pipe-in, pipe-out and local 
 *              listen points and then run the module by invoking run()
 * 
 *              This will start the real module worker thread, which later on
 *              will call work() and from within work() the process() method
 *              to process a new key.
 *
 *      7. Terminating the module
 *              Finally the module is terminated by calling terminate(). 
 * 
 * 
 * Note on the connection URLs:
 * 
 * All modules have at least 4 connection points:
 * 
 *      URL Pipe In:        input of the module within the QKD pipeline
 * 
 *      URL Pipe Out:       output of the module within the QKD pipeline
 * 
 *      URL Listen:         Listen port to which remote modules are connected.
 *                          This endpoint is served, if the module's role is ROLE_BOB.
 * 
 *      URL Peer:           This is the connection to the remote module is the 
 *                          module's role is ROLE_ALICE
 * 
 * DEFAULT settings are:
 * 
 *          Pipe In:        stdin://
 *          Pipe Out:       stdout://
 *          Listen:         VOID ("")
 *          Peer:           VOID ("")
 * 
 * Any of these URLs may be empty.
 * 
 * The Pipe In and -Out URL may be
 * 
 *  - ""                        "The Void" ... nothing to read from nor to write to
 *  - "ipc:///FILE"             a local UNIX Domain IPC Socket
 *  - "ipc:// *"                create a local UNIX Domain IPC Socket [omit the space!]
 *  - "tcp://INTERFACE:PORT"    a TCP/IP connection
 *  - "stdin://"                read from stdin (only Pipe IN)
 *  - "stdout://"               write to stdout (only Pipe OUT)
 * 
 * The listen URL may be
 * 
 *  - ""                        "The Void" ... nothing to read from nor to write to
 *  - "ipc:///FILE"             a local UNIX Domain IPC Socket
 *  - "ipc:// *"                create a local UNIX Domain IPC Socket [omit the space!]
 *  - "tcp://INTERFACE:PORT"    a TCP/IP connection
 * 
 * Samples for the INTERFACE part of the tcp:// connection:
 * 
 *  - *                 ... all current network card
 *  - 192.168.156.234   ... the network card which serves the IP 192.168.156.234
 *  - eth0              ... the "eth0" network interface card
 *  - localhost         ... the loopback device
 *
 * 
 *                  IMPORTANT ON REMOTE CONNECTIONS: 
 * 
 * When using "tcp://..." you MUST specify a port. Also we *highly* recommend to
 * exactly specify a concrete IP address. Since currently we do not guess on every
 * IP address a "tcp:// *:PORT" [omit the space!] my have. Well, we could. But it's 
 * erroneous and painful to guess it right. And there is simply no real excuse in
 * setting a real IP address here when it is a piece of cake from the user perspective.
 * 
 * Either way: when you connect (that is on alice's side to the peer and from the
 * previous module to the next: pipe-out) you MUST specify a valid address when using
 * tcp:// transport. Depending on the underlying network library, an asterisk "*" may
 * fail.
 * 
 * Only modules with an url_listen set to anything but "" will be listed on a node query
 * for modules (check node.h in bin/q3pd). Here the "hint" field is useful, since it
 * allows you to add arbitrary information to a module which is propagated across the net.
 * This hint maybe useful if you start a series of the very same module participating
 * on the very same pipeline.
 * 
 * To check all present modules on the current system use a investigation object.
 * 
 * 
 * On the session DBus it offers properties and methods under "/Module".
 * 
 *      DBus Interface: "at.ac.ait.qkd.module"
 * 
 * Properties of at.ac.ait.qkd.module
 * 
 *      -name-                      -read/write-         -description-
 * 
 *      debug                           R/W             enable/disable debug output on stderr
 * 
 *      debug_message_flow              R/W             enable/disable debug of message flow particles on stderr
 *
 *      description                      R              Description of the module
 * 
 *      hint                            R/W             an arbitrary text which helps to interconnect modules
 * 
 *      id                               R              ID of the module
 * 
 *      idle                             R              idle flag: finished work on a key for at least 1 sec ago
 * 
 *      organisation                     R              Organisation/Institute/Company of module
 * 
 *      paired                           R              Module has a peer module (but maybe not connected)
 * 
 *      process_id                       R              PID of the module process within the operating system
 * 
 *      process_image                    R              Full path to the module program
 * 
 *      processing                       R              Flag for currently processing a key
 * 
 *      random_url                      R/W             The random URL used to gain random values inside the module
 * 
 *      role                            R/W             The role the module inhabits
 * 
 *      start_time                       R              UNIX epoch timestamp of module launch
 * 
 *      state                            R              Current state of the module
 * 
 *      state_name                       R              Human readable state description of the module
 * 
 *      synchronize_keys                R/W             synchronize key-ids with remote peer module before processing
 * 
 *      synchronize_ttl                 R/W             TTL in seconds for not in-sync keys
 * 
 *      terminate_after                 R/W             number of keys left before terminating (0 --> do not terminate) [this is for testing and defines when to stop]
 *
 *      type                             R              Type of the module (Sifting, Error Correction, etc ...)
 * 
 *      type_name                        R              Human readable type description of the module
 * 
 *      url_listen                      R/W             URL for peer (serving endpoint)
 * 
 *      url_peer                        R/W             URL of a remote module this instance is connect to
 * 
 *      url_pipe_in                     R/W             URL of incoming Pipe (serving endpoint)
 * 
 *      url_pipe_out                    R/W             URL of outgoing Pipe
 * 
 * 
 *      ---- statistics of a QKD module ----
 * 
 *      keys_incoming                    R              total number of keys the module received so far
 * 
 *      keys_outgoing                    R              total number of keys the module sent so far
 * 
 *      key_bits_incoming                R              total number of key bits the module received so far
 * 
 *      key_bits_outgoing                R              total number of key bits the module sent so far
 * 
 *      disclosed_bits_incoming          R              total number of disclosed bits the module received so far in all keys
 * 
 *      disclosed_bits_outgoing          R              total number of disclosed bits the module sent so far in all keys
 * 
 *      error_bits_incoming              R              total number of error bits the module received so far in all keys
 * 
 *      error_bits_outgoing              R              total number of error bits the module sent so far in all keys
 * 
 * 
 * Methods of at.ac.ait.qkd.module
 * 
 *      -name-                                          -description-
 * 
 *      pause()                                         pauses current processing
 * 
 *      resume()                                        resumes processing (if paused)
 *      
 *      run()                                           starts the module (switch to READY state)
 * 
 *      set_urls(in, out, listen, peer)                 convenience function setting all URLs at once
 * 
 *      synchronize()                                   ensure we do have the same keys to process on both sides
 * 
 *      terminate()                                     stops the module
 *  
 * Signals of at.ac.ait.qkd.module
 * 
 *      -name-                                          -description-
 * 
 *      terminated                                      the module has just ceased functionality
 * 
 */
class module : public QObject {
    
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "at.ac.ait.qkd.module")

    Q_PROPERTY(bool debug READ debug WRITE set_debug)                                           /**< get/set module debug flag */
    Q_PROPERTY(bool debug_message_flow READ debug_message_flow WRITE set_debug_message_flow)    /**< get/set module debug flag */
    Q_PROPERTY(QString description READ description)                                            /**< get the description of the module */
    Q_PROPERTY(QString hint READ hint WRITE set_hint)                                           /**< get/set the arbitrary module hint */
    Q_PROPERTY(QString id READ id)                                                              /**< get the id of the module */
    Q_PROPERTY(bool idle READ idle)                                                             /**< get the idle flag: finished work on a key for at least 1 sec ago */
    Q_PROPERTY(QString organisation READ organisation)                                          /**< get the organisation/creator of the module */
    Q_PROPERTY(bool paired READ paired)                                                         /**< get module's paired condition */
    Q_PROPERTY(QString pipeline READ pipeline WRITE set_pipeline)                               /**< the pipeline ID this module is assigned to */
    Q_PROPERTY(unsigned int process_id READ process_id)                                         /**< process ID of the current module */
    Q_PROPERTY(QString process_image READ process_image)                                        /**< path of process image on disk */
    Q_PROPERTY(bool processing READ processing)                                                 /**< check if the module is currently processing a key  */
    Q_PROPERTY(QString random_url READ random_url WRITE set_random_url)                         /**< random source handling functions */
    Q_PROPERTY(qulonglong role READ role WRITE set_role)                                        /**< get/set module role */
    Q_PROPERTY(QString role_name READ role_name)                                                /**< human readable role name */
    Q_PROPERTY(qulonglong start_time READ start_time)                                           /**< UNIX epoch timestamp of module creation */       
    Q_PROPERTY(qulonglong state READ state)                                                     /**< the state of the module */    
    Q_PROPERTY(QString state_name READ state_name)                                              /**< the state name description of the module */
    Q_PROPERTY(bool synchronize_keys READ synchronize_keys WRITE set_synchronize_keys)          /**< get/set synchronize key ids flag */
    Q_PROPERTY(qulonglong synchronize_ttl READ synchronize_ttl WRITE set_synchronize_ttl)       /**< get/set synchronize TTL in seconds for not in-sync keys */
    Q_PROPERTY(qulonglong terminate_after READ terminate_after WRITE set_terminate_after)       /**< number of keys left before terminating (0 --> do not terminate) */    
    Q_PROPERTY(qulonglong type READ type)                                                       /**< the type of the module */    
    Q_PROPERTY(QString type_name READ type_name)                                                /**< the type name description of the module */

    Q_PROPERTY(QString url_listen READ url_listen WRITE set_url_listen)                         /**< URL for peer (serving endpoint) */
    Q_PROPERTY(QString url_peer READ url_peer WRITE set_url_peer)                               /**< URL of the peer connection (where this module connected to) */
    Q_PROPERTY(QString url_pipe_in READ url_pipe_in WRITE set_url_pipe_in)                      /**< URL of incoming Pipe (serving endpoint) */
    Q_PROPERTY(QString url_pipe_out READ url_pipe_out WRITE set_url_pipe_out)                   /**< URL of outgoing Pipe */
    
    Q_PROPERTY(qulonglong keys_incoming READ keys_incoming)                                     /**< total number of keys the module received so far */    
    Q_PROPERTY(qulonglong keys_outgoing READ keys_outgoing)                                     /**< total number of keys the module sent so far */    
    Q_PROPERTY(qulonglong key_bits_incoming READ key_bits_incoming)                             /**< total number of key bits the module received so far */    
    Q_PROPERTY(qulonglong key_bits_outgoing READ key_bits_outgoing)                             /**< total number of key bits the module sent so far */    
    Q_PROPERTY(qulonglong disclosed_bits_incoming READ disclosed_bits_incoming)                 /**< total number of disclosed bits the module received so far in all keys */    
    Q_PROPERTY(qulonglong disclosed_bits_outgoing READ disclosed_bits_outgoing)                 /**< total number of disclosed bits the module sent so far in all keys */    
    Q_PROPERTY(qulonglong error_bits_incoming READ error_bits_incoming)                         /**< total number of error bits the module received so far in all keys */    
    Q_PROPERTY(qulonglong error_bits_outgoing READ error_bits_outgoing)                         /**< total number of error bits the module sent so far in all keys */    

    // friends
    friend class communicator;
    friend class module_thread;    
    friend class stash;    

    
public:
    
    
    /**
     * statistic of a module
     */
    class module_stat {
        
        
    public:
        
        
        /**
         * ctor
         */
        module_stat() {
            
            nKeysIncoming = 0;
            nKeysOutgoing = 0;
            nKeyBitsIncoming = 0;
            nKeyBitsOutgoing = 0;
            nDisclosedBitsIncoming = 0;
            nDisclosedBitsOutgoing = 0;
            nErrorBitsIncoming = 0;
            nErrorBitsOutgoing = 0;
        };
        
        
        // ------------------------------------------------------------
        // members
        
        mutable std::recursive_mutex cMutex;                /**< sync access */
        
        uint64_t nKeysIncoming;                             /**< number of keys incoming */
        uint64_t nKeysOutgoing;                             /**< number of keys outgoing  */
        uint64_t nKeyBitsIncoming;                          /**< number of keys bits incoming */
        uint64_t nKeyBitsOutgoing;                          /**< number of keys bits outgoing */
        uint64_t nDisclosedBitsIncoming;                    /**< total amount of disclosed bits published by previous modules */
        uint64_t nDisclosedBitsOutgoing;                    /**< total amount of disclosed bits published by previous modules AND the current one */
        uint64_t nErrorBitsIncoming;                        /**< total amount of error bits detected by previous modules */
        uint64_t nErrorBitsOutgoing;                        /**< total amount of error bits detected by previous modules AND the current one */
        
        
    private:
        
        
        /**
         * copy ctor (disallowed)
         * 
         * @param   rhs     right hand side
         */
        module_stat(UNUSED module_stat const & rhs) {};
        
    };


    /**
     * ctor
     * 
     * @param   sId                         identification of the module
     * @param   eType                       type of the module
     * @param   sDescription                description of the module
     * @param   sOrganisation               organisation (vendor) of module
     */
    explicit module(std::string sId, module_type eType, std::string sDescription = "", std::string sOrganisation = "");
    
    
    /**
     * dtor
     */
    virtual ~module();
    
    
    /**
     * most exact age of module, that is: now() - process start timepoint
     * 
     * @return  age of module
     */
    inline std::chrono::high_resolution_clock::duration age() const { 
        return (std::chrono::high_resolution_clock::now() - birth()); 
    }  
    
    
    /**
     * most exact date of module birth (module process start)
     * 
     * @return  timepoint of birth as exact as possible
     */
    std::chrono::high_resolution_clock::time_point birth() const;    


    /**
     * get a easy communicator object
     *
     * returns a facade object to module's internal send/recv methods
     * to be used anywhere
     *
     * @param   nKeyId                  key id to bound the communicator to
     * @param   cIncomingContext        the incoming auth context
     * @param   cOutgoingContext        the outgoing auth context
     * @return  a module communication object
     */
    typename qkd::module::communicator communicator(qkd::key::key_id nKeyId,
                                                    qkd::crypto::crypto_context & cIncomingContext, 
                                                    qkd::crypto::crypto_context & cOutgoingContext) { 
        return qkd::module::communicator(this, nKeyId, cIncomingContext, cOutgoingContext); 
    }
    
    
    /**
     * return the connection object associated with a connection type
     * 
     * @param   eType           the connection type
     * @return  the connection associated with this type
     */
    qkd::module::connection const & connection(qkd::module::connection_type eType) const;

    
    /**
     * return the module config prefix as used in configfiles
     * 
     * the config prefix is the very first part of the config key in a
     * 
     *      key = value
     * 
     * INI file.
     * 
     * It is expected to be "module.ID." where ID is the current module id.
     * 
     * This helps to distinguish the module's options from other options
     * within the same configuration file.
     * 
     * @return  the config prefix
     */
    inline std::string config_prefix() const { 
        return std::string("module." + id().toStdString() + "."); 
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
     * Opposed to the D-Bus method slot of the same name
     * this method calls exit(1) if the configuration URL
     * failed to load.
     * 
     * @param   sConfigURL      configuration file to load
     * @param   bRequired       if true, on failure calls exit(1)
     * @return  true, if we opened the config URL successfully
     */
    bool configure(QString sConfigURL, bool bRequired);

    
    /**
     * check if we are in debug mode
     * 
     * @return  true, if debug messages ought to go to stderr
     */
    inline bool debug() const { return qkd::utility::debug::enabled(); }
    
    
    /**
     * check if message flow particles are printed on stderr
     * 
     * @return  true, if debug messages of communication are pasted on stderr
     */
    bool debug_message_flow() const;

    /**
     * check if we should print key sync issues on stderr.
     *
     * @return true, iff key sync debug messages are printed on stderr
     */
    bool debug_key_sync() const;


    /**
     * return the description of the module
     * 
     * @return  the description of the module
     */
    QString description() const;
    
    
    /**
     * return the number of disclosed bits in all keys received so far
     * 
     * @return  the number of all disclosed bits in all keys received so far
     */
    inline qulonglong disclosed_bits_incoming() const { 
        std::lock_guard<std::recursive_mutex> cLock(statistics().cMutex); 
        return statistics().nDisclosedBitsIncoming; 
    }
    
    
    /**
     * return the number of disclosed bits in all keys sent so far
     * 
     * @return  the number of all disclosed bits in all keys sent so far
     */
    inline qulonglong disclosed_bits_outgoing() const { 
        std::lock_guard<std::recursive_mutex> cLock(statistics().cMutex); 
        return statistics().nDisclosedBitsOutgoing; 
    }
    
    
    /**
     * return the number of error bits in all keys received so far
     * 
     * @return  the number of all error bits in all keys received so far
     */
    inline qulonglong error_bits_incoming() const { 
        std::lock_guard<std::recursive_mutex> cLock(statistics().cMutex); 
        return statistics().nErrorBitsIncoming; 
    }
    
    
    /**
     * return the number of error bits in all keys sent so far
     * 
     * @return  the number of all error bits in all keys sent so far
     */
    inline qulonglong error_bits_outgoing() const { 
        std::lock_guard<std::recursive_mutex> cLock(statistics().cMutex); 
        return statistics().nErrorBitsOutgoing; 
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
    module_state get_state() const;
    
    
    /**
     * return the module's hint
     * 
     * @return  the hint to this module instance
     */
    QString hint() const;
    
    
    /**
     * return the id of the module
     * 
     * @return  the id of the module
     */
    QString id() const;
    
    
    /**
     * finished work on a key for at least 1 sec ago
     * 
     * @return  idle flag
     */
    bool idle() const;

    
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
    void interrupt_worker();
    
    
    /**
     * quick helper function to decide if this module runs as alice
     * 
     * @return  true, if this module act as Alice
     */
    inline bool is_alice() const { 
        return (role() == (unsigned long)qkd::module::module_role::ROLE_ALICE); 
    }
    
    
    /**
     * quick helper function to decide if this module runs as bob
     * 
     * @return  true, if this module act as Bob
     */
    inline bool is_bob() const { 
        return (role() == (unsigned long)qkd::module::module_role::ROLE_BOB); 
    }
    
    
    /**
     * test if the given key may be a module config key
     * 
     * @param   sKey            a module key
     * @return  true, if the the given string does comply to a module config key
     */
    inline bool is_config_key(std::string const & sKey) const { 
        return (sKey.substr(0, config_prefix().size()) == config_prefix()); 
    }
    
    
    /**
     * test if this current module instance is in the dying state
     * 
     * @return  true, if the module's state is about to terminate
     */
    inline bool is_dying_state() const { 
        return is_dying_state(get_state()); 
    }
    
    
    /**
     * test if the given state describes a dying state ==> we are going down
     * 
     * @param   eState      the state to check
     * @return  true, if the module's state is about to terminate
     */
    static bool is_dying_state(module_state eState) { 
        return ((eState == module_state::STATE_TERMINATED) || (eState == module_state::STATE_TERMINATING)); 
    }
    
    
    /**
     * test if this module does currently process keys
     * 
     * @return  true if we are intensively working on keys
     */
    inline bool is_running() const { 
        return (get_state() == module_state::STATE_RUNNING); 
    }
    
    
    /**
     * test if the given key denotes a standard config key
     * 
     * @param   sKey            a module key
     * @return  true, if the the given string does comply to a standard module config key
     */
    bool is_standard_config_key(std::string const & sKey) const;
    
    
    /**
     * test if the current module will synchronize keys
     * 
     * @return  true, if keys read from the previous module will be synchronized
     */
    inline bool is_synchronizing() const { 
        return (paired() && synchronize_keys() && !url_pipe_in().isEmpty()); 
    }
    
    
    /**
     * test if the current module instance is in good working condition
     * 
     * @return  true, if the module's state can take some workload (even if currently paused)
     */
    inline bool is_working_state() const { 
        return is_working_state(get_state()); 
    }
    
    
    /**
     * test if the given state describes a good working condition (though paused)
     * 
     * @param   eState      the state to check
     * @return  true, if the module's state can take some workload (even if currently paused)
     */
    static bool is_working_state(module_state eState) { 
        return ((eState == module_state::STATE_READY) || (eState == module_state::STATE_RUNNING)); 
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
    void join() const;


    /**
     * return the number of all key bits received so far
     * 
     * @return  the number of all keys bits received so far
     */
    inline qulonglong key_bits_incoming() const { 
        std::lock_guard<std::recursive_mutex> cLock(statistics().cMutex); 
        return statistics().nKeyBitsIncoming; 
    }
    
    
    /**
     * return the number of all keys bits sent so far
     * 
     * @return  the number of all all keys bits sent so far
     */
    inline qulonglong key_bits_outgoing() const { 
        std::lock_guard<std::recursive_mutex> cLock(statistics().cMutex); 
        return statistics().nKeyBitsOutgoing; 
    }
    
    
    /**
     * return the number of all keys received so far
     * 
     * @return  the number of all keys received so far
     */
    inline qulonglong keys_incoming() const { 
        std::lock_guard<std::recursive_mutex> cLock(statistics().cMutex); 
        return statistics().nKeysIncoming; 
    }
    
    
    /**
     * return the number of all keys sent so far
     * 
     * @return  the number of all keys sent so far
     */
    inline qulonglong keys_outgoing() const { 
        std::lock_guard<std::recursive_mutex> cLock(statistics().cMutex); 
        return statistics().nKeysOutgoing; 
    }
    
    
    /**
     * return the organisation/creator of the module
     * 
     * @return  the organisation/creator of the module
     */
    QString organisation() const;
    
    
    /**
     * check if this module might have an remote peer
     * 
     * this does _not_ mean the module is connected. But the module
     * will try to connect its peer
     * 
     * @return  true, if the module 
     */
    inline bool paired() const { 
        return (!url_listen().isEmpty() || !url_peer().isEmpty()); 
    }
    

    /**
     * get the pipeline id this module is assigned
     *
     * @return  the pipeline id this module is assigned
     */
    QString pipeline() const;
    
    
    /**
     * get the process id of the module
     * 
     * @return  the operating system process id of the module
     */
    inline unsigned int process_id() const { 
        return qkd::utility::environment::process_id(); 
    }
    
    
    /**
     * get the process image path of the module binary on disk
     * 
     * @return  the operating system process image of the module binary on disk
     */
    inline QString process_image() const { 
        return QString::fromStdString(qkd::utility::environment::process_image_path().string()); 
    }
    
    
    /**
     * check if the module is currently processing a key_bits_incoming    
     * 
     * @return  true, if we currently processing a key
     */
    bool processing() const;


    /**
     * get the internally used random number source
     * 
     * @return  the random number source to be used by the module
     */
    qkd::utility::random & random();

    
    /**
     * get the url of the random value source
     *
     * @return  the url of the random value source
     */
    QString random_url() const;
    
    
    /**
     * rest 50 milliseconds for a next communication try
     */
    void rest() const;

    
    /**
     * return the role of the module
     * 
     * @return  the role as integer
     */
    qulonglong role() const;
    
    
    /**
     * return the role name of the module
     * 
     * @return  the human readable role name 
     */
    inline QString role_name() const { 
        return role_name((module_role)role()); 
    }
    
    
    /**
     * return the role name description for a given role
     * 
     * @param   eRole           the role in question
     * @return  the human readable role name 
     */
    static QString role_name(module_role eRole);
    
    
    /**
     * return the DBus service name
     * 
     * This returns "at.ac.ait.qkd.module." + id() + PID.
     * If you want to use a different service name, then 
     * overwrite this method in sub classes
     * 
     * @return  the DBus service name this module to register
     */
    virtual QString service_name() const;
    

    /**
     * set the debug flag
     * 
     * @param   bDebug      new debug value
     */
    inline void set_debug(bool bDebug) { 
        qkd::utility::debug::enabled() = bDebug; 
    }
    
    
    /**
     * set the debug message particle flow flag
     * 
     * @param   bDebug      new debug value for message particles
     */
    void set_debug_message_flow(bool bDebug);


    /**
     * set the debug key sync flag
     *
     * @param   bDebug      new debug value for key syncs
     */
    void set_debug_key_sync(bool bDebug);


    /**
     * set the module's hint
     * 
     * @param   sHint       the new hint to this module instance
     */
    void set_hint(QString sHint);
    
    
    /**
     * set the new pipeline id this module is assigned
     *
     * @param   sPipeline   the new pipeline id this module is assigned
     */
    void set_pipeline(QString sPipeline);
    
    
    /**
     * set the url of the random value source
     *
     * @param   sRandomUrl      the new url of the random value source
     */
    void set_random_url(QString sRandomUrl);
    
    
    /**
     * set the role
     * 
     * @param   nRole           the new role
     */
    void set_role(qulonglong nRole);
    
    
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
    void set_terminate_after(qulonglong nTerminateAfter);


    /**
     * set the synchronize key ids flag
     * 
     * @param   bSynchronize    the new synchronize key id flag
     */
    void set_synchronize_keys(bool bSynchronize);


    /**
     * set the synchronize TTL for not in-sync keys
     * 
     * @param   nTTL            the new synchronize TTL in seconds
     */
    void set_synchronize_ttl(qulonglong nTTL);


    /**
     * sets a new LISTEN URL
     *
     * @param   sURL        the new LISTEN URL
     */
    virtual void set_url_listen(QString sURL);
    
    
    /**
     * sets a new PEER URL
     *
     * @param   sURL        the new PEER URL
     */
    virtual void set_url_peer(QString sURL);
    
    
    /**
     * sets a new pipeline INCOMING URL
     *
     * @param   sURL        the new pipe in URL
     */
    virtual void set_url_pipe_in(QString sURL);
    
    
    /**
     * sets a new pipeline OUTGOING URL
     *
     * @param   sURL        the new pipe out URL
     */
    virtual void set_url_pipe_out(QString sURL);
    
    
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
    void start_later();
    
    
    /**
     * UNIX epoch timestamp of launch
     * 
     * Seconds since 1/1/1970 when this module
     * has been launched.
     * 
     * @return  UNIX epoch timestamp of module launch
     */
    unsigned long start_time() const;
    
    
    /**
     * return the state of the module
     * 
     * @return  the state as integer
     */
    qulonglong state() const;
    
    
    /**
     * return the state description of the module
     * 
     * @return  the human readable state name 
     */
    inline QString state_name() const { 
        return state_name((module_state)state()); 
    }
    
    
    /**
     * return the state name description for a given state
     * 
     * @param   eState      the module state of the module questioned
     * @return  the human readable module state name 
     */
    static QString state_name(module_state eState);
    
    
    /**
     * get the module statistic
     * 
     * @return  the module statistics
     */
    module_stat & statistics();
    
    
    /**
     * get the module statistic
     * 
     * @return  the module statistics
     */
    module_stat const & statistics() const;
    
    
    /**
     * get the synchronize key ids flag
     * 
     * @return  the synchronize key id flag
     */
    bool synchronize_keys() const;


    /**
     * get the synchronize TTL for not in-sync keys
     * 
     * @return  the synchronize TTL in seconds
     */
    qulonglong synchronize_ttl() const;


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
    qulonglong terminate_after() const;


    /**
     * return the type of the module
     * 
     * @return  the module-type as integer
     */
    qulonglong type() const;
    
    
    /**
     * return the type name description of the module
     * 
     * @return  the human readable module-type name 
     */
    inline QString type_name() const { 
        return type_name((module_type)type()); 
    }
    
    
    /**
     * return the type name description for a given type
     * 
     * @param   eType           the type of the module questioned
     * @return  the human readable module-type name 
     */
    static QString type_name(module_type eType);
    

    /**
     * return the URL for peer (serving endpoint)
     * 
     * @return  the URL for peer (serving endpoint)
     */
    QString url_listen() const;
    

    /**
     * return the URL of the peer connection (where this module connected to)
     * 
     * @return  the URL of the peer connection (where this module connected to)
     */
    QString url_peer() const;
    
    
    /**
     * return the URL of incoming Pipe (serving endpoint)
     * 
     * @return  the URL of incoming Pipe (serving endpoint)
     */
    QString url_pipe_in() const;
    
    
    /**
     * return the URL of outgoing Pipe
     * 
     * @return  the URL of outgoing Pipe
     */
    QString url_pipe_out() const;
    
    
public slots:
    
    
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
    Q_NOREPLY void configure(QString sConfigURL);
    
    
    /**
     * pauses current processing
     */
    Q_NOREPLY void pause();
    
    
    /**
     * resumes processing (if paused)
     */
    Q_NOREPLY void resume();
    
    
    /**
     * starts the module
     * 
     * This internally creates a new thread of execution which will finally
     * invoke the "work" function, which has to be overwritten with actual code.
     */
    Q_NOREPLY void run();
    
    
    /**
     * convenience method to set all URLs at once
     * 
     * @param   sURLPipeIn          pipe in URL
     * @param   sURLPipeOut         pipe out URL
     * @param   sURLListen          listen URL
     * @param   sURLPeer            peer URL
     */
    Q_NOREPLY void set_urls(QString sURLPipeIn, QString sURLPipeOut, QString sURLListen, QString sURLPeer);
    
    
    /**
     * ensure we have the same keys on both sides to process
     */
    Q_NOREPLY void synchronize();
    
    
    /**
     * stops the module
     * 
     * This is a graceful shutdown
     */
    Q_NOREPLY void terminate();
    

protected:
    
    
    /**
     * accept a key for processing
     * 
     * each time a key is ought to be processed by a module, this
     * method is called. if this method returns false the key is
     * discarded
     * 
     * The default implementation discards DISCLOSED keys regardless
     * of a key's data encoding.
     * 
     * @param   cKey            the key to check
     * @return  true, if the key should be processed by this module
     */
    virtual bool accept(qkd::key::key const & cKey) const;
    
    
    /**
     * add the module's data to a key's metadata on incoming
     * 
     * This method is invoked for every new key entering the
     * module's space.
     * 
     * The given property_tree already points to the current module
     * node inside the tree. You may add any value like this:
     * 
     *      cPropertyTree.put("alpha", 1234);
     *      cPropertyTree.put("beta", 3.1415);
     *      cPropertyTree.put("beta.<xmlattr>.math", "pi");
     *      cPropertyTree.put("some_group_name.sub_group.gamma", "this is a string value");
     * 
     * You can retrieve such values like:
     * 
     *      int a = cPropertyTree.get<int>("alpha");
     *      double b = cPropertyTree.get<double>("beta")
     *      std::string g = cPropertyTree.get<std::string>("some_group_name.sub_group.gamma");
     * 
     * Overwrite this method to add your own module's values to the key's meta-data.
     * 
     * @param   cPropertyTree       the key's current module data
     * @param   cKey                the new key
     */
    virtual void add_metadata_in(boost::property_tree::ptree & cPropertyTree, qkd::key::key const & cKey) const;
    
    
    /**
     * add the module's data to a key's metadata on outgoing
     * 
     * This method is invoked for every key leaving the
     * module's space.
     * 
     * The given property_tree already points to the current module
     * node inside the tree. You may add any value like this:
     * 
     *      cPropertyTree.put("alpha", 1234);
     *      cPropertyTree.put("beta", 3.1415);
     *      cPropertyTree.put("beta.<xmlattr>.math", "pi");
     *      cPropertyTree.put("some_group_name.sub_group.gamma", "this is a string value");
     * 
     * You can retrieve such values like:
     * 
     *      int a = cPropertyTree.get<int>("alpha");
     *      double b = cPropertyTree.get<double>("beta")
     *      std::string g = cPropertyTree.get<std::string>("some_group_name.sub_group.gamma");
     * 
     * Overwrite this method to add your own module's values to the key's meta-data.
     * 
     * @param   cPropertyTree       the key's current module data
     * @param   cKey                the new key
     */
    virtual void add_metadata_out(boost::property_tree::ptree & cPropertyTree, qkd::key::key const & cKey) const;
    
    
    /**
     * apply the loaded key value map to the module
     * 
     * @param   sURL            URL of config file loaded
     * @param   cConfig         map of key --> value
     */
    virtual void apply_config(std::string const & sURL, qkd::utility::properties const & cConfig);
    
    
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
    bool apply_standard_config(std::string const & sKey, std::string const & sValue);
    
   
    /**
     * create the metadata module's node
     * 
     * This method creates an entry in the key's metadata for the current module.
     * 
     * @param   cKey                the key for which to modify the metadata
     */
    void create_metadata_module_node(qkd::key::key & cKey) const;
    
    
    /**
     * get the next key from the previous module
     * 
     * This method is called inside of work(). Call this 
     * method inside of process() if you know _exactly_ what
     * you are doing.
     * 
     * You should not need to call this directly.
     * 
     * @param   cKey                    this will receive the next key
     * @return  true, if reading was successful
     */
    virtual bool read(qkd::key::key & cKey);


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
     * @return  true, if we have received a message, false else
     */
    virtual bool recv(qkd::module::message & cMessage, 
                      qkd::crypto::crypto_context & cAuthContext, 
                      qkd::module::message_type eType = qkd::module::message_type::MESSAGE_TYPE_DATA) DEPRECATED;

    
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
     * @param   nKeyId              the key id we are currently working on
     * @param   cMessage            this will receive the message
     * @param   cAuthContext        the authentication context involved
     * @param   eType               message type to receive
     * @return  true, if we have received a message, false else
     */
    virtual bool recv(qkd::key::key_id nKeyId,
                      qkd::module::message & cMessage, 
                      qkd::crypto::crypto_context & cAuthContext, 
                      qkd::module::message_type eType = qkd::module::message_type::MESSAGE_TYPE_DATA);

    
    /**
     * register this object on the DBus
     * 
     * this method calls service_name() and registers
     * the service as /Module on the DBus
     * 
     * overwrite this method for a different DBus
     * registration technique
     */
    virtual void register_dbus();

    
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
     * The path index holds the number of the path to choose. 
     * On -1 the next suitable path(s) are taken.
     * 
     * @deprecated
     * @param   cMessage            the message to send
     * @param   cAuthContext        the authentication context involved
     * @param   nPath               path index to send
     * @returns true, if successfully sent
     */
    virtual bool send(qkd::module::message & cMessage, 
                      qkd::crypto::crypto_context & cAuthContext, 
                      int nPath = -1) DEPRECATED;

    
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
    virtual bool send(qkd::key::key_id nKeyId, 
                      qkd::module::message & cMessage, 
                      qkd::crypto::crypto_context & cAuthContext, 
                      int nPath = -1);

    
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
     * @param   nPath       path number to write
     * @return  true, if writing was successful
     */
    virtual bool write(qkd::key::key & cKey, int nPath = -1);

    
signals:
    
    
    /**
     * the module has been paused
     * 
     * the module has been running a while (run() and/or resume()) and
     * a pause() command has been issued recently.
     * 
     * The module is in a waiting state.
     * 
     * beware: this may be called from within the module's worker thread
     */
    void paused();
    
    
    /**
     * the module is ready to process keys
     * 
     * beware: this may be called from within the module's worker thread
     */
    void ready();
    
    
    /**
     * the module starts key processing
     * 
     * this signal is emitted whenever the module resumes
     * key processing either be it as first start next to
     * run() or after a pause() command.
     * 
     * beware: this may be called from within the module's worker thread
     */
    void resumed();
    
    
    /**
     * the module has finished execution
     *
     * beware: this may be called from within the module's worker thread
     */
    void terminated();
    
    
private slots:
    
    
    /**
     * this is the start method call
     * 
     * (reason it is not public: it would be visible 
     * at DBus, which is currently not an option)
     */
    void delayed_start();
    
    
    /**
     * initialize the module
     */
    void init();
    
    
private:
    
    
    /**
     * add the module's data to a key's metadata when incoming
     * 
     * This method is invoked for every new key entering the
     * module's space.
     * 
     * @param   cKey                the new key
     */
    void add_metadata_in(qkd::key::key & cKey) const;
    
    
    /**
     * add the module's data to a key's metadata when outgoing
     * 
     * This method is invoked for every key leaving the
     * module's space.
     * 
     * @param   cKey                the new key
     */
    void add_metadata_out(qkd::key::key & cKey) const;
    
    
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
    virtual bool process(qkd::key::key & cKey, 
            qkd::crypto::crypto_context & cIncomingContext, 
            qkd::crypto::crypto_context & cOutgoingContext);
            
            
    /**
     * this is the module's working method on a list of keys
     * 
     * Overwrite this if your work results in multiple keys.
     * 
     * This method is called by work() for a new key. If the input pipe
     * has been set to void ("") then the input key is always a NULL key
     * and the crypto contexts are "null".
     * 
     * @param   cWorkload               the work to be done
     */
    virtual void process(qkd::module::workload & cWorkload) {
        for (auto & w : cWorkload) {
            w.bForward = process(w.cKey, w.cIncomingContext, w.cOutgoingContext);
        }
    }
    
    
    /**
     * this is the entry point of the main thread worker
     */
    void thread();
    
    
    /**
     * this is the real work function
     * 
     * This is called indirectly by run().
     * 
     * This contains the main worker loop, which is sketched (roughly!) here:
     * 
     * 1) as long we are PAUSED: wait
     * 2) exit if not RUNNING
     * 3) get a key (if input Pipe has been specified)
     * 4) invoke process()
     * 5) write key (if process return true)
     * 6) return to 1
     * 
     * You may overwrite this method. But this changes module operation
     * dramatically.
     * 
     * There will be dragons. You've been warned.
     */
    virtual void work();
    
    
    // pimpl
    class module_internal;
    std::shared_ptr<module_internal> d;
};


}
    
}


#endif


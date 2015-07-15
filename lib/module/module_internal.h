/* 
 * module_internal.h
 * 
 * QKD module internal definition
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


#ifndef __QKD_MODULE_MODULE_INTERNAL_H_
#define __QKD_MODULE_MODULE_INTERNAL_H_


// ------------------------------------------------------------
// incs

#include "config.h"

// 0MQ
#include <zmq.h>

#include <atomic>
#include <condition_variable>
#include <map>
#include <queue>
#include <thread>

#include <qkd/module/module.h>


// ------------------------------------------------------------
// decl

namespace qkd {

namespace module {


// fwd
class module;


/**
 * the internal private module class
 */
class qkd::module::module::module_internal {
    
public:

    
    module * cModule;                           /**< containg module */

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
    
    void * cSocketListener;                     /**< listener socket */
    void * cSocketPeer;                         /**< connection to peer */
    void * cSocketPipeIn;                       /**< incoming 0MQ socket of the pipe */
    void * cSocketPipeOut;                      /**< outgoing 0MQ socket of the pipe */
    
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
     * ctor
     */
    module_internal(module * cParentModule, std::string sId) : cModule(cParentModule), sId(sId), nStartTimeStamp(0) { 
        
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
        
        nTerminateAfter = 0;
    };
    
    
    /**
     * dtor
     */
    ~module_internal() {
        
        // clean up
        if (cSocketListener != nullptr) zmq_close(cSocketListener);
        cSocketListener = nullptr;
        if (cSocketPeer != nullptr) zmq_close(cSocketPeer);
        cSocketPeer = nullptr;
        if (cSocketPipeIn != nullptr) zmq_close(cSocketPipeIn);
        cSocketPipeIn = nullptr;
        if (cSocketPipeOut != nullptr) zmq_close(cSocketPipeOut);
        cSocketPipeOut = nullptr;
    };
    

    /**
     * add key statistics for incoming
     * 
     * @param   cKey        new key arrived
     */
    void add_stats_incoming(qkd::key::key const & cKey);


    /**
     * add key statistics for outgoing
     * 
     * @param   cKey        key sent
     */
    void add_stats_outgoing(qkd::key::key const & cKey);


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
     * dump current module config
     */
    void debug_config();


    /**
     * dump key PULL to stderr
     *
     * @param   cKey        key to dump
     */
    void debug_key_pull(qkd::key::key const & cKey);


    /**
     * dump key PUSH  to stderr
     *
     * @param   cKey        key to dump
     */
    void debug_key_push(qkd::key::key const & cKey);


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
     * clean resources on a socket
     *
     * @param   cSocket     the socket to release
     */
    void release_socket(void * & cSocket);

    
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
     * setup socket with high water mark and timeout
     *
     * also linger will be set to 0
     *
     * @param   cSocket             socket to modify
     * @param   nHighWaterMark      high water mark
     * @param   nTimeout            timeout on socket
     * @param   bTimeoutRecv        set timeout on receive
     * @param   bTimeoutSent        set timeout on send
     */
    void setup_socket(void * & cSocket, int nHighWaterMark, int nTimeout); 
    
    
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


}

}

#endif


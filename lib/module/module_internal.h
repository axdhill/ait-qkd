/* 
 * module_internal.h
 * 
 * QKD module internal definition
 *
 * Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *
 * Copyright (C) 2015-2016 AIT Austrian Institute of Technology
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

#include <atomic>
#include <condition_variable>
#include <map>
#include <queue>
#include <thread>

#include <qkd/module/connection.h>
#include <qkd/module/module.h>

#include "stash.h"


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
    
    // ---- variables ---

    
    module * cModule;                           /**< containing module */

    module_stat cStat;                          /**< the module statistic */
    
    std::string sId;                            /**< the id of the module */
    std::string sDescription;                   /**< the description of the module */
    std::string sOrganisation;                  /**< the organisation/creator of the module */
    std::string sPipeline;                      /**< the pipeline id this module is assigned */
    std::string sHint;                          /**< the module's hint */
    qkd::utility::random cRandom;               /**< random number generator */
    std::string sRandomUrl;                     /**< random number source URL */
    module_role eRole;                          /**< role of the module */
    unsigned long nStartTimeStamp;              /**< init UNIX epoch: time of birth */
    module_type eType;                          /**< the type of the module */

    std::atomic<uint64_t> nTerminateAfter;      /**< termination counter */
    
    std::string sDBusObjectPath;                /**< the DBus object path */

    std::mutex cURLMutex;                       /**< sync change on URLs */
    
    qkd::module::connection * cConListen;       /**< listen connection */
    qkd::module::connection * cConPeer;         /**< peer connection */
    qkd::module::connection * cConPipeIn;       /**< pipe in connection */
    qkd::module::connection * cConPipeOut;      /**< pipe out connection */
    
    
    std::chrono::high_resolution_clock::time_point cModuleBirth;        /**< timestamp of module birth */
    
    std::thread cModuleThread;                  /**< the real module worker */
    
    std::atomic<bool> bProcessing;              /**< processing flag */

    std::atomic<bool> bDebugMessageFlow;        /**< debug message flow for send and recv packages */

    
    qkd::module::stash * cStash;                /**< the module stash */

    std::chrono::system_clock::time_point cLastProcessedKey;    /**< timestamp of last processed key */
    
    
    // ---- methods ---
    
    
    /**
     * ctor
     * 
     * @param   cParentModule       the parent module of this inner module
     * @param   sId                 module id
     */
    module_internal(module * cParentModule, std::string sId);
    
    
    /**
     * dtor
     */
    ~module_internal();
    
    
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


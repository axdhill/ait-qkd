/* 
 * stash.h
 * 
 * QKD module key sync data
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


#ifndef __QKD_MODULE_STASH_H_
#define __QKD_MODULE_STASH_H_


// ------------------------------------------------------------
// incs

#include <atomic>
#include <chrono>
#include <exception>
#include <list>

#include <qkd/key/key.h>
#include <qkd/module/message.h>


// ------------------------------------------------------------
// decl

namespace qkd {

namespace module {

    
// fwd
class module;
    
    
/**
 * the internal private module class
 */
class stash {
    
public:
    

    /**
     * this holds the information for a single stashed key
     */
    typedef struct {
        
        qkd::key::key cKey;                                 /**< the key which is currently not present within the peer module */
        std::chrono::system_clock::time_point cStashed;     /**< time point of stashing */
        
        /**
         * age of the stashed key in seconds
         */
        inline uint64_t age() const { 
            return (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - cStashed)).count(); 
        };
        
    } stashed_key;
    
    
    /**
     * this is the (inorder) list of keys we received
     */
    std::list<stashed_key> m_cStash;
    

    /**
     * this is the (inorder) list of keys our peer has
     */
    std::list<qkd::key::key_id> m_cPeerStash;
    

    /**
     * synchronize key ids flag (do we sync at all?)
     */
    std::atomic<bool> m_bSynchronize;         
    
    
    /**
     * time to live in seconds for out-of-sync keys
     */
    std::atomic<uint64_t> m_nTTL;

    
    /**
     * ctor
     * 
     * @param   cModule     the parent module
     */
    stash(qkd::module::module * cModule) : m_bSynchronize(true), m_nTTL(10), m_cModule(cModule) { 
        if (!m_cModule) throw std::invalid_argument("stash: parent module is null"); 
    }
    
    
    /**
     * pick a key which occures first in both lists and remove it
     * 
     * If not such key exits, a key with is_null() == true is returned
     * 
     * @return  the first key in both lists
     */
    qkd::key::key pick();
    
    
    /**
     * removes keys which expired their TTL
     */
    void purge();
    
    
    /**
     * push a new key into our own current list
     * 
     * @param   cKey        key to push
     */
    void push(qkd::key::key & cKey);
    
    
    /**
     * process a received sync message
     * 
     * @param   cMessage        the message containing peer's keys
     */
    void recv(qkd::module::message & cMessage);
    
    
    /**
     * sends our keys to the peer
     */
    void send();
    
    
    /**
     * does a sync step
     */
    void sync();
    
    
private:
    

    /**
     * pick a key as alice which occures first in both lists and remove it
     * 
     * If not such key exits, a key with is_null() == true is returned
     * 
     * @return  the first key in both lists
     */
    qkd::key::key pick_alice();
    
    
    /**
     * pick a key as bob which occures first in both lists and remove it
     * 
     * If not such key exits, a key with is_null() == true is returned
     * 
     * @return  the first key in both lists
     */
    qkd::key::key pick_bob();
    
    

    /**
     * the module we operate on
     */
    qkd::module::module * m_cModule;
    
};


}

}

#endif


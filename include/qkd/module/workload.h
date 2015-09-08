/*
 * workload.h
 * 
 * This defines workload for a module to process
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

 
#ifndef __QKD_MODULE_WORKLOAD_H_
#define __QKD_MODULE_WORKLOAD_H_


// ------------------------------------------------------------
// incs

#include <list>

// ait
#include <qkd/crypto/context.h>
#include <qkd/key/key.h>


// ------------------------------------------------------------
// decls


namespace qkd {
    
namespace module {    
    
/**
 * Work is the single unit consiting of key and auth-contexts for a module to process
 */
typedef struct {
    
    qkd::key::key cKey;                                         /**< the current key to process */
    qkd::crypto::crypto_context cIncomingContext;               /**< authentication context for incoming messages */
    qkd::crypto::crypto_context cOutgoingContext;               /**< authentication context for outgoing messages */
    bool bForward;                                              /**< true, if work done and key shoudl be forwarded */
    int nPath;                                                  /**< path number to send the key, -1 == framework pick */
    
    inline bool is_null() const { return cKey.is_null(); }      /**< check if this is an empty workload */
    
} work;


/**
 * a workload is the list of work for a module in one single process step
 */
typedef std::list<work> workload;
    
    
}
    
}


#endif


/*
 * key_move.cpp
 *
 * implement the base Q3P key movement facility
 * 
 * Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
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

// ait
#include <qkd/key/key_ring.h>
#include <qkd/q3p/engine.h>
#include <qkd/utility/syslog.h>

#include "key_move.h"


using namespace qkd::q3p::protocol;


// ------------------------------------------------------------
// code


/**
 * ctor
 * 
 * @param   cSocket     the socket we operate on
 * @param   cEngine     the parent engine
 * @throws  protocol_no_engine
 */
key_move::key_move(QAbstractSocket * cSocket, qkd::q3p::engine_instance * cEngine) : protocol(cSocket, cEngine) {
}


/**
 * copy a number of keys from the common store to a buffer
 * 
 * a copy sets the count values on each key to 1.
 * if a copy could not be accomplished then the common store key
 * is also reset to 1
 * 
 * @param   cCommonStoreKeys        list of common store keys
 * @param   cBufferKeys             list of buffer keys
 * @param   cBuffer                 the buffer to copy to
 * @return  keys ids sucessfully copied
 */
qkd::key::key_vector key_move::copy_buffer(qkd::key::key_vector const & cCommonStoreKeys, qkd::key::key_vector const & cBufferKeys, qkd::q3p::key_db & cBuffer) {
    
    qkd::key::key_vector cCopied;
    
    // how many keys in buffer list for each common store key?
    uint64_t nCommonStoreToBufferRatio = engine()->common_store()->quantum() / cBuffer->quantum();
    
    uint64_t nBufferKeyIndex = 0;
    
    // iterate over the common store keys
    for (auto & nKeyId : cCommonStoreKeys) {

        // still enough buffer keys left to place?
        if ((cBufferKeys.size() - nBufferKeyIndex) < nCommonStoreToBufferRatio) break;
        
        // find key in the common store
        qkd::key::key cKey = engine()->common_store()->get(nKeyId);
        if (cKey == qkd::key::key::null()) {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "copy key from common store to buffer peer mismatch: unknown common store keyid.";
        }
        else {
            
            // place key in buffer
            qkd::key::key_ring cKeyRing(cBuffer->quantum());
            cKeyRing << cKey;
            
            // walk over the ring 
            for (unsigned int i = 0; i < cKeyRing.size(); i++) {
                
                // create the buffer key
                qkd::key::key cKey(cBufferKeys.at(nBufferKeyIndex), cKeyRing.at(i).data());
                cBuffer->set(cKey);
                cBuffer->set_key_count(cKey.id(), 1);
                cBuffer->set_eventual_sync(cKey.id());
                
                nBufferKeyIndex++;
            }

            // delete from common store
            engine()->common_store()->set_key_count(nKeyId, 1);
            
            // take into result set
            cCopied.push_back(nKeyId);
        }
    }
    
    return cCopied;
}


/**
 * copy a number of keys from the common store to the application buffer
 * 
 * this is a shortcut for copy_buffer
 * 
 * @param   cCommonStoreKeys        list of common store keys
 * @param   cBufferKeys             list of buffer keys
 * @return  keys ids sucessfully copied
 */
qkd::key::key_vector key_move::copy_application(qkd::key::key_vector const & cCommonStoreKeys, qkd::key::key_vector const & cBufferKeys) {
    return copy_buffer(cCommonStoreKeys, cBufferKeys, engine()->application_buffer());
}


/**
 * copy a number of keys from the common store to the incoming buffer
 * 
 * this is a shortcut for copy_buffer
 * 
 * @param   cCommonStoreKeys        list of common store keys
 * @param   cBufferKeys             list of buffer keys
 * @return  keys ids sucessfully copied
 */
qkd::key::key_vector key_move::copy_incoming(qkd::key::key_vector const & cCommonStoreKeys, qkd::key::key_vector const & cBufferKeys) {
    return copy_buffer(cCommonStoreKeys, cBufferKeys, engine()->incoming_buffer());
}


/**
 * copy a number of keys from the common store to the outgoing buffer
 * 
 * this is a shortcut for copy_buffer
 * 
 * @param   cCommonStoreKeys        list of common store keys
 * @param   cBufferKeys             list of buffer keys
 * @return  keys ids sucessfully copied
 */
qkd::key::key_vector key_move::copy_outgoing(qkd::key::key_vector const & cCommonStoreKeys, qkd::key::key_vector const & cBufferKeys) {
    return copy_buffer(cCommonStoreKeys, cBufferKeys, engine()->outgoing_buffer());
}


/**
 * move a number of keys from the common store to a buffer
 * 
 * a move resets the count values on each key to 0.
 * if a move could not be accomplished then the common store key
 * is also reset to 0
 * 
 * @param   cCommonStoreKeys        list of common store keys
 * @param   cBufferKeys             list of buffer keys
 * @param   cBuffer                 the buffer to move to
 * @return  keys ids sucessfully moved
 */
qkd::key::key_vector key_move::move_buffer(qkd::key::key_vector const & cCommonStoreKeys, qkd::key::key_vector const & cBufferKeys, qkd::q3p::key_db & cBuffer) {
    
    qkd::key::key_vector cMoved;
    
    // remember old charge
    uint64_t nOldBufferCharge = cBuffer->count();
    uint64_t nOldCommonStoreCharge = engine()->common_store()->count();
    
    // how many keys in buffer list for each common store key?
    uint64_t nCommonStoreToBufferRatio = engine()->common_store()->quantum() / cBuffer->quantum();
    
    uint64_t nBufferKeyIndex = 0;
    
    // iterate over the common store keys
    for (auto & nKeyId : cCommonStoreKeys) {

        // still enough buffer keys left to place?
        if ((cBufferKeys.size() - nBufferKeyIndex) < nCommonStoreToBufferRatio) break;
        
        // find key in the common store
        qkd::key::key cKey = engine()->common_store()->get(nKeyId);
        if (cKey == qkd::key::key::null()) {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "move key from common store to buffer peer mismatch: unknown common store keyid.";
        }
        else {
            
            // place key in buffer
            qkd::key::key_ring cKeyRing(cBuffer->quantum());
            cKeyRing << cKey;
            
            // walk over the ring 
            for (unsigned int i = 0; i < cKeyRing.size(); i++) {
                
                // create the buffer key
                qkd::key::key cKey(cBufferKeys.at(nBufferKeyIndex), cKeyRing.at(i).data());
                cBuffer->set(cKey);
                cBuffer->set_key_count(cKey.id(), 0);
                cBuffer->set_real_sync(cKey.id());
                
                nBufferKeyIndex++;
            }

            // delete from common store
            engine()->common_store()->del(nKeyId);
            
            // take into result set
            cMoved.push_back(nKeyId);
        }
    }
    
    // tell environment
    uint64_t nNewBufferCharge = cBuffer->count();
    uint64_t nNewCommonStoreCharge = engine()->common_store()->count();
    
    if (nNewBufferCharge >= nOldBufferCharge) cBuffer->emit_charge_change(nNewBufferCharge - nOldBufferCharge, 0);
    else cBuffer->emit_charge_change(0, nOldBufferCharge - nNewBufferCharge);

    if (nNewCommonStoreCharge >= nOldCommonStoreCharge) engine()->common_store()->emit_charge_change(nNewCommonStoreCharge - nOldCommonStoreCharge, 0);
    else engine()->common_store()->emit_charge_change(0, nOldCommonStoreCharge - nNewCommonStoreCharge);

    return cMoved;
}


/**
 * move a number of keys from the common store to the application buffer
 * 
 * @param   cKeyMap         key ids to move
 * @return  keys ids sucessfully moved
 */
qkd::key::key_vector key_move::move_application(qkd::key::key_vector const & cCommonStoreKeys, qkd::key::key_vector const & cBufferKeys) {
    return move_buffer(cCommonStoreKeys, cBufferKeys, engine()->application_buffer());
}


/**
 * move a number of keys from the common store to the incoming buffer
 * 
* @param   cKeyMap         key ids to move
 * @return  keys ids sucessfully moved
 */
qkd::key::key_vector key_move::move_incoming(qkd::key::key_vector const & cCommonStoreKeys, qkd::key::key_vector const & cBufferKeys) {
    return move_buffer(cCommonStoreKeys, cBufferKeys, engine()->incoming_buffer());
}


/**
 * move a number of keys from the common store to the outgoing buffer
 * 
* @param   cKeyMap         key ids to move
 * @return  keys ids sucessfully moved
 */
qkd::key::key_vector key_move::move_outgoing(qkd::key::key_vector const & cCommonStoreKeys, qkd::key::key_vector const & cBufferKeys) {
    return move_buffer(cCommonStoreKeys, cBufferKeys, engine()->outgoing_buffer());
}

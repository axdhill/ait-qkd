/*
 * key_move.h
 * 
 * base of load protocols: enable movement of keys between several buffers
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

 
#ifndef __QKD_Q3P_PROTOCOL_KEY_MOVE_H_
#define __QKD_Q3P_PROTOCOL_KEY_MOVE_H_


// ------------------------------------------------------------
// incs

#include <inttypes.h>

// Qt
#include <QtCore/QObject>
#include <QtNetwork/QAbstractSocket>

// ait
#include <qkd/key/key.h>
#include <qkd/q3p/db.h>

#include "protocol.h"


// ------------------------------------------------------------
// decls


namespace qkd {
    
namespace q3p {    

namespace protocol {    

    
/**
 * This class is the base for key movement protocols
 */
class key_move : public protocol {
    

    Q_OBJECT
    
    
public:


    /**
     * ctor
     * 
     * @param   cSocket     the socket we operate on
     * @param   cEngine     the parent engine
     * @throws  protocol_no_engine
     */
    key_move(QAbstractSocket * cSocket, qkd::q3p::engine_instance * cEngine);


protected:
    
    
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
     * @return  keys ids successfully copied
     */
    qkd::key::key_vector copy_buffer(qkd::key::key_vector const & cCommonStoreKeys, qkd::key::key_vector const & cBufferKeys, qkd::q3p::key_db & cBuffer);
    
    
    /**
     * copy a number of keys from the common store to the application buffer
     * 
     * this is a shortcut for copy_buffer
     * 
     * @param   cCommonStoreKeys        list of common store keys
     * @param   cBufferKeys             list of buffer keys
     * @return  keys ids successfully copied
     */
    qkd::key::key_vector copy_application(qkd::key::key_vector const & cCommonStoreKeys, qkd::key::key_vector const & cBufferKeys);
    
    
    /**
     * copy a number of keys from the common store to the incoming buffer
     * 
     * this is a shortcut for copy_buffer
     * 
     * @param   cCommonStoreKeys        list of common store keys
     * @param   cBufferKeys             list of buffer keys
     * @return  keys ids successfully copied
     */
    qkd::key::key_vector copy_incoming(qkd::key::key_vector const & cCommonStoreKeys, qkd::key::key_vector const & cBufferKeys);
    
    
    /**
     * copy a number of keys from the common store to the outgoing buffer
     * 
     * this is a shortcut for copy_buffer
     * 
     * @param   cCommonStoreKeys        list of common store keys
     * @param   cBufferKeys             list of buffer keys
     * @return  keys ids successfully copied
     */
    qkd::key::key_vector copy_outgoing(qkd::key::key_vector const & cCommonStoreKeys, qkd::key::key_vector const & cBufferKeys);

    
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
     * @return  keys ids successfully moved
     */
    qkd::key::key_vector move_buffer(qkd::key::key_vector const & cCommonStoreKeys, qkd::key::key_vector const & cBufferKeys, qkd::q3p::key_db & cBuffer);
    
    
    /**
     * move a number of keys from the common store to the application buffer
     * 
     * this is a shortcut for move_buffer
     * 
     * @param   cCommonStoreKeys        list of common store keys
     * @param   cBufferKeys             list of buffer keys
     * @return  keys ids successfully moved
     */
    qkd::key::key_vector move_application(qkd::key::key_vector const & cCommonStoreKeys, qkd::key::key_vector const & cBufferKeys);
    
    
    /**
     * move a number of keys from the common store to the incoming buffer
     * 
     * this is a shortcut for move_buffer
     * 
     * @param   cCommonStoreKeys        list of common store keys
     * @param   cBufferKeys             list of buffer keys
     * @return  keys ids successfully moved
     */
    qkd::key::key_vector move_incoming(qkd::key::key_vector const & cCommonStoreKeys, qkd::key::key_vector const & cBufferKeys);
    
    
    /**
     * move a number of keys from the common store to the outgoing buffer
     * 
     * this is a shortcut for move_buffer
     * 
     * @param   cCommonStoreKeys        list of common store keys
     * @param   cBufferKeys             list of buffer keys
     * @return  keys ids successfully moved
     */
    qkd::key::key_vector move_outgoing(qkd::key::key_vector const & cCommonStoreKeys, qkd::key::key_vector const & cBufferKeys);

    
};
  

}

}

}


#endif

/*
 * db_null.h
 * 
 * An invalid empty key store DB
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


#ifndef __QKD_Q3P_DB_DB_NULL_H_
#define __QKD_Q3P_DB_DB_NULL_H_


// ------------------------------------------------------------
// incs

#include <inttypes.h>

// Qt
#include <QtCore/QObject>

// ait
#include <qkd/key/key.h>
#include <qkd/q3p/db.h>


// ------------------------------------------------------------
// defs

// needed here to satisfy the MOC
#ifndef UNUSED
#   if defined(__GNUC__) || defined(__GNUCPP__)
#       define UNUSED   __attribute__((unused))
#   else
#       define UNUSED
#   endif
#endif


// ------------------------------------------------------------
// decls


namespace qkd {
    
namespace q3p {    
    
    
/**
 * This is an empty, invalid Key DB
 */
class db_null : public db {
    
    
    Q_OBJECT
    
    
public:    
    
 
    /**
     * ctor
     * 
     * @param   sURL        url of the DB to create
     */
    db_null(QString sURL) : db(sURL) {};
    
    
    /**
     * dtor
     */
    ~db_null() { close(); };
    
    
    /**
     * describe the key-DB
     * 
     * @return  a HR-string describing the key-DB
     */
    QString describe() const { return QString("null DB"); };
    
    
private:
    
    
    /**
     * close the key DB
     */
    void close_internal() {};
    

    /**
     * return the number of keys managed
     * 
     * this is actually very expensive the first time ...
     * 
     * @return  the number of keys stored in this DB
     */
    uint64_t count_internal() const { return 0; };
    
    
    /**
     * return the number of keys in real sync and not reserved
     * 
     * @return  the number of keys in real sync and not reserved
     */
    uint64_t count_real_sync_internal() const { return 0; };
    
    
    /**
     * delete the key with an ID
     * 
     * @param   nKeyId      the ID of the key to delete
     */
    void del_internal(UNUSED qkd::key::key_id nKeyId) {};
    

    /**
     * check if a given key id is in eventual sync
     * 
     * A key in eventual sync is is ought to be present in
     * real sync on the other side, though this has not been
     * confirmed yet.
     * 
     * @param   nKeyId      key id requested
     * @return  true, if the key with the id is in real sync state
     */
    bool eventual_sync_internal(UNUSED qkd::key::key_id nKeyId) const { return false; };
    

    /**
     * return a list of continuous key ids which cover at least nBytes of key material
     * 
     * all these keys are valid and do have a count of 0
     * 
     * in failure the returned list is empty
     * 
     * the given nCount is applied to all keys found, thus preventing this
     * key to be found again. A count != 0 marks the key as "reserved".
     * 
     * @param   nBytes      number of bytes
     * @param   nCount      applies nCount on the keys if nCount != 0
     * @return  a list of continuous key ids (list may be empty in case of failure)
     */
    qkd::key::key_vector find_continuous_internal(UNUSED uint64_t nBytes, UNUSED uint32_t nCount) { return qkd::key::key_vector(); };
    
    
    /**
     * returns a list of spare key IDs (no key data within the DB for those IDs)
     * 
     * the free space will cover the given space in bytes. this must be
     * a multiple of quantum(). otherwise NO key id will be found.
     * 
     * hence, the returned list may be also less or 0 if the DB is full.
     * 
     * @param   nBytes      number of bytes free (MUST be a multiple of quantum())
     * @param   nCount      applies nCount on the keys if nCount != 0
     * @return  a list of spare key ids (list may be empty in case of failure or full DB)
     */
    qkd::key::key_vector find_spare_internal(UNUSED uint64_t nBytes, UNUSED uint32_t nCount) { return qkd::key::key_vector(); };
    

    /**
     * returns a list of valid key IDs
     * 
     * the free space will cover the given space in bytes. this must be
     * a multiple of quantum(). otherwise NO key id will be found.
     * 
     * hence, the returned list may be also less or 0 if the DB is empty.
     * 
     * @param   nBytes      number of bytes valid (MUST be a multiple of quantum())
     * @param   nCount      applies nCount on the keys if nCount != 0
     * @return  a list of valid key ids (list may be empty in case of failure or empty DB)
     */
    qkd::key::key_vector find_valid_internal(UNUSED uint64_t nBytes, UNUSED uint32_t nCount) { return qkd::key::key_vector(); };
    

    /**
     * get the key with an ID
     * 
     * This looks up the DB and returns the
     * given key with the ID (or qkd::key::key::null() in
     * case of error)
     * 
     * @param   nKeyId      the ID of the key to get
     * @return  the key with the given id
     */
    qkd::key::key get_internal(UNUSED qkd::key::key_id nKeyId) const { return qkd::key::key::null(); };
    

    /**
     * inits the key-DB
     * 
     * This method MUST be overwritten in subclasses
     * 
     * @param   sURL        the url to the DB
     */
    void init(UNUSED QString sURL) {};
    
    
    /**
     * check if a given key has been injected
     * 
     * A key has been injected if the key was not 
     * negotiated with the peer
     * 
     * @param   nKeyId      key id requested
     * @return  true, if the key has been injected in DB without peer interaction
     */
    bool injected_internal(UNUSED qkd::key::key_id nKeyId) const { return false; };
    

    /**
     * inserts a key into the DB
     * 
     * this inserts a new key into DB. It tries to find a spare place,
     * places the key there and yields the ID found.
     * 
     * If the DB is full, 0 is returned.
     * 
     * NOTE: the given key MUST have the size of quantum() otherwise
     *       the call fails!
     * 
     * @param   cKey        the key to fill in
     * @return  the new key if (or 0 if full)
     */
    qkd::key::key_id insert_internal(UNUSED qkd::key::key cKey) { return 0; };
    

    /**
     * return a key counter associated with the key
     * 
     * @param   nKeyId      the ID of the key
     * @return  count of the key
     */
    uint32_t key_count_internal(UNUSED qkd::key::key_id nKeyId) const { return 0; };
    

    /**
     * return the maximum key count
     * 
     * @return  the maximum key count value
     */
    uint32_t key_count_max_internal() const { return 0; };
    

    /**
     * get the maximum ID of a key
     * 
     * @return  the maximum ID of a key
     */
    qkd::key::key_id max_id_internal() const { return 0; };
    

    /**
     * get the minimum ID of a key
     * 
     * @return  the minimum ID of a key
     */
    qkd::key::key_id min_id_internal() const { return 0; };
    

    /**
     * opened DB flag
     * 
     * @return  true, if the DB is opened
     */
    bool opened_internal() const { return false; };
    

    /**
     * get the fixed size of key material manged in the key DB
     * 
     * This is the amount of key material hold by a single
     * key in the database
     * 
     * @return  the fixed size in bytes of a key in the DB
     */
    uint64_t quantum_internal() const { return 0; };
    

    /**
     * check if a given key id is in real sync
     * 
     * A key in real sync is present at both sides
     * of the key store, at master's and at slave's
     * 
     * @param   nKeyId      key id requested
     * @return  true, if the key with the id is in real sync state
     */
    bool real_sync_internal(UNUSED qkd::key::key_id nKeyId) const { return false; };
    

    /**
     * does a reset of the intermediate stats of the DB
     * 
     * though this does not close the DB
     */
    void reset_internal() {};
    

    /**
     * retrieve a ring of keys
     * 
     * @param   cKeys       a list of keys
     * @return  a key ring holding all these keys
     */
    qkd::key::key_ring ring_internal(UNUSED qkd::key::key_vector const & cKeys) { return qkd::key::key_ring(1); };
    

    /**
     * retrieve a ring of keys
     * 
     * @param   cKeys       a list of keys
     * @return  a key ring holding all these keys
     */
    qkd::key::key_ring ring_internal(UNUSED qkd::key::key_vector const & cKeys) const { return qkd::key::key_ring(1); };
    

    /**
     * set a given key to be in eventual sync
     * 
     * A key in eventual sync is is ought to be present in
     * real sync on the other side, though this has not been
     * confirmed yet.
     * 
     * @param   nKeyId      the ID of the key to get
     * @return  true, if the key with the id is in real sync state
     */
    void set_eventual_sync_internal(UNUSED qkd::key::key_id nKeyId) {};
    

    /**
     * sets the key with the given key id to be injected
     * 
     * A key has been injected if the key was not 
     * negotiated with the peer
     * 
     * @param   nKeyId      key id requested
     */
    void set_injected_internal(UNUSED qkd::key::key_id nKeyId) {};
    

    /**
     * set the key in the DB
     * 
     * @param   cKey        the key to store in the DB
     */
    void set_internal(UNUSED qkd::key::key const & cKey) {};
    

    /**
     * sets a new key count value
     * 
     * @param   nKeyId      the ID of the key
     * @param   nCount      count of the key
     */
    void set_key_count_internal(UNUSED qkd::key::key_id nKeyId, UNUSED uint32_t nCount) {};
    

    /**
     * sets a new key count value for a list of keys
     * 
     * @param   cKeyIds     the IDs of the keys to set the key count
     * @param   nCount      count of the key
     */
    void set_key_count_internal(UNUSED qkd::key::key_vector const & cKeyIds, UNUSED uint32_t nCount) {};
    

    /**
     * sets the key with the given key id into real sync
     * 
     * A key in real sync is present at both sides
     * of the key store, at master's and at slave's
     * 
     * @param   nKeyId      key id requested
     */
    void set_real_sync_internal(UNUSED qkd::key::key_id nKeyId) {};
    
    
    /**
     * sync and flushes the DB to disk
     */
    void sync_internal() {};
    
    
    /**
     * check if a given key id is present in the key store
     * 
     * @return  true, if there is a key with this id
     */
    bool valid_internal(UNUSED qkd::key::key_id nKeyId) const { return false; };
    
};
  

}

}

#endif

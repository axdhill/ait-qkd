/*
 * db_ram.cpp
 * 
 * Implementation of the Q3P Key DB in memory
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

// ait
#include <qkd/common_macros.h>
#include <qkd/q3p/db.h>
#include <qkd/utility/checksum.h>
#include <qkd/utility/syslog.h>

#include "db_ram.h"


using namespace qkd;
using namespace qkd::q3p;


// ------------------------------------------------------------
// code


/**
 * close the key DB
 */
void db_ram::close_internal() {
    
    m_cKeyData = nullptr;
    m_cKeyMetaData = nullptr;
    
    m_nKeyLastAdded = 0;
    m_nKeyLastInserted = 0;
    m_nKeyLastPickedSpare = 0;
    m_nKeyLastPickedValid = 0;
    
    m_nCount = 0;
    m_nCountRealSync = 0;

    if (m_cKeyData) delete [] m_cKeyData;
    if (m_cKeyMetaData) delete [] m_cKeyMetaData;
    m_cKeyData = nullptr;
    m_cKeyMetaData = nullptr;
}


/**
 * return the number of keys managed
 * 
 * this is actually very expensive the first time ...
 * 
 * @return  the number of keys stored in this DB
 */
uint64_t db_ram::count_internal() const {
    if (!m_cKeyMetaData) return 0;
    return m_nCount;
}


/**
 * return the number of keys in real sync
 * 
 * @return  the number of keys in real sync
 */
uint64_t db_ram::count_real_sync_internal() const {
    if (!m_cKeyMetaData) return 0;
    return m_nCountRealSync;
}


/**
 * delete the key with an ID
 * 
 * @param   nKeyId      the ID of the key to delete
 */
void db_ram::del_internal(qkd::key::key_id nKeyId) {
    
    // sanity check
    if (nKeyId < min_id()) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "refused to delete key with id " << nKeyId << ": minimum key id is: " << min_id();
        return;
    }
    
    // sanity check
    if (nKeyId >= max_id()) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "refused to delete key with id " << nKeyId << ": maximum key id is: " << max_id() - 1;
        return;
    }

    // remember if this is a new key
    unsigned char & cKeyMeta = m_cKeyMetaData[nKeyId - min_id()];
    bool bNewKey = ((cKeyMeta & FLAG_VALID) == 0);
    
    cKeyMeta = 0;
    memset(m_cKeyData + quantum() * (nKeyId - min_id()), 0, quantum());
        
    if (!bNewKey) m_nCount--;
}


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
bool db_ram::eventual_sync_internal(qkd::key::key_id nKeyId) const {
    if (!valid(nKeyId)) return false;
    return ((m_cKeyMetaData[nKeyId - min_id()] & FLAG_EVENTUAL_SYNC) == FLAG_EVENTUAL_SYNC);
}


/**
 * return a list of coninuous key ids which cover at least nBytes of key material
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
 * @return  a list of coninuous key ids (list may be empty in case of failure)
 */
qkd::key::key_vector db_ram::find_continuous_internal(uint64_t nBytes, uint32_t nCount) {
    
    qkd::key::key_vector cKeys;
    
    // how many keys do we need?
    uint32_t nKeysNeeded = nBytes / quantum();
    if (nBytes % quantum()) nKeysNeeded++;
    
    // search!
    for (qkd::key::key_id i = min_id(); i < max_id(); i++) {
        
        // invalid key?
        if ((m_cKeyMetaData[i - min_id()] & (FLAG_VALID | FLAG_COUNTER)) != FLAG_VALID) {
            cKeys.clear();
            continue;
        }
        
        // add key
        cKeys.push_back(i);
        
        // enough?
        if (cKeys.size() == nKeysNeeded) break;
    }
    
    // check if we found enough keys
    if (cKeys.size() != nKeysNeeded) return qkd::key::key_vector();
    
    // reserve keys
    if (cKeys.size()) set_key_count(cKeys, nCount);
    
    return cKeys;
}


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
qkd::key::key_vector db_ram::find_spare_internal(uint64_t nBytes, uint32_t nCount) {
    
    qkd::key::key_vector cKeyIds;
    
    // check quantum
    if (nBytes % quantum()) return cKeyIds;
    
    // search key Ids
    qkd::key::key_id nKeyPick = min_id();
    while (nBytes > 0) {
        
        nKeyPick++;
        nKeyPick %= amount();
        
        // reached same position again: end
        if (nKeyPick == min_id()) break;
        
        // an invalid key with no counter data is ok
        if ((m_cKeyMetaData[nKeyPick - min_id()] & (FLAG_VALID | FLAG_COUNTER)) == 0) {
            
            // apply count
            if (nCount) set_key_count(nKeyPick, nCount);
            
            // hit: unvalid key is spare key
            cKeyIds.push_back(nKeyPick);
            nBytes -= quantum();
        }
    }
    
    return cKeyIds;
}


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
qkd::key::key_vector db_ram::find_valid_internal(uint64_t nBytes, uint32_t nCount) {
    
    qkd::key::key_vector cKeyIds;
    
    // check quantum
    if (nBytes % quantum()) return cKeyIds;
    
    // search key Ids
    qkd::key::key_id nKeyPick = m_nKeyLastPickedValid;
    while (nBytes > 0) {
        
        nKeyPick++;
        nKeyPick %= amount();
        
        // reached same position again: end
        if (nKeyPick == m_nKeyLastPickedValid) break;
        
        // an valid key with no counter data is ok
        if ((m_cKeyMetaData[nKeyPick - min_id()] & (FLAG_VALID | FLAG_COUNTER)) == FLAG_VALID) {

            // apply count
            if (nCount) set_key_count(nKeyPick, nCount);
            
            // hit: unvalid key is spare key
            cKeyIds.push_back(nKeyPick);
            nBytes -= quantum();
        }
    }
    
    // remember last pick position
    m_nKeyLastPickedValid = nKeyPick;
    
    return cKeyIds;
}


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
qkd::key::key db_ram::get_internal(qkd::key::key_id nKeyId) const {
    
    // check for existance
    if (!valid(nKeyId)) return qkd::key::key::null();
    
    // pick the key data
    unsigned char * cAddress = m_cKeyData + quantum() * (nKeyId - min_id());
    qkd::utility::memory cKeyData(quantum());
    memcpy(cKeyData.get(), cAddress, quantum());
    
    return qkd::key::key(nKeyId, cKeyData);
}


/**
 * return a key counter associated with the key
 * 
 * @param   nKeyId      the ID of the key
 * @return  count of the key
 */
uint32_t db_ram::key_count_internal(qkd::key::key_id nKeyId) const {
    if (!valid(nKeyId)) return 0;
    return m_cKeyMetaData[nKeyId - min_id()] & FLAG_COUNTER;
}


/**
 * inits the key-DB
 * 
 * This method MUST be overwritten in subclasses
 * 
 * @param   sURL        the url to the DB
 */
void db_ram::init(UNUSED QString sURL) {
    
    // setup values
    m_nCount = 0;
    m_nCountRealSync = 0;
    
    m_cKeyData = nullptr;
    m_cKeyMetaData = nullptr;
    
    m_cKeyData = new unsigned char[amount() * db::quantum()];
    m_cKeyMetaData = new unsigned char[amount()];
    memset(m_cKeyMetaData, 0, amount());
    
    m_nKeyLastAdded = 0;
    m_nKeyLastInserted = 0;
    m_nKeyLastPickedSpare = 0;
    m_nKeyLastPickedValid = 0;
    
    reset();
}


/**
 * check if a given key has been injected
 * 
 * A key has been injected if the key was not 
 * negotiated with the peer
 * 
 * @param   nKeyId      key id requested
 * @return  true, if the key has been injected in DB without peer interaction
 */
bool db_ram::injected_internal(qkd::key::key_id nKeyId) const {
    if (!valid(nKeyId)) return false;
    return ((m_cKeyMetaData[nKeyId - min_id()] & FLAG_INJECTED) == FLAG_INJECTED);
}


/**
 * inserts a key into the DB
 * 
 * this inserts a new key into DB. It tries to find a spare place,
 * places the key there and yields the ID found.
 * 
 * If the DB is full, 0 is returned.
 * 
 * @param   cKey        the key to fill in
 * @return  the new key if (or 0 if full)
 */
qkd::key::key_id db_ram::insert_internal(qkd::key::key cKey) {
    
    // when one is found, the next will likely be next
    // that's why we rely on m_nKeyLastInserted
    
    // sanity check
    if (cKey.size() != quantum()) return 0;
    
    // get the last one picked
    qkd::key::key_id nKeyId = m_nKeyLastInserted + 1;
    while (m_nKeyLastInserted != nKeyId) {
        if ((m_cKeyMetaData[nKeyId - min_id()] & FLAG_VALID) != FLAG_VALID) break;
        nKeyId++;
        if (nKeyId == max_id()) nKeyId = min_id();
    }
    
    // found?
    if (m_nKeyLastInserted == nKeyId) return 0;
    
    // insert key
    try {
        set(qkd::key::key(nKeyId, cKey.data()));
    }
    catch (...) {
        return 0;
    }
    
    // remember last hit
    m_nKeyLastInserted = nKeyId;
        
    return nKeyId;
}


/**
 * opened DB flag
 * 
 * @return  true, if the DB is opened
 */
bool db_ram::opened_internal() const {
    return (m_cKeyMetaData != nullptr);
}


/**
 * check if a given key id is in real sync
 * 
 * A key in real sync is present at both sides
 * of the key store, at master's and at slave's
 * 
 * @param   nKeyId      key id requested
 * @return  true, if the key with the id is in real sync state
 */
bool db_ram::real_sync_internal(qkd::key::key_id nKeyId) const {
    if (!valid(nKeyId)) return false;
    return ((m_cKeyMetaData[nKeyId - min_id()] & FLAG_REAL_SYNC) == FLAG_REAL_SYNC);
}


/**
 * does a reset of the intermediate stats of the DB
 * 
 * though this does not close the DB
 */
void db_ram::reset_internal() {
    
    m_nKeyLastAdded = 0;
    m_nKeyLastInserted = 0;
    m_nKeyLastPickedSpare = 0;
    m_nKeyLastPickedValid = 0;
    
    // clear meta data
    m_nCount = 0;
    m_nCountRealSync = 0;
    for (qkd::key::key_id nKeyId = min_id(); nKeyId < max_id(); nKeyId++) {
        m_cKeyMetaData[nKeyId - min_id()] &= FLAG_PERSISTENT;
        if ((m_cKeyMetaData[nKeyId - min_id()] & FLAG_VALID) == FLAG_VALID) m_nCount++;
        if ((m_cKeyMetaData[nKeyId - min_id()] & (FLAG_VALID | FLAG_REAL_SYNC)) == (FLAG_VALID | FLAG_REAL_SYNC)) m_nCountRealSync++;
    }
}


/**
 * retrieve a ring of keys
 * 
 * @param   cKeys       a list of keys
 * @return  a key ring holding all these keys
 */
qkd::key::key_ring db_ram::ring_internal(qkd::key::key_vector const & cKeys) {
    
    qkd::key::key_ring cRing(quantum());
    for (auto & nKeyId : cKeys) cRing << get(nKeyId);

    return cRing;    
}


/**
 * retrieve a ring of keys
 * 
 * @param   cKeys       a list of keys
 * @return  a key ring holding all these keys
 */
qkd::key::key_ring db_ram::ring_internal(qkd::key::key_vector const & cKeys) const {
    
    qkd::key::key_ring cRing(quantum());
    for (auto & nKeyId : cKeys) cRing << get(nKeyId);

    return cRing;    
}


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
void db_ram::set_eventual_sync_internal(qkd::key::key_id nKeyId) {
    
    // sanity check
    if (!valid(nKeyId)) return;
    
    // decrease number of real sync keys
    if ((m_cKeyMetaData[nKeyId - min_id()] & FLAG_REAL_SYNC) == FLAG_REAL_SYNC) m_nCountRealSync--;
    
    // remove the real flag and set the eventual flag
    m_cKeyMetaData[nKeyId - min_id()] &= ~FLAG_REAL_SYNC; 
    m_cKeyMetaData[nKeyId - min_id()] |= FLAG_EVENTUAL_SYNC;
}


/**
 * sets the key with the given key to be injected
 * 
 * A key has been injected if the key was not 
 * negotiated with the peer
 * 
 * @param   nKeyId      key id requested
 */
void db_ram::set_injected_internal(qkd::key::key_id nKeyId) {
    
    // sanity check
    if (!valid(nKeyId)) return;
    
    // remove the real flag and set the eventual flag
    m_cKeyMetaData[nKeyId - min_id()] |= FLAG_INJECTED;
}


/**
 * set the key in the DB
 * 
 * @param   cKey        the key to store in the DB
 */
void db_ram::set_internal(qkd::key::key const & cKey) {
    
    // sanity check
    if (cKey.id() < min_id()) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "refused to set key with id " << cKey.id() << ": minimum key id is: " << min_id();
        return;
    }
    
    // sanity check
    if (cKey.id() >= max_id()) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "refused to set key with id " << cKey.id() << ": maximum key id is: " << max_id() - 1;
        return;
    }
    
    // check key size: too small?
    if (cKey.size() < quantum()) {
        
        // the key is too small: refuse to add it!
        // we need at least quantum() size of key material
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "refused to set key with id " << cKey.id() << ": key size()=" << cKey.size() << " is less than minimum of " << quantum();
        return;
    }
    
    // check key size: too big?
    if (cKey.size() > quantum()) {
        
        // well, we have to discard the excess bits ... 
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "key with id " << cKey.id() << ": key size()=" << cKey.size() << " is bigger than quantum " << quantum() << " - dropping excess bits.";
    }
    
    // remember if this is a new key
    unsigned char & cKeyMeta = m_cKeyMetaData[cKey.id() - min_id()];
    bool bNewKey = ((cKeyMeta & FLAG_VALID) == 0);
    
    // set the key material
    cKeyMeta = FLAG_VALID;
    memcpy(m_cKeyData + quantum() * (cKey.id() - min_id()), cKey.data().get(), quantum());
    
    if (bNewKey) m_nCount++;
}


/**
 * sets a new key count value
 * 
 * @param   nKeyId      the ID of the key
 * @param   nCount      count of the key
 */
void db_ram::set_key_count_internal(qkd::key::key_id nKeyId, uint32_t nCount) {
    
    // sanity check
    if (nKeyId < min_id()) return;
    if (nKeyId >= max_id()) return;
    if (!opened()) return;
    
    // max is FLAG_COUNTER
    unsigned char nBitMask = std::min<unsigned char>(nCount, FLAG_COUNTER);
    m_cKeyMetaData[nKeyId - min_id()] = (m_cKeyMetaData[nKeyId - min_id()] & FLAG_PERSISTENT) | nBitMask;
}


/**
 * sets a new key count value for a list of keys
 * 
 * @param   cKeyIds     the IDs of the keys to set the key count
 * @param   nCount      count of the key
 */
void db_ram::set_key_count_internal(qkd::key::key_vector const & cKeyIds, uint32_t nCount) {
    for (auto & nKeyId : cKeyIds) set_key_count(nKeyId, nCount);
}


/**
 * sets the key with the given key id into real sync
 * 
 * A key in real sync is present at both sides
 * of the key store, at master's and at slave's
 * 
 * @param   nKeyId      key id requested
 */
void db_ram::set_real_sync_internal(qkd::key::key_id nKeyId) {
    
    // sanity check
    if (!valid(nKeyId)) return;
    
    // increase number of real sync keys if not already in real sync
    if ((m_cKeyMetaData[nKeyId - min_id()] & FLAG_REAL_SYNC) == FLAG_REAL_SYNC) return;
    
    m_nCountRealSync++;
    
    // remove the eventual flag and set the real flag
    m_cKeyMetaData[nKeyId - min_id()] &= ~FLAG_EVENTUAL_SYNC; 
    m_cKeyMetaData[nKeyId - min_id()] |= FLAG_REAL_SYNC;
}


/**
 * sync and flushes the DB to disk
 */
void db_ram::sync_internal() {
    // no sync for memory only DB
}


/**
 * check if a given key id is present in the key store
 * 
 * @return  true, if there is a key with this id
 */
bool db_ram::valid_internal(qkd::key::key_id nKeyId) const {
    
    // sanity check
    if (nKeyId < min_id()) return false;
    if (nKeyId >= max_id()) return false;
    if (!opened()) return false;
    
    // check the key flag
    unsigned char cKeyFlag = m_cKeyMetaData[nKeyId - min_id()];
    return ((cKeyFlag & FLAG_VALID) == FLAG_VALID);
}

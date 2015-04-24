/*
 * db.h
 * 
 * The Q3P Key DB interface
 *
 * Autor: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
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
 

#ifndef __QKD_Q3P_DB_H_
#define __QKD_Q3P_DB_H_


// ------------------------------------------------------------
// incs

#include <exception>
#include <mutex>
#include <string>

#include <inttypes.h>

#include <boost/shared_ptr.hpp>

// Qt
#include <QtCore/QObject>
#include <QtDBus/QtDBus>

// ait
#include <qkd/key/key.h>
#include <qkd/key/key_ring.h>


// ------------------------------------------------------------
// decls


namespace qkd {
    
namespace q3p {    
    
    
// fwd
class db;
typedef boost::shared_ptr<db> key_db;


/**
 * This is the Key DB.
 * 
 * This acts as an interface to the real DB implementation.
 * 
 * A key_db reference can be obtained by calling the static
 * 
 *          open(...)
 * 
 * method with a correct URL.
 * 
 * The URL is as always:
 * 
 *      SCHEME://[USER:PASSWORD@HOST:PORT]/PATH
 * 
 * Currently the only supportes shemes are
 * 
 *      file:///PATH            create or open flat files as key-DB
 *      ram://                  create a RAM only DB
 *      null://                 create an invalid, empty DB
 * 
 * other urls which might be supported in future:
 * 
 *      db:///PATH                                      Berkeley DB file
 *      sqlite:///PATH                                  SQLite DB file
 *      mysql://USER[:PASSWORD]@HOST[:PORT]/PATH        MySQL Key DB
 * 
 *
 * All keys in a DB do have the very same size, see: quantum(). Keys in the
 * DB may be in real sync or in eventual sync. 
 * 
 * Each key may also have an count variable associated which serves several 
 * purposes agnostic to the DB implementation. However, if the key count 
 * is > 0 then the key is treated as "reserved".
 * 
 * 
 * The offered DBus interface is
 * 
 *      DBus Interface: "at.ac.ait.q3p.database"
 * 
 * 
 * Properties of at.ac.ait.q3p.database
 * 
 *      -name-          -read/write-    -description-
 * 
 *      charge              R           number of keys in the DB
 * 
 *      description         R           human readable description of the DB
 * 
 *      max_id              R           The maximum ID of a key this key store manages
 * 
 *      min_id              R           The minimum ID of a key this key store manages
 * 
 *      quantum             R           The fixed size of a single key in bytes managed by this KeyStore
 * 
 *      url                 R           URL which creted this DB instance
 */
class db : public QObject {
    
    
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "at.ac.ait.q3p.database")
    
    Q_PROPERTY(qulonglong charge READ count)                /**< numbers of keys in the DB */
    Q_PROPERTY(QString description READ describe)           /**< human readable description of the DB */
    Q_PROPERTY(qulonglong max_id READ max_id)               /**< maximum ID of a Key */
    Q_PROPERTY(qulonglong min_id READ min_id)               /**< minimum ID of a Key */
    Q_PROPERTY(qulonglong quantum READ quantum)             /**< size of a single key in bytes */
    Q_PROPERTY(QString url READ url)                        /**< URL used to access the Db */
    
    
public:    
    
 
    /**
     * exception type thrown on init errors
     */
    class db_init_error : public std::exception { 
    public: 

        /**
         * exception description 
         * @return  a human readable exception description
         */
        const char * what() const noexcept { return "error during init of q3p database"; } 
    };



    /**
     * exception type thrown for unknown DB url schemes
     */
    class db_url_scheme_unknown : public std::exception { 
    public: 

        /**
         * exception description 
         * @return  a human readable exception description
         */
        const char * what() const noexcept { return "unknown database url scheme"; } 
    };
    
   
    /**
     * dtor
     */
    virtual ~db() { };
    
    
    /**
     * get the total number of keys possible
     * 
     * @return  the maximum count of keys in the db possible
     */
    inline qulonglong amount() const { return max_id() - min_id(); };
    

    /**
     * close the DB
     */
    inline void close() { close_internal(); }
    
    
    /**
     * return the number of keys managed
     * 
     * @return  the number of keys stored in this DB
     */
    inline qulonglong count() const { std::lock_guard<std::recursive_mutex> cLock(m_cMTX); return count_internal(); };
    
    
    /**
     * return the number of keys in real sync
     * 
     * @return  the number of keys in real
     */
    inline qulonglong count_real_sync() const { std::lock_guard<std::recursive_mutex> cLock(m_cMTX); return count_real_sync_internal(); };
    
    
    /**
     * delete the key with an ID
     * 
     * @param   nKeyId      the ID of the key to delete
     */
    inline void del(qkd::key::key_id nKeyId) { std::lock_guard<std::recursive_mutex> cLock(m_cMTX); del_internal(nKeyId); };
    

    /**
     * delete a list of keys
     * 
     * @param   cKeys       the list of keys to delete
     */
    inline void del(qkd::key::key_vector cKeys) { std::lock_guard<std::recursive_mutex> cLock(m_cMTX); for (auto & nKeyId : cKeys) del_internal(nKeyId); };
    

    /**
     * describe the key-DB
     * 
     * @return  a HR-string describing the key-DB
     */
    virtual QString describe() const = 0;
    
    
    /**
     * enforces a signal to emit the current add/del key charge
     * 
     * This is externally triggered, since doing this for every 
     * key change of the DB wil flood the DBus messaging system.
     * So we need way to tell the environment after we've done
     * heavy work on the DB.
     * 
     * @param   nAdded      keys recently added
     * @param   nDeleted    keys recently deleted
     */
    inline void emit_charge_change(uint64_t nAdded, uint64_t nDeleted) { emit charge_change(count(), nAdded, nDeleted); };
    
    
    /**
     * check if a given key id is in eventual sync
     * 
     * A key in eventual sync is is ought to be present in
     * real sync on the other side, though this has not been
     * confirmed yet.
     * 
     * @param   nKeyId      the ID of the key to get
     * @return  true, if the key with the id is in real sync state
     */
    inline bool eventual_sync(qkd::key::key_id nKeyId) const { std::lock_guard<std::recursive_mutex> cLock(m_cMTX); return eventual_sync_internal(nKeyId); };
    
    
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
    inline qkd::key::key_vector find_continuous(uint64_t nBytes, uint32_t nCount = 0) { std::lock_guard<std::recursive_mutex> cLock(m_cMTX); return find_continuous_internal(nBytes, nCount); };
    
    
    /**
     * returns a list of spare key IDs (no key data within the DB for those IDs)
     * 
     * a free key is not associated in the DB (invalid) and has a count of 0
     * 
     * the free space will cover the given space in bytes. this must be
     * a multiple of quantum(). otherwise NO key id will be found.
     * 
     * hence, the returned list may be also less or 0 if the DB is full.
     * 
     * the given nCount is applied to all keys found, thus preventing this
     * key to be found again. A count != 0 marks the key as "reserved".
     * 
     * @param   nBytes      number of bytes free (MUST be a multiple of quantum())
     * @param   nCount      applies nCount on the keys if nCount != 0
     * @return  a list of spare key ids (list may be empty in case of failure or full DB)
     */
    inline qkd::key::key_vector find_spare(uint64_t nBytes, uint32_t nCount = 0) { std::lock_guard<std::recursive_mutex> cLock(m_cMTX); return find_spare_internal(nBytes, nCount); };
    

    /**
     * returns a list of valid key IDs
     * 
     * a valid key is associated in the DB and has a count of 0.
     * 
     * the free space will cover the given space in bytes. this must be
     * a multiple of quantum(). otherwise NO key id will be found.
     * 
     * hence, the returned list may be also less or 0 if the DB is empty.
     * 
     * the given nCount is applied to all keys found, thus preventing this
     * key to be found again. A count != 0 marks the key as "reserved".
     * 
     * @param   nBytes      number of bytes valid (MUST be a multiple of quantum())
     * @param   nCount      applies nCount on the keys if nCount != 0
     * @return  a list of valid key ids (list may be empty in case of failure or empty DB)
     */
    inline qkd::key::key_vector find_valid(uint64_t nBytes, uint32_t nCount = 0) { std::lock_guard<std::recursive_mutex> cLock(m_cMTX); return find_valid_internal(nBytes, nCount); };
    

    /**
     * get the key with an ID
     * 
     * This looks up the DB and returns the given key with the ID 
     * or qkd::key::key::null() in case of error
     * 
     * @param   nKeyId      the ID of the key to get
     * @return  the key with the given id
     */
    inline qkd::key::key get(qkd::key::key_id nKeyId = 0) const { std::lock_guard<std::recursive_mutex> cLock(m_cMTX); return get_internal(nKeyId); };
    

    /**
     * check if a given key has been injected
     * 
     * A key has been injected if the key was not 
     * negotiated with the peer
     * 
     * @param   nKeyId      key id requested
     * @return  true, if the key has been injected in DB without peer interaction
     */
    inline bool injected(qkd::key::key_id nKeyId) const { std::lock_guard<std::recursive_mutex> cLock(m_cMTX); return injected_internal(nKeyId); };
    

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
    inline qkd::key::key_id insert(qkd::key::key cKey) { std::lock_guard<std::recursive_mutex> cLock(m_cMTX); return insert_internal(cKey); };
    

    /**
     * return a key counter associated with the key
     * 
     * any key count != 0 delcares the key as "reserved" (for some purpose).
     * 
     * @param   nKeyId      the ID of the key
     * @return  count of the key
     */
    inline uint32_t key_count(qkd::key::key_id nKeyId) const { std::lock_guard<std::recursive_mutex> cLock(m_cMTX); return key_count_internal(nKeyId); };
    

    /**
     * return the maximum key count
     * 
     * @return  the maximum key count value
     */
    inline uint32_t key_count_max() const { return key_count_max_internal(); };
    
    
    /**
     * get the maximum ID of a key
     * 
     * @return  the maximum ID of a key
     */
    inline qulonglong max_id() const { return max_id_internal(); };
    

    /**
     * get the minimum ID of a key
     * 
     * @return  the minimum ID of a key
     */
    inline qulonglong min_id() const { return min_id_internal(); };
    
    
    /**
     * get the mutex
     * 
     * @return  the mutex for synchronized access
     */
    inline std::recursive_mutex & mutex() const { return m_cMTX; };
    

    /**
     * factory method to create or open a key-db
     *
     * @param   sURL        a URL string indicating the key-db source and type
     * @return  an initialized key_db object
     */
    static key_db open(QString sURL);
    

    /**
     * opened DB flag
     * 
     * @return  true, if the DB is opened
     */
    inline bool opened() const { return opened_internal(); };
    
    
    /**
     * get the fixed size of key material manged in the key DB
     * 
     * This is the amount of key material hold by a single
     * key in the database
     * 
     * @return  the fixed size in bytes of a key in the DB
     */
    inline qulonglong quantum() const { return quantum_internal(); };
    

    /**
     * check if a given key id is in real sync
     * 
     * A key in real sync is present at both sides
     * of the key store, at master's and at slave's
     * 
     * @param   nKeyId      key id requested
     * @return  true, if the key with the id is in real sync state
     */
    inline bool real_sync(qkd::key::key_id nKeyId) const { std::lock_guard<std::recursive_mutex> cLock(m_cMTX); return real_sync_internal(nKeyId); };
    

    /**
     * does a reset of the intermediate stats of the DB
     * 
     * though this does not close the DB
     */
    inline void reset() { std::lock_guard<std::recursive_mutex> cLock(m_cMTX); return reset_internal(); };
    

    /**
     * retrieve a ring of keys
     * 
     * @param   cKeys       a list of keys
     * @return  a key ring holding all these keys
     */
    inline qkd::key::key_ring ring(qkd::key::key_vector const & cKeys) { std::lock_guard<std::recursive_mutex> cLock(m_cMTX); return ring_internal(cKeys); };
    

    /**
     * retrieve a ring of keys
     * 
     * @param   cKeys       a list of keys
     * @return  a key ring holding all these keys
     */
    inline qkd::key::key_ring ring(qkd::key::key_vector const & cKeys) const { std::lock_guard<std::recursive_mutex> cLock(m_cMTX); return ring_internal(cKeys); };
    

    /**
     * set the key in the DB
     * 
     * @param   cKey        the key to store in the DB
     */
    inline void set(qkd::key::key const & cKey) { std::lock_guard<std::recursive_mutex> cLock(m_cMTX); set_internal(cKey); };
    

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
    inline void set_eventual_sync(qkd::key::key_id nKeyId) { std::lock_guard<std::recursive_mutex> cLock(m_cMTX); set_eventual_sync_internal(nKeyId); };
    

    /**
     * set a given key to be injected
     * 
     * A key has been injected if the key was not 
     * negotiated with the peer
     * 
     * @param   nKeyId      the ID of the key to get
     * @return  true, if the key with the id is in real sync state
     */
    inline void set_injected(qkd::key::key_id nKeyId) { std::lock_guard<std::recursive_mutex> cLock(m_cMTX); set_injected_internal(nKeyId); };
    

    /**
     * sets a new key count value
     * 
     * @param   nKeyId      the ID of the key
     * @param   nCount      count of the key
     */
    inline void set_key_count(qkd::key::key_id nKeyId, uint32_t nCount) { std::lock_guard<std::recursive_mutex> cLock(m_cMTX); set_key_count_internal(nKeyId, nCount); };
    

    /**
     * sets a new key count value for a list of keys
     * 
     * @param   cKeyIds     the IDs of the keys to set the key count
     * @param   nCount      count of the key
     */
    inline void set_key_count(qkd::key::key_vector const & cKeyIds, uint32_t nCount) { std::lock_guard<std::recursive_mutex> cLock(m_cMTX); set_key_count_internal(cKeyIds, nCount); };
    

    /**
     * sets the key with the given key id into real sync
     * 
     * A key in real sync is present at both sides
     * of the key store, at master's and at slave's
     * 
     * @param   nKeyId      key id requested
     */
    inline void set_real_sync(qkd::key::key_id nKeyId) { std::lock_guard<std::recursive_mutex> cLock(m_cMTX); set_real_sync_internal(nKeyId); };
    

    /**
     * sync and flushes the DB to disk
     */
    inline void sync() { std::lock_guard<std::recursive_mutex> cLock(m_cMTX); sync_internal(); };
    
    
    /**
     * returns the url of the DB
     * 
     * @return  the URL of the KeyStore DB
     */
    inline QString const & url() const { return m_sURL; };
    
    
    /**
     * check if a given key id is present in the key store
     * 
     * @param   nKeyId          the key id in question
     * @return  true, if there is a key with this id
     */
    inline bool valid(qkd::key::key_id nKeyId) const { std::lock_guard<std::recursive_mutex> cLock(m_cMTX); return valid_internal(nKeyId); };
    

signals:
    
    
    /**
     * new count of keys
     * 
     * @param   nCharge     the new amount of keys in the DB
     * @param   nAdded      amount of newly added keys
     * @param   nDeleted    amount of removed keys
     */
    void charge_change(qulonglong nCharge, qulonglong nAdded, qulonglong nDeleted);
    
    
protected:


    /**
     * ctor
     * 
     * @param   sURL        the DB url
     */
    db(QString sURL) : m_sURL(sURL) { };
    
    
private:
    
    
    /**
     * close the key DB
     */
    virtual void close_internal() = 0;
    

    /**
     * return the number of keys managed
     * 
     * @return  the number of keys stored in this DB
     */
    virtual uint64_t count_internal() const = 0;
    
    
    /**
     * return the number of keys in real sync
     * 
     * @return  the number of keys in real sync
     */
    virtual uint64_t count_real_sync_internal() const = 0;
    
    
    /**
     * delete the key with an ID
     * 
     * @param   nKeyId      the ID of the key to delete
     */
    virtual void del_internal(qkd::key::key_id nKeyId) = 0;
    

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
    virtual bool eventual_sync_internal(qkd::key::key_id nKeyId) const = 0;
    

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
    virtual qkd::key::key_vector find_continuous_internal(uint64_t nBytes, uint32_t nCount) = 0;
    
    
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
    virtual qkd::key::key_vector find_spare_internal(uint64_t nBytes, uint32_t nCount) = 0;
    

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
    virtual qkd::key::key_vector find_valid_internal(uint64_t nBytes, uint32_t nCount) = 0;
    

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
    virtual qkd::key::key get_internal(qkd::key::key_id nKeyId) const = 0;
    

    /**
     * inits the key-DB
     * 
     * This method MUST be overwritten in subclasses
     * 
     * @param   sURL        the url to the DB
     */
    virtual void init(QString sURL) = 0;
    
    
    /**
     * check if a given key has been injected
     * 
     * A key has been injected if the key was not 
     * negotiated with the peer
     * 
     * @param   nKeyId      key id requested
     * @return  true, if the key has been injected in DB without peer interaction
     */
    virtual bool injected_internal(qkd::key::key_id nKeyId) const = 0;
    

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
    virtual qkd::key::key_id insert_internal(qkd::key::key cKey) = 0;
    

    /**
     * return a key counter associated with the key
     * 
     * @param   nKeyId      the ID of the key
     * @return  count of the key
     */
    virtual uint32_t key_count_internal(qkd::key::key_id nKeyId) const = 0;
    

    /**
     * return the maximum key count
     * 
     * @return  the maximum key count value
     */
    virtual uint32_t key_count_max_internal() const = 0;
    

    /**
     * get the maximum ID of a key
     * 
     * @return  the maximum ID of a key
     */
    virtual qkd::key::key_id max_id_internal() const = 0;
    

    /**
     * get the minimum ID of a key
     * 
     * @return  the minimum ID of a key
     */
    virtual qkd::key::key_id min_id_internal() const = 0;
    

    /**
     * opened DB flag
     * 
     * @return  true, if the DB is opened
     */
    virtual bool opened_internal() const = 0;
    

    /**
     * get the fixed size of key material manged in the key DB
     * 
     * This is the amount of key material hold by a single
     * key in the database
     * 
     * @return  the fixed size in bytes of a key in the DB
     */
    virtual uint64_t quantum_internal() const = 0;
    

    /**
     * check if a given key id is in real sync
     * 
     * A key in real sync is present at both sides
     * of the key store, at master's and at slave's
     * 
     * @param   nKeyId      key id requested
     * @return  true, if the key with the id is in real sync state
     */
    virtual bool real_sync_internal(qkd::key::key_id nKeyId) const = 0;
    

    /**
     * does a reset of the intermediate stats of the DB
     * 
     * though this does not close the DB
     */
    virtual void reset_internal() = 0;
    

    /**
     * retrieve a ring of keys
     * 
     * @param   cKeys       a list of keys
     * @return  a key ring holding all these keys
     */
    virtual qkd::key::key_ring ring_internal(qkd::key::key_vector const & cKeys) = 0;
    

    /**
     * retrieve a ring of keys
     * 
     * @param   cKeys       a list of keys
     * @return  a key ring holding all these keys
     */
    virtual qkd::key::key_ring ring_internal(qkd::key::key_vector const & cKeys) const = 0;
    

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
    virtual void set_eventual_sync_internal(qkd::key::key_id nKeyId) = 0;
    

    /**
     * set a given key to be injected
     * 
     * A key has been injected if the key was not 
     * negotiated with the peer
     * 
     * @param   nKeyId      the ID of the key to get
     * @return  true, if the key with the id is in real sync state
     */
    virtual void set_injected_internal(qkd::key::key_id nKeyId) = 0;
    

    /**
     * set the key in the DB
     * 
     * @param   cKey        the key to store in the DB
     */
    virtual void set_internal(qkd::key::key const & cKey) = 0;
    

    /**
     * sets a new key count value
     * 
     * @param   nKeyId      the ID of the key
     * @param   nCount      count of the key
     */
    virtual void set_key_count_internal(qkd::key::key_id nKeyId, uint32_t nCount) = 0;
    

    /**
     * sets a new key count value for a list of keys
     * 
     * @param   cKeyIds     the IDs of the keys to set the key count
     * @param   nCount      count of the key
     */
    virtual void set_key_count_internal(qkd::key::key_vector const & cKeyIds, uint32_t nCount) = 0;
    

    /**
     * sets the key with the given key id into real sync
     * 
     * A key in real sync is present at both sides
     * of the key store, at master's and at slave's
     * 
     * @param   nKeyId      key id requested
     */
    virtual void set_real_sync_internal(qkd::key::key_id nKeyId) = 0;
    

    /**
     * sync and flushes the DB to disk
     */
    virtual void sync_internal() = 0;
    

    /**
     * check if a given key id is present in the key store
     * 
     * @return  true, if there is a key with this id
     */
    virtual bool valid_internal(qkd::key::key_id nKeyId) const = 0;
    

    /**
     * read/write object mutex
     */
    mutable std::recursive_mutex m_cMTX;
    
    
    /**
     * the initial URL
     */
    QString m_sURL;
    
    
};
  

}

}

#endif


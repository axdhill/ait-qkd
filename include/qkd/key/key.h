/*
 * key.h
 * 
 * the QKD key
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

 
#ifndef __QKD_KEY_KEY_H_
#define __QKD_KEY_KEY_H_


// ------------------------------------------------------------
// incs

#include <chrono>
#include <stdexcept>
#include <vector>

#include <inttypes.h>

#include <boost/property_tree/ptree.hpp>

// ait
#include <qkd/utility/buffer.h>


// ------------------------------------------------------------
// decls


namespace qkd {
    
namespace key {    
    
    
/**
 * a key id
 */
typedef uint32_t key_id;
    

/**
 * a vector of key ids
 */
typedef std::vector<key_id> key_vector;
    

/**
 * a key state
 * 
 * a key may be in a specific state. During qkd postprocessing
 * the "natural" state sequence is 
 * 
 *      RAW --> SIFTED --> CORRECTED --> CONFIRMED --> AMPLIFIED --> AUTHENTICATED
 * 
 * However, every now and then other states may occur due to
 * algorithm and/or processing details.
 */
enum class key_state : uint8_t {
    
    KEY_STATE_OTHER = 0,            /**< key data are not key bits: they have to be treated in context of the previous module */
    
    KEY_STATE_RAW,                  /**< this is raw key data */
    KEY_STATE_SIFTED,               /**< this is a sifted key */
    KEY_STATE_CORRECTED,            /**< this is a corrected key */
    KEY_STATE_UNCORRECTED,          /**< this is a key for which correction failed */
    KEY_STATE_CONFIRMED,            /**< this is a key for which confirmation had been successful */
    KEY_STATE_UNCONFIRMED,          /**< this is a key for which confirmation failed */
    KEY_STATE_AMPLIFIED,            /**< this is a privacy amplified key */
    KEY_STATE_AUTHENTICATED,        /**< this is an authenticated key */

    KEY_STATE_DISCLOSED,            /**< this key has been disclosed */
    
    KEY_STATE_TAINTED,              /**< this key may be tainted: authentication failed */
    
    KEY_STATE_NEW = 99              /**< this is a new key */
};


/**
 * this is a QKD key
 * 
 * A QKD Key has an
 * 
 *  - ID
 *  - Meta Data
 *  - Key Data
 *      
 * The metadata itself is represented as an XML structure.
 * 
 * 
 * A QKD Key is read by a QKD Module, processed upon and then written to 
 * the next QKD Module in the QKD Post Processing Pipeline.
 * 
 * The key data (data()) contain the secret key material.
 * 
 * A Key can be used with the qkd::crypto classes to authenticate
 * or encrypt data.
 * 
 * Keys are passed from one module to the next on one side (e.g. alice).
 * A series of keys is the *keystream* which can be written and read to file.
 * The particles of a keystream are streamed keys which each is a record of:
 * 
 *  - key id                    (uint32_t)  [network byte ordering]
 *  - size of metadata          (uint64_t)  [network byte ordering]
 *  - key metadata as XML       (char)
 *  - key-size in bytes         (uint64_t)  [network byte ordering]
 *  - key-data                  (BLOB)
 */
class key {

    
public:
    
    
    
    /**
     * the number pattern describes how new ids are generated
     * 
     * the number pattern consist of a shift and an add number.
     * any new key id is a monotonic counter increased by 1.
     * then this number is shifted left + and then summed up.
     * 
     * That is:
     * 
     *      shift 3, add 2
     *      counter = 5 ==> new key_id = 42
     * 
     * The shift number corresponds to the maximum number
     * of parallel modules (2^shift) in a pipeline at certain 
     * stage. The add number is used to distinguish between
     * the parallel lines.
     */
    class key_id_counter {
        
    public:
        
        
        /**
         * ctor
         * 
         * @param   nShift      the shift value
         * @param   nAdd        the add value
         */
        key_id_counter(uint32_t nShift = 0, uint32_t nAdd = 0) : m_nCount(0), m_nShift(nShift), m_nAdd(nAdd) {}
        
        
        /**
         * copy ctor
         * 
         * @param   rhs         right hand side
         */
        key_id_counter(key_id_counter const & rhs) : m_nCount(rhs.m_nCount), m_nShift(rhs.m_nShift), m_nAdd(rhs.m_nAdd) {}
        
        
        /**
         * return the add value
         * 
         * @return      the add value used
         */
        inline uint32_t add_value() const { return m_nAdd; }
        
        
        /**
         * return the internal counter
         * 
         * @return      the internal counter used
         */
        inline key_id count() const { return m_nCount; }
        
        
        /**
         * increment the counter (and give back the new key_id)
         * 
         * @return  the new key_id
         */
        inline key_id inc() { return ((++m_nCount << m_nShift) + m_nAdd); }
        
        
        /**
         * sets the internal counter
         * 
         * @param   nCount      the new key id to start from
         */
        inline void set_count(key_id nCount) { m_nCount = nCount; }
        
        
        /**
         * return the shift value
         * 
         * @return      the shift value used
         */
        inline uint32_t shift_value() const { return m_nShift; }
        
        
    private:
        
        
        
        key_id m_nCount;            /**< the counter incremented by 1 */
        uint32_t m_nShift;          /**< the shift value */
        uint32_t m_nAdd;            /**< the add value */
    };
    
    
    /**
     * ctor
     */
    key();
    
    
    /**
     * copy ctor
     * 
     * this is a shallow copy
     * 
     * @param   rhs     right hand side
     */
    key(key const & rhs);


    /**
     * ctor
     * 
     * the given memory area is NOT(!) copied (shallow copy).
     * 
     * @param   nId         ID of the key
     * @param   cMemory     memory holding the key bits
     */
    explicit key(key_id nId, qkd::utility::memory & cMemory);
    
    
    /**
     * ctor
     * 
     * the given memory area is copied (deep copy).
     * 
     * @param   nId         ID of the key
     * @param   cMemory     memory holding the key bits
     */
    explicit key(key_id nId, qkd::utility::memory const & cMemory);
    
    
    /**
     * dtor
     * 
     * Virtual, so a key may be legally subclassed
     */
    virtual ~key() {}
    
    
    /**
     * compare ==
     * 
     * based on key id
     *
     * @param   rhs     right hand side
     * @return  true, if both objects are identical
     */
    inline bool operator==(qkd::key::key const & rhs) const { return (id() == rhs.id()); }


    /**
     * compare !=
     *
     * based on key id
     *
     * @param   rhs     right hand side
     * @return  true, if both objects are different
     */
    inline bool operator!=(qkd::key::key const & rhs) const { return !(*this == rhs); }
    
    
    /**
     * compare <
     * 
     * based on key id
     *
     * @param   rhs     right hand side
     * @return  true, if this key id is less then rhs
     */
    inline bool operator<(qkd::key::key const & rhs) const { return (id() < rhs.id()); }


    /**
     * compare <=
     * 
     * based on key id
     *
     * @param   rhs     right hand side
     * @return  true, if this key id is less then or equal to rhs
     */
    inline bool operator<=(qkd::key::key const & rhs) const { return (((*this) < rhs) || ((*this) == rhs)); }


    /**
     * compare >
     * 
     * based on key id
     *
     * @param   rhs     right hand side
     * @return  true, if this key id is greater then rhs
     */
    inline bool operator>(qkd::key::key const & rhs) const { return (id() > rhs.id()); }


    /**
     * compare >=
     * 
     * based on key id
     *
     * @param   rhs     right hand side
     * @return  true, if this key id is greater then or equal to rhs
     */
    inline bool operator>=(qkd::key::key const & rhs) const { return (((*this) > rhs) || ((*this) == rhs)); }


    /**
     * adds a memory BLOB to the key
     * 
     * The given memory is *deep* copied.
     * 
     * @param   cData       the memory to add
     */
    inline void add(qkd::utility::memory const & cData) { m_cData << cData; }
    
    
    /**
     * return the birth of the key inside the current process
     * 
     * The birth is a time point when the key has been created or
     * read by the current process
     * 
     * @return  time point of birth of key
     */
    std::chrono::high_resolution_clock::time_point birth() const { return m_cTimestampRead; }
    
    
    /**
     * return the birth of the key inside the current process
     * 
     * The birth is a time point when the key has been created or
     * read by the current process
     * 
     * @return  time point of birth of key
     */
    std::chrono::high_resolution_clock::time_point & birth() { return m_cTimestampRead; }
    
    
    /**
     * access the key id counter
     * 
     * @return  the class wide key_id_counter
     */
    static key_id_counter & counter();
    
    
    /**
     * the current crypto scheme used for this key
     * 
     * The crypto scheme string holds the algorithm, init-key
     * and current state of all incoiming messages from the peer 
     * bound to this key (see qkd/crypto/scheme.h)
     * 
     * @return  a string holding the current crypto scheme for incoming
     */
    std::string crypto_scheme_incoming() const { return m_cMetaData.get<std::string>("key.general.crypto.incoming"); }
    
    
    /**
     * the current crypto scheme used for this key
     * 
     * The crypto scheme string holds the algorithm, init-key
     * and current state of all outgoing messages from the peer 
     * bound to this key (see qkd/crypto/scheme.h)
     * 
     * @return  a string holding the current crypto scheme for outgoing
     */
    std::string crypto_scheme_outgoing() const { return m_cMetaData.get<std::string>("key.general.crypto.outgoing"); }
    
    
    /**
     * return the key material within
     * 
     * @return  key bits within this key
     */
    inline qkd::utility::memory & data() { return m_cData; }
    
    
    /**
     * return the key material within (const version)
     * 
     * @return  key bits within this key
     */
    inline qkd::utility::memory const & data() const { return m_cData; }
    
    
    /**
     * the number of disclosed information bits of this key
     * 
     * Returns the number of disclosed information bits which
     * have been leaked during key reconciliation
     * 
     * @return  the number of disclosed information bits
     */
    uint64_t disclosed() const { return m_cMetaData.get<uint64_t>("key.general.disclosed"); }
    
    
    /**
     * return the timespan how long the key is in the current process
     * 
     * @return  the duration of the key in the current process
     */
    inline std::chrono::high_resolution_clock::duration dwell() const { 
        return (std::chrono::high_resolution_clock::now() - m_cTimestampRead); 
    }
   

    /**
     * return a key bit
     * 
     * @param   pos         bit position (hence, size() is bytes!)
     * @return  true if bit set
     */
    inline bool get_bit(uint64_t pos) const { 
        if (pos >= size() * 8) throw std::out_of_range("key bit position out of range");
        return ((data()[pos / 8] & (1 << (pos % 8))) != 0);
    }


    /**
     * get ID of key
     * 
     * @return  the key ID
     */
    inline key_id id() const { return m_nId; }
    
    
    /**
     * check if this key is *deeply* (on byte basis) to another
     * 
     * @param   rhs         right hand side
     * @return  true if this key is equal to rhs on deep byte basis
     */
    inline bool is_equal(qkd::key::key const & rhs) const { 
        return ((size() == rhs.size()) && (memcmp(data().get(), rhs.data().get(), size()) == 0));
    }
    
    
    /**
     * check if this key is null
     * 
     * @return  true if this key is empty
     */
    inline bool is_null() const { return (((*this) == null()) && (data().size() == 0)); }
    
    
    /**
     * get metadata of the key
     * 
     * beware: any incausiuous manpiulation of the 'general'
     * part of the key may lead to uninspected behavior.
     * 
     * @return  the metadata property tree
     */
    inline boost::property_tree::ptree & metadata() { return m_cMetaData; }
    
    
    /**
     * get metadata of the key
     * 
     * @return  the metadata property tree
     */
    inline boost::property_tree::ptree const & metadata() const { return m_cMetaData; }
    
    
    /**
     * get current module section of the key's metadata
     * 
     * @return  the metadata property tree for the key's current module
     */
    boost::property_tree::ptree & metadata_current_module();
    
    
    /**
     * get current module section of the key's metadata
     * 
     * @return  the metadata property tree for the key's current module
     */
    boost::property_tree::ptree const & metadata_current_module() const;
    
    
    /**
     * get modules metadata of the key
     * 
     * @return  the metadata property tree for the key's modules
     */
    boost::property_tree::ptree & metadata_modules();
    
    
    /**
     * get modules metadata of the key
     * 
     * @return  the metadata property tree for the key's modules
     */
    boost::property_tree::ptree const & metadata_modules() const;
    
    
    /**
     * get metadata of the key as XML
     * 
     * @param   bPretty     pretty formatting enabled or not
     * @return  the metadata as XML
     */
    std::string metadata_xml(bool bPretty = false) const;
    
    
    /**
     * the NULL key.
     * 
     * @return  a key which is simply NULL
     */
    static qkd::key::key const & null() { 
        static qkd::key::key cNullKey; 
        return cNullKey; 
    }
    
    
    /**
     * get the key's QBER
     * 
     * this returns the quantum bit error rate associated this this key
     * 
     * @return  the key's QBER
     */
    inline double qber() const { return m_cMetaData.get<double>("key.general.qber"); }
    
    
    /**
     * read from a buffer
     * 
     * if we fail to read a key the key is equal to null()
     * 
     * @param   cBuffer     the buffer to read from
     */
    void read(qkd::utility::buffer & cBuffer);


    /**
     * read from stream
     * 
     * if we fail to read a key the key is equal to null()
     * 
     * @param   cStream     the stream to read from
     */
    void read(std::istream & cStream);


    /**
     * set a key bit
     * 
     * @param   pos         bit position (hence, size() is bytes!)
     * @param   bit         bit value
     */
    inline void set_bit(uint64_t pos, bool bit) { 
        if (pos >= size() * 8) throw std::out_of_range("key bit position out of range");
        auto mask = (1 << (pos % 8)) & 0xFF;
        if (bit) data()[pos / 8] |= mask;
        else data()[pos / 8] &= ~mask;
    }


    /**
     * sets the current crypto scheme used for this key
     * 
     * The crypto scheme string holds the algorithm, init-key
     * and current state of all incoming messages from the peer 
     * bound to this key (see qkd/crypto/scheme.h)
     * 
     * @param   sScheme     a string holding the new current crypto scheme for incoming
     */
    void set_crypto_scheme_incoming(std::string sScheme);
    
    
    /**
     * sets the current crypto scheme used for this key
     * 
     * The crypto scheme string holds the algorithm, init-key
     * and current state of all outgoing messages from the peer 
     * bound to this key (see qkd/crypto/scheme.h)
     * 
     * @param   sScheme     a string holding the new current crypto scheme for outgoing
     */
    void set_crypto_scheme_outgoing(std::string sScheme);
    
    
    /**
     * sets the number of disclosed information bits of this key
     * 
     * @param   nDisclosed  the new number of disclosed information bits
     */
    void set_disclosed(uint64_t nDisclosed);
    
    
    /**
     * set a new key id
     * 
     * @param   nId         the new key id
     */
    void set_id(key_id nId);


    /**
     * set the key's QBER
     * 
     * @param   nQBER       the new key's QBER
     */
    void set_qber(double nQBER);
    
    
    /**
     * set the key's state
     * 
     * @param   eKeyState   the new key state
     */
    void set_state(key_state eKeyState);
    
    
    /**
     * size of key measured in bytes
     * 
     * @return  bytes of the key
     */
    inline uint64_t size() const { return m_cData.size(); }
    
    
    /**
     * get the key's state
     * 
     * @return  current state of the key
     */
    inline key_state state() const { return static_cast<key_state>(m_cMetaData.get<int>("key.general.state")); }
    
    
    /**
     * give a stringified state
     * 
     * @return  a string holding the key state
     */
    inline std::string state_string() const { return state_string(state()); }
    

    /**
     * give a stringified state
     * 
     * @param   eKeyState           the state of the key questioned
     * @return  a string holding the key state
     */
    static std::string state_string(qkd::key::key_state eKeyState);
    

    /**
     * write to buffer
     * 
     * @param   cBuffer     the buffer to write to
     */
    void write(qkd::utility::buffer & cBuffer) const;


    /**
     * write to stream
     * 
     * @param   cStream     the stream to write to
     */
    void write(std::ostream & cStream) const;


private:
    
    
    /**
     * inits the meta data
     */
    void init_metadata();
    
    
    /**
     * key id
     */
    key_id m_nId;
    
    
    /**
     * key data
     */
    qkd::utility::memory m_cData;
    
    
    /**
     * meta data
     */
    boost::property_tree::ptree m_cMetaData;
    
    
    /**
     * timestamp when this key has come into the current process via a read action
     */
    std::chrono::high_resolution_clock::time_point m_cTimestampRead;     
    
};


/**
 * subtract one key_vector from the other
 * 
 * HENCE: lhs and rhs are meant to contain __sorted__ keys
 * 
 * @param   lhs     left hand side
 * @param   rhs     right hand side
 * @return  a key vector containing all key_ids in lhs not in rhs
 */
key_vector sub(key_vector const & lhs, key_vector const & rhs);
  

}

}


/**
 * << - add memory
 * 
 * add more memory to an existing key
 * the memory added uses deep copy
 * 
 * @param   lhs     the left hand side
 * @param   rhs     the right hand side
 * @return  lhs
 */
inline qkd::key::key & operator<<(qkd::key::key & lhs, qkd::utility::memory const & rhs) { 
    lhs.add(rhs); 
    return lhs; 
}


/**
 * << - save to stream
 * 
 * write a key into output stream
 * 
 * @param   lhs     the left hand side
 * @param   rhs     the right hand side
 * @return  the stream
 */
inline std::ostream & operator<<(std::ostream & lhs, qkd::key::key const & rhs) { 
    rhs.write(lhs); 
    return lhs; 
}


/**
 * << - save to buffer
 * 
 * write a key into output buffer
 * 
 * @param   lhs     the left hand side
 * @param   rhs     the right hand side
 * @return  the buffer
 */
inline qkd::utility::buffer & operator<<(qkd::utility::buffer & lhs, qkd::key::key const & rhs) { 
    rhs.write(lhs); 
    return lhs; 
}


/**
 * stream into
 * 
 * write a vector of key-ids into a buffer
 * 
 * @param   lhs     the buffer object
 * @param   rhs     the data to add
 * @return  the buffer object
 */
inline qkd::utility::buffer & operator<<(qkd::utility::buffer & lhs, qkd::key::key_vector const & rhs) { 
    uint64_t nSize = rhs.size(); 
    lhs.push(nSize); 
    for (qkd::key::key_id iter : rhs) {
        lhs.push(iter); 
    }
    return lhs; 
}


/**
 * >> - load from stream
 * 
 * Read a key from an input stream.
 * 
 * @param   lhs     the left hand side
 * @param   rhs     the right hand side
 * @return  the stream
 */
inline std::istream & operator>>(std::istream & lhs, qkd::key::key & rhs) { 
    rhs.read(lhs); 
    return lhs; 
}


/**
 * >> - load from buffer
 * 
 * read a key from a buffer
 * 
 * @param   lhs     the left hand side
 * @param   rhs     the right hand side
 * @return  the buffer
 */
inline qkd::utility::buffer & operator>>(qkd::utility::buffer & lhs, qkd::key::key & rhs) { 
    rhs.read(lhs); 
    return lhs; 
}


/**
 * stream out
 * 
 * Get some data from the buffer
 * 
 * @param   lhs     the buffer object
 * @param   rhs     the data to get
 * @return  the buffer object
 */
inline qkd::utility::buffer & operator>>(qkd::utility::buffer & lhs, qkd::key::key_vector & rhs) { 
    uint64_t nSize; 
    lhs.pop(nSize); 
    rhs.resize(nSize); 
    for (uint64_t i = 0; i < nSize; i++) { 
        qkd::key::key_id nItem; 
        lhs.pop(nItem); 
        rhs[i] = nItem; 
    } 
    return lhs; 
}


/**
 * subtract one key_vector from the other
 * 
 * HENCE: lhs and rhs are meant to contain __sorted__ keys
 * 
 * @param   lhs     left hand side
 * @param   rhs     right hand side
 * @return  a key vector containing all key_ids in lhs not in rhs
 */
inline qkd::key::key_vector operator-(qkd::key::key_vector const & lhs, qkd::key::key_vector const & rhs) { 
    return qkd::key::sub(lhs, rhs); 
}


#endif


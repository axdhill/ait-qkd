/*
 * key_ring.h
 * 
 * a bunch of QKD keys all of the same (but the last)
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

 
#ifndef __QKD_KEY_KEY_RING_H_
#define __QKD_KEY_KEY_RING_H_


// ------------------------------------------------------------
// incs

#include <vector>

#include <inttypes.h>

// ait
#include <qkd/key/key.h>


// ------------------------------------------------------------
// decls


namespace qkd {
    
namespace key {    


/**
 * A key_ring is a bunch (vector) of keys. All keys have the same
 * size but the last.
 * 
 * If a key is added to the key ring, it is cloned (deep copy). The
 * keys in the key_ring may receive a new numbering.
 * 
 * This behaves most likely as a std::vector
 * 
 * BUT: 
 *      - no add to the beginning allowed,
 *      - no arbitrary insert
 *      - all keys are added to the end
 *      - the last key will be slowly filled
 *        with new key material up to the maximum size
 * 
 * Note further:
 * 
 *      if you copy a whole key_ring then this is
 *      done as SHALLOW copy of the keys within. 
 *      Meaning: the memory  blobs of the keys 
 *      in the key_ring are shared.
 * 
 * Example:
 * 
 *      key_ring of size 8:
 * 
 *              | 0 | 1 | 2 | 4 | 5 | 6 | 7 |
 *              +---+---+---+---+---+---+---+
 *      data:   empty
 * 
 * 
 *      add key_1 with size 5:
 * 
 *              | 0 | 1 | 2 | 4 | 5 | 6 | 7 |
 *              +---+---+---+---+---+---+---+
 *      data:   |<--- key_1 --->|
 * 
 * 
 *      add key_2 with size 7:
 * 
 *              | 0 | 1 | 2 | 4 | 5 | 6 | 7 |
 *              +---+---+---+---+---+---+---+
 *      data:   |<--- key_1 --->|<-----------
 *              -- key_2------->|    
 * 
 * 
 *      add key_3 with size 4:
 * 
 *              | 0 | 1 | 2 | 4 | 5 | 6 | 7 |
 *              +---+---+---+---+---+---+---+
 *      data:   |<--- key_1 --->|<-----------
 *              -- key_2------->|<--- key_3 -
 *              --->|    
 *
 * 
 *      extract one key: "|<--- key_1 --->|<- key_2 ->|"
 * 
 *              | 0 | 1 | 2 | 4 | 5 | 6 | 7 |
 *              +---+---+---+---+---+---+---+
 *      data:   |<--- key_2---->|<--- key_3 -
 *              --->|    
 * 
 * This is useful for constructing a series of same length keys
 * from a random number of different keys with arbitrary length.
 */
class key_ring : private std::vector<qkd::key::key> {

public:

    
    /**
     * iterator
     */
    using std::vector<qkd::key::key>::iterator;
    
    
    /**
     * const_iterator
     */
    using std::vector<qkd::key::key>::const_iterator;
    
    
    /**
     * begin
     */
    using std::vector<qkd::key::key>::begin;
    
    
    /**
     * rbegin
     */
    using std::vector<qkd::key::key>::rbegin;
    
    
    /**
     * end
     */
    using std::vector<qkd::key::key>::end;
    
    
    /**
     * rend
     */
    using std::vector<qkd::key::key>::rend;
    
    
    /**
     * size
     */
    using std::vector<qkd::key::key>::size;
    
    
    /**
     * max_size
     */
    using std::vector<qkd::key::key>::max_size;
    
    
    /**
     * capacity
     */
    using std::vector<qkd::key::key>::capacity;
    
    
    /**
     * empty
     */
    using std::vector<qkd::key::key>::empty;
    
    
    /**
     * at
     */
    using std::vector<qkd::key::key>::at;
    
    
    /**
     * front
     */
    using std::vector<qkd::key::key>::front;
    
    
    /**
     * back
     */
    using std::vector<qkd::key::key>::back;
    
    
    /**
     * clear
     */
    using std::vector<qkd::key::key>::clear;
    
    
    /**
     * erase
     */
    using std::vector<qkd::key::key>::erase;
    
    
    /**
     * []
     */
    using std::vector<qkd::key::key>::operator[];
    
    
    /**
     * ctor
     * 
     * @param   nKeySize        maximum capacity of each key in the ring
     * @param   nId             id of the next key
     */
    explicit key_ring(uint64_t nKeySize = 0, key_id nId = 0) : m_nId(nId), m_nKeySize(nKeySize) { }
    
    
    /**
     * return the id of the next key
     * 
     * @return  the id of the next key (which is going to be added)
     */
    inline key_id id() const { return m_nId; }
    
    
    /**
     * the maximum size of a single key in the ring
     * 
     * @return  the maximum size of a single key in the ring
     */
    inline uint64_t key_size() const { return m_nKeySize; }
    
    
    /**
     * add a key to the key ring
     * 
     * depending on the key_ring's key size the given
     * key will be split into enough keys to
     * match the key_ring's specs.
     * 
     * Any key added will contain a new key id.
     * 
     * @param   cKey        the key to add
     */
    void push_back(qkd::key::key const & cKey);
    
    
private:
    
    
    /**
     * id of the next key added to the key ring
     */
    key_id m_nId;
    

    /**
     * one key's maximum size in the ring
     */
    uint64_t m_nKeySize;

    
};
  

}

}


/**
 * << - add key
 * 
 * add a key to a key ring
 * 
 * @param   lhs     the left hand side
 * @param   rhs     the right hand side
 * @return  lhs
 */
inline qkd::key::key_ring & operator<<(qkd::key::key_ring & lhs, qkd::key::key const & rhs) { 
    lhs.push_back(rhs); 
    return lhs; 
}


#endif


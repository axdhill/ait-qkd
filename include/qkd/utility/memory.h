/*
 * memory.h
 * 
 * more controlled memory management
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

 
#ifndef __QKD_UTILITY_MEMORY_H_
#define __QKD_UTILITY_MEMORY_H_


// ------------------------------------------------------------
// incs

#include <cstring>
#include <exception>
#include <string>

#include <inttypes.h>

#include <boost/shared_array.hpp>


// ------------------------------------------------------------
// decls


namespace qkd {
    
namespace utility {    


/**
 * this class represents a memory with some special adds like smart pointer
 * (free resource when last reference is deleted).
 * 
 * The least significant bit is stored at index 0. Hex string representations
 * are expected to have the byte at index postion 0 at the very first two
 * chars.
 * 
 * It also has the ability to act in shallow or deep copy manner.
 * 
 * shallow:     there is no cloning of memory areas on write access (or copy).
 * 
 * deep:        whenever a write is done, the object's memory is detached 
 *              and modified this is actually a Copy-On-Write principle.
 */
class memory {


public:
    
    
    /**
     * the single memory value managed
     */
    typedef unsigned char value_t;


    /**
     * exception type thrown when accessing information out of range
     */
    struct memory_out_of_range : virtual std::exception, virtual boost::exception { };
    

    /**
     * exception type thrown for using a wrong number base in string conversions
     */
    struct memory_string_base_unknown : virtual std::exception, virtual boost::exception { };


    /**
     * ctor
     */
    memory() : m_bShallow(true) , m_cMemory(static_cast<value_t*>(nullptr)), m_nSize(0), m_nInitialSize(0) {};


    /**
     * ctor
     *
     * create a memory area
     *
     * @param   nSize       size of memory controlled
     */
    explicit memory(uint64_t nSize) : m_bShallow(true), m_nSize(nSize), m_nInitialSize(nSize) { 
        m_cMemory = boost::shared_array<value_t>(new value_t[nSize]); 
    };


    /**
     * ctor
     *
     * takes ownership of the memory area pointed to by cMemory. It has
     * to be created with new char[].
     *
     * @param   cMemory     memory created with new char[]
     * @param   nSize       size of memory controlled
     */
    explicit memory(value_t * cMemory, uint64_t nSize) 
            : m_bShallow(true), m_cMemory(cMemory), m_nSize(nSize) , m_nInitialSize(nSize) {};


    /**
     * dtor
     */
    virtual ~memory() {};
    
    
    /**
     * indexer
     * 
     * @param   i       position to retrieve
     * @return  a reference to the data char
     * @throws  memory_out_of_range
     */
    inline value_t const & operator[](uint64_t i) const { 
        if (i > size()) throw memory_out_of_range(); 
        return m_cMemory[i]; 
    };


    /**
     * indexer
     * 
     * @param   i       position to retrieve
     * @return  a reference to the data char
     * @throws  memory_out_of_range
     */
    inline value_t & operator[](uint64_t i) { 
        if (i > size()) throw memory_out_of_range(); 
        if (!is_shallow()) detach(); 
        return m_cMemory[i]; 
    };


    /**
     * compare ==
     *
     * @param   rhs     right hand side
     * @return  true, if both objects are identical
     */
    inline bool operator==(qkd::utility::memory const & rhs) const { return m_cMemory == rhs.m_cMemory; };


    /**
     * compare !=
     *
     * @param   rhs     right hand side
     * @return  true, if both objects are different
     */
    inline bool operator!=(qkd::utility::memory const & rhs) const { return !(*this == rhs); };
    
    
    /**
     * adds more memory
     * 
     * The given memory is copied (deep copy), so this
     * method is expensive.
     * 
     * @param   cData       memory to add
     */
    virtual void add(qkd::utility::memory const & cData) { 
        uint64_t nOldSize = size(); 
        resize(nOldSize + cData.size()); 
        memcpy(get() + nOldSize, cData.get(), cData.size()); 
    };
    
    
    /**
     * give a hex representation of the memory
     *
     * There is NO leading "0x" attached to the string. The first
     * bytes on output are at memory index 0, i.e. the memory is
     * presented from left to right!
     *
     * @return  a string holding the hex representation
     */
    std::string as_hex() const;


    /**
     * give a canonical hex representation of the memory
     *
     * This is a canonical output like "hexdump".
     *
     * @param   sIndent     indent of each line of dump
     * @return  a string holding the canonical hex representation
     */
    std::string canonical(std::string const sIndent) const;


    /**
     * create a checksum of the memory
     * 
     * this draws a "crc32", "md5", ... (see qkd::utility::checksum) 
     * checksum of the memory area
     * 
     * @param   sAlgorithm      any of the known algorithms of qkd::utility::checksum
     * @return  a checksum value
     */
    qkd::utility::memory checksum(std::string const sAlgorithm = "crc32") const;


    /**
     * clones itself
     *
     * @return  a clone
     */
    inline memory clone() const {
        memory res(size());
        std::memcpy(res.get(), get(), size());
        return res;
    }
    
    
    /**
     * generate the crc32 checksum
     * 
     * @return  the crc32 checksum as string value
     */
    inline std::string crc32() const { return checksum("crc32").as_hex(); };


    /**
     * creates a memory object by duplicating a memory area (and taking ownership of the copy)
     * 
     * @param   cData       memory to be copied
     * @param   nSize       size of memory
     * @return  a memory object
     */
    static memory duplicate(value_t const * cData, uint64_t nSize);


    /**
     * check if this memory hold the same data as the givem argument
     *
     * @param   cMemory     the memory to compare to this
     * @return  true, if the memory areas are equal
     */
    virtual bool equal(qkd::utility::memory const & cMemory) const;


    /**
     * fills the complete memory with values
     *
     * @param   nValue      value used to fill
     */
    virtual void fill(value_t nValue);


    /**
     * create a memory object by a given string
     *
     * Leading type specifier (like "0x") are not allowed.
     * We try to parse the string as much as possible and stop.
     *
     * @param   sHex            a string with a hex representation
     * @return  a memory object
     */
    static qkd::utility::memory from_hex(std::string const & sHex);


    /**
     * return the address of the memory block
     *
     * @return  the address of the memory block
     */
    inline value_t const * get() const { return m_cMemory.get(); };


    /**
     * return the address of the memory block
     *
     * @return  the address of the memory block
     */
    inline value_t * get() { 
        if (!is_shallow()) detach(); 
        return m_cMemory.get(); 
    };
    
    
    /**
     * checks if this is NULL memory object
     * 
     * @return  true if this object refers to a NULL object
     */
    inline bool is_null() const { return m_cMemory.get() == nullptr; };
    
    
    /**
     * checks deep copy behavior
     * 
     * If is_shallow() returns true, then this object
     * does NOT copy-on-write
     * 
     * @return  shallow copy behavior
     */
    inline bool is_shallow() const { return m_bShallow; };
    
    
    /**
     * read from stream
     * 
     * @param   cStream     the stream to read from
     */
    void read(std::istream & cStream);


    /**
     * reserves memory 
     * 
     * this is to avoid overzealous enlarge() calls on behalf of resize()
     * 
     * @param   nSize       the size to reserve
     */
    void reserve(uint64_t nSize);


    /**
     * return size of memory reserved
     *
     * @return  the size of the memory reserved
     */
    inline uint64_t reserved() const { return m_nInitialSize; };


    /**
     * resizes the memory
     * 
     * if the given size is larger than any reserved size then
     * this operation is expensive and the content of the new space
     * allocated is random
     * 
     * @param   nSize       the new size of the array
     */
    inline void resize(uint64_t nSize) { 
        if (!is_shallow()) detach();  
        if (nSize < m_nInitialSize) { 
            m_nSize = nSize; 
            return; 
        } 
        enlarge(nSize); 
    };


    /**
     * modifies deep copy behavior
     * 
     * If shallow is turned off then modifing actions
     * copy the memory block.
     * 
     * @param   bShallow        new shallow copy behavior
     */
    inline void set_shallow(bool bShallow) { m_bShallow = bShallow; };


    /**
     * return size of memory
     *
     * @return  the size of the memory managed
     */
    inline uint64_t size() const { return m_nSize; };


    /**
     * verify if there is just one reference to this object
     *
     * @return  true if there is just one reference to this
     */
    inline bool unique() const { return m_cMemory.unique(); };
    
    
    /**
     * creates a memory object by wraping a memory area
     * this DOES NOT take ownership of the memory.
     * 
     * @param   cData       memory to be wrapped
     * @param   nSize       size of memory
     * @return  a memory object
     */
    static memory wrap(value_t * cData, uint64_t nSize);


    /**
     * write to stream
     * 
     * @param   cStream     the stream to write to
     */
    void write(std::ostream & cStream) const;


private:


    /**
     * make our own copy
     */
    inline void detach() {

        // coded within the header file to potentially
        // let the compiler inline that

        // don't do something if this object is unique anyway
        if (m_cMemory.unique()) return;

        // make a copy
        value_t * cMem = new value_t[size()]; 
        std::memcpy(cMem, m_cMemory.get(), size()); 
        m_cMemory = boost::shared_array<value_t>(cMem);
        m_nInitialSize = size();
    };
    
    
    /**
     * enlarge the memory
     * 
     * this operation is expensive and the content of the new space
     * allocated is random
     * 
     * @param   nSize       the new size of the array
     */
    void enlarge(uint64_t nSize);


    /**
     * flag for shallow copy behavior: don't do a full deep copy
     */
    bool m_bShallow;


    /**
     * the memory area managed by this object
     */
    boost::shared_array<value_t> m_cMemory;
    
    
    /**
     * size of memory area controlled
     */
    uint64_t m_nSize;
    
    
    /**
     * initial size of the memory area controlled (== reserved size)
     */
    uint64_t m_nInitialSize;
    
};


}
    
}


/**
 * << - add memory
 * 
 * add more memory to an existing memory block
 * the memory added uses deep copy
 * 
 * @param   lhs     the left hand side
 * @param   rhs     the right hand side
 * @return  lhs
 */
inline qkd::utility::memory & operator<<(qkd::utility::memory & lhs, qkd::utility::memory const & rhs) { 
    lhs.add(rhs); return lhs; 
}


/**
 * << - save to stream
 * 
 * write a memory block into output stream
 * 
 * @param   lhs     the left hand side
 * @param   rhs     the right hand side
 * @return  the stream
 */
inline std::ostream & operator<<(std::ostream & lhs, qkd::utility::memory const & rhs) { 
    rhs.write(lhs); return lhs; 
}


/**
 * >> - load from stream
 * 
 * Read memory block from an input stream.
 * 
 * @param   lhs     the left hand side
 * @param   rhs     the right hand side
 * @return  the stream
 */
inline std::istream & operator>>(std::istream & lhs, qkd::utility::memory & rhs) { 
    rhs.read(lhs); return lhs; 
}


#endif


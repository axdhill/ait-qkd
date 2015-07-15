/*
 * buffer.h
 * 
 * a send/recv buffer
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

 
#ifndef __QKD_UTILITY_BUFFER_H_
#define __QKD_UTILITY_BUFFER_H_


// ------------------------------------------------------------
// incs

#include <endian.h>
#include <exception>
#include <list>
#include <set>
#include <string>
#include <vector>

#include <inttypes.h>

// ait
#include <qkd/utility/memory.h>


// ------------------------------------------------------------
// decls


namespace qkd {
    
namespace utility {    


/**
 * This is a send/recv buffer
 *
 * It extends the functionality of qkd::utility::memory by neat
 * functions to push and pop common data particles in a system/network
 * independent manner.
 * 
 * The idea of this class is to have a nice send/recv buffer management at hand
 * with respects byte ordering.
 * 
 * Also the buffer grows more agressively by 1K blocks to avoid
 * memory defragmentation on consecuting pushes. 
 * 
 * with the position() and set_position() calls you may change the read/write position
 * freely.
 */
class buffer : public qkd::utility::memory {
    
    
public:


    /**
     * exception type thrown when reading beyond space
     */
    struct buffer_out_of_bounds : virtual std::exception, virtual boost::exception {};
    
    
    /**
     * ctor
     */
    buffer() : memory(), m_nPosition(0) {};
    
    
    /**
     * copy ctor
     * 
     * @param   rhs         right hand side
     */
    buffer(memory const & rhs) : memory(rhs), m_nPosition(0) {};
    
    
    /**
     * dtor
     */
    virtual ~buffer() {};
    
    
    /**
     * put another data particle on top of buffer at the read/write position
     * 
     * @param   cData       pointer to data
     * @param   nSize       size of data
     */
    inline void add(void * cData, uint64_t nSize) { 
        grow(nSize); 
        memcpy(get() + m_nPosition, cData, nSize); 
        m_nPosition += nSize; 
    };
    
    
    /**
     * check if we are at end of field (="buffer")
     * 
     * @return  true, if there is no more left to be read
     */
    inline bool eof() const { return (position() == size()); };
    
    
    /**
     * pick yet another data from the buffer
     * 
     * @param   cData       pointer to receiving data
     * @param   nSize       size of data
     */
    inline void pick(void * cData, uint64_t nSize) { 
        if ((m_nPosition + nSize) > size()) throw buffer_out_of_bounds(); 
        memcpy(cData, get() + m_nPosition, nSize); 
        m_nPosition += nSize; 
    };
    
    
    /**
     * read a bool from the current read/write position
     * 
     * @param   b       the bool to read
     */
    inline void pop(bool & b) { pick(&b, sizeof(b)); };
    
    
    /**
     * read a char from the current read/write position
     * 
     * @param   c       the char to read
     */
    inline void pop(char & c) { pick(&c, sizeof(c)); };
    
    
    /**
     * read an unsigned char from the current read/write position
     * 
     * @param   c       the char to read
     */
    inline void pop(unsigned char & c) { pick(&c, sizeof(c)); };
    
    
    /**
     * read an int16_t from the current read/write position
     * 
     * @param   i       the int16_t to read
     */
    inline void pop(int16_t & i) { int16_t x; pick(&x, sizeof(x)); i = be16toh(x); };
    
    
    /**
     * read an uint16_t from the current read/write position
     * 
     * @param   i       the uint16_t to read
     */
    inline void pop(uint16_t & i) { uint16_t x; pick(&x, sizeof(x)); i = be16toh(x); };
    
    
    /**
     * read an int32_t from the current read/write position
     * 
     * @param   i       the int32_t to read
     */
    inline void pop(int32_t & i) { int32_t x; pick(&x, sizeof(x)); i = be32toh(x); };
    
    
    /**
     * read an uint32_t from the current read/write position
     * 
     * @param   i       the uint32_t to read
     */
    inline void pop(uint32_t & i) { uint32_t x; pick(&x, sizeof(x)); i = be32toh(x); };
    
    
    /**
     * read an int64_t from the current read/write position
     * 
     * @param   i       the int64_t to read
     */
    inline void pop(int64_t & i) { int64_t x; pick(&x, sizeof(x)); i = be64toh(x); };
    
    
    /**
     * read an uint64_t from the current read/write position
     * 
     * @param   i       the uint64_t to read
     */
    inline void pop(uint64_t & i) { uint64_t x; pick(&x, sizeof(x)); i = be64toh(x); };
    
    
    /**
     * read an float from the current read/write position
     * 
     * @param   f       the float to read
     */
    inline void pop(float & f) { pick(&f, sizeof(f)); };
    
    
    /**
     * read an double from the current read/write position
     * 
     * @param   d       the double to read
     */
    inline void pop(double & d) { pick(&d, sizeof(d)); };
    
    
    /**
     * get a memory from the current read/write position
     * 
     * @param   m       the memory to get (out)
     */
    inline void pop(qkd::utility::memory & m) { 
        uint64_t nSize; pop(nSize); 
        m.resize(nSize); 
        pick(m.get(), nSize); 
    };
    
    
    /**
     * get a string from the current read/write position
     * 
     * @param   s       the string to get (out)
     */
    inline void pop(std::string & s) { 
        uint64_t nSize; pop(nSize); 
        char * d = new char[nSize]; 
        pick(d, nSize); 
        s = std::string(d, nSize); 
        delete [] d; 
    };
    
    
    /**
     * get the current read/write position
     * 
     * @return  the current read/write position
     */
    inline uint64_t position() const { return m_nPosition; };
    
    
    /**
     * add a bool at the current read/write position
     * 
     * @param   b       the bool to add
     */
    inline void push(bool b) { add(&b, sizeof(b)); };
    
    
    /**
     * add a char at the current read/write position
     * 
     * @param   c       the char to add
     */
    inline void push(char c) { add(&c, sizeof(c)); };
    
    
    /**
     * add an unsigned char at the current read/write position
     * 
     * @param   c       the char to add
     */
    inline void push(unsigned char c) { add(&c, sizeof(c)); };
    
    
    /**
     * add an int16_t at the current read/write position
     * 
     * @param   i       the int16_t to add
     */
    inline void push(int16_t i) { int16_t x = htobe16(i); add(&x, sizeof(x)); };
    
    
    /**
     * add an uint16_t at the current read/write position
     * 
     * @param   i       the uint16_t to add
     */
    inline void push(uint16_t i) { uint16_t x = htobe16(i); add(&x, sizeof(x)); };
    
    
    /**
     * add an int32_t at the current read/write position
     * 
     * @param   i       the int32_t to add
     */
    inline void push(int32_t i) { int32_t x = htobe32(i); add(&x, sizeof(x)); };
    
    
    /**
     * add an uint32_t at the current read/write position
     * 
     * @param   i       the uint32_t to add
     */
    inline void push(uint32_t i) { uint32_t x = htobe32(i); add(&x, sizeof(x)); };
    
    
    /**
     * add an int64_t at the current read/write position
     * 
     * @param   i       the int64_t to add
     */
    inline void push(int64_t i) { int64_t x = htobe64(i); add(&x, sizeof(x)); };
    
    
    /**
     * add an uint64_t at the current read/write position
     * 
     * @param   i       the uint64_t to add
     */
    inline void push(uint64_t i) { uint64_t x = htobe64(i); add(&x, sizeof(x)); };
    
    
    /**
     * add a float at the current read/write position
     * 
     * @param   f       the float to add
     */
    inline void push(float f) { add(&f, sizeof(f)); };
    
    
    /**
     * add a double at the current read/write position
     * 
     * @param   d       the double to add
     */
    inline void push(double d) { add(&d, sizeof(d)); };
    
    
    /**
     * add a memory at the current read/write position
     * 
     * @param   m       the memory to add
     */
    inline void push(qkd::utility::memory const & m) { 
        uint64_t nSize = m.size(); 
        push(nSize); 
        add((void *)m.get(), nSize); 
    };
    
    
    /**
     * add a string at the current read/write position
     * 
     * @param   s       the string to add
     */
    inline void push(std::string const & s) { 
        uint64_t nSize = s.size(); 
        push(nSize); 
        add((void *)s.data(), nSize); 
    };
    
    
    /**
     * reset read/write position
     *
     * this does not discard, free or delete any memory already 
     * hold within the buffer. it justs repositionate the read/write
     * headert to the beginning of the buffer
     */
    inline void reset() { set_position(0); };


    /**
     * set the read/write postion
     * 
     * @param   nPosition       the new read/write position
     */
    inline void set_position(uint64_t nPosition) { 
        if (nPosition > size()) grow(nPosition); 
        m_nPosition = nPosition; 
    };
    
    
private:
    
    
    /**
     * grows the memory managed by at least 1Kbyte
     * if necessary
     * 
     * The idea is to enforce the internal size of 
     * the memory class to be up to 1K larger then as necessary
     * with a overzelous resize() call. then resizing again will
     * leaver the size value correct, wheras internally we reserved
     * more bytes as needed
     * 
     * @param   nSize       amount of needed free size within the memory managed
     */
    inline void grow(uint64_t nSize) { 
        if (m_nPosition + nSize <= size()) return; 
        uint64_t nNewSize = m_nPosition + nSize; 
        resize(size() + nSize <= size() + grow_step() ? size() + grow_step() : size() + nSize); 
        resize(nNewSize); 
    };
        
    
    /**
     * get the grow step
     * 
     * the grow step is one 10th of the reserved space 
     * and at a minimum 1K and
     */
    inline uint64_t grow_step() const { return (reserved() / 10 < 1024 ? 1024 : reserved() / 10); };
        
    
    /**
     * read/write position
     */
    uint64_t m_nPosition;
    
};


/**
 * stream into
 * 
 * Add some data to the buffer
 * 
 * @param   lhs     the buffer object
 * @param   rhs     the data to add
 * @return  the buffer object
 */
inline buffer & operator<<(buffer & lhs, bool rhs) { lhs.push(rhs); return lhs; }


/**
 * stream into
 * 
 * Add some data to the buffer
 * 
 * @param   lhs     the buffer object
 * @param   rhs     the data to add
 * @return  the buffer object
 */
inline buffer & operator<<(buffer & lhs, char rhs) { lhs.push(rhs); return lhs; }


/**
 * stream into
 * 
 * Add some data to the buffer
 * 
 * @param   lhs     the buffer object
 * @param   rhs     the data to add
 * @return  the buffer object
 */
inline buffer & operator<<(buffer & lhs, unsigned char rhs) { lhs.push(rhs); return lhs; }


/**
 * stream into
 * 
 * Add some data to the buffer
 * 
 * @param   lhs     the buffer object
 * @param   rhs     the data to add
 * @return  the buffer object
 */
inline buffer & operator<<(buffer & lhs, int16_t rhs) { lhs.push(rhs); return lhs; }


/**
 * stream into
 * 
 * Add some data to the buffer
 * 
 * @param   lhs     the buffer object
 * @param   rhs     the data to add
 * @return  the buffer object
 */
inline buffer & operator<<(buffer & lhs, uint16_t rhs) { lhs.push(rhs); return lhs; }


/**
 * stream into
 * 
 * Add some data to the buffer
 * 
 * @param   lhs     the buffer object
 * @param   rhs     the data to add
 * @return  the buffer object
 */
inline buffer & operator<<(buffer & lhs, int32_t rhs) { lhs.push(rhs); return lhs; }


/**
 * stream into
 * 
 * Add some data to the buffer
 * 
 * @param   lhs     the buffer object
 * @param   rhs     the data to add
 * @return  the buffer object
 */
inline buffer & operator<<(buffer & lhs, uint32_t rhs) { lhs.push(rhs); return lhs; }


/**
 * stream into
 * 
 * Add some data to the buffer
 * 
 * @param   lhs     the buffer object
 * @param   rhs     the data to add
 * @return  the buffer object
 */
inline buffer & operator<<(buffer & lhs, int64_t rhs) { lhs.push(rhs); return lhs; }


/**
 * stream into
 * 
 * Add some data to the buffer
 * 
 * @param   lhs     the buffer object
 * @param   rhs     the data to add
 * @return  the buffer object
 */
inline buffer & operator<<(buffer & lhs, uint64_t rhs) { lhs.push(rhs); return lhs; }


/**
 * stream into
 * 
 * Add some data to the buffer
 * 
 * @param   lhs     the buffer object
 * @param   rhs     the data to add
 * @return  the buffer object
 */
inline buffer & operator<<(buffer & lhs, float rhs) { lhs.push(rhs); return lhs; }


/**
 * stream into
 * 
 * Add some data to the buffer
 * 
 * @param   lhs     the buffer object
 * @param   rhs     the data to add
 * @return  the buffer object
 */
inline buffer & operator<<(buffer & lhs, double rhs) { lhs.push(rhs); return lhs; }


/**
 * stream into
 * 
 * Add some data to the buffer
 * 
 * @param   lhs     the buffer object
 * @param   rhs     the data to add
 * @return  the buffer object
 */
inline buffer & operator<<(buffer & lhs, memory const & rhs) { lhs.push(rhs); return lhs; }


/**
 * stream into
 * 
 * Add some data to the buffer
 * 
 * @param   lhs     the buffer object
 * @param   rhs     the data to add
 * @return  the buffer object
 */
inline buffer & operator<<(buffer & lhs, std::string const & rhs) { lhs.push(rhs); return lhs; }


/**
 * stream into
 * 
 * Add some data to the buffer
 * 
 * @param   lhs     the buffer object
 * @param   rhs     the data to add
 * @return  the buffer object
 */
template<typename T> buffer & operator<<(buffer & lhs, std::list<T> const & rhs) { 
    lhs.push(rhs.size()); 
    for (T const & i: rhs) lhs << i; 
    return lhs; 
}


/**
 * stream into
 * 
 * Add some data to the buffer
 * 
 * @param   lhs     the buffer object
 * @param   rhs     the data to add
 * @return  the buffer object
 */
template<typename T> buffer & operator<<(buffer & lhs, std::set<T> const & rhs) { 
    lhs.push(rhs.size()); 
    for (T const & i: rhs) lhs << i; 
    return lhs; 
}


/**
 * stream into
 * 
 * Add some data to the buffer
 * 
 * @param   lhs     the buffer object
 * @param   rhs     the data to add
 * @return  the buffer object
 */
template<typename T> buffer & operator<<(buffer & lhs, std::vector<T> const & rhs) { 
    lhs.push(rhs.size()); 
    for (T const & i: rhs) lhs << i; 
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
inline buffer & operator>>(buffer & lhs, bool & rhs) { lhs.pop(rhs); return lhs; }


/**
 * stream out
 * 
 * Get some data from the buffer
 * 
 * @param   lhs     the buffer object
 * @param   rhs     the data to get
 * @return  the buffer object
 */
inline buffer & operator>>(buffer & lhs, char & rhs) { lhs.pop(rhs); return lhs; }


/**
 * stream out
 * 
 * Get some data from the buffer
 * 
 * @param   lhs     the buffer object
 * @param   rhs     the data to get
 * @return  the buffer object
 */
inline buffer & operator>>(buffer & lhs, unsigned char & rhs) { lhs.pop(rhs); return lhs; }


/**
 * stream out
 * 
 * Get some data from the buffer
 * 
 * @param   lhs     the buffer object
 * @param   rhs     the data to get
 * @return  the buffer object
 */
inline buffer & operator>>(buffer & lhs, int16_t & rhs) { lhs.pop(rhs); return lhs; }


/**
 * stream out
 * 
 * Get some data from the buffer
 * 
 * @param   lhs     the buffer object
 * @param   rhs     the data to get
 * @return  the buffer object
 */
inline buffer & operator>>(buffer & lhs, uint16_t & rhs) { lhs.pop(rhs); return lhs; }


/**
 * stream out
 * 
 * Get some data from the buffer
 * 
 * @param   lhs     the buffer object
 * @param   rhs     the data to get
 * @return  the buffer object
 */
inline buffer & operator>>(buffer & lhs, int32_t & rhs) { lhs.pop(rhs); return lhs; }


/**
 * stream out
 * 
 * Get some data from the buffer
 * 
 * @param   lhs     the buffer object
 * @param   rhs     the data to get
 * @return  the buffer object
 */
inline buffer & operator>>(buffer & lhs, uint32_t & rhs) { lhs.pop(rhs); return lhs; }


/**
 * stream out
 * 
 * Get some data from the buffer
 * 
 * @param   lhs     the buffer object
 * @param   rhs     the data to get
 * @return  the buffer object
 */
inline buffer & operator>>(buffer & lhs, int64_t & rhs) { lhs.pop(rhs); return lhs; }


/**
 * stream out
 * 
 * Get some data from the buffer
 * 
 * @param   lhs     the buffer object
 * @param   rhs     the data to get
 * @return  the buffer object
 */
inline buffer & operator>>(buffer & lhs, uint64_t & rhs) { lhs.pop(rhs); return lhs; }


/**
 * stream out
 * 
 * Get some data from the buffer
 * 
 * @param   lhs     the buffer object
 * @param   rhs     the data to get
 * @return  the buffer object
 */
inline buffer & operator>>(buffer & lhs, float & rhs) { lhs.pop(rhs); return lhs; }


/**
 * stream out
 * 
 * Get some data from the buffer
 * 
 * @param   lhs     the buffer object
 * @param   rhs     the data to get
 * @return  the buffer object
 */
inline buffer & operator>>(buffer & lhs, double & rhs) { lhs.pop(rhs); return lhs; }


/**
 * stream out
 * 
 * Get some data from the buffer
 * 
 * @param   lhs     the buffer object
 * @param   rhs     the data to get
 * @return  the buffer object
 */
inline buffer & operator>>(buffer & lhs, memory & rhs) { lhs.pop(rhs); return lhs; }


/**
 * stream out
 * 
 * Get some data from the buffer
 * 
 * @param   lhs     the buffer object
 * @param   rhs     the data to get
 * @return  the buffer object
 */
inline buffer & operator>>(buffer & lhs, std::string & rhs) { lhs.pop(rhs); return lhs; }


/**
 * stream out
 * 
 * Get some data from thebuffer
 * 
 * @param   lhs     the buffer object
 * @param   rhs     the data to get
 * @return  the buffer object
 */
template<typename T> buffer & operator>>(buffer & lhs, std::list<T> & rhs) { 
    uint64_t n; 
    lhs.pop(n); 
    for (uint64_t i = 0; i < n; ++i) { 
        T t; 
        lhs.pop(t); 
        rhs.push_back(t); 
    } 
    return lhs; 
}


/**
 * stream out
 * 
 * Get some data from thebuffer
 * 
 * @param   lhs     the buffer object
 * @param   rhs     the data to get
 * @return  the buffer object
 */
template<typename T> buffer & operator>>(buffer & lhs, std::set<T> & rhs) { 
    uint64_t n; 
    lhs.pop(n); 
    for (uint64_t i = 0; i < n; ++i) { 
        T t; 
        lhs.pop(t); 
        rhs.insert(t); 
    } 
    return lhs; 
}


/**
 * stream out
 * 
 * Get some data from thebuffer
 * 
 * @param   lhs     the buffer object
 * @param   rhs     the data to get
 * @return  the buffer object
 */
template<typename T> buffer & operator>>(buffer & lhs, std::vector<T> & rhs) { 
    uint64_t n; 
    lhs.pop(n); 
    for (uint64_t i = 0; i < n; ++i) { 
        T t; 
        lhs.pop(t); 
        rhs.push_back(t); 
    } 
    return lhs; 
}


}
    
}


#endif


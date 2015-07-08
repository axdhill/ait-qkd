/*
 * memory.cpp
 * 
 * implements functions in memory.h
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

 
// ------------------------------------------------------------
// incs

#include <iomanip>
#include <sstream>

#include <boost/format.hpp>

// ait
#include <qkd/common_macros.h>
#include <qkd/utility/backtrace.h>
#include <qkd/utility/checksum.h>
#include <qkd/utility/memory.h>


using namespace qkd::utility;


// ------------------------------------------------------------
// decls


/**
 * artificial wrap deleter
 * 
 * @param   p           pointer to memory to (not) delete
 */
void wrapped_memory_deleter(UNUSED qkd::utility::memory::value_t * p);


namespace qkd {
    
namespace utility {    

    
/**
 * our own dual table
 */
class dual_table {

public:

    /**
     * ctor
     */
    dual_table() {

        // create lookup table of strings
        for (int64_t i = 0; i < 256; i++) {

            std::stringstream cStringStream;

            if (i & 0x80) cStringStream << '1';
            else cStringStream << '0';
            if (i & 0x40) cStringStream << '1';
            else cStringStream << '0';
            if (i & 0x20) cStringStream << '1';
            else cStringStream << '0';
            if (i & 0x10) cStringStream << '1';
            else cStringStream << '0';
            if (i & 0x08) cStringStream << '1';
            else cStringStream << '0';
            if (i & 0x04) cStringStream << '1';
            else cStringStream << '0';
            if (i & 0x02) cStringStream << '1';
            else cStringStream << '0';
            if (i & 0x01) cStringStream << '1';
            else cStringStream << '0';

            cStringStream >> value[i];
        }
    }


    /**
     * table data
     */
    std::string value[256];
};

}

}


// ------------------------------------------------------------
// code


/**
 * give a hex representation of the memory
 *
 * There is NO leading "0x" attached to the string. The first
 * bytes on output are at memory index 0, i.e. the memory is
 * presented from left to right!
 *
 * @return  a string holding the hex representation
 */
std::string memory::as_hex() const {
    
    // dump memory into stream
    std::stringstream ss;
    for (uint64_t i = 0; i < size(); ++i) { 
        ss << std::setw(2) << std::setfill('0') << std::hex << (int)m_cMemory.get()[i];
    }
    
    return ss.str();
}


/**
 * give a canonical hex representation of the memory
 *
 * This is a canonical output like "hexdump".
 *
 * @param   sIndent     indent of each line of dump
 * @return  a string holding the canonical hex representation
 */
std::string memory::canonical(std::string const sIndent) const {
    
    std::stringstream ss;
    
    // walk over the memory
    for (uint64_t i = 0; i < size(); i += 16) {
        
        if (i) ss << "\n";
    
        // format each line with boost::format
        boost::format cLineFormater = boost::format("%s%08x   %-49s  |%-17s|");
        
        // get the hex and the ascii representation
        std::stringstream ss_hex;
        std::stringstream ss_ascii;
        
        // lower 8 bytes
        for (uint64_t j = i; j < std::min((uint64_t)size(), i + 8); j++) {
            
            // the values
            ss_hex << boost::format("%02x ") % (int)m_cMemory[j];
            if ((m_cMemory[j] >= ' ') && (m_cMemory[j] <= '~')) ss_ascii << m_cMemory[j];
            else ss_ascii << '.';
        }
        
        // spacer
        ss_hex << ' ';
        ss_ascii << ' ';
        
        // upper 8 bytes
        for (uint64_t j = i + 8; j < std::min((uint64_t)size(), i + 16); j++) {
            
            // the values
            ss_hex << boost::format("%02x ") % (int)m_cMemory[j];
            if ((m_cMemory[j] >= ' ') && (m_cMemory[j] <= '~')) ss_ascii << m_cMemory[j];
            else ss_ascii << '.';
        }
        
        // construct the line
        cLineFormater % sIndent;
        cLineFormater % i;
        cLineFormater % ss_hex.str();
        cLineFormater % ss_ascii.str();
        
        ss << cLineFormater.str();
    }
    
    return ss.str();
}


/**
 * create a checksum of the memory
 * 
 * this draws a "crc32", "md5", ... (see qkd::utility::checksum) 
 * checksum of the memory area
 * 
 * @param   sAlgorithm      any of the known algorithms of qkd::utility::checksum
 * @return  a checksum value
 */
memory memory::checksum(std::string const sAlgorithm) const {
    
    qkd::utility::memory cChecksum;
    
    // get an algorithm instance
    qkd::utility::checksum cAlgorithm = qkd::utility::checksum_algorithm::create(sAlgorithm);
    
    // do the algorithm
    cAlgorithm << (*this);
    cChecksum = cAlgorithm->finalize();
        
    return cChecksum;
}


/**
 * creates a memory object by duplicating a memory area (and taking ownership of the copy)
 * 
 * @param   cData       memory to be copied
 * @param   nSize       size of memory
 * @return  a memory object
 */
memory memory::duplicate(value_t const * cData, uint64_t nSize) {
    
    // sanity check
    if (!cData) return memory(0);
    if (nSize == 0) return memory(0);
    
    value_t * p = new value_t[nSize]; 
    memcpy(p, cData, nSize);
    return memory(p, nSize);
}
     
     
/**
 * enlarge the memory
 * 
 * this operation is expensive and the content of the new space
 * allocated is random
 * 
 * @param   nSize       the new size of the array
 */
void memory::enlarge(uint64_t nSize) {

    // expensive
    value_t * cMem = new value_t[nSize]; 
    std::memcpy(cMem, m_cMemory.get(), size()); 
    m_cMemory = boost::shared_array<value_t>(cMem);
    m_nSize = nSize;
    m_nInitialSize = nSize;
}    


/**
 * check if this memory hold the same data as the givem argument
 *
 * @param   cMemory     the memory to compare to this
 * @return  true, if the memory areas are equal
 */
bool memory::equal(qkd::utility::memory const & cMemory) const { 

    if ((*this) == cMemory) return true;
    if (size() != cMemory.size()) return false;
    return memcmp(m_cMemory.get(), cMemory.m_cMemory.get(), size()) == 0;
}


/**
 * fills the complete memory with values
 *
 * @param   nValue      value used to fill
 */
void memory::fill(value_t nValue) {
    if (!size()) return;
    memset(m_cMemory.get(), nValue, size());
}


/**
 * create a memory object by a given string
 *
 * Leading type specifier (like "0x") are not allowed.
 * We try to parse the string as much as possible and stop.
 *
 * @param   sHex            a string with a hex representation
 * @return  a memory object
 */
qkd::utility::memory memory::from_hex(std::string const & sHex) {
    
    // tried to use mpir functions. However, they
    // yield errornous results if the string has an odd
    // number of chars and is meant to be read from left
    // to right
    
    // doing this per pedes on our own then ...
    
    // lower case
    std::string sLower = sHex;
    std::transform(sLower.begin(), sLower.end(), sLower.begin(), ::tolower);
    
    // init space
    uint64_t nInitialSize = sHex.length() / 2 + 1;
    qkd::utility::memory cMemory(nInitialSize);
    
    // walk over the string
    uint64_t i = 0;
    value_t nValue = 0;
    for (auto & c : sLower) {

        // char comparision
        if ((c >= '0') && (c <= '9')) {
            nValue |= ((i % 2) ? c - '0'  : (c - '0') << 4);
        }
        else
        if ((c >= 'a') && (c <= 'f')) {
            nValue |= ((i % 2) ? c - 'a' + 0x0a : (c - 'a' + 0x0a) << 4);
        }
        else {
            // woha! illegal char detected!
            break;
        }

        i++;
        
        // write a full byte (if we had 2 read so far)
        if (!(i % 2)) {
            cMemory[(i - 1) / 2] = nValue;
            nValue = 0;
        }
    }
    if (i % 2) {
        cMemory[(i - 1) / 2] = nValue;
        i++;
    }
    
    // correct the size
    cMemory.resize(i / 2);
    
    return cMemory;
}


/**
 * read from stream
 * 
 * @param   cStream     the stream to read from
 */
void memory::read(std::istream & cStream) { 

    // get proper size value
    uint64_t nSize = 0; 
    cStream.read((char *)&nSize, sizeof(nSize)); 
    nSize = be64toh(nSize); 
    
    // read in blob
    resize(nSize); 
    cStream.read((char *)get(), nSize); 
}


/**
 * reserves memory 
 * 
 * this is to avoid overzealous enlarge() calls on behalf of resize()
 * 
 * @param   nSize       the size to reserve
 */
void memory::reserve(uint64_t nSize) {
    uint64_t nCurrentSize = size();
    enlarge(nSize);
    resize(nCurrentSize);
}


/**
 * creates a memory object by wraping a memory area
 * this DOES NOT take ownership of the memory.
 * 
 * @param   cData       memory to be wrapped
 * @param   nSize       size of memory
 * @return  a memory object
 */
memory memory::wrap(value_t * cData, uint64_t nSize) {
    
    // sanity check
    if (!cData) return memory(0);
    if (nSize == 0) return memory(0);
    
    qkd::utility::memory cMemory;
    cMemory.m_cMemory = boost::shared_array<value_t>(cData, wrapped_memory_deleter);
    cMemory.m_bShallow = true;
    cMemory.m_nSize = nSize;
    cMemory.m_nInitialSize = nSize;
    
    return cMemory;
}


/**
 * write to stream
 * 
 * @param   cStream     the stream to write to
 */
void memory::write(std::ostream & cStream) const { 
    
    // write size value
    uint64_t nSize = htobe64(size()); 
    cStream.write((char *)&nSize, sizeof(nSize)); 
    
    // write blob
    cStream.write((char *)get(), size()); 
}


/**
 * artificial wrap deleter
 * 
 * @param   p           pointer to memory to (not) delete
 */
void wrapped_memory_deleter(UNUSED qkd::utility::memory::value_t * p) {
}

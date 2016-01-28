/*
 * zip.cpp
 * 
 * implement ZIP by using the zlib
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

#include <zlib.h>

// ait
#include <qkd/utility/zip.h>


using namespace qkd::utility;


// ------------------------------------------------------------
// code


/**
 * compress a memory area
 * 
 * @param   cMemory         the memory to compress
 * @return  the compressed memory
 */
qkd::utility::memory zip::deflate(qkd::utility::memory const & cMemory) {
    
    // due to avail_in we cannot pass a const unsigned char *
    // therefore we have to clone the input ... :(
    qkd::utility::memory cInput = cMemory.clone();
    qkd::utility::memory cCompressedMemory;
    
    // setup ZLIB stream
    z_stream cZStream;
    cZStream.zalloc = Z_NULL;
    cZStream.zfree = Z_NULL;
    cZStream.opaque = Z_NULL;
    if (deflateInit(&cZStream, Z_DEFAULT_COMPRESSION) != Z_OK) return qkd::utility::memory(0);
    
    // all in
    cZStream.avail_in = cInput.size();
    cZStream.next_in = (unsigned char *)cInput.get();
    
    do {
        
        // compress 32 KByte
        unsigned char cChunk[32 * 1024];
        cZStream.avail_out = 32 * 1024;
        cZStream.next_out = cChunk;
        
        // compress
        if (::deflate(&cZStream, Z_FINISH) == Z_STREAM_ERROR) return qkd::utility::memory(0);
        
        // consume data
        uint64_t nOldSize = cCompressedMemory.size();
        uint64_t nCompressed = 32 * 1024 - cZStream.avail_out;
        cCompressedMemory.resize(cCompressedMemory.size() + nCompressed);
        memcpy(cCompressedMemory.get() + nOldSize, cChunk, nCompressed);
        
    } while (cZStream.avail_out == 0);
    
    // wind down zip progress
    deflateEnd(&cZStream);
    
    return cCompressedMemory;
}


/**
 * decompress a memory area
 * 
 * @param   cMemory         the memory to decompress
 * @return  the decompressed memory
 */
qkd::utility::memory zip::inflate(qkd::utility::memory const & cMemory) {
    
    // due to avail_in we cannot pass a const unsigned char *
    // therefore we have to clone the input ... :(
    qkd::utility::memory cInput = cMemory.clone();
    qkd::utility::memory cDecompressedMemory;
    
    // setup ZLIB stream
    z_stream cZStream;
    cZStream.zalloc = Z_NULL;
    cZStream.zfree = Z_NULL;
    cZStream.opaque = Z_NULL;
    if (inflateInit(&cZStream) != Z_OK) return qkd::utility::memory(0);
    
    // all in
    cZStream.avail_in = cInput.size();
    cZStream.next_in = (unsigned char *)cInput.get();
    
    do {
        
        // decompress 32 KByte
        unsigned char cChunk[32 * 1024];
        cZStream.avail_out = 32 * 1024;
        cZStream.next_out = cChunk;
        
        // compress
        if (::inflate(&cZStream, Z_FINISH) == Z_STREAM_ERROR) return qkd::utility::memory(0);
        
        // consume data
        uint64_t nOldSize = cDecompressedMemory.size();
        uint64_t nDecompressed = 32 * 1024 - cZStream.avail_out;
        cDecompressedMemory.resize(cDecompressedMemory.size() + nDecompressed);
        memcpy(cDecompressedMemory.get() + nOldSize, cChunk, nDecompressed);
        
    } while (cZStream.avail_out == 0);
    
    // wind down zip progress
    inflateEnd(&cZStream);
    
    return cDecompressedMemory;
}



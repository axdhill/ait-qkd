/*
 * ping.cpp
 * 
 * The ping mechanism itself
 * 
 * Autor: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *
 * Copyright (C) 2014-2015 AIT Austrian Institute of Technology
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

#include <iostream>

// ait
#include <qkd/module/module.h>
#include <qkd/utility/checksum.h>
#include <qkd/utility/syslog.h>

#include "ping.h"


// ------------------------------------------------------------
// code


/**
 * do ping as alice
 *
 * @param   cModuleComm         the module communicator
 * @param   nPackageSize        size of packet to send
 * @return  true, if succefully pinged
 */
bool ping_alice(qkd::module::communicator & cModuleComm, uint64_t nPackageSize) {

    qkd::utility::memory cPayload;
    //qkd::module::message cMessage;
    
    // generate a payload
    {
        if (nPackageSize > 0) {
            cPayload.resize(nPackageSize);
            cModuleComm.mod()->random() >> cPayload;
            //cMessage.data() << cPayload;
        }
    }
    
    // start clock
    std::chrono::time_point<std::chrono::system_clock> cStart = std::chrono::system_clock::now();
    
    // send to bob
    try {
        cModuleComm << cPayload;
    }
    catch (std::runtime_error const & cRuntimeError) {
        
        std::cout << "failed to send payload to \"" << cModuleComm.mod()->url_peer().toStdString() << "\"" << std::endl;
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to send message: " << cRuntimeError.what();
        
        // rest some time and retry later on ...
        cModuleComm.mod()->rest();
        
        return false;
    }
    
    // output info to user
    qkd::utility::checksum cChecksumAlgorithm;
    qkd::utility::memory cChecksumDigest;
    
    cChecksumAlgorithm = qkd::utility::checksum_algorithm::create("crc32");
    cChecksumAlgorithm << cPayload;
    cChecksumAlgorithm >> cChecksumDigest;
    std::cout << "sent " << cPayload.size() << " bytes to peer (crc32: " << cChecksumDigest.as_hex() << ")" << std::endl;
    
    // check for termination state
    if (cModuleComm.mod()->is_dying_state()) return false;
    
    // read from bob
    try {
        cPayload.resize(0);
        if (!(cModuleComm >> cPayload)) return false;
        // if (!cModuleComm.recv(cMessage)) return false;
    }
    catch (std::runtime_error const & cRuntimeError) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to receive message: " << cRuntimeError.what();
        return false;
    }
    
    // stop clock
    std::chrono::time_point<std::chrono::system_clock> cEnd = std::chrono::system_clock::now();
    std::chrono::nanoseconds cRoundtripTime = std::chrono::duration_cast<std::chrono::nanoseconds>(cEnd - cStart);

    // extract payload
    //cPayload.resize(0);
    //cMessage.data() >> cPayload;

    // output info to user
    cChecksumAlgorithm = qkd::utility::checksum_algorithm::create("crc32");
    cChecksumAlgorithm << cPayload;
    cChecksumAlgorithm >> cChecksumDigest;
    
    // nice time roundtrip value
    std::stringstream ss;
    ss.precision(4);
    ss << std::fixed << (cRoundtripTime.count() / 1000000.0);
    
    std::cout << "read " << cPayload.size() << " bytes from peer (crc32: " << cChecksumDigest.as_hex() << ") send/recv in " << ss.str() << " ms" << std::endl;
    
    return true;
}


/**
 * do ping as bob
 *
 * @param   cModuleComm         the module communicator
 * @param   nPackageSize        size of packet to send
 * @return  true, if succefully pinged
 */
bool ping_bob(qkd::module::communicator & cModuleComm, uint64_t nPackageSize) {

    qkd::utility::memory cPayload;
    //qkd::module::message cMessage;
    
    // read from alice
    try {
        if (!(cModuleComm >> cPayload)) return false;
    }
    catch (std::runtime_error const & cRuntimeError) {
        
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to receive message: " << cRuntimeError.what();
        
        // rest some time and retry later on ...
        cModuleComm.mod()->rest();
        return false;
    }
    
    // extract payload
    //cMessage.data() >> cPayload;

    // output info to user
    qkd::utility::checksum cChecksumAlgorithm;
    qkd::utility::memory cChecksumDigest;
    cChecksumAlgorithm = qkd::utility::checksum_algorithm::create("crc32");
    cChecksumAlgorithm << cPayload;
    cChecksumAlgorithm >> cChecksumDigest;
    std::cout << "read " << cPayload.size() << " bytes from peer (crc32: " << cChecksumDigest.as_hex() << ")" << std::endl;
    
    // check for termination state
    if (cModuleComm.mod()->is_dying_state()) return false;
    
    // generate a payload
    //cMessage = qkd::module::message();
    {
        if (nPackageSize > 0) {
            cPayload.resize(nPackageSize);
            cModuleComm.mod()->random() >> cPayload;
            //cMessage.data() << cPayload;
        }
    }
    
    // send to alice
    try {
        cModuleComm << cPayload;
        //cModuleComm.send(cMessage);
    }
    catch (std::runtime_error const & cRuntimeError) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to send message: " << cRuntimeError.what();
        return false;
    }
    
    // output info to user
    cChecksumAlgorithm = qkd::utility::checksum_algorithm::create("crc32");
    cChecksumAlgorithm << cPayload;
    cChecksumAlgorithm >> cChecksumDigest;
    std::cout << "sent " << cPayload.size() << " bytes to peer (crc32: " << cChecksumDigest.as_hex() << ")" << std::endl;

    return true;
}


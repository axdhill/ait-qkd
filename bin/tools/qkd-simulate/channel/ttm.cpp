/*
 * ttm.h
 * 
 * Implementation of a TTM (Time Tagging Module) imitation as used by the QKD Simulator
 * 
 * Author: Philipp Grabenweger
 *         Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *
 * Copyright (C) 2013-2015 AIT Austrian Institute of Technology
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

#include <fstream>
#include <iostream>
#include <unistd.h>

#include <arpa/inet.h>
#include <netdb.h>

// ait
#include "ttm.h"


using namespace qkd::simulate;



// ------------------------------------------------------------
// constants


/**
 * time resolution of the TTM in [s] 
 */
const double ttm::RESOLUTION = 82.3e-12;


// ------------------------------------------------------------
// code


/**
 * add an tag time
 * 
 * @param   cEvent      event holding the detect event
 */
void ttm::add_timetag(event const & cEvent) {
    
    // TODO: check format against TTM8000 Output format

    // new time tag event
    uint64_t nTimeTag = cEvent.cData.m_nDetectTime;
    nTimeTag &= 0x0FFFFFFFFFFFFFFF;

    switch (cEvent.cData.m_ePhotonState) {
        
    case photon_state::HORIZONTAL:
        nTimeTag |= 0x8000000000000000;
        break;
        
    case photon_state::VERTICAL:
        nTimeTag |= 0x4000000000000000;
        break;
        
    case photon_state::PLUS:
        nTimeTag |= 0x2000000000000000;
        break;
        
    case photon_state::MINUS:
        nTimeTag |= 0x1000000000000000;
        break;
        
    default:
        break;
    }

    // store the time tag (incl. safety net)
    if (cEvent.cData.m_bAlice) {
        m_cTimeTags.cTimeTagsAlice[m_cTimeTags.nCurrentTimeTagAlice] = nTimeTag;
        m_cTimeTags.nCurrentTimeTagAlice++;
        if (m_cTimeTags.nCurrentTimeTagAlice >= 2048) m_cTimeTags.nCurrentTimeTagAlice = 2047;
    }
    else {
        m_cTimeTags.cTimeTagsBob[m_cTimeTags.nCurrentTimeTagBob] = nTimeTag;
        m_cTimeTags.nCurrentTimeTagBob++;
        if (m_cTimeTags.nCurrentTimeTagBob >= 2048) m_cTimeTags.nCurrentTimeTagBob = 2047;
    }
}


/**
 * remove output files
 */
void ttm::delete_files() {
    
    // enfore deleting of output files
    unlink(get_filename_alice().c_str());
    unlink(get_filename_bob().c_str());
}


/**
 * flush the output
 * 
 * @param   bForce      force flushing
 */
void ttm::flush_timetags(bool bForce) {
    
    // flush all the timetags we have so far if:
    // alice and bob have more or equal to 1024 tags
    // or one has reached the upper bound
    
    // if no force is used test if we need to flush
    if (!bForce) {
        bool bFlush = (m_cTimeTags.nCurrentTimeTagAlice >= 1024) && (m_cTimeTags.nCurrentTimeTagAlice >= 1024);
        if (!bFlush) bFlush = (m_cTimeTags.nCurrentTimeTagAlice >= 2047) || (m_cTimeTags.nCurrentTimeTagAlice >= 2047);
        if (!bFlush) return;
    }
    
    if (m_eOutputMode == ttm::output_mode::OUTPUT_MODE_UDP) {
        
        // flush UDP packets
        send_udp(m_sUDPAddressAlice, (char const *)m_cTimeTags.cTimeTagsAlice, m_cTimeTags.nCurrentTimeTagAlice * sizeof(m_cTimeTags.cTimeTagsAlice[0]));
        send_udp(m_sUDPAddressBob, (char const *)m_cTimeTags.cTimeTagsBob, m_cTimeTags.nCurrentTimeTagBob * sizeof(m_cTimeTags.cTimeTagsBob[0]));
    }
    else
    if (m_eOutputMode == ttm::output_mode::OUTPUT_MODE_FILE) {
        
        // write file
        write_file(m_sFileNameAlice, (char const *)m_cTimeTags.cTimeTagsAlice, m_cTimeTags.nCurrentTimeTagAlice * sizeof(m_cTimeTags.cTimeTagsAlice[0]));
        write_file(m_sFileNameBob, (char const *)m_cTimeTags.cTimeTagsBob, m_cTimeTags.nCurrentTimeTagBob * sizeof(m_cTimeTags.cTimeTagsBob[0]));
    }
    else {
        std::cerr << "Huh! Donow how to flush TTM values for the current output mode." << std::endl;
    }
    
    m_cTimeTags.nCurrentTimeTagAlice = 0;
    m_cTimeTags.nCurrentTimeTagBob = 0;
}


/**
 * handle a channel event. 
 * 
 * @param   cEvent          the channel event to be handled
 */
void ttm::handle(event const & cEvent) {
    
    switch (cEvent.eType) {
        
    case event::type::DETECTOR_PULSE:
        
        // signal: only when output is needed
        if (get_output_mode() != OUTPUT_MODE_NONE) {
            add_timetag(cEvent);
            flush_timetags(false);
        }
        break;
    
    case event::type::INIT:
        
        // simulation initialization
        break;
    
    case event::type::STOP: 
        
        // simulation stop: only when output is needed
        if (get_output_mode() != OUTPUT_MODE_NONE) {
            flush_timetags(true);
        }
        break;
    
    default:
        break;
    
    }
}


/**
 * UDP send packet
 * 
 * @param   sAddress        HOST:PORT pair
 * @param   cData           data buffer
 * @param   nSize           bytes to send
 */
void ttm::send_udp(std::string const & sAddress, char const * cData, uint64_t nSize) {
    
    // split the address
    std::string::size_type nColon = sAddress.find_first_of(':');
    if (nColon == std::string::npos) {
        std::cerr << "can't deduce port in '" << sAddress << "' - failed to send UDP packet." << std::endl;
        return;
    }
    
    // deduce host and port
    std::string sHost = sAddress.substr(0, nColon);
    std::string sPort = sAddress.substr(nColon + 1);
    if (sHost.empty()) {
        std::cerr << "host address seems empty in '" << sAddress << "' - failed to send UDP packet." << std::endl;
        return;
    }
    
    uint16_t nPort = 0;
    try {
        nPort = std::stoi(sPort);
    }
    catch (...) {
        std::cerr << "port number seems illegal in '" << sAddress << "' - failed to send UDP packet." << std::endl;
        return;
    }
    
    // try to convert to IP (old-school POSIX way)
    struct in_addr cAddress;
    if (inet_aton(sHost.c_str(), &cAddress) == 0) {
        // failed. A host name?
        struct hostent * cHostEntry = gethostbyname(sHost.c_str());
        if (!cHostEntry) {
            std::cerr << "can't get an IPv4 address for this host '" << sHost << "' - failed to send UDP packet." << std::endl;
            return;
        }
        memcpy(&cAddress, cHostEntry->h_addr_list[0], sizeof(cAddress));
    }
    
    // prepare sending
    struct sockaddr_in cDestinationAddress;
    cDestinationAddress.sin_family = AF_INET;
    cDestinationAddress.sin_addr = cAddress;
    cDestinationAddress.sin_port = htons(nPort);
    int nSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
    // WHHOOOP! There it is!
    int nErrorCode = sendto(nSocket, cData, nSize, 0, (struct sockaddr*)&cDestinationAddress, sizeof(cDestinationAddress));
    if (nErrorCode == -1) {
        std::cerr << "failed to send UDP packet to '" << sAddress << "' - errno is: " << errno << " - '" << strerror(errno) << "'" << std::endl;
        close(nSocket);
        return;
    }
    
    // free resources
    close(nSocket);
}


/**
 * write packet to file
 * 
 * @param   sFile           path to file
 * @param   cData           data buffer
 * @param   nSize           bytes to send
 */
void ttm::write_file(std::string const & sFile, char const * cData, uint64_t nSize) {

    // append to target file
    std::ofstream cFile(sFile, std::ios_base::out | std::ios_base::app | std::ios_base::binary);
    if (!cFile.good()) {
        std::cerr << "failed to open file '" << sFile << "' for writing" << std::endl;
        return;
    }
    
    // flush to file
    cFile.write(cData, nSize);
    cFile.flush();
}


/**
 * write out all parameters of this event handler and its child event handlers
 * 
 * @param   cStream     the stream to write to
 */
void ttm::write_parameters(std::ofstream & cStream) {
    cStream << "NAME: " << get_name() << std::endl;
    cStream << std::endl;
}

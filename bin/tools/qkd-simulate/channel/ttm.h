/*
 * ttm.h
 * 
 * Declaration of a TTM (Time Tagging Module) imitation as used by the QKD Simulator
 * 
 * Author: Philipp Grabenweger
 *         Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *
 * Copyright (C) 2013-2016 AIT Austrian Institute of Technology
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


#ifndef __QKD_QKD_SIMULATE_TTM_H_
#define __QKD_QKD_SIMULATE_TTM_H_


// ------------------------------------------------------------
// include

// ait
#include "channel_event_handler.h"


// ------------------------------------------------------------
// decl


namespace qkd {
    
namespace simulate {

    
/**
 * imitation of a TTM (Time Tagging Module) as used by the QKD Simulator
 */
class ttm : public channel_event_handler {
    
    
public:
    
    
    /**
     * output modes of the TTM
     */
    enum output_mode : uint8_t {
        
        OUTPUT_MODE_NONE,           /**< no output */
        OUTPUT_MODE_UDP,            /**< send via UDP packets */
        OUTPUT_MODE_FILE            /**< send via file */
    };
    
    
    /**
     * time resolution in [s] 
     */
    static const double RESOLUTION; 
    
    
    /**
     * ctor
     */
    ttm() : channel_event_handler(), m_eOutputMode(OUTPUT_MODE_NONE) {};
    
    
    /**
     * remove output files
     */
    void delete_files();
    
    
    /**
     * get output file name for alice
     * 
     * @return  the output file name for alice
     */
    std::string const & get_filename_alice() const { return m_sFileNameAlice; };
    
    
    /**
     * get output file name for bob
     * 
     * @return  the output file name for bob
     */
    std::string const & get_filename_bob() const { return m_sFileNameBob; };
    
    
    /**
     * get output UDP address for alice
     * 
     * @return  the output UDP address for alice
     */
    std::string const & get_udp_address_alice() const { return m_sUDPAddressAlice; };
    
    
    /**
     * get output UDP address for bob
     * 
     * @return  the output UDP address for bob
     */
    std::string const & get_udp_address_bob() const { return m_sUDPAddressBob; };
    
    
    /**
     * get the current output mode
     * 
     * @return  the current output mode
     */
    output_mode get_output_mode() const { return m_eOutputMode; };
    
    
    /**
     * handle a channel event. 
     * 
     * @param   cEvent      the channel event to be handled
     */
    void handle(event const & cEvent);
    
    
    /**
     * set output file name for alice
     * 
     * @param   sFilename       the output file name for alice
     */
    void set_filename_alice(std::string sFilename) { m_sFileNameAlice = sFilename; };
    
    
    /**
     * set output file name for bob
     * 
     * @param   sFilename       the output file name for bob
     */
    void set_filename_bob(std::string sFilename) { m_sFileNameBob = sFilename; };
    
    
    /**
     * set the current output mode
     * 
     * @param   eOutputMode     the new output mode
     */
    void set_output_mode(output_mode eOutputMode) { m_eOutputMode = eOutputMode; };
    
    
    /**
     * set output UDP address for alice
     * 
     * @param   sUDPAddress       the output UDP address for alice
     */
    void set_udp_address_alice(std::string sUDPAddress) { m_sUDPAddressAlice = sUDPAddress; };
    
    
    /**
     * set output UDP address for bob
     * 
     * @param   sUDPAddress       the output UDP address for bob
     */
    void set_udp_address_bob(std::string sUDPAddress) { m_sUDPAddressBob = sUDPAddress; };
    
    
    /**
     * write out all parameters of this event handler and its child event handlers
     * 
     * @param   cStream     the stream to write to
     */
    void write_parameters(std::ofstream & cStream);
    
    
private:
    
    
    /**
     * add an tag time
     * 
     * @param   cEvent      event holding the detect event
     */
    void add_timetag(event const & cEvent);
    
    
    /**
     * flush the output
     * 
     * @param   bForce      force flushing
     */
    void flush_timetags(bool bForce);
    
    
    /**
     * UDP send packet
     * 
     * @param   sAddress        HOST:PORT pair
     * @param   cData           data buffer
     * @param   nSize           bytes to send
     */
    void send_udp(std::string const & sAddress, char const * cData, uint64_t nSize);
    
    
    /**
     * write packet to file
     * 
     * @param   sFile           path to file
     * @param   cData           data buffer
     * @param   nSize           bytes to send
     */
    void write_file(std::string const & sFile, char const * cData, uint64_t nSize);
    
    
    /**
     * time tags management
     */
    struct {
        
        uint64_t cTimeTagsAlice[2048];          /**< current time tags of alice */
        uint64_t cTimeTagsBob[2048];            /**< current time tags of bob */
        int nCurrentTimeTagAlice = 0;           /**< next time tag index to fill alice */
        int nCurrentTimeTagBob = 0;             /**< next time tag index to fill bob */
        
    } m_cTimeTags;
    
    
    /**
     * output file name for alice
     */
    std::string m_sFileNameAlice; 
    
    
    /**
     * output file name for bob
     */
    std::string m_sFileNameBob; 
    
    
    /**
     * current output mode
     */
    output_mode m_eOutputMode;
    
    
    /**
     * output UDP address for alice
     */
    std::string m_sUDPAddressAlice; 
    
    
    /**
     * output UDP address for bob
     */
    std::string m_sUDPAddressBob; 
};

}
}

#endif

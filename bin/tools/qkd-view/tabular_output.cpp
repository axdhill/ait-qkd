/*
 * tabular_output.cpp
 *
 * This is shows the current QKD system snapshot
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
// incl

#include <ostream>
#include <sstream>

#include <boost/format.hpp>

#include "tabular_output.h"
#include "tabular_writer.h"


// ------------------------------------------------------------
// code


/**
 * ctor
 * 
 * @param  cProgramOptions  formatting-related runtime options
 */
tabular_output::tabular_output(configuration_options const & cProgramOptions) {
    bPrintHeader = ! cProgramOptions.bOmitHeader;
    bPrintModuleIO = cProgramOptions.bOnlyModuleIO;
    bPrintShort = cProgramOptions.bOutputShort;
}


/**
 * Writes the basic investigation details to the specified stream.
 * 
 * @param  cOut            the stream to write to.
 * @param  cInvestigation  the particular investigation instance to format.
 */
void tabular_output::dump_investigation_details(std::ostream & cOut, qkd::utility::investigation const & cInvestigation) const {
    
    std::time_t cTimestamp = std::chrono::system_clock::to_time_t(cInvestigation.timestamp());
    cOut << "QKD system investigation results from " << std::ctime(&cTimestamp);
    cOut << "QKD system investigation took " <<
    std::chrono::duration_cast<std::chrono::milliseconds>(cInvestigation.duration()).count() << "ms" << std::endl;
}


/**
 * Writes link information to the specified stream.
 * 
 * @param  cOut            the stream to write to.
 * @param  cLinkMap        the particular link data to format.
 */
void tabular_output::dump_links(std::ostream & cOut, std::map<std::string, qkd::utility::properties> const & cLinkMap) {
    
    static std::list<std::string> const cFields = { 
            "id", 
            "node", 
            "dbus", 
            "state", 
            "connected", 
            "db_opened", 
            "uri_local", 
            "uri_peer", 
            "master", 
            "slave", 
            "mq", 
            "nic" };
            
    tabular_writer(cOut, cLinkMap, cFields, true, "link: ");
}


/**
 * Writes module information to the specified stream.
 * 
 * @param  cOut            the stream to write to.
 * @param  cModuleMap      the particular module data to format.
 */
void tabular_output::dump_modules(std::ostream & cOut, std::map<std::string, qkd::utility::properties> const & cModuleMap) {
    
    if (bPrintShort) {
        dump_modules_short(cOut, cModuleMap);
    }
    else if (bPrintModuleIO) {
        dump_modules_io(cOut, cModuleMap);
    }
    else {
        dump_modules_full(cOut, cModuleMap);
    }
}


/**
 * Writes full module information to the specified stream.
 * 
 * @param  cOut            the stream to write to.
 * @param  cModuleMap      the particular module data to format.
 */
void tabular_output::dump_modules_full(std::ostream & cOut, const std::map<std::string, qkd::utility::properties> & cModuleMap) {
    
    static std::list<std::string> const cFields = { 
            "id", 
            "dbus", 
            "pipeline", 
            "process_id", 
            "type", 
            "type_name", 
            "start_time", 
            "state", 
            "state_name", 
            "role", 
            "role_name", 
            "url_pipe_in", 
            "url_pipe_out", 
            "url_listen", 
            "url_peer", 
            "idle", 
            "random_url", 
            "keys_incoming", 
            "keys_outgoing", 
            "key_bits_incoming", 
            "key_bits_outgoing", 
            "disclosed_bits_incoming", 
            "disclosed_bits_outgoing", 
            "error_bits_incoming", 
            "error_bits_outgoing", 
            "debug", 
            "description", 
            "organisation", 
            "process_image"};
        
    tabular_writer(cOut, cModuleMap, cFields, true, "module: ");
}


/**
 * Writes io module information to the specified stream.
 * 
 * @param  cOut            the stream to write to.
 * @param  cModuleMap      the particular module data to format.
 */
void tabular_output::dump_modules_io(std::ostream & cOut, const std::map<std::string, qkd::utility::properties> & cModuleMap) {

    static std::list<std::string> const cFields = { 
            "id", 
            "url_pipe_in", 
            "url_pipe_out", 
            "url_listen", 
            "url_peer"};
        
    tabular_writer(cOut, cModuleMap, cFields, true, "module: ");
}


/**
 * Writes short module information to the specified stream.
 * 
 * @param  cOut            the stream to write to.
 * @param  cModuleMap      the particular module data to format.
 */
void tabular_output::dump_modules_short(std::ostream & cOut, const std::map<std::string, qkd::utility::properties> & cModuleMap) {
    
    static std::list<std::string> const cFields = { 
            "id", 
            "dbus", 
            "keys_incoming", 
            "keys_outgoing", 
            "key_bits_incoming", 
            "key_bits_outgoing", 
            "pipeline", 
            "process_id", 
            "state_name", 
            "role_name", 
            "url_pipe_in", 
            "url_pipe_out", 
            "url_listen", 
            "url_peer", 
            "idle" };
        
    tabular_writer(cOut, cModuleMap, cFields, true, "module: ");
}


/**
 * Writes node information to the specified stream.
 * 
 * @param  cOut            the stream to write to.
 * @param  cNodeMap        the particular node data to format.
 */
void tabular_output::dump_nodes(std::ostream & cOut, std::map<std::string, qkd::utility::properties> const & cNodeMap) {

    static std::list<std::string> const cFields = { 
            "id", 
            "dbus", 
            "start_time", 
            "process_id", 
            "process_image", 
            "config_file", 
            "random_url", 
            "debug" };
        
    tabular_writer(cOut, cNodeMap, cFields, true, "node: ");
}


/**
 * Writes investigation results to the specified output stream.
 * 
 * @param  cOut            the stream to write to.
 * @param  cInvestigation  the particular investigation instance to format.
 */
void tabular_output::write(std::ostream & cOut, qkd::utility::investigation const & cInvestigation) {
    
    if (bPrintHeader) {
        dump_investigation_details(cOut, cInvestigation);
    }
    
    if (!bPrintModuleIO) {
        if (!cInvestigation.nodes().empty()) {
            dump_nodes(cOut, cInvestigation.nodes());
        }
        if (!cInvestigation.links().empty()) {
            dump_links(cOut, cInvestigation.links());
        }
    }
    
    if (!cInvestigation.modules().empty()) {
        dump_modules(cOut, cInvestigation.modules());
    }
}

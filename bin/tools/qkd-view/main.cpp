/*
 * main.cpp
 * 
 * This is shows the current QKD system snapshot
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

#include <iostream>

#include <boost/format.hpp>
#include <boost/program_options.hpp>

// Qt
#include <QtCore/QCoreApplication>

// ait
#include <qkd/utility/debug.h>
#include <qkd/utility/investigation.h>


// ------------------------------------------------------------
// types


/**
 * columnwidth map
 */
typedef std::map<std::string, std::string::size_type> column_width;


// ------------------------------------------------------------
// fwd

void dump_investigation_details(qkd::utility::investigation const & cInvestigation);
void dump_links(std::map<std::string, qkd::utility::properties> const & cLinks);
void dump_modules(std::map<std::string, qkd::utility::properties> const & cModules);
void dump_nodes(std::map<std::string, qkd::utility::properties> const & cNodes);
void set_column_width(column_width & cColumnWidth, qkd::utility::properties const & cProperties);


// ------------------------------------------------------------
// vars


/**
 * the width of the columns for the output
 */
struct {
    
    column_width cLink;         /**< width of link columns */
    column_width cModule;       /**< width of module columns */
    column_width cNode;         /**< width of node columns */
    
} g_cColumnWidth;


/**
 * flag for printing headers
 */
static bool g_bPrintHeader = true;


/**
 * flag for printing only IO addresses
 */
static bool g_bPrintModuleIO = false;


/**
 * flag for printing short
 */
static bool g_bPrintShort = false;



// ------------------------------------------------------------
// code


/**
 * show investigation details
 * 
 * @param   cInvestigation      the investigation object
 */
void dump_investigation_details(qkd::utility::investigation const & cInvestigation) {
    
    // only show investigation if we do have a header
    if (!g_bPrintHeader) return;
    
    std::time_t cTimestamp = std::chrono::system_clock::to_time_t(cInvestigation.timestamp());
    std::cout << "QKD system investigation results from " << std::ctime(&cTimestamp);
    std::cout << "QKD system investigation took " << std::chrono::duration_cast<std::chrono::milliseconds>(cInvestigation.duration()).count() << "ms" << std::endl;
}


/**
 * show found links to user
 * 
 * @param   cLinks      the links found
 */
void dump_links(std::map<std::string, qkd::utility::properties> const & cLinks) {
    
    // something to show at all?
    if (cLinks.empty()) return;
    
    // only interested in module output
    if (g_bPrintModuleIO) return;
    
    // set the column width for links
    g_cColumnWidth.cLink.clear();
    for (auto const & cLink : cLinks) {
        set_column_width(g_cColumnWidth.cLink, cLink.second);
    }
    
    // construct the format string: "link:" - id - node - dbus - state - connected - db_opened - uri_local - uri_peer - master - slave - mq - nic
    std::stringstream ss;
    ss << "%-5s    " 
        << "%-" << g_cColumnWidth.cLink["id"] << "s    "
        << "%-" << g_cColumnWidth.cLink["node"] << "s    "
        << "%-" << g_cColumnWidth.cLink["dbus"] << "s    "
        << "%-" << g_cColumnWidth.cLink["state"] << "s    "
        << "%-" << g_cColumnWidth.cLink["connected"] << "s    "
        << "%-" << g_cColumnWidth.cLink["db_opened"] << "s    "
        << "%-" << g_cColumnWidth.cLink["uri_local"] << "s    "
        << "%-" << g_cColumnWidth.cLink["uri_peer"] << "s    "
        << "%-" << g_cColumnWidth.cLink["master"] << "s    "
        << "%-" << g_cColumnWidth.cLink["slave"] << "s    "
        << "%-" << g_cColumnWidth.cLink["mq"] << "s    "
        << "%-" << g_cColumnWidth.cLink["nic"] << "s";
        
    // the column format
    boost::format cFormat(ss.str());

    // print the heading
    if (g_bPrintHeader) {
        
        boost::format cHeading(cFormat);
        cHeading 
            % "type" 
            % "id" 
            % "node" 
            % "dbus" 
            % "state" 
            % "connected" 
            % "db_opened" 
            % "uri_local" 
            % "uri_peer" 
            % "master" 
            % "slave" 
            % "mq" 
            % "nic";
            
        std::cout << cHeading.str() << std::endl;
    }
        
    // go over the found links
    for (auto const & cLink : cLinks) {
    
        boost::format cLinkFormat(cFormat);
        
        cLinkFormat % "link:";
        cLinkFormat % cLink.second.at("id");
        cLinkFormat % cLink.second.at("node");
        cLinkFormat % cLink.second.at("dbus");
        cLinkFormat % cLink.second.at("state");
        cLinkFormat % cLink.second.at("connected");
        cLinkFormat % cLink.second.at("db_opened");
        cLinkFormat % cLink.second.at("uri_local");
        cLinkFormat % cLink.second.at("uri_peer");
        cLinkFormat % cLink.second.at("master");
        cLinkFormat % cLink.second.at("slave");
        cLinkFormat % cLink.second.at("mq");
        cLinkFormat % cLink.second.at("nic");

        std::cout << cLinkFormat.str() << std::endl;
    }

    // add an artificial empty line for nice viewing if headers are on
    if (g_bPrintHeader) std::cout << std::endl;
}


/**
 * show found modules to user
 * 
 * @param   cModules        the found modules
 */
void dump_modules(std::map<std::string, qkd::utility::properties> const & cModules) {
    
    // something to show at all?
    if (cModules.empty()) return;
    
    // set the column width for modules
    g_cColumnWidth.cModule.clear();
    for (auto const & cModule : cModules) {
        set_column_width(g_cColumnWidth.cModule, cModule.second);
    }
    
    // construct the format string: "module:" id - dbus - pipeline - process_id - type - type_name - start_time - state - state_name - role - role_name - url_pipe_in - url_pipe_out - url_listen - url_peer - timeout - random_url - keys_incoming - keys_outgoing - key_bits_incoming - key_bits_outgoing - keys_per_second_in - keys_per_second_out - bits_per_second_in - bits_per_second_out - error_bits_incoming - error_bits_outgoing - disclosed_bits_incoming - disclosed_bits_outgoing - debug - description - organisation - process_image
    std::stringstream ss;
    
    if (g_bPrintShort) {
        
        // short version
        ss << "%-7s    " 
            << "%-" << g_cColumnWidth.cModule["id"] << "s    "
            << "%-" << g_cColumnWidth.cModule["dbus"] << "s    "
            << "%-" << g_cColumnWidth.cModule["keys_incoming"] << "s    "
            << "%-" << g_cColumnWidth.cModule["keys_outgoing"] << "s    "
            << "%-" << g_cColumnWidth.cModule["key_bits_incoming"] << "s    "
            << "%-" << g_cColumnWidth.cModule["key_bits_outgoing"] << "s    "
            << "%-" << g_cColumnWidth.cModule["pipeline"] << "s    "
            << "%-" << g_cColumnWidth.cModule["process_id"] << "s    "
            << "%-" << g_cColumnWidth.cModule["state_name"] << "s    "
            << "%-" << g_cColumnWidth.cModule["role_name"] << "s    "
            << "%-" << g_cColumnWidth.cModule["url_pipe_in"] << "s    "
            << "%-" << g_cColumnWidth.cModule["url_pipe_out"] << "s    "
            << "%-" << g_cColumnWidth.cModule["url_listen"] << "s    "
            << "%-" << g_cColumnWidth.cModule["url_peer"] << "s    "
            << "%-" << g_cColumnWidth.cModule["stalled"] << "s";
    }
    else 
    if (g_bPrintModuleIO) {
        
        // short version
        ss << "%-" << g_cColumnWidth.cModule["id"] << "s    "
            << "%-" << g_cColumnWidth.cModule["url_pipe_in"] << "s    "
            << "%-" << g_cColumnWidth.cModule["url_pipe_out"] << "s    "
            << "%-" << g_cColumnWidth.cModule["url_listen"] << "s    "
            << "%-" << g_cColumnWidth.cModule["url_peer"] << "s";
    }
    else {
    
        // full version
        ss << "%-7s    " 
            << "%-" << g_cColumnWidth.cModule["id"] << "s    "
            << "%-" << g_cColumnWidth.cModule["dbus"] << "s    "
            << "%-" << g_cColumnWidth.cModule["pipeline"] << "s    "
            << "%-" << g_cColumnWidth.cModule["process_id"] << "s    "
            << "%-" << g_cColumnWidth.cModule["type"] << "s    "
            << "%-" << g_cColumnWidth.cModule["type_name"] << "s    "
            << "%-" << g_cColumnWidth.cModule["start_time"] << "s    "
            << "%-" << g_cColumnWidth.cModule["state"] << "s    "
            << "%-" << g_cColumnWidth.cModule["state_name"] << "s    "
            << "%-" << g_cColumnWidth.cModule["role"] << "s    "
            << "%-" << g_cColumnWidth.cModule["role_name"] << "s    "
            << "%-" << g_cColumnWidth.cModule["url_pipe_in"] << "s    "
            << "%-" << g_cColumnWidth.cModule["url_pipe_out"] << "s    "
            << "%-" << g_cColumnWidth.cModule["url_listen"] << "s    "
            << "%-" << g_cColumnWidth.cModule["url_peer"] << "s    "
            << "%-" << g_cColumnWidth.cModule["stalled"] << "s    "
            << "%-" << g_cColumnWidth.cModule["timeout_network"] << "s    "
            << "%-" << g_cColumnWidth.cModule["timeout_pipe"] << "s    "
            << "%-" << g_cColumnWidth.cModule["random_url"] << "s    "
            << "%-" << g_cColumnWidth.cModule["keys_incoming"] << "s    "
            << "%-" << g_cColumnWidth.cModule["keys_outgoing"] << "s    "
            << "%-" << g_cColumnWidth.cModule["key_bits_incoming"] << "s    "
            << "%-" << g_cColumnWidth.cModule["key_bits_outgoing"] << "s    "
            << "%-" << g_cColumnWidth.cModule["disclosed_bits_incoming"] << "s    "
            << "%-" << g_cColumnWidth.cModule["disclosed_bits_outgoing"] << "s    "
            << "%-" << g_cColumnWidth.cModule["debug"] << "s    "
            << "%-" << g_cColumnWidth.cModule["description"] << "s    "
            << "%-" << g_cColumnWidth.cModule["organisation"] << "s    "
            << "%-" << g_cColumnWidth.cModule["process_image"] << "s";
    }
    
    // the column format
    boost::format cFormat(ss.str());
    
    // print the heading
    if (g_bPrintHeader) {
        
        boost::format cHeading(cFormat);
        
        if (g_bPrintShort) {
            
            // short version
            cHeading 
                % "type" 
                % "id" 
                % "dbus" 
                % "keys_incoming"
                % "keys_outgoing"
                % "key_bits_incoming"
                % "key_bits_outgoing"
                % "pipeline" 
                % "process_id" 
                % "state_name" 
                % "role_name" 
                % "url_pipe_in" 
                % "url_pipe_out" 
                % "url_listen" 
                % "url_peer"
                % "stalled";
        }
        else
        if (g_bPrintModuleIO) {
            
            // module IO
            cHeading 
                % "id" 
                % "url_pipe_in" 
                % "url_pipe_out" 
                % "url_listen" 
                % "url_peer";
        }
        else {
            
            // full version
            cHeading 
                % "type" 
                % "id" 
                % "dbus" 
                % "pipeline" 
                % "process_id" 
                % "type" 
                % "type_name" 
                % "start_time" 
                % "state" 
                % "state_name" 
                % "role" 
                % "role_name" 
                % "url_pipe_in" 
                % "url_pipe_out" 
                % "url_listen" 
                % "url_peer" 
                % "stalled"
                % "timeout_network" 
                % "timeout_pipe" 
                % "random_url" 
                % "keys_incoming" 
                % "keys_outgoing" 
                % "key_bits_incoming" 
                % "key_bits_outgoing" 
                % "disclosed_bits_incoming" 
                % "disclosed_bits_outgoing" 
                % "debug" 
                % "description" 
                % "organisation" 
                % "process_image";            
        }
        
        std::cout << cHeading.str() << std::endl;
    }
        
    // go over the found modules
    for (auto const & cModule : cModules) {
        
        boost::format cModuleFormat(cFormat);
        
        if (g_bPrintShort) {
            
            // short version
            cModuleFormat % "module:";
            cModuleFormat % cModule.second.at("id");
            cModuleFormat % cModule.second.at("dbus");
            cModuleFormat % cModule.second.at("keys_incoming");
            cModuleFormat % cModule.second.at("keys_outgoing");
            cModuleFormat % cModule.second.at("key_bits_incoming");
            cModuleFormat % cModule.second.at("key_bits_outgoing");
            cModuleFormat % cModule.second.at("pipeline");
            cModuleFormat % cModule.second.at("process_id");
            cModuleFormat % cModule.second.at("state_name");
            cModuleFormat % cModule.second.at("role_name");
            cModuleFormat % cModule.second.at("url_pipe_in");
            cModuleFormat % cModule.second.at("url_pipe_out");
            cModuleFormat % cModule.second.at("url_listen");
            cModuleFormat % cModule.second.at("url_peer");
            cModuleFormat % cModule.second.at("stalled");
        }
        else 
        if (g_bPrintModuleIO) {
            
            // short version
            cModuleFormat % cModule.second.at("id");
            cModuleFormat % cModule.second.at("url_pipe_in");
            cModuleFormat % cModule.second.at("url_pipe_out");
            cModuleFormat % cModule.second.at("url_listen");
            cModuleFormat % cModule.second.at("url_peer");
        }
        else {
            
            // full version
            cModuleFormat % "module:";
            cModuleFormat % cModule.second.at("id");
            cModuleFormat % cModule.second.at("dbus");
            cModuleFormat % cModule.second.at("pipeline");
            cModuleFormat % cModule.second.at("process_id");
            cModuleFormat % cModule.second.at("type");
            cModuleFormat % cModule.second.at("type_name");
            cModuleFormat % cModule.second.at("start_time");
            cModuleFormat % cModule.second.at("state");
            cModuleFormat % cModule.second.at("state_name");
            cModuleFormat % cModule.second.at("role");
            cModuleFormat % cModule.second.at("role_name");
            cModuleFormat % cModule.second.at("url_pipe_in");
            cModuleFormat % cModule.second.at("url_pipe_out");
            cModuleFormat % cModule.second.at("url_listen");
            cModuleFormat % cModule.second.at("url_peer");
            cModuleFormat % cModule.second.at("stalled");
            cModuleFormat % cModule.second.at("timeout_network");
            cModuleFormat % cModule.second.at("timeout_pipe");
            cModuleFormat % cModule.second.at("random_url");
            cModuleFormat % cModule.second.at("keys_incoming");
            cModuleFormat % cModule.second.at("keys_outgoing");
            cModuleFormat % cModule.second.at("key_bits_incoming");
            cModuleFormat % cModule.second.at("key_bits_outgoing");
            cModuleFormat % cModule.second.at("disclosed_bits_incoming");
            cModuleFormat % cModule.second.at("disclosed_bits_outgoing");
            cModuleFormat % cModule.second.at("debug");
            cModuleFormat % cModule.second.at("description");
            cModuleFormat % cModule.second.at("organisation");
            cModuleFormat % cModule.second.at("process_image");
        }
        
        std::cout << cModuleFormat.str() << std::endl;
    }

    // add an artificial empty line for nice viewing if headers are on
    if (g_bPrintHeader) std::cout << std::endl;
}


/**
 * show found nodes to user
 * 
 * @param   cNodes      the found nodes
 */
void dump_nodes(std::map<std::string, qkd::utility::properties> const & cNodes) {
    
    // something to show at all?
    if (cNodes.empty()) return;
    
    // only interested in module output
    if (g_bPrintModuleIO) return;
    
    // set the column width for nodes
    g_cColumnWidth.cNode.clear();
    for (auto const & cNode : cNodes) {
        set_column_width(g_cColumnWidth.cNode, cNode.second);
    }
    
    // construct the format string: "node:" - id - dbus - start_time - process_id - process_image - config_file - random_url - debug
    std::stringstream ss;
    ss << "%-5s    " 
        << "%-" << g_cColumnWidth.cNode["id"] << "s    "
        << "%-" << g_cColumnWidth.cNode["dbus"] << "s    "
        << "%-" << g_cColumnWidth.cNode["start_time"] << "s    "
        << "%-" << g_cColumnWidth.cNode["process_id"] << "s    "
        << "%-" << g_cColumnWidth.cNode["process_image"] << "s    "
        << "%-" << g_cColumnWidth.cNode["config_file"] << "s    "
        << "%-" << g_cColumnWidth.cNode["random_url"] << "s    "
        << "%-" << g_cColumnWidth.cNode["debug"] << "s";
        
    // the column format
    boost::format cFormat(ss.str());
    
    // print the heading
    if (g_bPrintHeader) {
        
        boost::format cHeading(cFormat);
        cHeading 
            % "type" 
            % "id" 
            % "dbus" 
            % "start_time" 
            % "process_id" 
            % "process_image" 
            % "config_file" 
            % "random_url" 
            % "debug";
            
        std::cout << cHeading.str() << std::endl;
    }
        
    // go over the found nodes
    for (auto const & cNode : cNodes) {
        
        boost::format cNodeFormat(cFormat);
        
        cNodeFormat % "node:";
        cNodeFormat % cNode.second.at("id");
        cNodeFormat % cNode.second.at("dbus");
        cNodeFormat % cNode.second.at("start_time");
        cNodeFormat % cNode.second.at("process_id");
        cNodeFormat % cNode.second.at("process_image");
        cNodeFormat % cNode.second.at("config_file");
        cNodeFormat % cNode.second.at("random_url");
        cNodeFormat % cNode.second.at("debug");
        
        std::cout << cNodeFormat.str() << std::endl;
    }

    // add an artificial empty line for nice viewing if headers are on
    if (g_bPrintHeader) std::cout << std::endl;
}


/**
 * start
 * 
 * @param   argc        as usual
 * @param   argv        as usual
 * @return  as usual
 */
int main(int argc, char ** argv) {
    
    // get up Qt
    QCoreApplication cApp(argc, argv);
    
    // create the command line header
    std::string sApplication = std::string("qkd-view - AIT QKD System View V") + VERSION;
    std::string sDescription = std::string("\nThis shows the current QKD system.\nThe values of the found nodes, links and modules are sperated by tabs.\n\nCopyright 2012-2015 AIT Austrian Institute of Technology GmbH");
    std::string sSynopsis = std::string("Usage: ") + argv[0] + " [OPTIONS]";
    
    // define program options
    boost::program_options::options_description cOptions(sApplication + "\n" + sDescription + "\n\n    " + sSynopsis + "\n\nAllowed Options");
    cOptions.add_options()("debug,d", "enable debug output on stderr");
    cOptions.add_options()("help,h", "this page");
    cOptions.add_options()("module-io,i", "only show modules I/O addresses");
    cOptions.add_options()("omit-header,o", "dont print headers on each table");
    cOptions.add_options()("short,s", "output is shorten to more important data");
    cOptions.add_options()("version,v", "print version string");
    
    // construct overall options
    boost::program_options::options_description cCmdLineOptions("Command Line");
    cCmdLineOptions.add(cOptions);

    // option variable map
    boost::program_options::variables_map cVariableMap;
    
    try {
        // parse action
        boost::program_options::command_line_parser cParser(argc, argv);
        boost::program_options::store(cParser.options(cCmdLineOptions).run(), cVariableMap);
        boost::program_options::notify(cVariableMap);        
    }
    catch (std::exception & cException) {
        std::cerr << "error parsing command line: " << cException.what() << "\ntype '--help' for help" << std::endl;        
        return 1;
    }
    
    // check for "help" set
    if (cVariableMap.count("help")) {
        std::cout << cOptions << std::endl;
        return 0;
    }
    
    // check for "version" set
    if (cVariableMap.count("version")) {
        std::cout << sApplication << std::endl;
        return 0;
    }

    // check for "debug" set
    if (cVariableMap.count("debug")) qkd::utility::debug::enabled() = true;
    
    // check for "omit-header" set
    if (cVariableMap.count("omit-header")) g_bPrintHeader = false;
    
    // check for "module-io" set
    if (cVariableMap.count("module-io")) g_bPrintModuleIO = true;
    
    // check for "short" set
    if (cVariableMap.count("short")) g_bPrintShort = true;
    
    // investigate the system
    qkd::utility::investigation cInvestigation = qkd::utility::investigation::investigate();
    
    // show user
    dump_investigation_details(cInvestigation);
    dump_nodes(cInvestigation.nodes());
    dump_links(cInvestigation.links());
    dump_modules(cInvestigation.modules());
    
    return 0;
}


/**
 * set the maximum column width for a given set of properties
 * 
 * @param   cColumnWidth            the column width map
 * @param   cProperties             the properties to inspect
 */
void set_column_width(column_width & cColumnWidth, qkd::utility::properties const & cProperties) {
    
    // set the column names as initial width
    for (auto const & cColumn : cProperties) {
        
        // if this is the first entry with this id, take the length of the name as initial width
        if (cColumnWidth.find(cColumn.first) == cColumnWidth.end()) {
            cColumnWidth[cColumn.first] = cColumn.first.length();
        }
        
        // set the new column to the maximum    
        cColumnWidth[cColumn.first] = std::max(cColumnWidth[cColumn.first], cColumn.second.length());
    }    
}

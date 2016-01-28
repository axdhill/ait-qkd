/*
 * main.cpp
 * 
 * This is the PING QKD Module.
 * 
 * This QKD Module tests the remote module to module connection
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

#include <iostream>

#include <boost/program_options.hpp>

// ait
#include "qkd-ping.h"


// ------------------------------------------------------------
// code


/**
 * startup
 * 
 * @param   argc        as usual ...
 * @param   argv        as usual ...
 * @return  as usual ...
 */
int main(int argc, char ** argv) {
    
    QCoreApplication cApp(argc, argv);
    
    std::string sApplication = std::string("qkd-ping - AIT QKD Module 'ping' V") + VERSION;
    std::string sDescription = std::string("\nThis is an AIT QKD module.\n\nIt tests the remote module to module connection.\n\nCopyright 2012-2016 AIT Austrian Institute of Technology GmbH");
    std::string sSynopsis = std::string("Usage: ") + argv[0] + " [OPTIONS]";
    
    boost::program_options::options_description cOptions(sApplication + "\n" + sDescription + "\n\n\t" + sSynopsis + "\n\nAllowed Options");
    cOptions.add_options()("bob,b", "set this as bob's instance, the responder");
    cOptions.add_options()("connect,c", boost::program_options::value<std::string>()->default_value("tcp://127.0.0.1:6789"), "connection string to connect to or listen on");
    cOptions.add_options()("count,t", boost::program_options::value<uint64_t>()->default_value(0), "number of roundtrips (0 = infinite)");
    cOptions.add_options()("debug-message-flow", "enable message debug dump output on stderr");
    cOptions.add_options()("debug,d", "enable debug output on stderr");
    cOptions.add_options()("help,h", "this page");
    cOptions.add_options()("payload,p", boost::program_options::value<uint64_t>()->default_value(1000), "number of bytes to send as payload");
    cOptions.add_options()("sleep,s", boost::program_options::value<uint64_t>()->default_value(1000), "number of milliseconds to sleep between calls");
    cOptions.add_options()("run,r", "run immediately");
    cOptions.add_options()("version,v", "print version string");
    
    boost::program_options::options_description cCmdLineOptions("Command Line");
    cCmdLineOptions.add(cOptions);

    boost::program_options::variables_map cVariableMap;
    
    try {
        boost::program_options::command_line_parser cParser(argc, argv);
        boost::program_options::store(cParser.options(cCmdLineOptions).run(), cVariableMap);
        boost::program_options::notify(cVariableMap);        
    }
    catch (std::exception & cException) {
        std::cerr << "error parsing command line: " << cException.what() << "\ntype '--help' for help" << std::endl;        
        return 1;
    }
    
    if (cVariableMap.count("help")) {
        std::cout << cOptions << std::endl;
        return 0;
    }
    
    if (cVariableMap.count("version")) {
        std::cout << sApplication << std::endl;
        return 0;
    }
    
    if (cVariableMap.count("debug")) qkd::utility::debug::enabled() = true;
    
    qkd_ping cQKDPing;
    cQKDPing.set_debug_message_flow(cVariableMap.count("debug-message-flow") > 0);
    cQKDPing.set_payload_size(cVariableMap["payload"].as<uint64_t>());
    cQKDPing.set_sleep_time(cVariableMap["sleep"].as<uint64_t>());
    if (cVariableMap.count("bob")) {
        cQKDPing.set_role((unsigned long)qkd::module::module_role::ROLE_BOB);
        cQKDPing.set_url_listen(QString::fromStdString(cVariableMap["connect"].as<std::string>()));
    }
    else {
        cQKDPing.set_role((unsigned long)qkd::module::module_role::ROLE_ALICE);
        cQKDPing.set_url_peer(QString::fromStdString(cVariableMap["connect"].as<std::string>()));
    }
    if (cVariableMap.count("count")) cQKDPing.set_max_roundtrip(cVariableMap["count"].as<uint64_t>());
    if (cVariableMap.count("run")) cQKDPing.start_later();

    cApp.connect(&cQKDPing, SIGNAL(terminated()), SLOT(quit()));
    int nAppExit = cApp.exec();
    cQKDPing.join();
    
    return nAppExit;
}


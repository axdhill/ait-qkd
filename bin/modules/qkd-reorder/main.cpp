/*
 * main.cpp
 * 
 * This is the REORDER QKD Module.
 * 
 * Autor: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
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

#include <iostream>

#include <boost/program_options.hpp>

// ait
#include "qkd-reorder.h"


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
    
    // get up Qt
    QCoreApplication cApp(argc, argv);
    
    // create the command line header
    std::string sApplication = std::string("qkd-reorder - AIT QKD Module 'reorder' V") + VERSION;
    std::string sDescription = std::string("\nThis is an AIT QKD module.\n\nThis module randomly reorders keys sequence when forwarding (for testing purpose).\n\nCopyright 2013-2015 AIT Austrian Institute of Technology GmbH");
    std::string sSynopsis = std::string("Usage: ") + argv[0] + " [OPTIONS]";
    
    // define program options
    boost::program_options::options_description cOptions(sApplication + "\n" + sDescription + "\n\n\t" + sSynopsis + "\n\nAllowed Options");
    cOptions.add_options()("bob,b", "set this as bob's instance, the responder");
    cOptions.add_options()("config,c", boost::program_options::value<std::string>(), "configuration file URL");
    cOptions.add_options()("debug,d", "enable debug output on stderr");
    cOptions.add_options()("help,h", "this page");
    cOptions.add_options()("run,r", "run immediately");
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
    
    // instantiate module
    qkd_reorder cQKDReorder;
    
    if (cVariableMap.count("bob")) {
        // BOB's role
        cQKDReorder.set_role((unsigned long)qkd::module::module_role::ROLE_BOB);
    }
    else {
        // ALICE's role
        cQKDReorder.set_role((unsigned long)qkd::module::module_role::ROLE_ALICE);
    }
    
    // configuration file given?
    if (cVariableMap.count("config")) cQKDReorder.configure(QString::fromStdString(cVariableMap["config"].as<std::string>()), true);
        
    // check for "run" set
    if (cVariableMap.count("run")) cQKDReorder.start_later();
    
    // terminate if module has finished
    cApp.connect(&cQKDReorder, SIGNAL(terminated()), SLOT(quit()));
    
    // run Qt
    int nAppExit = cApp.exec();

    // join worker thread (cleanup)
    cQKDReorder.join();
    
    return nAppExit;
}


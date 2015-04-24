/*
 * main.cpp
 * 
 * This is the ENKEY QKD Module.
 * 
 * This QKD Module picks up a file and pushes it to Pipe-Out.
 * Pipe in is ignored
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

#include <boost/program_options.hpp>

// ait
#include "qkd-enkey.h"


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
    std::string sApplication = std::string("qkd-enkey - AIT QKD Module 'enkey' V") + VERSION;
    std::string sDescription = std::string("\nThis is an AIT QKD module.\n\nIt takes a file as input consisting of keys and feeds this as input of a QKD pipeline.\n\nCopyright 2012, 2013 AIT Austrian Institute of Technology GmbH");
    std::string sSynopsis = std::string("Usage: ") + argv[0] + " [OPTIONS]";
    
    // define program options
    boost::program_options::options_description cOptions(sApplication + "\n" + sDescription + "\n\n\t" + sSynopsis + "\n\nAllowed Options");
    cOptions.add_options()("bob,b", "set this as bob's instance, the responder");
    cOptions.add_options()("config,c", boost::program_options::value<std::string>(), "configuration file URL");
    cOptions.add_options()("debug,d", "enable debug output on stderr");
    cOptions.add_options()("file,f", boost::program_options::value<std::string>(), "key file to read");
    cOptions.add_options()("help,h", "this page");
    cOptions.add_options()("loop,l", "loop over file");
    cOptions.add_options()("size,s", boost::program_options::value<std::uint64_t>(), "size of single key");
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
    qkd_enkey cQKDEnkey;
    
    if (cVariableMap.count("bob")) {
        // BOB's role
        cQKDEnkey.set_role((unsigned long)qkd::module::module_role::ROLE_BOB);
    }
    else {
        // ALICE's role
        cQKDEnkey.set_role((unsigned long)qkd::module::module_role::ROLE_ALICE);
    }
    
    // configuration file given?
    if (cVariableMap.count("config")) cQKDEnkey.configure(QString::fromStdString(cVariableMap["config"].as<std::string>()), true);
        
    // check for more than 1 "file"
    if (cVariableMap.count("file") > 1) {
        std::cerr << "more than 1 file argument given." << std::endl;
        return 1;
    }
    
    // do we have a single "file"?
    if (cVariableMap.count("file") == 1) {

        // check file
        boost::filesystem::path cFile(cVariableMap["file"].as<std::string>());
        if (!boost::filesystem::is_regular_file(cFile)) {
            std::cerr << "file '" << cFile.string() << "' seems not to be a regular file." << std::endl;
            return 1;
        }

        // apply value
        cQKDEnkey.set_file_url(QString("file://") + QString::fromStdString(boost::filesystem::canonical(cFile).string()));
    }
    
    // "loop" argument
    if (cVariableMap.count("loop") == 1) cQKDEnkey.set_loop(true);
    
    // "size" argument
    if (cVariableMap.count("size") == 1) cQKDEnkey.set_key_size(cVariableMap["size"].as<std::uint64_t>());
    
    // check for "run" set
    if (cVariableMap.count("run")) cQKDEnkey.start_later();
    
    // terminate if module has finished
    cApp.connect(&cQKDEnkey, SIGNAL(terminated()), SLOT(quit()));
    
    // run Qt
    int nAppExit = cApp.exec();

    // join worker thread (cleanup)
    cQKDEnkey.join();
    
    return nAppExit;
}


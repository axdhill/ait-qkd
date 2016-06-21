/*
 * main.cpp
 * 
 * This is the STATISTICS QKD Module.
 * 
 * This QKD Module receives keys from previous modules and spills 
 * out the raw key data without key-headers to a file.
 * 
 * Much like qkd-tee but with raw key data.
 * 
 * Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *
 * Copyright (C) 2015-2016 AIT Austrian Institute of Technology
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
#include <qkd/version.h>
#include "qkd-statistics.h"


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
    
    std::string sApplication = std::string("qkd-statistics - AIT QKD Module 'statistics' V") + qkd::version();
    std::string sDescription = std::string("\nThis is an AIT QKD module.\n\nIt takes keys from a previous module and places some statistic data into a file..\n\nCopyright 2015-2016 AIT Austrian Institute of Technology GmbH");
    std::string sSynopsis = std::string("Usage: ") + argv[0] + " [OPTIONS]";
    
    boost::program_options::options_description cOptions(sApplication + "\n" + sDescription + "\n\n\t" + sSynopsis + "\n\nAllowed Options");
    cOptions.add_options()("bob,b", "set this as bob's instance, the responder");
    cOptions.add_options()("config,c", boost::program_options::value<std::string>(), "configuration file URL");
    cOptions.add_options()("debug,d", "enable debug output on stderr");
    cOptions.add_options()("file,f", boost::program_options::value<std::string>(), "statistic file to write");
    cOptions.add_options()("help,h", "this page");
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

    qkd_statistics cQKDStatistics;
    if (cVariableMap.count("bob")) {
        cQKDStatistics.set_role((unsigned long)qkd::module::module_role::ROLE_BOB);
    }
    else {
        cQKDStatistics.set_role((unsigned long)qkd::module::module_role::ROLE_ALICE);
    }
    if (cVariableMap.count("config")) {
        cQKDStatistics.configure(QString::fromStdString(cVariableMap["config"].as<std::string>()), true);
    }
    if (cVariableMap.count("run")) cQKDStatistics.start_later();
    
    if (cVariableMap.count("file") > 1) {
        std::cerr << "more than 1 file argument given." << std::endl;
        return 1;
    }
    if (cVariableMap.count("file") == 1) {

        boost::filesystem::path cFile(cVariableMap["file"].as<std::string>());
        if (boost::filesystem::exists(cFile)) {
            if (!boost::filesystem::is_regular_file(cFile)) {
                std::cerr << "file '" << cFile.string() << "' seems not to be a regular file." << std::endl;
                return 1;
            }
        }
    
        cQKDStatistics.set_file_url(QString("file://") + QString::fromStdString(boost::filesystem::absolute(cFile).string()));
    }
    
    cApp.connect(&cQKDStatistics, SIGNAL(terminated()), SLOT(quit()));
    int nAppExit = cApp.exec();
    cQKDStatistics.join();
    
    return nAppExit;
}


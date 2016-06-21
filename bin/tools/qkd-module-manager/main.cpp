/*
 * main.cpp
 * 
 * This is the QKD MODULE MANAGER main startup file.
 *
 * Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
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

 
// ------------------------------------------------------------
// incs

#include <iostream>

#include <boost/program_options.hpp>

// Qt
#include <QtGui/QApplication>
#include <QtGui/QFontDatabase>

// ait
#include <qkd/version.h>
#include <qkd/utility/debug.h>
#include <qkd/version.h>

#include "main_window.h"


// ------------------------------------------------------------
// code


/**
 * start
 * 
 * @param   argc        as usual
 * @param   argv        as usual
 * @return  as usual
 */
int main(int argc, char ** argv) {
    
    std::string sApplication = std::string("qkd-module-manager - AIT QKD Module Manager V") + qkd::version();
    std::string sDescription = std::string("\nVisually manage QKD modules.\n\nCopyright 2013-2016 AIT Austrian Institute of Technology GmbH");
    std::string sSynopsis = std::string("Usage: ") + argv[0] + " [OPTIONS]";
    
    boost::program_options::options_description cOptions(sApplication + "\n" + sDescription + "\n\n\t" + sSynopsis + "\n\nAllowed Options");
    cOptions.add_options()("debug,d", "enable debug output on stderr");
    cOptions.add_options()("help,h", "this page");
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

    QApplication cApplication(argc, argv);
    cApplication.setOrganizationName("AIT Austrian Institute of Technology GmbH");
    cApplication.setOrganizationDomain("ait.ac.at");
    cApplication.setApplicationName("QKD Module Manager");
    cApplication.setApplicationVersion(QKD_VERSION);
    cApplication.setStyleSheet("QFrame { padding: 0px; margin: 0px; }");
    
    qkd::module_manager::main_window cMainWindow;
    cMainWindow.show();
    
    return cApplication.exec();
}
   

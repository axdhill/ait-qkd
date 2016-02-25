/*
 * main.cpp
 * 
 * This is shows the current QKD system snapshot
 *
 * Authors: Oliver Maurhart, <oliver.maurhart@ait.ac.at>,
 *             Manuel Warum, <manuel.warum@ait.ac.at>
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

#include <boost/format.hpp>
#include <boost/program_options.hpp>

// Qt
#include <QtCore/QCoreApplication>

// ait
#include <qkd/utility/debug.h>
#include <qkd/utility/investigation.h>

#include "output_format.h"
#include "tabular_output.h"
#include "json_output.h"


// ------------------------------------------------------------
// code

inline output_format * getOutputFormatFor(boost::program_options::variables_map const & cProgramOptions) {
    output_format * returnValue;
    if (cProgramOptions.count("json") == 0)
        returnValue = new tabular_output();
    else
        returnValue = new json_output();
    return returnValue;
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
    std::string sDescription = std::string("\nThis shows the current QKD system.\nThe values of the found nodes, links and modules are separated by tabs.\n\nCopyright 2012-2016 AIT Austrian Institute of Technology GmbH");
    std::string sSynopsis = std::string("Usage: ") + argv[0] + " [OPTIONS]";
    
    // define program options
    boost::program_options::options_description cOptions(sApplication + "\n" + sDescription + "\n\n    " + sSynopsis + "\n\nAllowed Options");
    cOptions.add_options()("debug,d", "enable debug output on stderr");
    cOptions.add_options()("help,h", "this page");
    cOptions.add_options()("module-io,i", "only show modules I/O addresses");
    cOptions.add_options()("omit-header,o", "don't print headers on each table");
    cOptions.add_options()("short,s", "output is limited to more important data");
    cOptions.add_options()("json,j", "output is using a JSON syntax");
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
    
    // investigate the system
    qkd::utility::investigation cInvestigation = qkd::utility::investigation::investigate();

    // prepare output
    output_format * cOutputFormat = getOutputFormatFor(cVariableMap);
    cOutputFormat->initialize(cVariableMap);
    cOutputFormat->write(cInvestigation);
    
    return 0;
}



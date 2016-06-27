/*
 * main.cpp
 * 
 * This is the qkd pipeline tool
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

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <QtCore/QCoreApplication>

#include <qkd/version.h>

#include "pipeline.h"


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
    
    std::string sApplication = std::string("qkd-pipeline - AIT QKD Key Pipeline Tool V") + qkd::version();
    std::string sDescription = std::string("\nThis tool lets you start/stop/restart a full QKD pipeline.\n\nCopyright 2013-2016 AIT Austrian Institute of Technology GmbH");
    std::string sSynopsis = std::string("Usage: ") + argv[0] + " [OPTIONS] COMMAND PIPELINE-CONFIG";
    
    boost::program_options::options_description cOptions(sApplication + "\n" + sDescription + "\n\n\t" + sSynopsis + "\n\nAllowed Options");
    cOptions.add_options()("help,h", "this page");
    cOptions.add_options()("log,l", boost::program_options::value<std::string>(), "path to log folder");
    cOptions.add_options()("version,v", "print version string");
    
    boost::program_options::options_description cArgs("Arguments");
    
    cArgs.add_options()("COMMAND", "COMMAND is either 'start', 'stop' or 'restart'.");
    cArgs.add_options()("PIPELINE-CONFIG", "PIPELINE-CONFIG is the path to the pipeline configuration XML.");
    
    boost::program_options::positional_options_description cPositionalDescription; 
    cPositionalDescription.add("COMMAND", 1);
    cPositionalDescription.add("PIPELINE-CONFIG", 1);
    
    boost::program_options::options_description cCmdLineOptions("Command Line");
    cCmdLineOptions.add(cOptions);
    cCmdLineOptions.add(cArgs);
    
    boost::program_options::variables_map cVariableMap;
    try {
        boost::program_options::command_line_parser cParser(argc, argv);
        boost::program_options::store(cParser.options(cCmdLineOptions).positional(cPositionalDescription).run(), cVariableMap);
        boost::program_options::notify(cVariableMap);        
    }
    catch (std::exception & cException) {
        std::cerr << "error parsing command line: " 
                << cException.what() 
                << "\ntype '--help' for help" 
                << std::endl;        
        return 1;
    }
    
    if (cVariableMap.count("help")) {
        std::cout << cOptions << std::endl;
        std::cout << cArgs.find("COMMAND", false).description() << "\n";      
        std::cout << cArgs.find("PIPELINE-CONFIG", false).description() << "\n" << std::endl;      
        return 0;
    }
    if (cVariableMap.count("version")) {
        std::cout << sApplication << std::endl;
        return 0;
    }
    
    pipeline cPipeline;
    
    if (cVariableMap.count("log")) {
        cPipeline.set_log_folder(cVariableMap["log"].as<std::string>());
        if (!boost::filesystem::exists(boost::filesystem::path(cPipeline.log_folder()))) {
            std::cerr << "cannot access log folder '" << cPipeline.log_folder() << "'." << std::endl;
            return 1;
        }
    }
    if (!cVariableMap.count("COMMAND")) {
        std::cerr << "no pipeline command.\ntype '--help' for information." << std::endl;
        return 1;
    }
    std::string sPipelineCommand = cVariableMap["COMMAND"].as<std::string>();
    if (!sPipelineCommand.size()) {
        std::cerr 
                << "neither 'start', 'stop' nor 'restart' specified.\nchoose one command - type '--help' for help." 
                << std::endl;
        return 1;
    }
    
    bool bStart = (sPipelineCommand == "start");
    bool bStop = (sPipelineCommand == "stop");
    bool bRestart = (sPipelineCommand == "restart");
    if (!bStart && !bStop && !bRestart) {
        std::cerr << "command '" 
                << sPipelineCommand 
                << "' unknown.\nchoose one command - type '--help' for help." 
                << std::endl;
        return 1;
    }

    if (!cVariableMap.count("PIPELINE-CONFIG")) {
        std::cerr << "no pipeline-config specified.\ntype '--help' for information." << std::endl;
        return 1;
    }
    std::string sPipelineConfiguration = cVariableMap["PIPELINE-CONFIG"].as<std::string>();
    
    QCoreApplication cApp(argc, argv);

    int nConfigErrorCode = cPipeline.parse(sPipelineConfiguration);
    if (nConfigErrorCode != 0) return nConfigErrorCode;
    
    int nStartErrorCode = 0;
    int nStopErrorCode = 0;
    if (bStop || bRestart) nStopErrorCode = cPipeline.stop();
    if (bStart || bRestart) nStartErrorCode = cPipeline.start();
    
    return std::max<int>(nStartErrorCode, nStopErrorCode);
}

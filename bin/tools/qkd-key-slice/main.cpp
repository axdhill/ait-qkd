/*
 * main.cpp
 * 
 * This is the qkd key slice
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

#include <fstream>
#include <iostream>

#include <boost/format.hpp>
#include <boost/program_options.hpp>

// ait
#include <qkd/key/key.h>
#include <qkd/utility/checksum.h>
#include <qkd/version.h>


// ------------------------------------------------------------
// fwd


/**
 * slice the keystream file
 * 
 * @param   cIn         input stream
 * @param   cOut        output stream
 * @param   nKeys       number of keys to slice (0 == all)
 * @param   nSkip       number of keys to skip at the beginning
 */
void slice(std::istream & cIn, std::ostream & cOut, uint64_t nKeys, uint64_t nSkip);


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
    
    std::string sApplication = std::string("qkd-key-slice - AIT QKD Key Slice Tool V") + qkd::version();
    std::string sDescription = std::string("\nThis tools let you slice keys out of a keystream file into a sperate file.\n\nCopyright 2012-2016 AIT Austrian Institute of Technology GmbH");
    std::string sSynopsis = std::string("Usage: ") + argv[0] + " [OPTIONS] [FILE]";
    
    boost::program_options::options_description cOptions(sApplication + "\n" + sDescription + "\n\n\t" + sSynopsis + "\n\nAllowed Options");
    cOptions.add_options()("help,h", "this page");
    cOptions.add_options()("input-file,i", boost::program_options::value<std::string>(), "input file");
    cOptions.add_options()("output-file,o", boost::program_options::value<std::string>(), "output file (if omitted stdout is used)");
    cOptions.add_options()("keys,k", boost::program_options::value<uint64_t>()->default_value(0), "number of keys to dump [0 == all]");
    cOptions.add_options()("skip", boost::program_options::value<uint64_t>()->default_value(0), "number of keys to skip at beginning");
    cOptions.add_options()("version,v", "print version string");
    
    boost::program_options::options_description cArgs("Arguments");
    cArgs.add_options()("FILE", "FILE is the name of file to read, if omitted stdin is used.");
    boost::program_options::positional_options_description cPositionalDescription; 
    cPositionalDescription.add("input-file", 1);
    
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
        std::cerr << "error parsing command line: " << cException.what() << "\ntype '--help' for help" << std::endl;        
        return 1;
    }
    if (cVariableMap.count("help")) {
        std::cout << cOptions << std::endl;
        std::cout << cArgs.find("FILE", false).description() << "\n" << std::endl;      
        return 0;
    }
    if (cVariableMap.count("version")) {
        std::cout << sApplication << std::endl;
        return 0;
    }

    std::ifstream cInFile;
    std::ofstream cOutFile;
    if (cVariableMap.count("input-file")) {
        cInFile.open(cVariableMap["input-file"].as<std::string>());
        if (!cInFile.good()) {
            std::cerr << "failed to open input file '" << cVariableMap["input-file"].as<std::string>() << ": " << strerror(errno) << std::endl;
            return 1;
        }
    }
    if (cVariableMap.count("output-file")) {
        cOutFile.open(cVariableMap["output-file"].as<std::string>());
        if (!cOutFile.good()) {
            std::cerr << "failed to open output file '" << cVariableMap["output-file"].as<std::string>() << ": " << strerror(errno) << std::endl;
            return 1;
        }
    }
   
    uint64_t nKeys = cVariableMap["keys"].as<uint64_t>();
    uint64_t nSkip = cVariableMap["skip"].as<uint64_t>();
    slice(cInFile.is_open() ? cInFile : std::cin, cOutFile.is_open() ? cOutFile : std::cout, nKeys, nSkip);
    
    return 0;
}


/**
 * slice the keystream file
 * 
 * @param   cIn         input stream
 * @param   cOut        output stream
 * @param   nKeys       number of keys to slice (0 == all)
 * @param   nSkip       number of keys to skip at the beginning
 */
void slice(std::istream & cIn, std::ostream & cOut, uint64_t nKeys, uint64_t nSkip) {

    while (!cIn.eof()) {

        qkd::key::key cKey;
        cIn >> cKey;
        if (cKey == qkd::key::key::null()) continue;
        
        if (nSkip > 0) {
            --nSkip;
            continue;
        }
        
        cOut << cKey;
        
        if (nKeys > 0) {
            --nKeys;
            if (!nKeys) break;
        }
    }
}

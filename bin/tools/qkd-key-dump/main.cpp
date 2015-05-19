/*
 * main.cpp
 * 
 * This is the qkd key dump
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

#include <fstream>
#include <iostream>

#include <boost/format.hpp>
#include <boost/program_options.hpp>

// ait
#include <qkd/key/key.h>


// ------------------------------------------------------------
// fwd


int dump(std::istream & cIn, std::ostream & cOut);


// ------------------------------------------------------------
// code


/**
 * key dump loop
 * 
 * @param   cIn         input stream
 * @param   cOut        output stream
 * @return  0 = sucess, else failure
 */
int dump(std::istream & cIn, std::ostream & cOut) {

    // loop until we are eof
    while (!cIn.eof()) {

        // read key
        qkd::key::key cKey;
        cIn >> cKey;
        
        // check if valid
        if (cKey == qkd::key::key::null()) continue;
        
        // create output
        std::stringstream ss;
        
        uint64_t nBits = cKey.size() * 8;
        double nDisclosedBitsRate = (double)cKey.meta().nDisclosedBits / (double)nBits;
        
        ss << "key #" << cKey.id() << "\n";
        ss << "\tbits:                \t" << nBits << "\n";
        ss << "\tdisclosed bits:      \t" << cKey.meta().nDisclosedBits << " (" << boost::format("%05.2f") % (nDisclosedBitsRate * 100.0) << "%)\n";
        ss << "\terror bits:          \t" << cKey.meta().nErrorBits << "\n";
        ss << "\terror rate:          \t" << cKey.meta().nErrorRate << "\n";
        ss << "\tauth-scheme-incoming:\t" << cKey.meta().sCryptoSchemeIncoming << "\n";
        ss << "\tauth-scheme-outgoing:\t" << cKey.meta().sCryptoSchemeOutgoing << "\n";
        ss << "\tstate:               \t" << cKey.state_string() << "\n";
        
        // checksum
        ss << "\tcrc32:               \t" << cKey.data().crc32() << "\n";
        
        // the key data itself
        ss << "\tdata:                \t" << cKey.data().as_hex() << "\n";

        // write
        cOut << ss.str();
    }
    
    return 0;
}

    
/**
 * key dump loop short version
 * 
 * @param   cIn         input stream
 * @param   cOut        output stream
 * @return  0 = sucess, else failure
 */
int dump_short(std::istream & cIn, std::ostream & cOut) {

    std::string sHeading = "key        bits     disclosed bits error bits error rate state         crc\n";
    std::string sFormat = "%010lu %08lu %08lu       %08lu  %7.4f     %-13s %8s\n";
    bool bPrintHeading = true;

    // loop until we are eof
    while (!cIn.eof()) {

        // read key
        qkd::key::key cKey;
        cIn >> cKey;
        
        // check if valid
        if (cKey == qkd::key::key::null()) continue;

        // print data
        if (bPrintHeading) {
            cOut << sHeading;
            bPrintHeading = false;
        }
        
        uint64_t nBits = cKey.size() * 8;
        cOut << boost::format(sFormat) % cKey.id() % nBits % cKey.meta().nDisclosedBits % cKey.meta().nErrorBits % cKey.meta().nErrorRate % cKey.state_string() % cKey.data().crc32();
    }
    
    return 0;
}


/**
 * start
 * 
 * @param   argc        as usual
 * @param   argv        as usual
 * @return  as usual
 */
int main(int argc, char ** argv) {
    
    // create the command line header
    std::string sApplication = std::string("qkd-key-dump - AIT QKD Key Dump Tool V") + VERSION;
    std::string sDescription = std::string("\nThis tools let you dump the content of a key file in human readable output.\n\nCopyright 2012-2015 AIT Austrian Institute of Technology GmbH");
    std::string sSynopsis = std::string("Usage: ") + argv[0] + " [OPTIONS] [FILE]";
    
    // define program options
    boost::program_options::options_description cOptions(sApplication + "\n" + sDescription + "\n\n\t" + sSynopsis + "\n\nAllowed Options");
    cOptions.add_options()("help,h", "this page");
    cOptions.add_options()("input-file,i", boost::program_options::value<std::string>(), "input file");
    cOptions.add_options()("output-file,o", boost::program_options::value<std::string>(), "output file (if omitted stdout is used)");
    cOptions.add_options()("short,s", "short version omitting data itself");
    cOptions.add_options()("version,v", "print version string");
    
    // final arguments
    boost::program_options::options_description cArgs("Arguments");
    cArgs.add_options()("FILE", "FILE is the name of file to read, if omitted stdin is used.");
    boost::program_options::positional_options_description cPositionalDescription; 
    cPositionalDescription.add("input-file", 1);
    
    // construct overall options
    boost::program_options::options_description cCmdLineOptions("Command Line");
    cCmdLineOptions.add(cOptions);
    cCmdLineOptions.add(cArgs);

    // option variable map
    boost::program_options::variables_map cVariableMap;
    
    try {
        // parse action
        boost::program_options::command_line_parser cParser(argc, argv);
        boost::program_options::store(cParser.options(cCmdLineOptions).positional(cPositionalDescription).run(), cVariableMap);
        boost::program_options::notify(cVariableMap);        
    }
    catch (std::exception & cException) {
        std::cerr << "error parsing command line: " << cException.what() << "\ntype '--help' for help" << std::endl;        
        return 1;
    }
    
    // check for "help" set
    if (cVariableMap.count("help")) {
        std::cout << cOptions << std::endl;
        std::cout << cArgs.find("FILE", false).description() << "\n" << std::endl;      
        return 0;
    }
    
    // check for "version" set
    if (cVariableMap.count("version")) {
        std::cout << sApplication << std::endl;
        return 0;
    }

    // short?
    bool bShort = (cVariableMap.count("short") > 0);
    
    std::ifstream cInFile;
    std::ofstream cOutFile;
    
    // input file
    if (cVariableMap.count("input-file")) {
        cInFile.open(cVariableMap["input-file"].as<std::string>());
        if (!cInFile) {
            std::cerr << "failed to open input file '" << cVariableMap["input-file"].as<std::string>() << ": " << strerror(errno) << std::endl;
            return 1;
        }
    }
    
    // output file
    if (cVariableMap.count("output-file")) {
        cOutFile.open(cVariableMap["output-file"].as<std::string>());
        if (!cOutFile) {
            std::cerr << "failed to open output file '" << cVariableMap["output-file"].as<std::string>() << ": " << strerror(errno) << std::endl;
            return 1;
        }
    }
    
    // on with it
    if (bShort) {
        return dump_short((cInFile.is_open() ? cInFile : std::cin), (cOutFile.is_open() ? cOutFile : std::cout));
    }

    return dump((cInFile.is_open() ? cInFile : std::cin), (cOutFile.is_open() ? cOutFile : std::cout));
}

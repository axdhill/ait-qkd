/*
 * main.cpp
 * 
 * This is the qkd key dump
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
// defs

/**
 * 4 bit detector bit masks as strings
 */
std::string const g_c4DetectorBits[16] = {
    "....", "...X", "..X.", "..XX",
    ".X..", ".X.X", ".XX.", ".XXX",
    "X...", "X..X", "X.X.", "X.XX",
    "XX..", "XX.X", "XXX.", "XXXX" 
};


/**
 * dump configuration
 */
typedef struct {
    
    uint64_t nKeys;             /**< number of keys to dump */
    uint64_t nSkip;             /**< number of keys to skip */
    bool bCanonical;            /**< ignore encoding but print canonical */
    bool bFlatData;             /**< ignore encoding but print flat */
    
} dump_configuration;


// ------------------------------------------------------------
// fwd


/**
 * get the key data as string
 * 
 * @param   cKey                the key whose data is to be stringified
 * @param   cConfig             dump output config
 * @param   sIndent             indent of every line
 * @return  a string holding the key's data representation
 */
std::string key_data(qkd::key::key const & cKey, dump_configuration const & cConfig, std::string sIndent = "\t");


// ------------------------------------------------------------
// code


/**
 * key dump loop
 * 
 * @param   cConfig             dump configuration
 * @return  0 = success, else failure
 */
int dump(std::istream & cIn, std::ostream & cOut, dump_configuration & cConfig) {

    while (!cIn.eof()) {

        qkd::key::key cKey;
        cIn >> cKey;
        if (cKey == qkd::key::key::null()) continue;
        
        if (cConfig.nSkip > 0) {
            --cConfig.nSkip;
            continue;
        }
        
        uint64_t nBits = cKey.size() * 8;
        double nDisclosedBitsRate = (double)cKey.disclosed() / (double)nBits;
        
        cOut << "key #" << cKey.id() << "\n";
        cOut << "\tbits:                \t" << nBits << "\n";
        cOut << "\tdisclosed bits:      \t" << cKey.disclosed() << " (" << boost::format("%05.2f") % (nDisclosedBitsRate * 100.0) << "%)\n";
        cOut << "\terror rate:          \t" << cKey.qber() << "\n";
        cOut << "\tauth-scheme-incoming:\t" << cKey.crypto_scheme_incoming() << "\n";
        cOut << "\tauth-scheme-outgoing:\t" << cKey.crypto_scheme_outgoing() << "\n";
        cOut << "\tstate:               \t" << cKey.state_string() << "\n";
        cOut << "\tcrc32:               \t" << cKey.data().crc32() << "\n";
        cOut << "\tencoding:            \t" << cKey.encoding() << "\n";
        cOut << "\tdata:                \t" << key_data(cKey, cConfig, "\t                     \t") << "\n";
        
        if (cConfig.nKeys > 0) {
            --cConfig.nKeys;
            if (!cConfig.nKeys) break;
        }
    }
    
    return 0;
}

    
/**
 * key dump loop md5sum version
 * 
 * @param   cIn         input stream
 * @param   cOut        output stream
 * @param   bTotal      print only the total MD5 of all key material
 * @return  0 = success, else failure
 */
int dump_md5sum(std::istream & cIn, std::ostream & cOut, bool bTotal) {
    
    qkd::utility::checksum cMD5AlgorithmAll = qkd::utility::checksum_algorithm::create("md5");
    
    while (!cIn.eof()) {

        qkd::key::key cKey;
        cIn >> cKey;
        if (cKey == qkd::key::key::null()) continue;
        
        if (!bTotal) {
        
            qkd::utility::checksum cMD5Algorithm = qkd::utility::checksum_algorithm::create("md5");
            cMD5Algorithm << cKey.data();
            qkd::utility::memory cMD5Sum;
            cMD5Algorithm >> cMD5Sum;
            cOut << cMD5Sum.as_hex() << std::endl;
        }
        else {
            cMD5AlgorithmAll << cKey.data();
        }
    }
    
    if (bTotal) {
        qkd::utility::memory cMD5Sum;
        cMD5AlgorithmAll >> cMD5Sum;
        cOut << cMD5Sum.as_hex() << std::endl;
    }
    
    return 0;
}


/**
 * key dump loop metadata version
 * 
 * @param   cIn         input stream
 * @param   cOut        output stream
 * @return  0 = success, else failure
 */
int dump_metadata(std::istream & cIn, std::ostream & cOut) {
    
    while (!cIn.eof()) {

        qkd::key::key cKey;
        cIn >> cKey;
        if (cKey == qkd::key::key::null()) continue;
        
        cOut << "key# " << cKey.id() << std::endl;
        cOut << cKey.metadata_xml(true) << "\n" << std::endl;
    }
    
    return 0;
}


/**
 * key dump loop short version
 * 
 * @param   cIn         input stream
 * @param   cOut        output stream
 * @return  0 = success, else failure
 */
int dump_short(std::istream & cIn, std::ostream & cOut) {

    std::string sHeading = "key        bits     disclosed bits error rate state         crc\n";
    std::string sFormat = "%010lu %08lu %08lu      %7.4f     %-13s %8s\n";
    bool bPrintHeading = true;

    while (!cIn.eof()) {

        qkd::key::key cKey;
        cIn >> cKey;
        if (cKey == qkd::key::key::null()) continue;

        if (bPrintHeading) {
            cOut << sHeading;
            bPrintHeading = false;
        }
        
        uint64_t nBits = cKey.size() * 8;
        cOut << boost::format(sFormat) % cKey.id() % nBits % cKey.disclosed() % cKey.qber() % cKey.state_string() % cKey.data().crc32();
    }
    
    return 0;
}


/**
 * get the key data as string
 * 
 * @param   cKey                the key whose data is to be stringified
 * @param   cConfig             dump output config
 * @param   sIndent             indent of every line
 * @return  a string holding the key's data representation
 */
std::string key_data(qkd::key::key const & cKey, dump_configuration const & cConfig, std::string sIndent) {
    
    if (cConfig.bFlatData) {
        return cKey.data().as_hex();
    }
    
    if (cConfig.bCanonical || (cKey.encoding() == qkd::key::ENCODING_SHARED_SECRET_BITS)) {
        return "\n" + cKey.data().canonical(sIndent);
    }
    
    if (cKey.encoding() == qkd::key::ENCODING_4_DETECTOR_CLICKS) {
        
        qkd::utility::memory::value_t const * d = cKey.data().get();
        std::stringstream ss;
        for (uint64_t i = 0; i < cKey.data().size(); ++i) {
            
            if (!(i % 8)) {
                ss << "\n" << sIndent;
            }
            else {
                ss << " - ";
            }
            
            ss << g_c4DetectorBits[d[i] >> 4];
            ss << " ";
            ss << g_c4DetectorBits[d[i] & 0x0F];
        }
        
        return ss.str();
    }
    
    return "don't know how to represent this key data encoding";
}


/**
 * start
 * 
 * @param   argc        as usual
 * @param   argv        as usual
 * @return  as usual
 */
int main(int argc, char ** argv) {
    
    std::string sApplication = std::string("qkd-key-dump - AIT QKD Key Dump Tool V") + qkd::version();
    std::string sDescription = std::string("\nThis tools let you dump the content of a key file in human readable output.\n\nCopyright 2012-2016 AIT Austrian Institute of Technology GmbH");
    std::string sSynopsis = std::string("Usage: ") + argv[0] + " [OPTIONS] [FILE]";
    
    boost::program_options::options_description cOptions(sApplication + "\n" + sDescription + "\n\n\t" + sSynopsis + "\n\nAllowed Options");
    cOptions.add_options()("help,h", "this page");
    cOptions.add_options()("input-file,i", boost::program_options::value<std::string>(), "input file");
    cOptions.add_options()("output-file,o", boost::program_options::value<std::string>(), "output file (if omitted stdout is used)");
    cOptions.add_options()("short,s", "short version omitting data itself");
    cOptions.add_options()("keys,k", boost::program_options::value<uint64_t>()->default_value(0), "number of keys to dump [0 == all]");
    cOptions.add_options()("skip", boost::program_options::value<uint64_t>()->default_value(0), "number of keys to skip at beginning");
    cOptions.add_options()("metadata,m", "print full XML metadata of each key");
    cOptions.add_options()("md5sum", "print only MD5 checksum of each key material (without metadata)");
    cOptions.add_options()("md5sum-all", "print the overall MD5 checksum of all key material (without metadata)");
    cOptions.add_options()("canonical", "ignore key data encoding but provide a canonical output");
    cOptions.add_options()("flat", "ignore key data encoding but provide a flat line of key data bytes in hex");
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

    bool bShort = (cVariableMap.count("short") > 0);
    bool bMetadata = (cVariableMap.count("metadata") > 0);
    bool bMD5Sum = (cVariableMap.count("md5sum") > 0);
    bool bMD5SumAll = (cVariableMap.count("md5sum-all") > 0);
    int nMutualOptions = (bShort ? 1 : 0) + (bMetadata ? 1 : 0) + (bMD5Sum ? 1 : 0) + (bMD5SumAll ? 1 : 0);
    if (nMutualOptions > 1) {
        std::cerr << "please choose either --short, --metadata, --md5sum or --md5sum-all but not a combination of them" << std::endl;
        return 1;
    }
    
    dump_configuration cConfig;
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
   
    cConfig.nKeys = cVariableMap["keys"].as<uint64_t>();
    cConfig.nSkip = cVariableMap["skip"].as<uint64_t>();
    cConfig.bCanonical = cVariableMap.count("canonical") > 0;
    cConfig.bFlatData = cVariableMap.count("flat") > 0;
    
    if (bShort) {
        return dump_short(cInFile.is_open() ? cInFile : std::cin, cOutFile.is_open() ? cOutFile : std::cout);
    }
    if (bMetadata) {
        return dump_metadata(cInFile.is_open() ? cInFile : std::cin, cOutFile.is_open() ? cOutFile : std::cout);
    }
    if (bMD5Sum || bMD5SumAll) {
        return dump_md5sum(cInFile.is_open() ? cInFile : std::cin, cOutFile.is_open() ? cOutFile : std::cout, bMD5SumAll);
    }

    if (cConfig.bCanonical && cConfig.bFlatData) {
        std::cerr << "error: please choose either --canonical or --flat but not both" << std::endl;
        return 1;
    }
    
    return dump(cInFile.is_open() ? cInFile : std::cin, cOutFile.is_open() ? cOutFile : std::cout, cConfig);
}

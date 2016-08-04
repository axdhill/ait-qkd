/*
 * main.cpp
 * 
 * This is the qkd key compare
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

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>

// ait
#include <qkd/key/key.h>
#include <qkd/utility/bigint.h>
#include <qkd/common_macros.h>
#include <qkd/version.h>


// ------------------------------------------------------------
// decl


/**
 * comparment config
 */
typedef struct {
    
    std::string sFile1;             /**< name of first key stream file */
    std::string sFile2;             /**< name of second key stream file */
    
    uint64_t nSkip1;                /**< number of keys to skip in first stream */
    uint64_t nSkip2;                /**< number of keys to skip in second stream */
    
    bool bCompareAll;               /**< compare all keys */
    uint64_t nCompare;              /**< number of comparisons (if not compare all keys) */
    
    std::ifstream cStreamIn1;       /**< input stream 1 */
    std::ifstream cStreamIn2;       /**< input stream 2 */
    
    
    /**
     * set to default values
     */
    void init() {
        nSkip1 = 0;
        nSkip2 = 0;
        bCompareAll = true;
        nCompare = 0;
    }
    
    
} compare_config;


/**
 * the diff result of two keys
 */
typedef struct {
    
    bool bSizeDiffer;               /**< key lengths do differ  */
    uint64_t nCompareLength;        /**< length of keys taken for comparison (if key lengths do differ) */
    uint64_t nBitsDiffer;           /**< how many bits differ */
    double nBitsDifferRate;         /**< rate of different bits */
    
} compare_result;


// ------------------------------------------------------------
// fwd


/**
 * check if the file does exist and is readable
 * 
 * @param   sFile           name of the file to check
 * @return  true, for success
 */
bool check_file(std::string const & sFile);


/**
 * compare keys from two streams and write to result to an out stream
 * 
 * @param   cConfig         the comparison config
 * @param   cStreamOut      where to stream to
 * @return  0 = success, else failure
 */
int compare(compare_config & cConfig, std::ostream & cStreamOut);


/**
 * compare two keys
 * 
 * @param   cKey1           first key
 * @param   cKey2           second key
 * @return  a comparison result
 */
compare_result compare_keys(qkd::key::key const & cKey1, qkd::key::key const & cKey2);


/**
 * skip first keys in streams 
 * 
 * @param   cConfig         the comparison config
 * @param   cStreamOut      where to stream to
 * @return  true: still some keys in the streams to compare
 */
bool fast_forward(compare_config & cConfig, std::ostream & cStreamOut);


// ------------------------------------------------------------
// code


/**
 * check if the file does exist and is readable
 * 
 * @param   sFile           name of the file to check
 * @return  true, for success
 */
bool check_file(std::string const & sFile) {
    
    boost::filesystem::path cPath(sFile);
    if (!boost::filesystem::exists(cPath)) {
        std::cerr << "error: file '" << sFile << "' does not exist" << std::endl;
        return false;
    }
    
    if (!boost::filesystem::is_regular_file(cPath)) {
        std::cerr << "error: file '" << sFile << "' is no regular file" << std::endl;
        return false;
    }
    
    return true;
}


/**
 * compare keys from two streams and write to result to an out stream
 * 
 * @param   cConfig         the comparison config
 * @param   cStreamOut      where to stream to
 * @return  0 = success, else failure
 */
int compare(compare_config & cConfig, std::ostream & cStreamOut) {
    
    cStreamOut << "comparing keys..." << "\nfile 1: " << cConfig.sFile1 << "\nfile 2: " << cConfig.sFile2 << std::endl;
    
    uint64_t nCompare = cConfig.nCompare;
    if (!fast_forward(cConfig, cStreamOut)) {
        return 0;
    }
    
    bool bHeaderShown = false;
    while ((cConfig.bCompareAll || (nCompare > 0)) && (!cConfig.cStreamIn1.eof()) && (!cConfig.cStreamIn2.eof())) {
        
        qkd::key::key cKey1;
        qkd::key::key cKey2;
        
        cConfig.cStreamIn1 >> cKey1;
        cConfig.cStreamIn2 >> cKey2;
        
        if (cConfig.cStreamIn1.eof()) break;
        if (cConfig.cStreamIn2.eof()) break;
        
        if (!bHeaderShown) {
            std::string sHeading = "key        bits     disclosed bits error rate state         crc      - key        bits     disclosed bits error rate state         crc      - diff. bits  diff. rate";
            cStreamOut << sHeading << std::endl;
            bHeaderShown = true;
        }
        
        compare_result cResult = compare_keys(cKey1, cKey2);
        
        std::string sFormat = "%010lu %08lu %08lu      %7.4f     %-13s %8s - %010lu %08lu %08lu      %7.4f     %-13s %8s - %010lu %7.4f\n";
        boost::format cFormat(sFormat);
        cFormat 
            % cKey1.id() % (cKey1.size() * 8) % cKey1.disclosed() % cKey1.qber() % cKey1.state_string() % cKey1.data().crc32()
            % cKey2.id() % (cKey2.size() * 8) % cKey2.disclosed() % cKey2.qber() % cKey2.state_string() % cKey2.data().crc32()
            % cResult.nBitsDiffer % cResult.nBitsDifferRate;
        cStreamOut << cFormat.str();
        
        
        if (!cConfig.bCompareAll) {
            --nCompare;
        }
    }
    
    return 0;
}


/**
 * compare two keys
 * 
 * @param   cKey1           first key
 * @param   cKey2           second key
 * @return  a comparison result
 */
compare_result compare_keys(qkd::key::key const & cKey1, qkd::key::key const & cKey2) {
    
    compare_result res;
    res.bSizeDiffer = (cKey1.size() != cKey2.size());
    res.nCompareLength = cKey1.size() * 8;
    if (res.bSizeDiffer) {
        res.nCompareLength = cKey1.size() <= cKey2.size() ? cKey1.size() * 8 : cKey2.size() * 8;
    }
    
    qkd::utility::bigint cBI1 = qkd::utility::bigint(cKey1.data());
    qkd::utility::bigint cBI2 = qkd::utility::bigint(cKey2.data());
    cBI1.resize(res.nCompareLength);
    cBI2.resize(res.nCompareLength);
    
    qkd::utility::bigint cBIRes = cBI1 ^ cBI2;
    res.nBitsDiffer = cBIRes.bits_set();
    res.nBitsDifferRate = (double)res.nBitsDiffer / (double)res.nCompareLength;
    
    return res;
}


/**
 * skip first keys in streams 
 * 
 * @param   cConfig         the comparison config
 * @param   cStreamOut      where to stream to
 * @return  true: still some keys in the streams present to compare
 */
bool fast_forward(compare_config & cConfig, std::ostream & cStreamOut) {
    
    qkd::key::key cKey1;
    qkd::key::key cKey2;
    
    uint64_t nSkip1 = cConfig.nSkip1;
    uint64_t nSkip2 = cConfig.nSkip2;

    while (!cConfig.cStreamIn1.eof() && (nSkip1 > 0)) {
        cConfig.cStreamIn1 >> cKey1;
        --nSkip1;
    }
    if (cConfig.cStreamIn1.eof()) {
        cStreamOut << "skipped keys in first stream: none left to compare" << std::endl;
        return false;
    }
    
    while (!cConfig.cStreamIn2.eof() && (nSkip2 > 0)) {
        cConfig.cStreamIn2 >> cKey2;
        --nSkip2;
    }
    if (cConfig.cStreamIn2.eof()) {
        cStreamOut << "skipped keys in second stream: none left to compare" << std::endl;
        return false;
    }
    
    return true;
}


/**
 * start
 * 
 * @param   argc        as usual
 * @param   argv        as usual
 * @return  as usual
 */
int main(int argc, char ** argv) {
    
    std::string sApplication = std::string("qkd-key-compare - AIT QKD Key Compare Tool V") + qkd::version();
    std::string sDescription = std::string("\nThis tools let you compare the content of two key files and writes a human readable result.\n\nCopyright 2016 AIT Austrian Institute of Technology GmbH");
    std::string sSynopsis = std::string("Usage: ") + argv[0] + " [OPTIONS] KEY-FILE1 KEY-FILE2";
    
    boost::program_options::options_description cOptions(sApplication + "\n" + sDescription + "\n\n\t" + sSynopsis + "\n\nAllowed Options");
    cOptions.add_options()("help,h", "this page");
    cOptions.add_options()("skip1", boost::program_options::value<uint64_t>(), "number of keys to skip in first stream");
    cOptions.add_options()("skip2", boost::program_options::value<uint64_t>(), "number of keys to skip in second stream");
    cOptions.add_options()("count,c", boost::program_options::value<uint64_t>(), "number of compares");
    cOptions.add_options()("version,v", "print version string");
    
    boost::program_options::options_description cArgs("Arguments");
    cArgs.add_options()("KEY-FILE1", "KEY-FILE1 is the name of the first file to read");
    cArgs.add_options()("KEY-FILE2", "KEY-FILE2 is the name of the second file to read");
    boost::program_options::positional_options_description cPositionalDescription; 
    cPositionalDescription.add("KEY-FILE1", 1);
    cPositionalDescription.add("KEY-FILE2", 1);
    
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
        std::cout << cArgs.find("KEY-FILE1", false).description() << "\n";
        std::cout << cArgs.find("KEY-FILE2", false).description() << "\n" << std::endl;      
        return 0;
    }
    if (cVariableMap.count("version")) {
        std::cout << sApplication << std::endl;
        return 0;
    }
    
    if (cVariableMap.count("KEY-FILE1") == 0) {
        std::cerr << "error: missing first key stream file\ntype '--help' for help" << std::endl;
        return 1;
    }
    if (cVariableMap.count("KEY-FILE2") == 0) {
        std::cerr << "error: missing second key stream file\ntype '--help' for help" << std::endl;
        return 1;
    }
    
    compare_config cConfig;
    cConfig.init();
    
    cConfig.sFile1 = cVariableMap["KEY-FILE1"].as<std::string>();
    if (!check_file(cConfig.sFile1)) return 1;
    cConfig.cStreamIn1.open(cConfig.sFile1);
    if (!cConfig.cStreamIn1.is_open()) {
        std::cerr << "error: failed to open first key stream file: " << strerror(errno) << std::endl;
        return 1;
    }
    
    cConfig.sFile2 = cVariableMap["KEY-FILE2"].as<std::string>();
    if (!check_file(cConfig.sFile2)) return 2;
    cConfig.cStreamIn2.open(cConfig.sFile2);
    if (!cConfig.cStreamIn2.is_open()) {
        std::cerr << "error: failed to open second key stream file: " << strerror(errno) << std::endl;
        return 2;
    }

    if (cVariableMap.count("skip1")) {
        cConfig.nSkip1 = cVariableMap["skip1"].as<uint64_t>();
    }
    if (cVariableMap.count("skip2")) {
        cConfig.nSkip2 = cVariableMap["skip2"].as<uint64_t>();
    }
    if (cVariableMap.count("count")) {
        cConfig.bCompareAll = false;
        cConfig.nCompare = cVariableMap["count"].as<uint64_t>();
    }
    
    return compare(cConfig, std::cout);
}

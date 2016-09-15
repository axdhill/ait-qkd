/*
 * main.cpp
 * 
 * This is the qkd key generator for entanlged discrete variables
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
#include <random>

#include <boost/program_options.hpp>

// ait
#include <qkd/key/key.h>
#include <qkd/utility/bigint.h>
#include <qkd/utility/random.h>
#include <qkd/version.h>


// ------------------------------------------------------------
// decl


/**
 * key generation config
 */
class config {

public:    
    

    /**
     * ctor
     */
    config() : nKeys(0), 
            nId(0), 
            nSize(0), 
            bRandomizeSize(false), 
            nStandardDeviation(0.0), 
            nRate(0.0) {};
    
    std::string sFile;              /**< file name */
    std::string sRandomSource;      /**< random source */
    uint64_t nKeys;                 /**< number of keys to produce */
    qkd::key::key_id nId;           /**< first key id */
    uint64_t nSize;                 /**< size of each key */
    bool bRandomizeSize;            /**< randomize the size */
    double nStandardDeviation;      /**< standard deviation when randomizing key size */
    double nRate;                   /**< error rate of each key */
    bool bSilent;                   /**< no console output */
};


// ------------------------------------------------------------
// fwd

qkd::key::key create(qkd::key::key_id nKeyId, config const & cConfig);
qkd::key::key convert_to_bob(qkd::key::key const & cKey);
void disturb(qkd::key::key & cKey, config const & cConfig);
int generate(config const & cConfig);
void show_config(config const & cConfig);
unsigned char swap_base(unsigned char nBase, double nRandom);


// ------------------------------------------------------------
// code


/**
 * create a key based on the config values
 * 
 * This creates the final, "clean" key.
 * 
 * @param   nKeyId      the new key id
 * @param   cConfig     the config values (relevant: size and zero)
 * @return  a key as specified by config
 */
qkd::key::key create(qkd::key::key_id nKeyId, config const & cConfig) {
    
    static const unsigned char g_nQuantum[4] = { 0x1, 0x02, 0x04, 0x08 };
    
    static std::random_device cRandomDevice;
    static std::mt19937 cRandomNumberGenerator(cRandomDevice());

    // prepare key memory
    uint64_t nSize = cConfig.nSize;
    if (cConfig.bRandomizeSize) {
        std::normal_distribution<double> cDistribution(cConfig.nSize, cConfig.nStandardDeviation);
        nSize = cDistribution(cRandomNumberGenerator);
    }
    qkd::utility::memory cMemory(nSize);
    
    for (uint64_t i = 0; i < cMemory.size(); ++i) {
        
        unsigned int nRandom1 = 0;
        unsigned int nRandom2 = 0;
        qkd::utility::random_source::source() >> nRandom1;
        qkd::utility::random_source::source() >> nRandom2;
        nRandom1 %= 4;
        nRandom2 %= 4;

        cMemory.get()[i] = (g_nQuantum[nRandom1] << 4) | g_nQuantum[nRandom2];
    }
    
    return qkd::key::key(nKeyId, cMemory, qkd::key::ENCODING_4_DETECTOR_CLICKS);
}


/**
 * convert an alice key to a bob key by switching the bases
 * 
 * In half the cases the bases are mismatched.
 * 
 * @param   cKey        alice key
 * @return  a key with switched bases
 */
qkd::key::key convert_to_bob(qkd::key::key const & cKey) {
    
    qkd::utility::memory cMemory(cKey.size());
    for (uint64_t i = 0; i < cMemory.size(); i++) {
        
        double nRandomLow = 0.0;
        double nRandomHigh = 0.0;
        qkd::utility::random_source::source() >> nRandomLow;
        qkd::utility::random_source::source() >> nRandomHigh;
        
        unsigned nLowerHalf = cKey.data()[i] & 0x0F;
        unsigned nUpperHalf = cKey.data()[i] & 0xF0;
        cMemory.get()[i] = swap_base(nLowerHalf, nRandomLow) | (swap_base(nUpperHalf >> 4, nRandomHigh) << 4);
    }
    
    return qkd::key::key(cKey.id(), cMemory, qkd::key::ENCODING_4_DETECTOR_CLICKS);
}


/**
 * disturb a key as specified by config
 * 
 * @param   cKey            the key to disturb
 * @param   cConfig         the config values (relevant: rate and exact)
 * @return  a disturbed key
 */
void disturb(qkd::key::key & cKey, config const & cConfig) {
    
    unsigned char * d = cKey.data().get();
    for (uint64_t i = 0; i < cKey.data().size(); ++i, ++d) {
        
        double nRandom = 0.0;
        
        qkd::utility::random_source::source() >> nRandom;
        if (nRandom <= cConfig.nRate) {
            
            switch ((*d) & 0x0F) {
            case 0x01:
                (*d) = ((*d) & 0xF0) | 0x02;
                break;
            case 0x02:
                (*d) = ((*d) & 0xF0) | 0x01;
                break;
            case 0x04:
                (*d) = ((*d) & 0xF0) | 0x08;
                break;
            case 0x08:
                (*d) = ((*d) & 0xF0) | 0x04;
                break;
            }
        }
            
        qkd::utility::random_source::source() >> nRandom;
        if (nRandom <= cConfig.nRate) {
            
            switch ((*d) & 0xF0) {
            case 0x10:
                (*d) = ((*d) & 0x0F) | 0x20;
                break;
            case 0x20:
                (*d) = ((*d) & 0x0F) | 0x10;
                break;
            case 0x40:
                (*d) = ((*d) & 0x0F) | 0x80;
                break;
            case 0x80:
                (*d) = ((*d) & 0x0F) | 0x40;
                break;
            }
        }
    }
}


/**
 * generate the keys
 * 
 * @param   cConfig     the config setting, holding all necessary data
 * @return  exitcode: 0 success, else error
 */
int generate(config const & cConfig) {
    
    if (cConfig.nRate > 1.0) {
        std::cerr << "rate is " << cConfig.nRate << " which is quite impossible to fulfill." << std::endl;
        return 1;
    }
    if (cConfig.nRate < 0.0) {
        std::cerr << "rate is " << cConfig.nRate << " which is quite impossible to fulfill." << std::endl;
        return 1;
    }
    
    // files
    std::ofstream cFileAlice(cConfig.sFile + ".alice");
    if (!cFileAlice.is_open()) {
        std::cerr << "failed to open Alice's file '" << cConfig.sFile << ".alice': " << strerror(errno) << std::endl;
        return 2;
    }
    std::ofstream cFileBob(cConfig.sFile + ".bob");
    if (!cFileBob.is_open()) {
        std::cerr << "failed to open Bob's file '" << cConfig.sFile << ".bob': " << strerror(errno) << std::endl;
        return 2;
    }

    if (!cConfig.sRandomSource.empty()) {
        qkd::utility::random cRandomSource = qkd::utility::random_source::create(cConfig.sRandomSource);
        qkd::utility::random_source::set_source(cRandomSource);
    }
    
    for (qkd::key::key_id nKeyId = cConfig.nId; nKeyId < (cConfig.nId + cConfig.nKeys); ++nKeyId) {
        
        qkd::key::key cKeyAlice = create(nKeyId, cConfig);
        qkd::key::key cKeyBob = convert_to_bob(cKeyAlice);
        disturb(cKeyBob, cConfig);
        
        cFileAlice << cKeyAlice;
        cFileBob << cKeyBob;
        
        if (!cConfig.bSilent) std::cout << "created key #" << cKeyAlice.id() << std::endl;
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
    
    std::string sApplication = std::string("qkd-key-gen - AIT QKD Test Key Generator Tool V") + qkd::version();
    std::string sDescription = std::string("\nThis lets one create a pair of key files to feed a pipeline with.\nThis tool creates keys with 4 bit detector click encoding suitable for entangled BB84.\nThese are keys for testing ONLY.\n\nCopyright 2012-2016 AIT Austrian Institute of Technology GmbH");
    std::string sSynopsis = std::string("Usage: ") + argv[0] + " [OPTIONS] FILE";
    
    boost::program_options::options_description cOptions(sApplication + "\n" + sDescription + "\n\n\t" + sSynopsis + "\n\nAllowed Options");
    cOptions.add_options()("help,h", "this page");
    cOptions.add_options()("id,i", boost::program_options::value<qkd::key::key_id>()->default_value(1), "first key id");
    cOptions.add_options()("keys,k", boost::program_options::value<uint64_t>()->default_value(10), "number of keys to produce");
    cOptions.add_options()("size,s", boost::program_options::value<uint64_t>()->default_value(1024), "number of bytes of each key to produce");
    cOptions.add_options()("randomize-size", "randomize the key size within 2% standard deviation");
    cOptions.add_options()("rate,r", boost::program_options::value<double>()->default_value(0.05, "0.05"), "error rate in each key");
    cOptions.add_options()("random-url", boost::program_options::value<std::string>()->default_value(""), "force the random number generator to use a specific algorithm.");
    cOptions.add_options()("silent", "don't be so chatty");
    cOptions.add_options()("version,v", "print version string");
    
    boost::program_options::options_description cArgs("Arguments");
    cArgs.add_options()("FILE", "FILE is the name of files to create. There will be 2 files created: \none with suffix '.alice' and one with suffix '.bob'.");
    boost::program_options::positional_options_description cPositionalDescription; 
    cPositionalDescription.add("FILE", 1);
    
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
    
    if (cVariableMap.count("FILE") != 1) {
        std::cerr << "need exactly one FILE argument" << "\ntype '--help' for help" << std::endl;
        return 1;
    }
    
    config cConfig;
    cConfig.sFile = cVariableMap["FILE"].as<std::string>();
    cConfig.nId = cVariableMap["id"].as<qkd::key::key_id>();
    cConfig.nKeys = cVariableMap["keys"].as<uint64_t>();
    cConfig.nSize = cVariableMap["size"].as<uint64_t>();
    cConfig.bRandomizeSize = (cVariableMap.count("randomize-size") > 0);
    cConfig.nStandardDeviation = sqrt(cConfig.nSize);
    cConfig.nRate = cVariableMap["rate"].as<double>();
    cConfig.bSilent = (cVariableMap.count("silent") > 0);
    cConfig.sRandomSource = cVariableMap["random-url"].as<std::string>();
    
    show_config(cConfig);
    
    return generate(cConfig);
}


/**
 * tell the config to the user
 * 
 * @param   cConfig     the config to show
 */
void show_config(config const & cConfig) {
    
    if (cConfig.bSilent) return;
    
    std::cout << "qkd key generation setting:" << std::endl;
    std::cout << "\tfile:               " << cConfig.sFile << std::endl;
    std::cout << "\trandom source:      " << cConfig.sRandomSource << std::endl;
    std::cout << "\tkeys:               " << cConfig.nKeys << std::endl;
    std::cout << "\tfirst id:           " << cConfig.nId << std::endl;
    std::cout << "\tsize:               " << cConfig.nSize << std::endl;
    std::cout << "\trandomize-size:     " << (cConfig.bRandomizeSize ? "yes" : "no") << std::endl;
    std::cout << "\trate:               " << cConfig.nRate << std::endl;
}


/**
 * swap a base randomly from alice to bob
 * 
 * @param   nBase       the base (lower 4 bits)
 * @param   nRandom     random value
 * @return  bases are swaped if random is < 0.5
 */
unsigned char swap_base(unsigned char nBase, double nRandom) {

    if (nRandom < 0.5) {
        
        // same base measurment case
        if (nBase == 0x01) return 0x02;
        if (nBase == 0x02) return 0x01;
        if (nBase == 0x04) return 0x08;
        if (nBase == 0x08) return 0x04;
    }
    
    // different base measurment
    if ((nBase == 0x01) && (nRandom < 0.75)) return 0x08;
    if ((nBase == 0x01) && (nRandom >= 0.75)) return 0x04;
    if ((nBase == 0x02) && (nRandom < 0.75)) return 0x08;
    if ((nBase == 0x02) && (nRandom >= 0.75)) return 0x04;
    if ((nBase == 0x04) && (nRandom < 0.75)) return 0x01;
    if ((nBase == 0x04) && (nRandom >= 0.75)) return 0x02;
    if ((nBase == 0x08) && (nRandom < 0.75)) return 0x01;
    if ((nBase == 0x08) && (nRandom >= 0.75)) return 0x02;
    
    return 0x00;
}

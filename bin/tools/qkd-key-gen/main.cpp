/*
 * main.cpp
 * 
 * This is the qkd key generator
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
            nRate(0.0), 
            bExact(false), 
            bZero(false), 
            bSetErrorBits(false), 
            nDisclosedRate(0.0) {};
    
    std::string sFile;              /**< file name */
    std::string sRandomSource;      /**< random source */
    uint64_t nKeys;                 /**< number of keys to produce */
    qkd::key::key_id nId;           /**< first key id */
    uint64_t nSize;                 /**< size of each key */
    bool bRandomizeSize;            /**< randomize the size */
    double nStandardDeviation;      /**< standard deviation when randomizing key size */
    double nRate;                   /**< error rate of each key */
    bool bExact;                    /**< error rate must match exactly */
    bool bZero;                     /**< start if zero key instead of random key */
    bool bSetErrorBits;             /**< set error bits in the key */
    double nDisclosedRate;          /**< set disclosed bits in the key */
    bool bSilent;                   /**< no console output */
};


// ------------------------------------------------------------
// fwd

qkd::key::key create(qkd::key::key_id nKeyId, config const & cConfig);
qkd::key::key disturb(qkd::key::key const & cKey, config const & cConfig, uint64_t & nErrorBits);
qkd::key::key disturb_exact(qkd::key::key const & cKey, config const & cConfig, uint64_t & nErrorBits);
int generate(config const & cConfig);
void show_config(config const & cConfig);


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
    
    static std::random_device cRandomDevice;
    static std::mt19937 cRandomNumberGenerator(cRandomDevice());

    // prepare key memory
    uint64_t nSize = cConfig.nSize;
    if (cConfig.bRandomizeSize) {
        std::normal_distribution<double> cDistribution(cConfig.nSize, cConfig.nStandardDeviation);
        nSize = cDistribution(cRandomNumberGenerator);
    }
    qkd::utility::memory cMemory(nSize);
    
    if (!cConfig.bZero) {
        qkd::utility::random_source::source() >> cMemory;
    }
    else {
        cMemory.fill(0);
    }
    
    return qkd::key::key(nKeyId, cMemory, qkd::key::ENCODING_SHARED_SECRET_BITS);
}


/**
 * disturb a key as specified by config
 * 
 * @param   cKey            the input key
 * @param   cConfig         the config values (relevant: rate and exact)
 * @param   nErrorBits      [out] will receive the number of error bits
 * @return  a disturbed key
 */
qkd::key::key disturb(qkd::key::key const & cKey, config const & cConfig, uint64_t & nErrorBits) {
    
    qkd::key::key cResultKey;
    
    if (cConfig.bExact) return disturb_exact(cKey, cConfig, nErrorBits);
    
    qkd::utility::bigint cBI = qkd::utility::bigint(cKey.data());
    nErrorBits = 0;
    for (uint64_t i = 0; i < cBI.bits(); ++i) {
        
        double nRandom = 0.0;
        qkd::utility::random_source::source() >> nRandom;
        if (nRandom <= cConfig.nRate) {
            cBI.set(i, !cBI.get(i));
            nErrorBits++;
        }
    }
    
    cResultKey = qkd::key::key(cKey.id(), cBI.memory(), qkd::key::ENCODING_SHARED_SECRET_BITS);
    cResultKey.set_state(cKey.state());
    
    return cResultKey;
}


/**
 * disturb a key as specified by config
 * 
 * @param   cKey            the input key
 * @param   cConfig         the config values (relevant: rate and exact)
 * @param   nErrorBits      [out] will receive the number of error bits
 * @return  a disturbed key
 */
qkd::key::key disturb_exact(qkd::key::key const & cKey, config const & cConfig, uint64_t & nErrorBits) {
    
    qkd::key::key cResultKey;
    
    qkd::utility::bigint cBI = qkd::utility::bigint(cKey.data());
    uint64_t nBitsToFlip = cBI.bits() * cConfig.nRate;
    
    // this is the idea:
    //  - we have a set of bits to be flipped
    //  - and we have a list of bits not yet touched
    //  from the list of not-yet-touched bits (possible bits)
    //  we randomly pick one and add it to the set of bits to
    //  flip.
    //
    //  advantage: picking exact bits is quite easy
    //  drawback: creating the list of possible bits is expensive
    //
    //  if we not create such possible bit list, the algorithm
    //  may find it hard to find possible not-yet-flipped bits when
    //  the error rate is rather high
    //
    // therefore: below a rate of 20% we guess the bits in a more stupid fashion
    // this might be faster than the possible bit list on low error rates.
    
    // collect bits to flip
    std::set<uint64_t> cBits;
    
    if (cConfig.nRate > 0.2) {
    
        // create the not-yet-flipped-bits 
        std::vector<uint64_t> cBitsPossible;
        for (uint64_t i = 0; i < cBI.bits(); ++i) {
            cBitsPossible.push_back(i);
        }
        
        for (uint64_t i = 0; i < nBitsToFlip; ++i) {
            
            uint64_t nPossibleIndex;
            qkd::utility::random_source::source() >> nPossibleIndex;
            nPossibleIndex %= cBitsPossible.size();
            auto iter = cBitsPossible.begin() + nPossibleIndex;
            
            cBits.insert(*iter);
            cBitsPossible.erase(iter);
        }
    }
    else {
        
        for (uint64_t i = 0; i < nBitsToFlip; ) {
            
            uint64_t nBit;
            qkd::utility::random_source::source() >> nBit;
            nBit %= cBI.bits();
            
            if (cBits.find(nBit) != cBits.end()) continue;
            
            cBits.insert(nBit);
            ++i;
        }
    }

    for (auto & iter : cBits) {
        cBI.set(iter, !cBI.get(iter));
    }
    nErrorBits = cBits.size();
    cResultKey = qkd::key::key(cKey.id(), cBI.memory(), qkd::key::ENCODING_SHARED_SECRET_BITS);
    cResultKey.set_state(cKey.state());
    
    return cResultKey;
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
        
        uint64_t nErrorBits = 0;

        qkd::key::key cKeyAlice = create(nKeyId, cConfig);
        qkd::key::key cKeyBob = disturb(cKeyAlice, cConfig, nErrorBits);
        
        if (cConfig.bSetErrorBits) {
            cKeyAlice.set_qber((double)nErrorBits / (double)(cKeyAlice.data().size() * 8));
            cKeyBob.set_qber((double)nErrorBits / (double)(cKeyBob.data().size() * 8));
        }
        
        double nDisclosedRate = cConfig.nDisclosedRate;
        if (nDisclosedRate < 0.0) nDisclosedRate = 0.0;
        if (nDisclosedRate > 1.0) nDisclosedRate = 1.0;
        cKeyAlice.set_disclosed(cKeyAlice.size() * 8 * nDisclosedRate);
        cKeyBob.set_disclosed(cKeyBob.size() * 8 * nDisclosedRate);
        
        cKeyAlice.set_encoding(qkd::key::ENCODING_SHARED_SECRET_BITS);
        cKeyBob.set_encoding(qkd::key::ENCODING_SHARED_SECRET_BITS);
        
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
    std::string sDescription = std::string("\nThis lets one create a pair of key files to feed a pipeline with.\nThese are keys for testing ONLY.\n\nCopyright 2012-2016 AIT Austrian Institute of Technology GmbH");
    std::string sSynopsis = std::string("Usage: ") + argv[0] + " [OPTIONS] FILE";
    
    boost::program_options::options_description cOptions(sApplication + "\n" + sDescription + "\n\n\t" + sSynopsis + "\n\nAllowed Options");
    cOptions.add_options()("errorbits,e", "set number error bits in the key");
    cOptions.add_options()("disclosed,d", boost::program_options::value<double>()->default_value(0.0, "0.0"), "set rate of disclosed bits in the key");
    cOptions.add_options()("help,h", "this page");
    cOptions.add_options()("id,i", boost::program_options::value<qkd::key::key_id>()->default_value(1), "first key id");
    cOptions.add_options()("keys,k", boost::program_options::value<uint64_t>()->default_value(10), "number of keys to produce");
    cOptions.add_options()("size,s", boost::program_options::value<uint64_t>()->default_value(1024), "number of bytes of each key to produce");
    cOptions.add_options()("randomize-size", "randomize the key size within 2% standard deviation");
    cOptions.add_options()("rate,r", boost::program_options::value<double>()->default_value(0.05, "0.05"), "error rate in each key");
    cOptions.add_options()("random-url", boost::program_options::value<std::string>()->default_value(""), "force the random number generator to use a specific algorithm.");
    cOptions.add_options()("silent", "don't be so chatty");
    cOptions.add_options()("version,v", "print version string");
    cOptions.add_options()("exact,x", "produce exact amount of errors");
    cOptions.add_options()("zero,z", "instead of random bits, start with all 0");
    
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
    cConfig.bExact = (cVariableMap.count("exact") > 0);
    cConfig.bZero = (cVariableMap.count("zero") > 0);
    cConfig.bSetErrorBits = (cVariableMap.count("errorbits") > 0);
    cConfig.nDisclosedRate = cVariableMap["disclosed"].as<double>();
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
    std::cout << "\texact:              " << cConfig.bExact << std::endl;
    std::cout << "\tzero:               " << cConfig.bZero << std::endl;
    std::cout << "\tset error bits:     " << cConfig.bSetErrorBits << std::endl;
    std::cout << "\tdisclosed bit rate: " << cConfig.nDisclosedRate << std::endl;
}

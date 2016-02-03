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
    config() : nKeys(0), nId(0), nSize(0), bRandomizeSize(false), nStandardDeviation(0.0), nRate(0.0), bExact(false), bZero(false), bSetErrorBits(false), nDisclosedRate(0.0), bQuantumTables(false) {};
    
    std::string sFile;              /**< file name */
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
    bool bQuantumTables;            /**< create quantum tables instead of key material */
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
    
    // normal key data
    if (!cConfig.bQuantumTables) {
        if (!cConfig.bZero) qkd::utility::random_source::source() >> cMemory;
        else cMemory.fill(0);
    }
    else {
        
        // quantum tables
        for (uint64_t i = 0; i < cMemory.size(); i++) {
            
            unsigned int nRandom1 = 0;
            unsigned int nRandom2 = 0;
            qkd::utility::random_source::source() >> nRandom1;
            qkd::utility::random_source::source() >> nRandom2;
            nRandom1 %= 4;
            nRandom2 %= 4;

            cMemory.get()[i] = (g_nQuantum[nRandom1] << 4) | g_nQuantum[nRandom2];
        }
    }
    
    return qkd::key::key(nKeyId, cMemory);
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
    
    // when exact, we flip a concrete number of bits
    if (cConfig.bExact) return disturb_exact(cKey, cConfig, nErrorBits);
    
    // normal keys or quantum tables
    if (!cConfig.bQuantumTables) {
    
        // extract a bigint
        qkd::utility::bigint cBI = qkd::utility::bigint(cKey.data());

        // walk over all bits
        nErrorBits = 0;
        for (uint64_t i = 0; i < cBI.bits(); i++) {
            
            double nRandom = 0.0;
            qkd::utility::random_source::source() >> nRandom;
            
            // flip or no flip?
            if (nRandom <= cConfig.nRate) {
                cBI.set(i, !cBI.get(i));
                nErrorBits++;
            }
        }
        
        // create the key with the same id but from the disturbed bigint
        cResultKey = qkd::key::key(cKey.id(), cBI.memory());
    }
    else {

        // quantum tables: copy the quantum events accordingly
        qkd::utility::memory cMemory(cKey.size());
        for (uint64_t i = 0; i < cMemory.size(); i++) {
            
            unsigned char nValue = 0;
            unsigned nLowerHalf = cKey.data()[i] & 0x0F;
            unsigned nUpperHalf = cKey.data()[i] & 0xF0;
            
            // map alice's detector clicks to bob's
            if (nLowerHalf == 0x01) nValue = 0x02;
            if (nLowerHalf == 0x02) nValue = 0x01;
            if (nLowerHalf == 0x04) nValue = 0x08;
            if (nLowerHalf == 0x08) nValue = 0x04;
            
            cMemory.get()[i] = nValue;
            
            // map alice's detector clicks to bob's
            if (nUpperHalf == 0x10) nValue = 0x20;
            if (nUpperHalf == 0x20) nValue = 0x10;
            if (nUpperHalf == 0x40) nValue = 0x80;
            if (nUpperHalf == 0x80) nValue = 0x40;
            
            cMemory.get()[i] |= nValue;
        }
        
        // walk over all event-doubles
        nErrorBits = 0;
        for (uint64_t i = 0; i < cMemory.size(); i++) {
            
            // an error has any bits set (or unset)
            double nRandom = 0.0;
            
            // lower half of byte
            qkd::utility::random_source::source() >> nRandom;
            if (nRandom <= cConfig.nRate) {
                
                unsigned char nValue;
                qkd::utility::random_source::source() >> nValue;
                nValue &= 0x0F;
                cMemory.get()[i] = cMemory.get()[i] & 0xF0;
                cMemory.get()[i] |= nValue;
                
                nErrorBits++;
            }
            
            // upper half of byte
            qkd::utility::random_source::source() >> nRandom;
            if (nRandom <= cConfig.nRate) {
                
                unsigned char nValue;
                qkd::utility::random_source::source() >> nValue;
                nValue &= 0xF0;
                cMemory.get()[i] = cMemory.get()[i] & 0x0F;
                cMemory.get()[i] |= nValue;
                
                nErrorBits++;
            }
        }
        
        cResultKey = qkd::key::key(cKey.id(), cMemory);
    }

    // get the state
    cResultKey.meta().eKeyState = cKey.meta().eKeyState;
    
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
    
    // extract a bigint
    qkd::utility::bigint cBI = qkd::utility::bigint(cKey.data());
    if (cConfig.bQuantumTables) {
        cBI = qkd::utility::bigint(cKey.size() * 2);
        
        // quantum tables: copy the quantum events accordingly
        qkd::utility::memory cMemory(cKey.size());
        for (uint64_t i = 0; i < cMemory.size(); i++) {
            
            unsigned char nValue = 0;
            unsigned nLowerHalf = cKey.data()[i] & 0x0F;
            unsigned nUpperHalf = cKey.data()[i] & 0xF0;
            
            if (nLowerHalf == 0x01) nValue = 0x02;
            if (nLowerHalf == 0x02) nValue = 0x01;
            if (nLowerHalf == 0x04) nValue = 0x08;
            if (nLowerHalf == 0x08) nValue = 0x04;
            
            cMemory.get()[i] = nValue;
            
            if (nUpperHalf == 0x10) nValue = 0x20;
            if (nUpperHalf == 0x20) nValue = 0x10;
            if (nUpperHalf == 0x40) nValue = 0x80;
            if (nUpperHalf == 0x80) nValue = 0x40;
            
            cMemory.get()[i] |= nValue;
        }
        
        cResultKey = qkd::key::key(cKey.id(), cMemory);
    }
    
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
        for (uint64_t i = 0; i < cBI.bits(); i++) cBitsPossible.push_back(i);
        
        // fetch bits
        for (uint64_t i = 0; i < nBitsToFlip; i++) {
            
            // choose a bit
            uint64_t nPossibleIndex;
            qkd::utility::random_source::source() >> nPossibleIndex;
            nPossibleIndex %= cBitsPossible.size();
            auto iter = cBitsPossible.begin() + nPossibleIndex;
            
            // put the bits into the set and out of the list
            cBits.insert(*iter);
            cBitsPossible.erase(iter);
        }
    }
    else {
        
        // fetch bits (more stupid)
        for (uint64_t i = 0; i < nBitsToFlip; ) {
            
            // choose a bit
            uint64_t nBit;
            qkd::utility::random_source::source() >> nBit;
            nBit %= cBI.bits();
            
            // if we chose that in the past ... retry
            if (cBits.find(nBit) != cBits.end()) continue;
            
            // put the bits into the set
            cBits.insert(nBit);
            i++;
        }
    }

    
    // now the cBits set holds the positions to flip
        
    // walk over the set of bits and flip them
    for (auto & iter : cBits) {
        
        if (!cConfig.bQuantumTables) cBI.set(iter, !cBI.get(iter));
        else {
            
            // quantum table mode
            unsigned char nValue;
            qkd::utility::random_source::source() >> nValue;
            nValue &= 0x0F;
            
            uint64_t nPosition = iter / 2;
            if (iter % 2) {
                
                // upper half
                cResultKey.data()[nPosition] = (nValue << 4) | (cResultKey.data()[nPosition] & 0x0F);
            }
            else {
                
                // lower half
                cResultKey.data()[nPosition] = (cResultKey.data()[nPosition] & 0xF0) | nValue;
            }
        }
    }
    
    // record number of errors
    nErrorBits = cBits.size();
    
    if (!cConfig.bQuantumTables) cResultKey = qkd::key::key(cKey.id(), cBI.memory());

    // get the state
    cResultKey.meta().eKeyState = cKey.meta().eKeyState;
    
    // create the key with the same id but from the bigint
    return cResultKey;
}


/**
 * generate the keys
 * 
 * @param   cConfig     the config setting, holding all necessary data
 * @return  exitcode: 0 success, else error
 */
int generate(config const & cConfig) {
    
    // sanity checks
    if (cConfig.nRate > 1.0) {
        std::cerr << "rate is " << cConfig.nRate << " which is quite impossible to fullfill." << std::endl;
        return 1;
    }
    if (cConfig.nRate < 0.0) {
        std::cerr << "rate is " << cConfig.nRate << " which is quite impossible to fullfill." << std::endl;
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
    
    // generate key by key
    for (qkd::key::key_id nKeyId = cConfig.nId; nKeyId < (cConfig.nId + cConfig.nKeys); nKeyId++) {
        
        // generation
        uint64_t nErrorBits = 0;
        
        // create alice's key
        qkd::key::key cKeyAlice = create(nKeyId, cConfig);
    
        // bob's key is a disturbed version of alice's key
        qkd::key::key cKeyBob = disturb(cKeyAlice, cConfig, nErrorBits);
        
        // in quantum table mode we have to artificially introduce about 50% error
        // as this is expected due to wrong basis
        if (cConfig.bQuantumTables) {
            
            for (uint64_t i = 0; i < cConfig.nSize; i++) {
                
                double nRandom = 0.0;

                unsigned nLowerHalf = cKeyAlice.data()[i] & 0x0F;
                unsigned nUpperHalf = cKeyAlice.data()[i] & 0xF0;
                
                // lower half
                qkd::utility::random_source::source() >> nRandom;
                if (nRandom > 0.5) {
                    
                    char nRandomClick = 0;
                    qkd::utility::random_source::source() >> nRandomClick;
                    nRandomClick = nRandomClick & 0x0F;
                    
                    // do not pick the same base but a different click (any)
                    cKeyBob.data()[i] = cKeyBob.data()[i] | nRandomClick;
                    if (nLowerHalf & 0x03) cKeyBob.data()[i] = cKeyBob.data()[i] & ~0x03;
                    if (nLowerHalf & 0x0C) cKeyBob.data()[i] = cKeyBob.data()[i] & ~0x0C;
                }
                
                // upper half
                qkd::utility::random_source::source() >> nRandom;
                if (nRandom > 0.5) {
                    
                    char nRandomClick = 0;
                    qkd::utility::random_source::source() >> nRandomClick;
                    nRandomClick = nRandomClick & 0xF0;
                    
                    // do not pick the same base but a different click (any)
                    cKeyBob.data()[i] = cKeyBob.data()[i] | nRandomClick;
                    if (nUpperHalf & 0x30) cKeyBob.data()[i] = cKeyBob.data()[i] & ~0x30;
                    if (nUpperHalf & 0xC0) cKeyBob.data()[i] = cKeyBob.data()[i] & ~0xC0;
                }
            }
        }
        
        // check for setting error bits
        if (cConfig.bSetErrorBits) {
            cKeyAlice.meta().nErrorRate = (double)nErrorBits / (double)(cKeyAlice.data().size() * 8);
            cKeyBob.meta().nErrorRate = (double)nErrorBits / (double)(cKeyBob.data().size() * 8);
        }
        
        // set disclosed bits
        double nDisclosedRate = cConfig.nDisclosedRate;
        if (nDisclosedRate < 0.0) nDisclosedRate = 0.0;
        if (nDisclosedRate > 1.0) nDisclosedRate = 1.0;
        cKeyAlice.meta().nDisclosedBits = cKeyAlice.size() * 8 * nDisclosedRate;
        cKeyBob.meta().nDisclosedBits = cKeyBob.size() * 8 * nDisclosedRate;
        
        // write to file
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
    
    // create the command line header
    std::string sApplication = std::string("qkd-key-gen - AIT QKD Test Key Generator Tool V") + VERSION;
    std::string sDescription = std::string("\nThis lets one create a pair of key files to feed a pipeline with.\nThese are keys for testing ONLY.\n\nCopyright 2012-2016 AIT Austrian Institute of Technology GmbH");
    std::string sSynopsis = std::string("Usage: ") + argv[0] + " [OPTIONS] FILE";
    
    // define program options
    boost::program_options::options_description cOptions(sApplication + "\n" + sDescription + "\n\n\t" + sSynopsis + "\n\nAllowed Options");
    cOptions.add_options()("errorbits,e", "set number error bits in the key");
    cOptions.add_options()("disclosed,d", boost::program_options::value<double>()->default_value(0.0, "0.0"), "set rate of dislosed bits in the key");
    cOptions.add_options()("help,h", "this page");
    cOptions.add_options()("id,i", boost::program_options::value<qkd::key::key_id>()->default_value(1), "first key id");
    cOptions.add_options()("keys,k", boost::program_options::value<uint64_t>()->default_value(10), "number of keys to produce");
    cOptions.add_options()("size,s", boost::program_options::value<uint64_t>()->default_value(1024), "number of bytes of each key to produce");
    cOptions.add_options()("randomize-size", "randomize the key size within 2% standard deviation");
    cOptions.add_options()("rate,r", boost::program_options::value<double>()->default_value(0.05, "0.05"), "error rate in each key");
    cOptions.add_options()("quantum,q", "create quantum detector tables as key material (whereas 1 byte holds 2 events which are 2 key bits)");
    cOptions.add_options()("silent", "don't be see chatty");
    cOptions.add_options()("version,v", "print version string");
    cOptions.add_options()("exact,x", "produce exact amount of errors");
    cOptions.add_options()("zero,z", "instead of random bits, start with all 0");
    
    // final arguments
    boost::program_options::options_description cArgs("Arguments");
    cArgs.add_options()("FILE", "FILE is the name of files to create. There will be 2 files created: \none with suffix '.alice' and one with suffix '.bob'. \n\nWhen creating quantum tables the --errorbits and --dislosed flags are ignored.");
    boost::program_options::positional_options_description cPositionalDescription; 
    cPositionalDescription.add("FILE", 1);
    
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
    
    // we need a file
    if (cVariableMap.count("FILE") != 1) {
        std::cerr << "need excactly one FILE argument" << "\ntype '--help' for help" << std::endl;
        return 1;
    }
    
    // construct the config
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
    cConfig.bQuantumTables = (cVariableMap.count("quantum") > 0);
    cConfig.bSilent = (cVariableMap.count("silent") > 0);
    
    // show config to user
    show_config(cConfig);
    
    // on with it
    return generate(cConfig);
}


/**
 * tell the config to the user
 * 
 * @param   cConfig     the config to show
 */
void show_config(config const & cConfig) {
    
    if (cConfig.bSilent) return;
    
    std::cout << "qkd key generation setting: \n";
    std::cout << "\tfile:              " << cConfig.sFile << "\n";
    std::cout << "\tkeys:              " << cConfig.nKeys << "\n";
    std::cout << "\tfirst id:          " << cConfig.nId << "\n";
    std::cout << "\tsize:              " << cConfig.nSize << "\n";
    std::cout << "\trandomize-size:    " << (cConfig.bRandomizeSize ? "yes" : "no") << "\n";
    std::cout << "\trate:              " << cConfig.nRate << "\n";
    std::cout << "\texact:             " << cConfig.bExact << "\n";
    std::cout << "\tzero:              " << cConfig.bZero << "\n";
    std::cout << "\tset error bits:    " << cConfig.bSetErrorBits << "\n";
    std::cout << "\tdislosed bit rate: " << cConfig.nDisclosedRate << "\n";
    std::cout << "\tquantum:           " << cConfig.bQuantumTables << std::endl;
}

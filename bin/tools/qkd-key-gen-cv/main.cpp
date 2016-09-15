/*
 * main.cpp
 * 
 * This is the qkd key generator for continuous variables
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
#include <memory>
#include <random>

#include <boost/program_options.hpp>

// ait
#include <qkd/key/key.h>
#include <qkd/utility/bigint.h>
#include <qkd/utility/random.h>
#include <qkd/version.h>

#include "entangled.h"
#include "prepare_measure_homodyne.h"
#include "prepare_measure_heterodyne.h"

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
    config() {
        nKeys = 0; 
        nId = 0; 
        nSize = 0; 
        bRandomizeSize = false; 
        nSizeStandardDeviation = 0.0; 
//         nSigmaAlice = 1.0;
//         nSigmaNoise = 1.0;
//         nTranspose = 0.8;
//         nSNR = 1.0;
        bSilent = false;
    }
    
    std::string sFile;              /**< file name */
    std::string sRandomSource;      /**< random source */
    uint64_t nKeys;                 /**< number of keys to produce */
    qkd::key::key_id nId;           /**< first key id */
    uint64_t nSize;                 /**< size of each key */
    bool bRandomizeSize;            /**< randomize the size */
    double nSizeStandardDeviation;  /**< standard deviation when randomizing key size */
    
//     double nSigmaAlice;             /**< sigma alice */
//     double nSigmaNoise;             /**< sigma noise */
//     double nTranspose;              /**< transpose */
//     double nSNR;                    /**< signal noise ratio */
//     
//     double nSigmaAlicePOW2;         /**< (sigma alice)^2 */
//     double nSigmaNoisePOW2;         /**< (sigma noise)^2 */
    
    bool bSilent;                   /**< no console output */
};


/**
 * helper struct for base and float key data encoding
 */
typedef struct {
    uint32_t nBase;         /**< base value */
    float nMeasurement;     /**< measurement */
} base_and_float;


// ------------------------------------------------------------
// fwd

void create(qkd::key::key & cKeyAlice, qkd::key::key & cKeyBob, config const & cConfig);
// qkd::key::key convert_to_bob(qkd::key::key const & cKey);
// void disturb(qkd::key::key & cKey, config const & cConfig);
// int generate(config const & cConfig);
void show_config(config const & cConfig);
// unsigned char swap_base(unsigned char nBase, double nRandom);


// ------------------------------------------------------------
// code


/**
 * create a pair of key data
 * 
 * @param   cKeyAlice   the alice key
 * @param   cKeyBob     the bob key
 * @param   cConfig     the config values
 */
void create(qkd::key::key & cKeyAlice, qkd::key::key & cKeyBob, config const & cConfig) {
    
    static std::random_device cRandomDevice;
    static std::mt19937 cRandomNumberGenerator(cRandomDevice());

    // prepare key memory
    uint64_t nSize = cConfig.nSize;
    if (cConfig.bRandomizeSize) {
        std::normal_distribution<double> cDistribution(cConfig.nSize, cConfig.nSizeStandardDeviation);
        nSize = cDistribution(cRandomNumberGenerator);
    }
    qkd::utility::memory cMemoryAlice(nSize * sizeof(base_and_float));
    UNUSED base_and_float * d_alice = reinterpret_cast<base_and_float *>(cMemoryAlice.get());
    qkd::utility::memory cMemoryBob(nSize * sizeof(base_and_float));
    UNUSED base_and_float * d_bob = reinterpret_cast<base_and_float *>(cMemoryBob.get());
    
    
    
    cKeyAlice.data() = cMemoryAlice;
    cKeyBob.data() = cMemoryBob;
}


/**
 * convert an alice key to a bob key by switching the bases
 * 
 * In half the cases the bases are mismatched.
 * 
 * @param   cKey        alice key
 * @return  a key with switched bases
 */
// qkd::key::key convert_to_bob(qkd::key::key const & cKey) {
//     
//     qkd::utility::memory cMemory(cKey.size());
//     for (uint64_t i = 0; i < cMemory.size(); i++) {
//         
//         double nRandomLow = 0.0;
//         double nRandomHigh = 0.0;
//         qkd::utility::random_source::source() >> nRandomLow;
//         qkd::utility::random_source::source() >> nRandomHigh;
//         
//         unsigned nLowerHalf = cKey.data()[i] & 0x0F;
//         unsigned nUpperHalf = cKey.data()[i] & 0xF0;
//         cMemory.get()[i] = swap_base(nLowerHalf, nRandomLow) | (swap_base(nUpperHalf >> 4, nRandomHigh) << 4);
//     }
//     
//     return qkd::key::key(cKey.id(), cMemory, qkd::key::ENCODING_4_DETECTOR_CLICKS);
// }


/**
 * disturb a key as specified by config
 * 
 * @param   cKey            the key to disturb
 * @param   cConfig         the config values (relevant: rate and exact)
 * @return  a disturbed key
 */
// void disturb(qkd::key::key & cKey, config const & cConfig) {
//     
//     unsigned char * d = cKey.data().get();
//     for (uint64_t i = 0; i < cKey.data().size(); ++i, ++d) {
//         
//         double nRandom = 0.0;
//         
//         qkd::utility::random_source::source() >> nRandom;
//         if (nRandom <= cConfig.nRate) {
//             
//             switch ((*d) & 0x0F) {
//             case 0x01:
//                 (*d) = ((*d) & 0xF0) | 0x02;
//                 break;
//             case 0x02:
//                 (*d) = ((*d) & 0xF0) | 0x01;
//                 break;
//             case 0x04:
//                 (*d) = ((*d) & 0xF0) | 0x08;
//                 break;
//             case 0x08:
//                 (*d) = ((*d) & 0xF0) | 0x04;
//                 break;
//             }
//         }
//             
//         qkd::utility::random_source::source() >> nRandom;
//         if (nRandom <= cConfig.nRate) {
//             
//             switch ((*d) & 0xF0) {
//             case 0x10:
//                 (*d) = ((*d) & 0x0F) | 0x20;
//                 break;
//             case 0x20:
//                 (*d) = ((*d) & 0x0F) | 0x10;
//                 break;
//             case 0x40:
//                 (*d) = ((*d) & 0x0F) | 0x80;
//                 break;
//             case 0x80:
//                 (*d) = ((*d) & 0x0F) | 0x40;
//                 break;
//             }
//         }
//     }
// }


/**
 * generate the keys
 * 
 * @param   cConfig     the config setting, holding all necessary data
 * @return  exitcode: 0 success, else error
 */
// int generate(config const & cConfig) {
//     
//     // files
//     std::ofstream cFileAlice(cConfig.sFile + ".alice");
//     if (!cFileAlice.is_open()) {
//         std::cerr << "failed to open Alice's file '" << cConfig.sFile << ".alice': " << strerror(errno) << std::endl;
//         return 2;
//     }
//     std::ofstream cFileBob(cConfig.sFile + ".bob");
//     if (!cFileBob.is_open()) {
//         std::cerr << "failed to open Bob's file '" << cConfig.sFile << ".bob': " << strerror(errno) << std::endl;
//         return 2;
//     }
// 
//     if (!cConfig.sRandomSource.empty()) {
//         qkd::utility::random cRandomSource = qkd::utility::random_source::create(cConfig.sRandomSource);
//         qkd::utility::random_source::set_source(cRandomSource);
//     }
//     
//     for (qkd::key::key_id nKeyId = cConfig.nId; nKeyId < (cConfig.nId + cConfig.nKeys); ++nKeyId) {
//         
//         qkd::key::key cKeyAlice(nKeyId, qkd::utility::memory(0), qkd::key::ENCODING_BASE_FLOAT);
//         qkd::key::key cKeyBob(nKeyId, qkd::utility::memory(0), qkd::key::ENCODING_BASE_FLOAT);
//         create(cKeyAlice, cKeyBob, cConfig);
//         
// //         disturb(cKeyBob, cConfig);
//         
//         cFileAlice << cKeyAlice;
//         cFileBob << cKeyBob;
//         
//         if (!cConfig.bSilent) std::cout << "created key #" << cKeyAlice.id() << std::endl;
//     }
//     
//     return 0;        
// }


/**
 * start
 * 
 * @param   argc        as usual
 * @param   argv        as usual
 * @return  as usual
 */
int main(int argc, char ** argv) {
    
    std::string sApplication = std::string("qkd-key-gen - AIT QKD Test Key Generator Tool V") + qkd::version();
    std::string sDescription = std::string("\nThis lets one create a pair of key files to feed a pipeline with.\nThis tool creates keys with base and float values suitable for continuous variables QKD.\nThese are keys for testing ONLY.\n\nCopyright 2012-2016 AIT Austrian Institute of Technology GmbH");
    std::string sSynopsis = std::string("Usage: ") + argv[0] + " [OPTIONS] FILE";
    
    boost::program_options::options_description cOptions(sApplication + "\n" + sDescription + "\n\n\t" + sSynopsis + "\n\nAllowed Options");
    cOptions.add_options()("help,h", "this page");
    cOptions.add_options()("id,i", boost::program_options::value<qkd::key::key_id>()->default_value(1), "first key id");
    cOptions.add_options()("keys,k", boost::program_options::value<uint64_t>()->default_value(10), "number of keys to produce");
    cOptions.add_options()("size,s", boost::program_options::value<uint64_t>()->default_value(1024), "number of events for each key to produce");
    cOptions.add_options()("randomize-size", "randomize the key size within 2% standard deviation");
    cOptions.add_options()("random-url", boost::program_options::value<std::string>()->default_value(""), "force the random number generator to use a specific algorithm.");
    
    cOptions.add_options()("mode", boost::program_options::value<std::string>(), "continuous key generation mode");
    
    cOptions.add_options()("sigma-alice-q", boost::program_options::value<double>(), "standard deviation for alice measurements in Q");
    cOptions.add_options()("sigma-alice-p", boost::program_options::value<double>(), "standard deviation for alice measurements in P");
    cOptions.add_options()("sigma-bob-q", boost::program_options::value<double>(), "standard deviation for bob measurements in Q");
    cOptions.add_options()("sigma-bob-p", boost::program_options::value<double>(), "standard deviation for bob measurements in P");
    cOptions.add_options()("sigma-noise-q", boost::program_options::value<double>(), "standard deviation for noise in Q");
    cOptions.add_options()("sigma-noise-p", boost::program_options::value<double>(), "standard deviation for noise in P");
    
    cOptions.add_options()("transmission", boost::program_options::value<double>()->default_value(0.9, "0.9"), "transmission value for bob's measurements");
    cOptions.add_options()("rho", boost::program_options::value<double>()->default_value(0.9, "0.9"), "correlation coefficient");
    cOptions.add_options()("snr", boost::program_options::value<double>(), "signal noise ratio");
    
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
        std::cout << "Keys are created according to different key generation modes." << std::endl;
        std::cout << "The following modes are knows:\n" << std::endl;
        std::cout << qkd::cv::entangled::help() << "\n" << std::endl;      
        std::cout << qkd::cv::prepare_measure_homodyne::help() << "\n" << std::endl;      
        std::cout << qkd::cv::prepare_measure_heterodyne::help() << "\n" << std::endl;      
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
    cConfig.nSizeStandardDeviation = sqrt(cConfig.nSize);
    cConfig.bSilent = (cVariableMap.count("silent") > 0);
    cConfig.sRandomSource = cVariableMap["random-url"].as<std::string>();
    
    if (cVariableMap.count("mode") != 1) {
        std::cerr << "please specify one valid key generation mode." << std::endl;
        return 1;
    }
    
    std::shared_ptr<qkd::cv::mode> cMode;
    if (cVariableMap["mode"].as<std::string>() == "entangled") {
        cMode = std::shared_ptr<qkd::cv::mode>(new qkd::cv::entangled);
    }
    if (cVariableMap["mode"].as<std::string>() == "prepare_measure_homodyne") {
        cMode = std::shared_ptr<qkd::cv::mode>(new qkd::cv::prepare_measure_homodyne);
    }
    if (cVariableMap["mode"].as<std::string>() == "prepare_measure_heterodyne") {
        cMode = std::shared_ptr<qkd::cv::mode>(new qkd::cv::prepare_measure_heterodyne);
    }
    if (!cMode) {
        std::cerr << "unknown generation mode." << std::endl;
        return 1;
    }
    
    cMode->consume_arguments(cVariableMap);
    
    
    
/*    
    
//     cConfig.nTranspose = cVariableMap["transpose"].as<double>();
    
    
    
    unsigned int nPresentArguments = 0;
    
    bool bSigmaAlicePresent = (cVariableMap.count("sigma-alice") > 0);
    if (bSigmaAlicePresent) {
        cConfig.nSigmaAlice = cVariableMap["sigma-alice"].as<double>();
        if (cConfig.nSigmaAlice <= 0.0) {
            std::cerr << "sigma-alice must be greater than 0." << std::endl;
            return 1;
        }
        cConfig.nSigmaAlicePOW2 = cConfig.nSigmaAlice * cConfig.nSigmaAlice;
        ++nPresentArguments;
    }
    
    bool bSigmaNoisePresent = (cVariableMap.count("sigma-noise") > 0);
    if (bSigmaNoisePresent) {
        cConfig.nSigmaNoise = cVariableMap["sigma-noise"].as<double>();
        if (cConfig.nSigmaNoise <= 0.0) {
            std::cerr << "sigma-noise must be greater than 0." << std::endl;
            return 1;
        }
        cConfig.nSigmaNoisePOW2 = cConfig.nSigmaNoise * cConfig.nSigmaNoise;
        ++nPresentArguments;
    }
    
    bool bSNRPresent = (cVariableMap.count("snr") > 0);
    if (bSNRPresent) {
        cConfig.nSNR = cVariableMap["snr"].as<double>();
        if (cConfig.nSNR <= 0.0) {
            std::cerr << "signal noise ratio must be greater than 0." << std::endl;
            return 1;
        }
        ++nPresentArguments;
    }
    
    if (nPresentArguments != 2) {
        
        if (nPresentArguments == 3) {
            std::cerr << "From sigma-alice, sigma-noise and snr are all 3 present, which is invalid.\n";
            std::cerr << "Please choose exactly two out of this set.\n";
            return 1;
        }
        
        std::cerr << "From sigma-alice, sigma-noise and snr too few are present, which is invalid.\n";
        std::cerr << "Please choose exactly two out of this set.\n";
        return 1;
    }
    if (!bSigmaAlicePresent) {
        cConfig.nSigmaAlicePOW2 = cConfig.nSNR * cConfig.nSigmaNoisePOW2;
        cConfig.nSigmaAlice = sqrt(cConfig.nSigmaAlicePOW2);
    }
    if (!bSigmaNoisePresent) {
        cConfig.nSigmaNoisePOW2 = cConfig.nSigmaAlicePOW2 / cConfig.nSNR;
        cConfig.nSigmaNoise = sqrt(cConfig.nSigmaNoisePOW2);
    }
    if (!bSNRPresent) {
        cConfig.nSNR = cConfig.nSigmaAlicePOW2 / cConfig.nSigmaNoisePOW2;
    }*/
    
    show_config(cConfig);
    
//     return generate(cConfig);
    return 0;
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
//     std::cout << "\tsigma alice:        " << cConfig.nSigmaAlice << std::endl;
//     std::cout << "\t(sigma alice)^2:    " << cConfig.nSigmaAlicePOW2 << std::endl;
//     std::cout << "\tsigma noise:        " << cConfig.nSigmaNoise << std::endl;
//     std::cout << "\t(sigma noise)^2:    " << cConfig.nSigmaNoisePOW2 << std::endl;
//     std::cout << "\ttranspose:          " << cConfig.nTranspose << std::endl;
//     std::cout << "\tsignal noise ratio: " << cConfig.nSNR << std::endl;
    
    
    
}

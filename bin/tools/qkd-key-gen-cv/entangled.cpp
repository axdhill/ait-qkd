/*
 * entangled.cpp
 * 
 * continue creation for entangled data
 *
 * Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *
 * Copyright (C) 2016 AIT Austrian Institute of Technology
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
#include <random>
#include <sstream>

// ait
#include <qkd/utility/random.h>

#include "entangled.h"

using namespace qkd::cv;


// ------------------------------------------------------------
// decl


/**
 * helper struct for key data
 */
typedef struct {
    
    uint32_t nBase;
    float nMeasurement;
    
} base_and_float;


// ------------------------------------------------------------
// code


/**
 * add program arguments to our mode settings
 * 
 * @param   cArguments      the arguments as passed from the command line
 * @return  true, if all arguments are ok
 */
bool entangled::consume_arguments(boost::program_options::variables_map const & cArguments) {
    
    if (cArguments.count("sigma-alice-q") < 1) {
        std::cerr << "missing sigma-alice-q" << std::endl;
        return false;
    }
    m_nSigmaAliceQ = cArguments["sigma-alice-q"].as<double>();
    if (m_nSigmaAliceQ <= 0.0) {
        std::cerr << "sigma-alice-q must not be less or euqal than 0.0" << std::endl;
        return false;
    }
    
    if (cArguments.count("sigma-alice-p") < 1) {
        std::cerr << "missing sigma-alice-p" << std::endl;
        return false;
    }
    m_nSigmaAliceP = cArguments["sigma-alice-p"].as<double>();
    if (m_nSigmaAliceP <= 0.0) {
        std::cerr << "sigma-alice-p must not be less or euqal than 0.0" << std::endl;
        return false;
    }
    
    if (cArguments.count("sigma-bob-q") < 1) {
        std::cerr << "missing sigma-bob-q" << std::endl;
        return false;
    }
    m_nSigmaBobQ = cArguments["sigma-bob-q"].as<double>();
    if (m_nSigmaBobQ <= 0.0) {
        std::cerr << "sigma-bob-q must not be less or euqal than 0.0" << std::endl;
        return false;
    }
    
    if (cArguments.count("sigma-bob-p") < 1) {
        std::cerr << "missing sigma-bob-p" << std::endl;
        return false;
    }
    m_nSigmaBobP = cArguments["sigma-bob-p"].as<double>();
    if (m_nSigmaBobP <= 0.0) {
        std::cerr << "sigma-bob-p must not be less or euqal than 0.0" << std::endl;
        return false;
    }
    
    if (cArguments.count("rho") < 1) {
        std::cerr << "missing rho" << std::endl;
        return false;
    }
    m_nRho = cArguments["rho"].as<double>();
    if ((m_nRho < 0.0) || (m_nRho > 1.0)) {
        std::cerr << "rho must be between 0.0 and 1.0" << std::endl;
        return false;
    }
    m_nSqrt_1Rho2 = sqrt(1.0 - (m_nRho * m_nRho));
    
    return true;
}


/**
 * dump a string about the mode's configuration
 * 
 * @return  a string describing the mode's settings
 */
std::string entangled::dump_configuration() const {
    
    std::stringstream ss;
    
    ss << "\tsigma alice Q:      " << m_nSigmaAliceQ << "\n";
    ss << "\tsigma alice P:      " << m_nSigmaAliceP << "\n";
    ss << "\tsigma bob Q:        " << m_nSigmaBobQ << "\n";
    ss << "\tsigma bob P:        " << m_nSigmaBobP << "\n";
    ss << "\trho:                " << m_nRho << "\n";
    
    return ss.str();
}


/**
 * report some help on this key generation mode
 * 
 * @return  help shown to the user on stdout about this mode
 */
std::string entangled::help() {
    
    std::stringstream ss;
    
    ss << "mode: 'entangled'\n";
    ss << "This mode creates a pair of entangled CV pseudo keys.\n";
    ss << "The resulting keys do have ENCODING_BASE_FLOAT encoding: the \n";
    ss << "first 32 bits hold either Q (== 0) or P (== 1) and the next \n";
    ss << "32 bits hold the measurment.\n\n";
    
    ss << "Parameters needed for mode 'entangled':\n";
    ss << "    --sigma-alice-q\n";
    ss << "    --sigma-alice-p\n";
    ss << "    --sigma-bob-q\n";
    ss << "    --sigma-bob-p\n";
    ss << "    --rho";
    
    return ss.str();
}


/**
 * init the genration mode
 * 
 * this inits the generation mode, adds all necessary precalculated values
 * after argument consumation
 */
void entangled::init() {
    
    // the c++11 normal_distribution needs a UniformRandomBitGenerator
    // as input to draw random numbers
    // we do currently not support this directly
    // so we simply seed the standard mt19937 generator
    // with a value from our own random number genrator
    double nSeed = 0.0;
    qkd::utility::random_source::source() >> nSeed;
    m_cRandomGenerator = std::mt19937(nSeed);
}


/**
 * produce a set of pseudo random cv-keys
 * 
 * @param   cKeyAlice       alice key
 * @param   cKeyBob         bob key
 * @param   nEvents         number of events for each key 
 */
void entangled::produce(qkd::key::key & cKeyAlice, qkd::key::key & cKeyBob, uint64_t nEvents) {
    
    cKeyAlice.data() = qkd::utility::memory(nEvents * sizeof(base_and_float));
    cKeyBob.data() = qkd::utility::memory(nEvents * sizeof(base_and_float));
    base_and_float * a = reinterpret_cast<base_and_float *>(cKeyAlice.data().get());
    base_and_float * b = reinterpret_cast<base_and_float *>(cKeyBob.data().get());
    
    std::normal_distribution<> cNormalDistribution(0, 1.0);
    qkd::utility::random r = qkd::utility::random_source::source();
    
    double nBaseAlice = 0.0;
    double nBaseBob = 0.0;
    
    while (nEvents > 0) {
        
        r >> nBaseAlice;
        r >> nBaseBob;
        a->nBase = (nBaseAlice < 0.5 ? 0 : 1);
        b->nBase = (nBaseBob < 0.5 ? 0 : 1);
     
        double y1 = cNormalDistribution(m_cRandomGenerator);
        double y2 = cNormalDistribution(m_cRandomGenerator);
        
        if (a->nBase == b->nBase) {
            
            // same base measurment
            double y_a = y1;
            double y_b = m_nRho * y1 + m_nSqrt_1Rho2 * y2;
            
            if (a->nBase == 0) {
                a->nMeasurement = m_nSigmaAliceQ * y_a;
                b->nMeasurement = m_nSigmaBobQ * y_b;
            }
            else {
                a->nMeasurement = m_nSigmaAliceP * y_a;
                b->nMeasurement = -m_nSigmaBobP * y_b;
            }
        }
        else {
            
            // different base measurment
            double y_a = y1;
            double y_b = y2;
            
            if (a->nBase == 0) {
                a->nMeasurement = m_nSigmaAliceQ * y_a;
            }
            else {
                a->nMeasurement = m_nSigmaAliceP * y_a;
            }
            if (b->nBase == 0) {
                b->nMeasurement = m_nSigmaBobQ * y_b;
            }
            else {
                b->nMeasurement = m_nSigmaBobP * y_b;
            }
        }
    
        ++a;
        ++b;
        --nEvents;
    }
    
    cKeyAlice.set_encoding(qkd::key::ENCODING_BASE_FLOAT);
    cKeyBob.set_encoding(qkd::key::ENCODING_BASE_FLOAT);
}

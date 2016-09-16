/*
 * prepare_measure_homodyne.cpp
 * 
 * continue creation for prepare_measure_homodyne data
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
#include <sstream>

// ait
#include <qkd/utility/random.h>

#include "prepare_measure_homodyne.h"

using namespace qkd::cv;


// ------------------------------------------------------------
// code

/**
 * helper struct for key data
 */
typedef struct {
    
    uint32_t nBase;
    float nMeasurement;
    
} base_and_float;


/**
 * helper struct for key data
 */
typedef struct {
    
    float q;
    float p;
    
} float_and_float;


// ------------------------------------------------------------
// code


/**
 * add program arguments to our mode settings
 * 
 * @param   cArguments      the arguments as passed from the command line
 * @return  true, if all arguments are ok
 */
bool prepare_measure_homodyne::consume_arguments(boost::program_options::variables_map const & cArguments) {
    
    unsigned int nQValues = 0;
    bool bSigmaAliceQ = cArguments.count("sigma-alice-q") ? true : false;
    if (bSigmaAliceQ) {
        ++nQValues;
        m_nSigmaAliceQ = cArguments["sigma-alice-q"].as<double>();
    }
    bool bSigmaNoiseQ = cArguments.count("sigma-noise-q") ? true : false;
    if (bSigmaNoiseQ) {
        ++nQValues;
        m_nSigmaNoiseQ = cArguments["sigma-noise-q"].as<double>();
    }
    bool bSNRQ = cArguments.count("snr-q") ? true : false;
    if (bSNRQ) {
        ++nQValues;
        m_nSNRQ = cArguments["snr-q"].as<double>();
    }
    
    if (nQValues != 2) {
        std::cerr << "please specify two out from sigma-alice-q, sigma-noise-q and snr-q" << std::endl;
        return false;
    }
    
    if (!bSigmaAliceQ) {
        m_nSigmaAliceQ = m_nSigmaNoiseQ * m_nSNRQ;
    }
    if (!bSigmaNoiseQ) {
        m_nSigmaNoiseQ = m_nSigmaAliceQ / m_nSNRQ;
    }
    if (!bSNRQ) {
        m_nSNRQ = m_nSigmaAliceQ / m_nSigmaNoiseQ;
    }
    
    unsigned int nPValues = 0;
    bool bSigmaAliceP = cArguments.count("sigma-alice-p") ? true : false;
    if (bSigmaAliceP) {
        ++nPValues;
        m_nSigmaAliceP = cArguments["sigma-alice-p"].as<double>();
    }
    bool bSigmaNoiseP = cArguments.count("sigma-noise-p") ? true : false;
    if (bSigmaNoiseP) {
        ++nPValues;
        m_nSigmaNoiseP = cArguments["sigma-noise-p"].as<double>();
    }
    bool bSNRP = cArguments.count("snr-p") ? true : false;
    if (bSNRP) {
        ++nPValues;
        m_nSNRP = cArguments["snr-p"].as<double>();
    }
    
    if (nPValues != 2) {
        std::cerr << "please specify two out from sigma-alice-p, sigma-noise-p and snr-p" << std::endl;
        return false;
    }

    if (!bSigmaAliceP) {
        m_nSigmaAliceP = m_nSigmaNoiseP * m_nSNRP;
    }
    if (!bSigmaNoiseP) {
        m_nSigmaNoiseP = m_nSigmaAliceP / m_nSNRP;
    }
    if (!bSNRP) {
        m_nSNRP = m_nSigmaAliceP / m_nSigmaNoiseP;
    }
    
    if (cArguments.count("transmission") < 1) {
        std::cerr << "missing transmission" << std::endl;
        return false;
    }
    m_nTransmission = cArguments["transmission"].as<double>();
    if ((m_nTransmission < 0.0) || (m_nTransmission > 1.0)) {
        std::cerr << "transmission must be between 0.0 and 1.0" << std::endl;
        return false;
    }
    
    return true;
}


/**
 * dump a string about the mode's configuration
 * 
 * @return  a string describing the mode's settings
 */
std::string prepare_measure_homodyne::dump_configuration() const {
    
    std::stringstream ss;
    
    ss << "\tsigma alice Q:      " << m_nSigmaAliceQ << "\n";
    ss << "\tsigma alice P:      " << m_nSigmaAliceP << "\n";
    ss << "\t(sigma alice Q)^2:  " << m_nSigmaAliceQPow2 << "\n";
    ss << "\t(sigma alice P)^2:  " << m_nSigmaAlicePPow2 << "\n";
    
    ss << "\tsigma noise Q:      " << m_nSigmaNoiseQ << "\n";
    ss << "\tsigma noise P:      " << m_nSigmaNoiseP << "\n";
    ss << "\t(sigma noise Q)^2:  " << m_nSigmaNoiseQPow2 << "\n";
    ss << "\t(sigma noise P)^2:  " << m_nSigmaNoisePPow2 << "\n";
    
    ss << "\ttransmission:       " << m_nTransmission << "\n";
    ss << "\tSNR Q:              " << m_nSNRQ << "\n";
    ss << "\tSNR P:              " << m_nSNRP << "\n";
    
    return ss.str();
}


/**
 * report some help on this key generation mode
 * 
 * @return  help shown to the user on stdout about this mode
 */
std::string prepare_measure_homodyne::help() {
    
    std::stringstream ss;
    
    ss << "mode: 'prepare_measure_homodyne'\n";
    ss << "This mode creates a pair of CV pseudo keys.\n";
    ss << "The resulting key for alice has ENCODING_FLOAT_Q_FLOAT_P encoding: \n";
    ss << "The first 32 bits hold Q measurment wheras the second 32 bit hold \n";
    ss << "P measurment. The key data for bob has ENCODING_BASE_FLOAT syntax: \n";
    ss << "The first 32 Bit hold the base, either Q (== 0) or P (== 1). The \n";
    ss << "second 32 bit hold the measurment value.\n\n";
    
    ss << "Parameters needed for mode 'prepare_measure_homodyne':\n";
    ss << "    --sigma-alice-q\n";
    ss << "    --sigma-alice-p\n";
    ss << "    --sigma-noise-q\n";
    ss << "    --sigma-noise-p\n";
    ss << "    --transmission\n";
    ss << "    --snr";
    
    return ss.str();
}


/**
 * init the genration mode
 * 
 * this inits the generation mode, adds all necessary precalculated values
 * after argument consumation
 */
void prepare_measure_homodyne::init() {
    m_nSigmaAliceQPow2 = m_nSigmaAliceQ * m_nSigmaAliceQ;
    m_nSigmaAlicePPow2 = m_nSigmaAliceP * m_nSigmaAliceP;
    m_nSigmaNoiseQPow2 = m_nSigmaNoiseQ * m_nSigmaNoiseQ;
    m_nSigmaNoisePPow2 = m_nSigmaNoiseP * m_nSigmaNoiseP;
}


/**
 * produce a set of pseudo random cv-keys
 * 
 * @param   cKeyAlice       alice key
 * @param   cKeyBob         bob key
 * @param   nEvents         number of events for each key 
 */
void prepare_measure_homodyne::produce(qkd::key::key & cKeyAlice, qkd::key::key & cKeyBob, uint64_t nEvents) {
    
    cKeyAlice.data() = qkd::utility::memory(nEvents * sizeof(float_and_float));
    cKeyBob.data() = qkd::utility::memory(nEvents * sizeof(base_and_float));
    float_and_float * a = reinterpret_cast<float_and_float *>(cKeyAlice.data().get());
    base_and_float * b = reinterpret_cast<base_and_float *>(cKeyBob.data().get());
    
    std::normal_distribution<> cNormalDistAliceQ(0, m_nSigmaAliceQ);
    std::normal_distribution<> cNormalDistAliceP(0, m_nSigmaAliceP);
    std::normal_distribution<> cNormalDistNoiseQ(0, m_nSigmaNoiseQ);
    std::normal_distribution<> cNormalDistNoiseP(0, m_nSigmaNoiseP);
    qkd::utility::random r = qkd::utility::random_source::source();
    
    while (nEvents > 0) {
        
        a->q = cNormalDistAliceQ(m_cRandomGenerator);
        a->p = cNormalDistAliceP(m_cRandomGenerator);
        
        float nNoiseQ = cNormalDistNoiseQ(m_cRandomGenerator);
        float nNoiseP = cNormalDistNoiseP(m_cRandomGenerator);
        float b_q = m_nTransmission * a->q + nNoiseQ;
        float b_p = m_nTransmission * a->p + nNoiseP;
        
        double nBaseBob = 0.0;
        r >> nBaseBob;
        b->nBase = (nBaseBob < 0.5 ? 0 : 1);
        
        if (!b->nBase) {
            b->nMeasurement = b_q;
        }
        else {
            b->nMeasurement = b_p;
        }
     
        ++a;
        ++b;
        --nEvents;
    }
    
    cKeyAlice.set_encoding(qkd::key::ENCODING_FLOAT_Q_FLOAT_P);
    cKeyBob.set_encoding(qkd::key::ENCODING_BASE_FLOAT);
}


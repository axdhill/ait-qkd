/*
 * prepare_measure_heterodyne.cpp
 * 
 * continue creation for prepare_measure_heterodyne data
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
#include <qkd/common_macros.h>

#include "prepare_measure_heterodyne.h"

using namespace qkd::cv;


// ------------------------------------------------------------
// code


/**
 * add program arguments to our mode settings
 * 
 * @param   cArguments      the arguments as passed from the command line
 * @return  true, if all arguments are ok
 */
bool prepare_measure_heterodyne::consume_arguments(boost::program_options::variables_map const & cArguments) {
    
    unsigned int nQValues = 0;
    bool bSigmaAliceQ = cArguments.count("sigma-alice-q") ? true : false;
    if (bSigmaAliceQ) {
        ++nQValues;
        m_nSigmaAliceQ = cArguments["sigma-alice-q"].as<float>();
    }
    bool bSigmaNoiseQ = cArguments.count("sigma-noise-q") ? true : false;
    if (bSigmaNoiseQ) {
        ++nQValues;
        m_nSigmaNoiseQ = cArguments["sigma-noise-q"].as<float>();
    }
    bool bSNRQ = cArguments.count("snr-q") ? true : false;
    if (bSNRQ) {
        ++nQValues;
        m_nSNRQ = cArguments["snr-q"].as<float>();
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
        m_nSigmaAliceP = cArguments["sigma-alice-p"].as<float>();
    }
    bool bSigmaNoiseP = cArguments.count("sigma-noise-p") ? true : false;
    if (bSigmaNoiseP) {
        ++nPValues;
        m_nSigmaNoiseP = cArguments["sigma-noise-p"].as<float>();
    }
    bool bSNRP = cArguments.count("snr-p") ? true : false;
    if (bSNRP) {
        ++nPValues;
        m_nSNRP = cArguments["snr-p"].as<float>();
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
    
    m_nSigmaAliceQPow2 = m_nSigmaAliceQ * m_nSigmaAliceQ;
    m_nSigmaAlicePPow2 = m_nSigmaAliceP * m_nSigmaAliceP;
    m_nSigmaNoiseQPow2 = m_nSigmaNoiseQ * m_nSigmaNoiseQ;
    m_nSigmaNoisePPow2 = m_nSigmaNoiseP * m_nSigmaNoiseP;
    
    if (cArguments.count("transmission") < 1) {
        std::cerr << "missing transmission" << std::endl;
        return false;
    }
    m_nTransmission = cArguments["transmission"].as<float>();
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
std::string prepare_measure_heterodyne::dump_configuration() const {
    
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
std::string prepare_measure_heterodyne::help() {
    
    std::stringstream ss;
    
    ss << "mode: 'prepare_measure_heterodyne'\n";
    ss << "This mode creates a pair of  CV pseudo keys.\n";
    ss << "The resulting keys have ENCODING_FLOAT_Q_FLOAT_P encoding: \n";
    ss << "The first 32 bits hold Q measurment wheras the second 32 bit hold v";
    ss << "P measurment.\n\n";
    
    ss << "Parameters needed for mode 'prepare_measure_heterodyne':\n";
    ss << "    --sigma-alice-q\n";
    ss << "    --sigma-alice-p\n";
    ss << "    --sigma-noise-q\n";
    ss << "    --sigma-noise-p\n";
    ss << "    --transmission\n";
    ss << "    --snr";
    
    return ss.str();
}


/**
 * produce a set of pseudo random cv-keys
 * 
 * @param   cKeyAlice       alice key
 * @param   cKeyBob         bob key
 */
void prepare_measure_heterodyne::produce(UNUSED qkd::key::key & cKeyAlice, UNUSED qkd::key::key & cKeyBob) {
}


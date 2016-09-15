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
#include <sstream>

// ait
#include <qkd/common_macros.h>

#include "entangled.h"

using namespace qkd::cv;


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
    m_nSigmaAliceQ = cArguments["sigma-alice-q"].as<float>();
    if (m_nSigmaAliceQ <= 0.0) {
        std::cerr << "sigma-alice-q must not be less or euqal than 0.0" << std::endl;
        return false;
    }
    m_nSigmaAliceQPow2 = m_nSigmaAliceQ * m_nSigmaAliceQ;
    
    if (cArguments.count("sigma-alice-p") < 1) {
        std::cerr << "missing sigma-alice-p" << std::endl;
        return false;
    }
    m_nSigmaAliceP = cArguments["sigma-alice-p"].as<float>();
    if (m_nSigmaAliceP <= 0.0) {
        std::cerr << "sigma-alice-p must not be less or euqal than 0.0" << std::endl;
        return false;
    }
    m_nSigmaAlicePPow2 = m_nSigmaAliceP * m_nSigmaAliceP;
    
    if (cArguments.count("sigma-bob-q") < 1) {
        std::cerr << "missing sigma-bob-q" << std::endl;
        return false;
    }
    m_nSigmaBobQ = cArguments["sigma-bob-q"].as<float>();
    if (m_nSigmaBobQ <= 0.0) {
        std::cerr << "sigma-bob-q must not be less or euqal than 0.0" << std::endl;
        return false;
    }
    m_nSigmaBobQPow2 = m_nSigmaBobQ * m_nSigmaBobQ;
    
    if (cArguments.count("sigma-bob-p") < 1) {
        std::cerr << "missing sigma-bob-p" << std::endl;
        return false;
    }
    m_nSigmaBobP = cArguments["sigma-bob-p"].as<float>();
    if (m_nSigmaBobP <= 0.0) {
        std::cerr << "sigma-bob-p must not be less or euqal than 0.0" << std::endl;
        return false;
    }
    m_nSigmaBobPPow2 = m_nSigmaBobP * m_nSigmaBobP;
    
    if (cArguments.count("rho") < 1) {
        std::cerr << "missing rho" << std::endl;
        return false;
    }
    m_nRho = cArguments["rho"].as<float>();
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
    ss << "\t(sigma alice Q)^2:  " << m_nSigmaAliceQPow2 << "\n";
    ss << "\t(sigma alice P)^2:  " << m_nSigmaAlicePPow2 << "\n";
    
    ss << "\tsigma bob Q:        " << m_nSigmaBobQ << "\n";
    ss << "\tsigma bob P:        " << m_nSigmaBobP << "\n";
    ss << "\t(sigma bob Q)^2:    " << m_nSigmaBobQPow2 << "\n";
    ss << "\t(sigma bob P)^2:    " << m_nSigmaBobPPow2 << "\n";
    
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
 * produce a set of pseudo random cv-keys
 * 
 * @param   cKeyAlice       alice key
 * @param   cKeyBob         bob key
 */
void entangled::produce(UNUSED qkd::key::key & cKeyAlice, UNUSED qkd::key::key & cKeyBob) {
}

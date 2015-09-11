/*
 * photon_pair.h
 * 
 * Declaration of a photon_pair and associated constant definitions
 *
 * Author: Philipp Grabenweger
 *         Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *
 * Copyright (C) 2013-2015 AIT Austrian Institute of Technology
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


#ifndef __QKD_QKD_SIMULATE_PHOTON_PAIR_H_
#define __QKD_QKD_SIMULATE_PHOTON_PAIR_H_


// -----------------------------------------------------------------
// decl


namespace qkd {
    
namespace simulate {
    

/**
 * photon states
 */
enum photon_state : uint8_t {
    
    NONPOLARIZED,
    ENTANGLED,
    HORIZONTAL,
    VERTICAL,
    PLUS,
    MINUS,
    ABSORBED
};


/**
 * constant strings naming the photon states defined in photon_state
 */
std::string const photon_state_str[] = {
    "NONPOLARIZED",
    "ENTANGLED",
    "HORIZONTAL",
    "VERTICAL",
    "PLUS",
    "MINUS",
    "ABSORBED"
};


/**
 * photon pair
 */
struct photon_pair {
    
    photon_state eStateA;           /**< state of photon travelling to Alice */
    photon_state eStateB;           /**< state of photon travelling to Bob */
    double nEntanglementError;      /**< probability for detecting wrong polarization of second photon at side B [0 - 1] (only valid for entangled photons) */
};

}
}


#endif

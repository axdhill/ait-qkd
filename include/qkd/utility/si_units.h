/*
 * si_units.h
 * 
 * helper to give std::string to SI units
 *
 * Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *
 * Copyright (C) 2012-2015 AIT Austrian Institute of Technology
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

 
#ifndef __QKD_UTILITY_RATIO_H_
#define __QKD_UTILITY_RATIO_H_


// ------------------------------------------------------------
// incs

#include <chrono>
#include <ratio>
#include <string>


// ------------------------------------------------------------
// defs

#if defined(__GNUC__) || defined(__GNUCPP__)
#   define UNUSED   __attribute__((unused))
#else
#   define UNUSED
#endif


// ------------------------------------------------------------
// decls


namespace qkd {
    
namespace utility {    

    
//
// problem: most of this stuff is defined at compiled time only :(
// 
// maybe there's a better solution to this in C++14
//
    
    
inline std::string ratio_to_string(UNUSED std::atto const & cRatio)  { return "a"; }
inline std::string ratio_to_string(UNUSED std::femto const & cRatio) { return "f"; }
inline std::string ratio_to_string(UNUSED std::pico const & cRatio)  { return "p"; }
inline std::string ratio_to_string(UNUSED std::nano const & cRatio)  { return "n"; }
inline std::string ratio_to_string(UNUSED std::micro const & cRatio) { return "mu"; }
inline std::string ratio_to_string(UNUSED std::milli const & cRatio) { return "m"; }
inline std::string ratio_to_string(UNUSED std::centi const & cRatio) { return "c"; }
inline std::string ratio_to_string(UNUSED std::deci const & cRatio)  { return "d"; }
inline std::string ratio_to_string(UNUSED std::deca const & cRatio)  { return "da"; }
inline std::string ratio_to_string(UNUSED std::hecto const & cRatio) { return "h"; }
inline std::string ratio_to_string(UNUSED std::kilo const & cRatio)  { return "k"; }
inline std::string ratio_to_string(UNUSED std::mega const & cRatio)  { return "M"; }
inline std::string ratio_to_string(UNUSED std::giga const & cRatio)  { return "G"; }
inline std::string ratio_to_string(UNUSED std::tera const & cRatio)  { return "T"; }
inline std::string ratio_to_string(UNUSED std::peta const & cRatio)  { return "P"; }
inline std::string ratio_to_string(UNUSED std::exa const & cRatio)   { return "E"; }

inline std::string duration_to_string(UNUSED std::chrono::nanoseconds const &cDuration) { return "ns"; }
inline std::string duration_to_string(UNUSED std::chrono::microseconds const &cDuration) { return "us"; }
inline std::string duration_to_string(UNUSED std::chrono::milliseconds const &cDuration) { return "ms"; }
inline std::string duration_to_string(UNUSED std::chrono::seconds const &cDuration) { return "s"; }
inline std::string duration_to_string(UNUSED std::chrono::minutes const &cDuration) { return "m"; }
inline std::string duration_to_string(UNUSED std::chrono::hours const &cDuration) { return "h"; }
    
    
    
}

}

#endif


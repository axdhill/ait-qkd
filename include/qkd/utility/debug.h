/*
 * debug.h
 * 
 * The debug flag singelton
 *
 * Autor: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
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

 

#ifndef __QKD_UTILITY_DEBUG_H_
#define __QKD_UTILITY_DEBUG_H_


// ------------------------------------------------------------
// incl

#include <string>
#include <sstream>
#include <vector>


// ------------------------------------------------------------
// defs


/*
 * some debug helper: assemble source code location information
 */
#ifndef __DEBUG_LOCATION__
#define __DEBUG_LOCATION__  "=dbg= " << __func__ << "(...)@" << __FILENAME__ << ":" << __LINE__ << " "
#endif


// ------------------------------------------------------------
// decls


namespace qkd {
    
namespace utility {    
    
    
/**
 * This is the debug facility.
 * 
 * One creates an instance on the fly and on instance
 * termination the buffered data is flushed to std::cerr.
 * 
 * Example:
 * 
 * 
 *      qkd::utility::debug() << "This is line " << __LINE__ << " and a double: " << 14.3; 
 * 
 */
class debug : public std::stringstream {
    
    
public:    
    
    
    /**
     * ctor
     */
    debug() : debug(false) {}


    /**
     * ctor
     *
     * @param   bForceOutput    force output to stderr, even if global debug flag is disabled
     */
    debug(bool bForceOutput) : std::stringstream(), m_bForceOutput(bForceOutput) {}


     /**
     * dtor
     */
    virtual ~debug() { flush(); }
    
    
    /**
     * the debug flag
     * 
     * @return  the reference to the global wide debug flag
     */
    static bool & enabled();
    
    
    /**
     * write to log
     */
    void flush();
 

    /**
     * sets the callback function for new logs
     * 
     * @param   fLog        the callback function
     */
    static void set_callback(void (* fLog)(std::string const & ));
    

private:


    /**
     * force output (ignore the static debug flag)
     */
    bool m_bForceOutput;

};


/**
 * dumps a C style array as a string as "{i_0, i_1, ... i_n}"
 * 
 * @param   a       the array to dump
 * @param   n       the size of the array
 * @return  a string listening the elements of the vector
 */
template<class T> std::string debug_array(T * const & a, uint64_t n) {
    std::stringstream ss; 
    ss << "{"; 
    for (uint64_t i = 0; i < n; i++) {
        ss << (i ? ", " : "") << a[i]; 
    } 
    ss << "}"; 
    return ss.str(); 
}


/**
 * dumps a std::vector as a string as "{i_0, i_1, ... i_n}"
 * 
 * @param   v       the vector to dump
 * @return  a string listening the elements of the vector
 */
template<class T> std::string debug_vector(std::vector<T> const & v) {
    bool bFirst = true; 
    std::stringstream ss; 
    ss << "{"; 
    for (auto i: v) { 
        if (!bFirst) ss << ", "; 
        bFirst = false; 
        ss << i; 
    } 
    ss << "}"; 
    return ss.str(); 
}


}

}

#endif


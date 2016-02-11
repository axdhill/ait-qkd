/*
 * average_data.h
 * 
 * internal data of the moving average interface
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

 
#ifndef __QKD_UTLITY_AVERAGE_DATA_H_
#define __QKD_UTLITY_AVERAGE_DATA_H_


// ------------------------------------------------------------
// incs

#include <chrono>

// ait
#include <qkd/utility/average.h>


// ------------------------------------------------------------
// decls


namespace qkd {
    
namespace utility {    


/**
 * the average_data particle
 */
class average_technique::average_data {

    
public:
    
    
    /**
     * ctor
     */
    average_data(double nValue) : m_nValue(nValue) { m_cTimeStamp = std::chrono::high_resolution_clock::now(); }
    
    
    /**
     * copy ctor
     */
    average_data(average_data const & rhs) : m_nValue(rhs.m_nValue), m_cTimeStamp(rhs.m_cTimeStamp) {}
    
    
    /**
     * retrieves the age
     * 
     * @return  the age of the value in milliseconds
     */
    std::chrono::high_resolution_clock::duration age() const { return std::chrono::high_resolution_clock::now() - m_cTimeStamp; }
    

    /**
     * give date and time of birth
     * 
     * @return  timestamp of data creation
     */
    std::chrono::high_resolution_clock::time_point birth() const { return m_cTimeStamp; }
    
    
    /**
     * retrieves the value
     * 
     * @return  the value
     */
    double value() const { return m_nValue; }
    

private:
    
    
    /**
     * the value
     */
    double m_nValue;
    
    
    /**
     * the timestamp
     */
    std::chrono::high_resolution_clock::time_point m_cTimeStamp;
    
};
    

    
}

}

#endif


/*
 * average_time.h
 * 
 * moving average over a timespan interface
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

 
#ifndef __QKD_UTLITY_AVERAGE_TIME_H_
#define __QKD_UTLITY_AVERAGE_TIME_H_


// ------------------------------------------------------------
// incs

#include <inttypes.h>

// ait
#include <qkd/utility/average.h>


// ------------------------------------------------------------
// decls


namespace qkd {
    
namespace utility {    


/**
 * this class is a moving average over a timespan
 * 
 */
class average_time: public average_technique {

    
public:


    /**
     * ctor
     */
    explicit average_time(uint64_t nWindowSize) : average_technique(nWindowSize), m_nSum(0) {};


    /**
     * dtor
     */
    virtual ~average_time() {};


    /**
     * describe the moving average
     * 
     * @return  a HR-string describing the moving average object
     */
    virtual std::string describe() const { return std::string("moving average over a timespan"); };

    
private:

    
    /**
     * add a value to the average calculation
     * 
     * @param   nValue      value to add
     */
    virtual void add_internal(double nValue);


    /**
     * get the current average value as average
     *
     * @return  the current average value as average
     */
    virtual double avg_internal() const;
    
    
    /**
     * get the current average value as sum
     *
     * @return  the current average value as sum
     */
    virtual double sum_internal() const;
    
    
    /**
     * trim the data according to the window size
     */
    virtual void trim() const;
    

    /**
     * current sum of all values in the list
     */
    mutable double m_nSum;
};
  

    
}

}

#endif


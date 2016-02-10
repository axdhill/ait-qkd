/*
 * average.h
 * 
 * moving average interface
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

 
#ifndef __QKD_UTILITY_AVERAGE_H_
#define __QKD_UTILITY_AVERAGE_H_


// ------------------------------------------------------------
// incs

#include <exception>
#include <list>
#include <string>

#include <inttypes.h>

#include <boost/shared_ptr.hpp>


// ------------------------------------------------------------
// decls


namespace qkd {
    
namespace utility {    


// fwd
class average_technique;
typedef boost::shared_ptr<average_technique> average;


/**
 * this encapsulates a moving average
 * 
 * a moving average instance is called with
 * 
 *      create(NAME, WINDOW-SIZE);
 * 
 * Current value for NAME are
 *  - "value"       which will create a moving average based on sample size
 *  - "time"        which will create a moving average based on a timespan
 * 
 * The create() functions returns an "average" object, which is actually
 * a smart pointer to an average_technique.
 * 
 * The WINDOW-SIZE is the size of the moving average. 
 * 
 * If the moving average is "value"-based, then the WINDOW-SIZE 
 * specifies the numbers of values taken into account for the average.
 * The calculation is then the arithmetic average of all values within
 * WINDOW-SIZE.
 * 
 * If the name is "time" then the WINDOW-SIZE is the number of millisecs
 * in the past starting with NOW() for which the average is calculated. The
 * calculation is then the sum of all items within WINDOW-SIZE divided by the
 * age of the oldest entry in seconds.
 * 
 * Having an average object, one now adds values to it via add() or << stream.
 * 
 * The average value as sum of all single values within WINDOW-SIZE can be 
 * retrieved via sum(). The average value as average of all single values within
 * WINDOW-SIZE can be retrieved via avg().
 * 
 * Example:
 * 
 *      average cAvg = average_technique::create("value", 10);
 * 
 *      cAvg << 3.4;
 *      cAvg << 6.9;
 *      cAvg << -12.0 << 5.0 << 1;
 * 
 *      double nAverage1 = cAvg.avg();
 *      double nAverage1 = cAvg.sum();
 */
class average_technique {

    
protected:

    
    // fwd
    class average_data;
    typedef boost::shared_ptr<average_data> average_data_ptr;           /**< a average data particle */
    
    
public:


    /**
     * exception type thrown for unknown technique
     */
    struct average_technique_unknown : virtual std::exception, virtual boost::exception { };
    

    /**
     * dtor
     */
    virtual ~average_technique() {}


    /**
     * stream in
     * 
     * this adds a value (rhs) to the average calculation
     *
     * @param   rhs         right hand side: a new value to add to the object
     * @return  average
     */
    inline average_technique & operator<<(double const & rhs) { 
        add(rhs); 
        return *this; 
    }
    
    
    /**
     * add a value to the average calculation
     * 
     * @param   nValue      value to add
     */
    inline void add(double nValue) { add_internal(nValue); }


    /**
     * get the current average value as average of all single values inside the window
     * 
     * @return  the current average value as average of all single values inside the window
     */
    inline double avg() const { trim(); return avg_internal(); }


    /**
     * factory method to create a moving average
     * 
     * the technique is either
     * 
     *      "value"     which creates a moving average object for the
     *                  last WINDOW-SIZE values stored
     * 
     *      "time"      which creates a moving average object for the
     *                  last values having a maximum of WINDOW-SIZE millisecs
     *                  of age
     *
     * @param   sTechnique      the technique of the moving average
     * @param   nWindowSize     the window size of the moving average
     * @return  an initialized average object
     * @throws  average_technique_unknown
     */
    static average create(std::string sTechnique, uint64_t nWindowSize);
    
    
    /**
     * describe the moving average
     * 
     * @return  a HR-string describing the moving average object
     */
    virtual std::string describe() const = 0;
    
    
    /**
     * get the oldest value
     * 
     * @return  the oldest value
     */
    inline double oldest() const { 
        trim(); 
        return oldest_internal(); 
    }


    /**
     * get the slope given windowsize :== 1
     * 
     * @return  the current slope
     */
    inline double slope() const { return youngest() - oldest(); }


    /**
     * get the current average value as sum of all values inside the window size
     * 
     * @return  the current average value as sum
     */
    inline double sum() const { 
        trim(); 
        return sum_internal(); 
    }


    /**
     * return the window size of the moving average
     * 
     * @return  the window size
     */
    inline double window() const { return m_nWindowSize; }


    /**
     * get the youngest value
     * 
     * @return  the youngest value
     */
    inline double youngest() const { 
        trim(); 
        return youngest_internal(); 
    }

    /**
     * Gets the lowest recorded value within the window size.
     *
     * @return the lowest value
     */
    inline double lowest() const {
        trim();
        return lowest_internal();
    }

    /**
     * Gets the highest recorded value within the window size.
     *
     * @return the highest value
     */
    inline double highest() const {
        trim();
        return highest_internal();
    }


protected:


    /**
     * ctor
     * 
     * @param   nWindowSize     the window size of the moving average
     */
    average_technique(uint64_t nWindowSize) : m_nWindowSize(nWindowSize) {}
    
    
    /**
     * retrieve the internal data pointer list
     * 
     * Design artfice:
     * 
     * Hence, although this function is "const" it returns the modifiable internal data set.
     * This is intended, since the avg() and sum() calls are meant be "const" and the other side, 
     * this avg() and sum() calls are also the ideal place to get rid of unneeded average values, 
     * which drop out of the window via trim().
     * 
     * Short: while avg() and sum() are "const", we ought to modify the internal data via trim() here.
     *        As avg() and sum() are "const" so must be trim().
     *        Via this call - which is protected anyway - trim() may modify the internal
     *        data even if the object itself is "const".
     * 
     * @return  the internal data pointer list
     */
    inline std::list<average_data_ptr> & data() const { return d; }
    
    
private:

    
    /**
     * add a value to the average calculation
     * 
     * @param   nValue      value to add
     */
    virtual void add_internal(double nValue) = 0;


    /**
     * get the current average value as average
     *
     * @return  the current average value as average
     */
    virtual double avg_internal() const = 0;
    
    
    /**
     * get the oldest value
     * 
     * @return  the oldest value
     */
    double oldest_internal() const;


    /**
     * get the current average value as sum
     *
     * @return  the current average value as sum
     */
    virtual double sum_internal() const = 0;
    
    
    /**
     * trim the data according to the window size
     */
    virtual void trim() const = 0;
    
    
    /**
     * get the youngest value
     * 
     * @return  the youngest value
     */
    double youngest_internal() const;

    /**
     * Gets the highest recorded value within the window size.
     *
     * @return the highest value
     */
    double highest_internal() const;

    /**
     * Gets the lowest recorded value within the window size.
     *
     * @return the lowest value
     */
    double lowest_internal() const;
    
    
    /**
     * the window size of the moving average
     */
    double m_nWindowSize;
    

    /**
     * This is the list of internal data
     * 
     * This is mutable so const functions,
     * like trim() may modify it via call
     * to data().
     */
    mutable std::list<average_data_ptr> d;
    
};
  

/**
 * stream into
 * 
 * Add values (rhs) to the average object
 * 
 * @param   lhs     the average object
 * @param   rhs     the value to add
 * @return  the average object
 */
inline average & operator<<(average & lhs, double const & rhs) { 
    (*lhs) << rhs; 
    return lhs; 
}


}

}

#endif


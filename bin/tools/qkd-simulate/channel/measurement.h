/*
 * measurement.h
 * 
 * definition of a quantum channel measurement
 *
 * Autor: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
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

 
#ifndef __QKD_QKD_SIMULATE_MEASUREMENT_H_
#define __QKD_QKD_SIMULATE_MEASUREMENT_H_


// -----------------------------------------------------------------
// incs

#include <boost/shared_ptr.hpp>

// ait
#include <qkd/key/key.h>


// -----------------------------------------------------------------
// decl


namespace qkd {
    
namespace simulate {

    
// fwd
class measurement_base;
typedef boost::shared_ptr<measurement_base> measurement;


/**
 * an abstract quantum channel measurement
 * 
 * a measurment object holds the results from a single
 * measurment step. each measurement creates a pair of
 * keys: one for alice and one for bob.
 */
class measurement_base {
    
    
public:
    

    /** 
     * ctor
     */
    explicit measurement_base() {};
    
    
    /** 
     * dtor
     */
    virtual ~measurement_base() {};

    
    /**
     * get acquisition duration 
     * 
     * Returns the duration of this measurement in nanoseconds [ns]
     * 
     * @return  duration of table acquisition in nanoseconds
     */
    virtual double acquisition_duration() const = 0;
    
    
    /**
     * state if this measurment has been made with free running detectros
     * 
     * @return  true, if this measurement has been done with free running detetcors
     */
    virtual bool free_running() const = 0;


    /**
     * get the alice key
     * 
     * @return  the key meant for alice
     */
    virtual qkd::key::key & key_alice() = 0;


    /**
     * get the alice key
     * 
     * @return  the key meant for alice
     */
    virtual qkd::key::key const & key_alice() const = 0;


    /**
     * get the bob key
     * 
     * @return  the key meant for bob
     */
    virtual qkd::key::key & key_bob() = 0;


    /**
     * get the bob key
     * 
     * @return  the key meant for bob
     */
    virtual qkd::key::key const & key_bob() const = 0;

};


}
}

#endif

/*
 * measurement_bb84.h
 * 
 * definition of a quantum channel BB84 measurement 
 *
 * Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *
 * Copyright (C) 2013-2016 AIT Austrian Institute of Technology
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

 
#ifndef __QKD_QKD_SIMULATE_MEASUREMENT_BB84_H_
#define __QKD_QKD_SIMULATE_MEASUREMENT_BB84_H_


// ------------------------------------------------------------
// incs

// ait
#include "measurement.h"


// -----------------------------------------------------------------
// decl


namespace qkd {
    
namespace simulate {

    
/**
 * an abstract quantum channel measurement
 * 
 * a measurement object holds the results from a single
 * measurement step. each measurement creates a pair of
 * keys: one for alice and one for bob.
 */
class measurement_bb84 : public measurement_base {
    
    
public:
    

    /** 
     * ctor
     */
    explicit measurement_bb84() {};
    
    
    /** 
     * dtor
     */
    virtual ~measurement_bb84() {};

    
    /**
     * get acquisition duration 
     * 
     * Returns the duration of this measurement in nanoseconds [ns]
     * 
     * @return  duration of table acquisition in nanoseconds
     */
    double acquisition_duration() const { return m_nAcquisitionDuration; }


    /**
     * state if this measurement has been made with free running detectors
     * 
     * @return  true, if this measurement has been done with free running detectors
     */
    bool free_running() const { return m_bFreeRunning; };


    /**
     * get the alice key
     * 
     * @return  the key meant for alice
     */
    qkd::key::key & key_alice() { return m_cKeyAlice; };


    /**
     * get the alice key
     * 
     * @return  the key meant for alice
     */
    qkd::key::key const & key_alice() const { return m_cKeyAlice; };


    /**
     * get the bob key
     * 
     * @return  the key meant for bob
     */
    qkd::key::key & key_bob() { return m_cKeyBob; };
    
    
    /**
     * get the bob key
     * 
     * @return  the key meant for bob
     */
    qkd::key::key const & key_bob() const { return m_cKeyBob; };
    
    
    /**
     * set the acquisition duration
     * 
     * @param   nAcquisitionDuration        the acquisition duration in [ns]
     */
    void set_acquisition_duration(double nAcquisitionDuration) { m_nAcquisitionDuration = nAcquisitionDuration; }
    
    
    /**
     * change the free running detector flag
     * 
     * @param   bFreeRunning        new free running detector flags
     */
    void set_free_running(bool bFreeRunning) { m_bFreeRunning = bFreeRunning; };


private:

    
    double m_nAcquisitionDuration;      /**< the measurement acquisition duration in [ns] */
    bool m_bFreeRunning;                /**< measurement with free running detectors flag */
    qkd::key::key m_cKeyAlice;          /**< alice key */
    qkd::key::key m_cKeyBob;            /**< bob key */
};


}
}

#endif

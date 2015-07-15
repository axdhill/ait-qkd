/*
 * channel_bb84.h
 *
 * definition of the bb84 quantum channel
 *
 * Author: Mario Kahn
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

 
#ifndef __QKD_QKD_SIMULATE_CHANNEL_BB84_H_
#define __QKD_QKD_SIMULATE_CHANNEL_BB84_H_


// ------------------------------------------------------------
// incs

// ait
#include "channel.h"


// -----------------------------------------------------------------
// decl


namespace qkd {
    
namespace simulate {


/**
 * this is the bb84 optical quantum channel for raw key generation based on entangled photons
 */
class channel_bb84 : public channel {


public:
    

    /** 
     * ctor
     */
    channel_bb84() {};
    

    /** 
     * dtor
     */
    virtual ~channel_bb84() {};

    
private:    
    
    
    /**
     * perform a measurement 
     * to be overwritten by derived classes
     * 
     * @return  the measurment
     */
    measurement measure_internal();
    
    
    /**
     * current measured key id
     */
    qkd::key::key_id m_nKeyId;
    
};


}
}

#endif

/*
 * qauth.h
 * 
 * Implements the QAuth protocol parts as depicted at
 *
 *      http://www.iaria.org/conferences2015/awardsICQNM15/icqnm2015_a3.pdf
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


#ifndef __QKD_MODULE_QAUTH_H_
#define __QKD_MODULE_QAUTH_H_


// ------------------------------------------------------------
// incs

#include <stdint.h>

#include <list>
#include <memory>
#include <ostream>
#include <string>

#include <qkd/utility/buffer.h>

#include "bb84_base.h"


// ------------------------------------------------------------
// const


/**
 * qauth default modulus value
 */
static uint64_t const QAUTH_DEFAULT_MODULUS = 16;



// ------------------------------------------------------------
// decl


/**
 * qauth init values
 */
typedef struct {
    
    uint32_t nKv;               /**< k_v */
    uint32_t nKp;               /**< k_p */
    uint32_t nModulus;          /**< m */
    uint32_t nPosition0;        /**< p_0 */
    uint32_t nValue0;           /**< v_0 */
    
    /**
     * dump values hr-readable into a stream
     * 
     * @param   cStream     the stream to dump to
     */
    void dump(std::ostream & cStream) const;
    
    
    /**
     * dump to a string
     * 
     * @return  a string containing the values
     */
    std::string str() const;
    
} qauth_init;


/**
 * a qauth data particle
 */
typedef struct {
    
    uint64_t nPosition;         /**< the position */
    uint32_t nValue;            /**< the value */
    
    /**
     * dump value hr-readable into a stream
     * 
     * @param   cStream     the stream to dump to
     */
    void dump(std::ostream & cStream) const;
    
    
    /**
     * dump to a string
     * 
     * @return  a string containing the values
     */
    std::string str() const;

} qauth_value;


/**
 * a list of qauth particles
 */
class qauth_values : public std::list<qauth_value> {
    
    
public:
    
    
    /**
     * == equality operator
     * 
     * deep check for each element of the values
     * 
     * @param   rhs         right hand side
     * @return  true, if each element of this is matched in rhs in the correct order
     */
    bool operator==(qauth_values const & rhs) const;
    
    
    /**
     * == equality operator
     * 
     * deep check for each element of the values
     * 
     * @param   rhs         right hand side
     * @return  true, if each element of this is matched in rhs in the correct order
     */
    bool operator!=(qauth_values const & rhs) const  { return !((*this) == rhs); }
    
    
    /**
     * dump the qauth particle list to a stream
     * 
     * @param   cStream     the stream to dump to
     * @param   sIndent     the indent on each line
     * @param   cList       the qauth particle list
     */
    void dump(std::ostream & cStream, std::string const sIndent = "") const;


    /**
     * dump to a string
     * 
     * @param   sIndent     the indent on each line
     * @return  a string containing the values
     */
    std::string str(std::string const sIndent = "") const;

};


/**
 * Implements the QAuth protocol parts for BB84
 */
class qauth {
    
public:


    /**
     * ctor
     * 
     * @param   cQAuthInit      init values of qauth
     */
    qauth(qauth_init const & cQAuthInit);


    /**
     * dtor
     */
    virtual ~qauth();
    
    
    /**
     * create a series of data particles starting at position0
     * 
     * the amount of particles created will be such
     * that the hightest position value will be within
     * the set of elements of size nSize with the returned
     * list of data paticles.
     * 
     * That is
     * 
     *      l = create_max(m) ==> l.last().position < nSize
     * 
     * @param   nSize           size of container with mixed data particles within
     * @return  container with qauth data values
     */
    qauth_values create_max(uint64_t nSize);

    
    /**
     * create a series of data particles starting at position0
     * 
     * the amount of particles created will be such
     * that the hightest position value will be within
     * the merged set of elements of size nSize with the returned
     * list of data paticles.
     * 
     * That is
     * 
     *      l = create_min(nSize) ==> l.last().position < (nSize + l.size())
     * 
     * @param   nSize           size of container to mix data particles into
     * @return  container with qauth data values
     */
    qauth_values create_min(uint64_t nSize);

    
private:
    
    
    /**
     * return the next qauth_value
     * 
     * @return  the next in qauth data values in the series
     */
    qauth_value next();
    
    
    // pimpl
    class qauth_data;
    std::shared_ptr<qauth_data> d;
    
};


/**
 * stream into a memory
 * 
 * @param   lhs     left hand side
 * @param   rhs     right hand side
 * @return  memory object holding rhs
 */
qkd::utility::buffer & operator<<(qkd::utility::buffer & lhs, qauth_init const & rhs);


/**
 * stream out from memory
 * 
 * @param   lhs     left hand side
 * @param   rhs     right hand side
 * @return  memory object fromt which rhs has been retrieved
 */
qkd::utility::buffer & operator>>(qkd::utility::buffer & lhs, qauth_init & rhs);


#endif

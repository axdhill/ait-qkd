/*
 * evhash.h
 * 
 * implement the evaluation hash 
 *
 * Author: Thomas Themel - thomas.themel@ait.ac.at
 *         Oliver Maurhart, <oliver.maurhart@ait.ac.at>
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


#ifndef __QKD_CRYPTO_EVHASH_H_
#define __QKD_CRYPTO_EVHASH_H_


// ------------------------------------------------------------
// incs

#include <boost/shared_ptr.hpp>

#include <qkd/key/key.h>
#include <qkd/utility/buffer.h>
#include <qkd/utility/memory.h>

// ------------------------------------------------------------
// decl

// fwd
class evhash_abstract;
typedef boost::shared_ptr<evhash_abstract> evhash;


/**
 * interface to the GF2 instances with necessary 
 * crypto methods to talk to gf2 in a uniform way 
 * regardless of current size of the GF2
 */
class evhash_abstract {


public:


    /**
     * dtor
     */
    virtual ~evhash_abstract() {};


    /**
     * add a tag
     * 
     * This performs a simple add in the GF2
     *
     * @param   cTag        tag to add
     */
    virtual void add(qkd::utility::memory const & cTag) = 0;
    
    
    /**
     * bit width of GF2
     *
     * @return  bit width of GF2
     */
    virtual unsigned int bits() const = 0;


    /**
     * number of blocks added so far
     *
     * @return  number of blocks in the tag
     */
    virtual uint64_t blocks() const = 0;


    /**
     * size of a single block
     *
     * @return  size of a single block in bytes
     */
    virtual unsigned int block_size() const = 0;
    
    
    /**
     * fabric method
     * 
     * The size of the init key also determines the size 
     * of the evhash variant
     * 
     * @param   cKey        init key to create the evhash with
     * @return  an evaluation hash instance
     */
    static evhash create(qkd::key::key const & cKey);


    /**
     * get the final tag
     *
     * @return  the final tag
     */
    virtual qkd::utility::memory finalize() = 0;


    /**
     * set the current state
     *
     * @param   cState      the state serialized
     */
    virtual void set_state(qkd::utility::buffer & cState) = 0;


    /**
     * get the current state
     *
     * @return  serialized current state
     */
    virtual qkd::utility::buffer state() const = 0;


    /**
     * get the current tag
     *
     * @return  the current tag
     */
    virtual qkd::utility::memory tag() const = 0;
    
    
    /**
     * multiply the existing tag with the init key nRounds times
     * 
     * this creates t_n = t_n-1 * k^nRounds
     * 
     * this also increases the number of calculated blocks by nRounds
     * 
     * @param   nRounds     number of rounds to multiply
     */
    virtual void times(uint64_t nRounds) = 0;
    

    /**
     * add a memory BLOB to the algorithm
     *
     * This transforms the tag stored in this object
     * 
     * This is the working horse method of evaluation hash.
     * Here the actual tag is calculated.
     *
     * @param   cMemory         memory block to be added
     */
    virtual void update(qkd::utility::memory const & cMemory) = 0; 
    
};


#endif


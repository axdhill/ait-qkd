/*
 * netlink_base.h
 * 
 * base class for all netlink message objects
 *
 * Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *
 * Copyright (C) 2016 AIT Austrian Institute of Technology
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


// this is only Linux code
#if defined(__linux__)
 
#ifndef __QKD_Q3P_LINUX_NETLINK_BASE_H_
#define __QKD_Q3P_LINUX_NETLINK_BASE_H_


// ------------------------------------------------------------
// incs

#include <stdint.h>

#include <memory>
#include <string>


// ------------------------------------------------------------
// decls


namespace qkd {
    
namespace q3p {    

   
/**
 * This is the base class for all netlink message objects
 */
class netlink_base {
    

public:
    
    
    /**
     * dtor
     */
    virtual ~netlink_base() {}
    
    
    /**
     * clone the current object and return a pointer to it
     * 
     * @return  a new instance of this on the heap
     */
    virtual std::shared_ptr<netlink_base> clone() const = 0;
    
    
    /**
     * data of the netlink kernel object (the struct itself)
     * 
     * @return  pointer to netlink kernel object
     */
    virtual void * data() = 0;

    
    /**
     * data of the netlink kernel object (the struct itself)
     * 
     * @return  pointer to netlink kernel object
     */
    virtual void const * data() const = 0;

    
    /**
     * the name of the netlink kernel struct managed
     * 
     * @return  the name of the kernek struct managed
     */
    virtual std::string const & name() const = 0;
    
    
    /**
     * reset/init the netlink message to an empty (but valid) state
     */
    virtual void reset() = 0;
    
    
    /**
     * size of the netlink kernel object managed
     * 
     * @return  size in bytes of the object returned by data()
     */
    virtual uint64_t size() const = 0;
    

    /**
     * this method provides a JSON representation
     *
     * @return  a string holding a JSON of the current values
     */
    virtual std::string str() const = 0;


};
  

}

}


#endif

#endif

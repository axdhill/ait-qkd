/*
 * netlink_rtmsg.h
 * 
 * rtmsg wrapper
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
 
#ifndef __QKD_Q3P_LINUX_NETLINK_RTMSG_H_
#define __QKD_Q3P_LINUX_NETLINK_RTMSG_H_


// ------------------------------------------------------------
// incs

#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#include "netlink_base.h"


// ------------------------------------------------------------
// decls


namespace qkd {
    
namespace q3p {    

   
/**
 * Wrapper for rtmsg
 */
class netlink_rtmsg : public netlink_base {
    

public:
    
    
    /**
     * ctor
     */
    netlink_rtmsg();
    
    
    /**
     * -> operator
     * 
     * @return  access to members of the wrapped netlink object
     */
    rtmsg * operator->() { return &m_cRoutingMessage; }
    
    
    /**
     * -> operator
     * 
     * @return  access to members of the wrapped netlink object
     */
    rtmsg const * operator->() const { return &m_cRoutingMessage; }
    
    
    /**
     * ctor
     * 
     * @param   cRouteAttribute             the kernel routing message wrapped
     */
    netlink_rtmsg(rtmsg const & cRoutingMessage);


    /**
     * clone the current object and return a pointer to it
     * 
     * @return  a new instance of this on the heap
     */
    virtual std::shared_ptr<netlink_base> clone() const { return std::shared_ptr<netlink_base>(new netlink_rtmsg(*this)); }
    
    
    /**
     * data of the netlink kernel object (the struct itself)
     * 
     * @return  pointer to netlink kernel object
     */
    virtual void * data() { return &m_cRoutingMessage; }

    
    /**
     * data of the netlink kernel object (the struct itself)
     * 
     * @return  pointer to netlink kernel object
     */
    virtual void const * data() const { return &m_cRoutingMessage; }

    
    /**
     * the name of the netlink kernel struct managed
     * 
     * @return  the name of the kernek struct managed
     */
    virtual std::string const & name() const;
    
    
    /**
     * reset/init the netlink message to an empty (but valid) state
     */
    virtual void reset();
    
    
    /**
     * size of the netlink kernel object managed
     * 
     * @return  size in bytes of the object returned by data()
     */
    virtual uint64_t size() const { return sizeof(m_cRoutingMessage); }
    

    /**
     * this method provides a JSON representation
     *
     * @return  a string holding a JSON of the current values
     */
    virtual std::string str() const;
    
    
private:    

    
    /**
     * the rtmsg object
     */
    rtmsg m_cRoutingMessage;
    
    
};
  

}

}


#endif

#endif

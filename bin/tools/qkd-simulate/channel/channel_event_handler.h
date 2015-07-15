/*
 * channel_event_handler.h
 * 
 * Declaration of a channel event handler
 *
 * Author: Philipp Grabenweger
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


#ifndef __QKD_QKD_SIMULATE_CHANNEL_EVENT_HANDLER_H_
#define __QKD_QKD_SIMULATE_CHANNEL_EVENT_HANDLER_H_


// -----------------------------------------------------------------
// incs

// ait
#include "event.h"


// -----------------------------------------------------------------
// decl


namespace qkd {
    
namespace simulate {

    
// fwd. decl.
class channel_event_manager;
class photon_pair_manager;


/**
 * channel event handler
 */
class channel_event_handler {

    
public:
    
    
    /** 
     * ctor
     */
    channel_event_handler() : m_cParent(nullptr) {}
    
    
    /** 
     * ctor that sets parent channel event handler
     * 
     * @param   cParent     parent handler
     */
    channel_event_handler(channel_event_handler * cParent) : m_cParent(cParent) {}
    
    
    /** 
     * dtor
     */
    virtual ~channel_event_handler() {};
    
    
    /**
     * get the channel event handler's fully qualified name
     *
     * @return  the channel event handler's name
     */
    std::string get_name() const { if (!parent()) return m_sName; return parent()->get_name() + "." + m_sName; };
    
    
    /**
     * handle a channel event. 
     * 
     * @param   cEvent      the channel event to be handled
     */
    virtual void handle(event const & cEvent) = 0;
    
    
    /**
     * function that initializes the channel event handler. 
     * 
     * This function can be overrided by derived classes 
     * to do their simulation initialization, however, they should also call
     * the parent instance
     * 
     * Also should each channel event handler call this function for all 
     * of its contained components which are event handlers.
     * 
     * @param   cParent                 the parent channel event handler
     * @param   cManager              the event manager
     * @param   cPhotonPairManager      the photon pair manager
     */
    virtual void init(channel_event_handler * cParent, channel_event_manager * cManager, photon_pair_manager * cPhotonPairManager);  
    
    
    /**
     * get the manager of this
     * 
     * @return  the event manager of this one
     */
    channel_event_manager * manager() { return m_cManager; };
    
    
    /**
     * get the manager of this
     * 
     * @return  the event manager handler of this one
     */
    channel_event_manager const * manager() const { return m_cManager; };
    
    
    /**
     * get the parent of this
     * 
     * @return  the parent handler of this one
     */
    channel_event_handler * parent() { return m_cParent; };
    
    
    /**
     * get the parent of this
     * 
     * @return  the parent handler of this one
     */
    channel_event_handler const * parent() const { return m_cParent; };
    
    
    /**
     * get the manager of this
     * 
     * @return  the event manager of this one
     */
    photon_pair_manager * pp_manager() { return m_cPhotonPairManager; };
    
    
    /**
     * get the manager of this
     * 
     * @return  the event manager handler of this one
     */
    photon_pair_manager const * pp_manager() const { return m_cPhotonPairManager; };
    
    
    /**
     * set the channel event handler's name. 
     * 
     * @param   sName       channel event handler name
     */
    void set_name(std::string sName) { m_sName = sName; }
    
    
    /**
     * write out all parameters of this event handler and its child event handlers
     * 
     * @param   cStream     the stream to write to
     */
    virtual void write_parameters(std::ofstream & cStream) = 0;
    
    
private:
    

    /**
     * the parent channel event handler 
     */
    channel_event_handler * m_cParent;               

    
    /**
     * the channel event manager 
     */    
    channel_event_manager * m_cManager;                      

    
    /**
     * the channel event handler's name 
     */
    std::string m_sName;                                
    
    
    /**
     * the photon pair manager 
     */    
    photon_pair_manager * m_cPhotonPairManager;                       
};

}
}

#endif

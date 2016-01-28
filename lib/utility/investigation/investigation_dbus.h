/*
 * investigation_dbus.h
 * 
 * This is the concrete DBus investigation worker
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

 
#ifndef __QKD_UTILITY_INVESTIGATION_DBUS_H_
#define __QKD_UTILITY_INVESTIGATION_DBUS_H_


// ------------------------------------------------------------
// incs

// Qt
#include <QtDBus/QDBusConnection>

// ait
#include <qkd/utility/investigation.h>


// ------------------------------------------------------------
// decls


namespace qkd {
    
namespace utility {    

   
/**
 * this class works on DBus to find the current QKD System properties
 */
class investigation_dbus {


public:
    
    
    /**
     * invest and put the found items into collection
     * 
     * @param   cResult     the container to be filled
     */
    void investigate(qkd::utility::investigation_result & cResult);
    
    
private:    
    
    
    /**
     * add a link to our collection
     * 
     * @param   cInvestigationResult    to be filled
     * @param   cDBus                   the DBus
     * @param   sNodeServiceName        the service name of the node
     * @param   sNodeId                 the node id
     * @param   sLinkId                 the link id
     */
    void add_link(qkd::utility::investigation_result & cInvestigationResult, QDBusConnection & cDBus, QString const & sNodeServiceName, std::string const & sNodeId, std::string const & sLinkId);
    
    
    /**
     * add a module to our collection
     * 
     * @param   cInvestigationResult    to be filled
     * @param   cDBus                   the DBus
     * @param   sServiceName            the service name of the module
     */
    void add_module(qkd::utility::investigation_result & cInvestigationResult, QDBusConnection & cDBus, QString const & sServiceName);
    
    
    /**
     * add a node to our collection
     * 
     * @param   cInvestigationResult    to be filled
     * @param   cDBus                   the DBus
     * @param   sServiceName            the service name of the node
     */
    void add_node(qkd::utility::investigation_result & cInvestigationResult, QDBusConnection & cDBus, QString const & sServiceName);
    
};



}

}

#endif


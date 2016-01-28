/*
 * dbus.h
 * 
 * some dbus helpers
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

 
#ifndef __QKD_UTILITY_DBUS_H_
#define __QKD_UTILITY_DBUS_H_


// ------------------------------------------------------------
// incs

// Qt
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QVariant>


// ------------------------------------------------------------
// decls


namespace qkd {
    
namespace utility {    

    
/**
 * this class provides some dbus helper functions
 */
class dbus {


public:
    
    
    /**
     * turn the arguments in a QDBusMessage into a key-value map of strings and variants
     * 
     * The idea is to have a neat function to turn a DBus reply in a quick
     * access key-value container.
     * 
     * @param   cMessage        the DBus messge (presumable a reply)
     * @return  the arguments found as key-value map
     */
    static QMap<QString, QVariant> map(QDBusMessage const & cMessage);


    /**
     * get the QKD D-Bus connection
     *
     * @return  a QDBusConnection object for our QKD D-Bus
     */
    static QDBusConnection qkd_dbus();


    /**
     * check the given name if it can be used as a DBus service name particle
     * 
     * A DBus service name is the name under which a service
     * registeres itself on the DBus, like "at.ac.ait.q3p.node-alice".
     * There are certain restrictions to these names. Especially the
     * name particles ("at", "ac", ... "node-alice") may not contain
     * some reserved literals. This function checks this in advance.
     * 
     * @param   sName       the name to check
     * @return  true, if the given name is a good one to be used with DBus
     */
    static bool valid_service_name_particle(std::string sName);


private:


    /**
     * ctor
     */
    dbus() {};

};



}

}

#endif


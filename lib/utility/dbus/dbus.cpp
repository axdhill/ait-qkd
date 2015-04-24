/*
 * dbus.cpp
 * 
 * DBus helper implementation
 *
 * Autor: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *
 * Copyright (C) 2012-2015 AIT Austrian Institute of Technology
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

 
// ------------------------------------------------------------
// incs

// Qt
#include <QtDBus/QDBusArgument>
#include <QtDBus/QDBusVariant>

// ait
#include <qkd/utility/dbus.h>
#include <qkd/utility/debug.h>
#include <qkd/utility/syslog.h>


using namespace qkd::utility;


// ------------------------------------------------------------
// code


/**
 * turn the arguments in a QDBusMessage into a key-value map of strings and variants
 * 
 * The idea is to have a neat function to turn a DBus reply in a quick
 * access key-value container.
 * 
 * @param   cMessage        the DBus messge (presumable a reply)
 * @return  the arguments found as key-value map
 */
QMap<QString, QVariant> dbus::map(QDBusMessage const & cMessage) {
    
    QMap<QString, QVariant> cMap;

    // walk over what we've been told
    for (int64_t i = 0; i < cMessage.arguments().size(); i++) {
        
        // we expect an associative container of key, value pairs
        const QDBusArgument cArg = cMessage.arguments()[i].value<QDBusArgument>();
        if (cArg.currentType() == QDBusArgument::MapType) {
            
            // enter map
            cArg.beginMap();
            while (!cArg.atEnd()) {
                
                QString sKey;
                QDBusVariant cValue;
                
                cArg.beginMapEntry();
                cArg >> sKey;
                cArg >> cValue;
                cArg.endMapEntry();
                
                cMap.insert(sKey, cValue.variant());
            }

            cArg.endMap();
        }
    }
    
    return cMap;
}


/**
 * get the QKD D-Bus connection
 *
 * @return  a QDBusConnection object for our QKD D-Bus
 */
QDBusConnection dbus::qkd_dbus() {

    // grap environment var
    std::string sDBusAddress = "";
    char * sQKDDBusAddress = getenv("QKD_DBUS_SESSION_ADDRESS");
    if (sQKDDBusAddress != nullptr) sDBusAddress = sQKDDBusAddress;

    // check var
    if (sDBusAddress.empty()) {
        qkd::utility::syslog::warning() << "Can't get valid QKD D-Bus address from $QKD_DBUS_SESSION_ADDRESS environment variable. Using standard session D-Bus.";
        return QDBusConnection::sessionBus();
    }

    qkd::utility::debug() << "using QKD D-Bus at " << sDBusAddress;
    return QDBusConnection::connectToBus(QString::fromStdString(sDBusAddress), "qkd-dbus");
}


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
 */
bool dbus::valid_service_name_particle(std::string sName) {
    if (sName.length() == 0) return false;
    uint64_t nPosition = sName.find_first_of(" !\"§$%&/()=?;,:.\'");
    return (nPosition == sName.npos);
}

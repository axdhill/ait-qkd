/*
 * investigation_dbus.cpp
 * 
 * investigation DBus worker implementation
 *
 * Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
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
#include <QtDBus/QDBusConnectionInterface>
#include <QtDBus/QDBusMessage>

// ait
#include <qkd/utility/dbus.h>
#include <qkd/utility/debug.h>
#include <qkd/utility/syslog.h>

#include "investigation_dbus.h"


using namespace qkd::utility;


// ------------------------------------------------------------
// code


/**
 * add a link to our collection
 * 
 * @param   cInvestigationResult    to be filled
 * @param   cDBus                   the DBus
 * @param   sNodeServiceName        the service name of the node
 * @param   sNodeId                 the node id
 * @param   sLinkId                 the link id
 */
void investigation_dbus::add_link(qkd::utility::investigation_result & cInvestigationResult, QDBusConnection & cDBus, QString const & sNodeServiceName, std::string const & sNodeId, std::string const & sLinkId) {
    
    qkd::utility::debug() << "found link: '" << sLinkId << "' on node '" << sNodeId << "' collecting data ...";
    
    // call "GetAll" on the link
    QDBusMessage cMessage = QDBusMessage::createMethodCall(sNodeServiceName, QString("/Link/") + QString::fromStdString(sLinkId), "org.freedesktop.DBus.Properties", "GetAll");
    cMessage << "at.ac.ait.q3p.link";
    QDBusMessage cReply = cDBus.call(cMessage);
    
    if (cReply.type() == QDBusMessage::ErrorMessage) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to call org.freedesktop.DBus.Properties.GetAll on " << sNodeServiceName.toStdString() << " at /Link/" << sLinkId << " with interface at.ac.ait.q3p.link - omitting ...";
        return;
    }
    if (cReply.type() != QDBusMessage::ReplyMessage) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "unexpected reply from DBus from call org.freedesktop.DBus.Properties.GetAll on " << sNodeServiceName.toStdString() << " at /Link/" << sLinkId << " with interface at.ac.ait.q3p.link - omitting ...";
        return;
    }
    
    // parse the result
    QMap<QString, QVariant> cResult = qkd::utility::dbus::map(cReply);
    if (!cResult.contains("id")) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << sNodeServiceName.toStdString() << ": link has no id - huh? Go, tell Oliver! This mustnot happen! o.O";
        return;
    }
    
    // fetch the entry for the node
    qkd::utility::properties cLinkProperties;
    
    // get all key-value data
    QMap<QString, QVariant>::const_iterator cResultItem = cResult.constBegin();
    while (cResultItem != cResult.constEnd()) {
        cLinkProperties[cResultItem.key().toStdString()] = cResultItem.value().toString().toStdString();
        ++cResultItem;
    }
    
    // remember DBus setting
    cLinkProperties["node"] = sNodeId;    
    cLinkProperties["dbus"] = sNodeServiceName.toStdString() + " /Node/" + cLinkProperties["id"];
    
    // final move
    cInvestigationResult.cLinks[sNodeId + "/" + cLinkProperties["id"]] = cLinkProperties;
}


/**
 * add a module to our collection
 * 
 * @param   cInvestigationResult    to be filled
 * @param   cDBus                   the DBus
 * @param   sServiceName            the service name of the module
 */
void investigation_dbus::add_module(qkd::utility::investigation_result & cInvestigationResult, QDBusConnection & cDBus, QString const & sServiceName) {

    qkd::utility::debug() << "found module: '" << sServiceName.toStdString() << "' collecting data ...";
    
    // call "GetAll" on the module
    QDBusMessage cMessage = QDBusMessage::createMethodCall(sServiceName, "/Module", "org.freedesktop.DBus.Properties", "GetAll");
    cMessage << "at.ac.ait.qkd.module";
    QDBusMessage cReply = cDBus.call(cMessage);
    
    if (cReply.type() == QDBusMessage::ErrorMessage) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to call org.freedesktop.DBus.Properties.GetAll on " << sServiceName.toStdString() << " with interface at.ac.ait.qkd.module - omitting ...";
        return;
    }
    if (cReply.type() != QDBusMessage::ReplyMessage) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "unexpected reply from DBus from call org.freedesktop.DBus.Properties.GetAll on " << sServiceName.toStdString() << " with interface at.ac.ait.qkd.module - omitting ...";
        return;
    }
    
    // parse the result
    QMap<QString, QVariant> cResult = qkd::utility::dbus::map(cReply);
    if (!cResult.contains("id")) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << sServiceName.toStdString() << ": module has no id - huh? Go, tell Oliver! This mustnot happen! o.O";
        return;
    }
    
    // get all key-value data
    qkd::utility::properties cModuleProperties;
    QMap<QString, QVariant>::const_iterator cResultItem = cResult.constBegin();
    while (cResultItem != cResult.constEnd()) {
        cModuleProperties[cResultItem.key().toStdString()] = cResultItem.value().toString().toStdString();
        ++cResultItem;
    }
    
    // remember DBus setting
    cModuleProperties["dbus"] = sServiceName.toStdString();
    
    // final move
    cInvestigationResult.cModules[cModuleProperties["id"] + "-" + cModuleProperties["process_id"]] = cModuleProperties;
    
    // TODO: add also to pipeline
}


/**
 * add a node to our collection
 * 
 * @param   cInvestigationResult    to be filled
 * @param   cDBus                   the DBus
 * @param   sServiceName            the service name of the node
 */
void investigation_dbus::add_node(qkd::utility::investigation_result & cInvestigationResult, QDBusConnection & cDBus, QString const & sServiceName) {
    
    qkd::utility::debug() << "found node: '" << sServiceName.toStdString() << "' collecting data ...";
    
    // call "GetAll" on the module
    QDBusMessage cMessage = QDBusMessage::createMethodCall(sServiceName, "/Node", "org.freedesktop.DBus.Properties", "GetAll");
    cMessage << "at.ac.ait.q3p.node";
    QDBusMessage cReply = cDBus.call(cMessage);
    
    if (cReply.type() == QDBusMessage::ErrorMessage) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to call org.freedesktop.DBus.Properties.GetAll on " << sServiceName.toStdString() << " with interface at.ac.ait.q3p.node - omitting ...";
        return;
    }
    if (cReply.type() != QDBusMessage::ReplyMessage) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "unexpected reply from DBus from call org.freedesktop.DBus.Properties.GetAll on " << sServiceName.toStdString() << " with interface at.ac.ait.q3p.node - omitting ...";
        return;
    }
    
    // parse the result
    QMap<QString, QVariant> cResult = qkd::utility::dbus::map(cReply);
    if (!cResult.contains("id")) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << sServiceName.toStdString() << ": node has no id - huh? Go, tell Oliver! This mustnot happen! o.O";
        return;
    }
    
    // get all key-value data
    qkd::utility::properties cNodeProperties;
    QMap<QString, QVariant>::const_iterator cResultItem = cResult.constBegin();
    while (cResultItem != cResult.constEnd()) {
        cNodeProperties[cResultItem.key().toStdString()] = cResultItem.value().toString().toStdString();
        ++cResultItem;
    }
    
    // remember DBus setting
    cNodeProperties["dbus"] = sServiceName.toStdString();
    
    // final move
    cInvestigationResult.cNodes[cNodeProperties["id"]] = cNodeProperties;

    // iterate trough the links of the node
    cMessage = QDBusMessage::createMethodCall(sServiceName, "/Node", "at.ac.ait.q3p.node", "links");
    cReply = cDBus.call(cMessage);
    if (cReply.type() != QDBusMessage::ReplyMessage) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to call to get the list of links from the node '" << sServiceName.toStdString() << "'";
        return;
    }
        
    // get all the links
    QStringList cLinkList = cReply.arguments()[0].toStringList();
    for (int i = 0; i < cLinkList.size(); i++) {
        add_link(cInvestigationResult, cDBus, sServiceName, cNodeProperties["id"], cLinkList[i].toStdString());
    }
}


/**
 * invest and put the found items into collection
 * 
 * @param   cResult     the container to be filled
 */
void investigation_dbus::investigate(qkd::utility::investigation_result & cResult) {

    // get the DBus
    QDBusConnection cDBus = qkd::utility::dbus::qkd_dbus();
    
    // scan the system
    qkd::utility::debug() << "Scanning DBus ...";
    
    const QStringList sServiceNames = cDBus.interface()->registeredServiceNames();
    for (int i = 0; i < sServiceNames.size(); i++) {
        if (sServiceNames[i].startsWith("at.ac.ait.q3p.node")) add_node(cResult, cDBus, sServiceNames[i]);
        if (sServiceNames[i].startsWith("at.ac.ait.qkd.module")) add_module(cResult, cDBus, sServiceNames[i]);
    }
}

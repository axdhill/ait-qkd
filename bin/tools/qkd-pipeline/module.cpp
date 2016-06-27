/*
 * module.cpp
 * 
 * declares a module to be loaded by the qkd-pipeline tool
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

 
// ------------------------------------------------------------
// incs

#include <iostream>
#include <thread>

#include <QtXml/QDomDocument>

#include <qkd/utility/dbus.h>
#include <qkd/utility/investigation.h>

#include "module.h"


// ------------------------------------------------------------
// code


/**
 * ctor
 */
module::module() {
    clear();
}


/**
 * ctor
 * 
 * @param   sDBusServiceName        the known DBus service name of the module
 */
module::module(std::string const & sDBusServiceName) {
    clear();
    m_sDBusServiceName = sDBusServiceName;
}


/**
 * clear the module values
 */
void module::clear() {
    m_sProcessImage = "";
    m_sConfiguration = "",
    m_bAlice = true;
    m_sLogFile = "";
}


/**
 * create the command line for this module
 * 
 * @param   cArgs       the command line arguments
 * @param   nArgs       the number of command line arguments
 */
unsigned int module::command_line(char * cArgs[1024]) {
    
    unsigned int nArgs = 0;
    
    cArgs[nArgs++] = strdup(executable().c_str());
    if (!is_alice()) cArgs[nArgs++] = strdup("--bob");
    cArgs[nArgs++] = strdup("--config");
    cArgs[nArgs++] = strdup(configuration_file().c_str());
    for (auto const s : arguments()) {
        cArgs[nArgs++] = strdup(s.c_str());
        if (nArgs == 1024) break;
    }
    
    cArgs[nArgs++] = nullptr;
    
    return nArgs;
}


/**
 * "pause()" on the DBus object
 */
void module::dbus_call_pause() {
    
    QDBusConnection cDBus = qkd::utility::dbus::qkd_dbus();
    QDBusMessage cMessage;

    cMessage = QDBusMessage::createMethodCall(
            QString::fromStdString(dbus_service_name()), 
            "/Module", 
            "at.ac.ait.qkd.module",
            "pause");
    cDBus.call(cMessage, QDBus::NoBlock);
}


/**
 * "terminate()" on the DBus object
 */
void module::dbus_call_terminate() {
    
    QDBusConnection cDBus = qkd::utility::dbus::qkd_dbus();
    QDBusMessage cMessage;

    cMessage = QDBusMessage::createMethodCall(
            QString::fromStdString(dbus_service_name()), 
            "/Module", 
            "at.ac.ait.qkd.module",
            "terminate");
    cDBus.call(cMessage, QDBus::NoBlock);
}


/**
 * get the "url_pipe_in" property on the DBus object
 * 
 * @return  the DBus property of URL pipe in
 */
QString module::dbus_get_url_pipe_in() const {
    
    QDBusConnection cDBus = qkd::utility::dbus::qkd_dbus();
    QDBusMessage cMessage;

    cMessage = QDBusMessage::createMethodCall(
            QString::fromStdString(dbus_service_name()), 
            "/Module", 
            "org.freedesktop.DBus.Properties", 
            "Get");
    cMessage << "at.ac.ait.qkd.module" << "url_pipe_in";
    QDBusReply<QDBusVariant> cReplyPipeIn = cDBus.call(cMessage);
    
    return cReplyPipeIn.value().variant().toString();
}


/**
 * get the "url_pipe_out" property on the DBus object
 * 
 * @return  the DBus property of URL pipe out
 */
QString module::dbus_get_url_pipe_out() const {

    QDBusConnection cDBus = qkd::utility::dbus::qkd_dbus();
    QDBusMessage cMessage;

    cMessage = QDBusMessage::createMethodCall(
            QString::fromStdString(dbus_service_name()), 
            "/Module", 
            "org.freedesktop.DBus.Properties", 
            "Get");
    cMessage << "at.ac.ait.qkd.module" << "url_pipe_out";
    QDBusReply<QDBusVariant> cReplyPipeIn = cDBus.call(cMessage);
    
    return cReplyPipeIn.value().variant().toString();
}


/**
 * set the "url_pipe_in" property on the DBus object
 * 
 * @param   sURLPipeIn      the new URL pipe in
 */
void module::dbus_set_url_pipe_in(QString sURLPipeIn) {
    
    QDBusConnection cDBus = qkd::utility::dbus::qkd_dbus();
    QDBusMessage cMessage;
    
    cMessage = QDBusMessage::createMethodCall(
            QString::fromStdString(dbus_service_name()), 
            "/Module", 
            "org.freedesktop.DBus.Properties", 
            "Set");

    cMessage 
            << "at.ac.ait.qkd.module" 
            << "url_pipe_in" 
            << QVariant::fromValue(QDBusVariant(sURLPipeIn)); 

    cDBus.call(cMessage, QDBus::NoBlock);
}


/**
 * set the "url_pipe_out" property on the DBus object
 * 
 * @param   sURLPipeOut     the new URL pipe out
 */
void module::dbus_set_url_pipe_out(QString sURLPipeOut) {
    
    QDBusConnection cDBus = qkd::utility::dbus::qkd_dbus();
    QDBusMessage cMessage;
    cMessage = QDBusMessage::createMethodCall(
            QString::fromStdString(dbus_service_name()), 
            "/Module", 
            "org.freedesktop.DBus.Properties", 
            "Set");

    cMessage 
            << "at.ac.ait.qkd.module" 
            << "url_pipe_out" 
            << QVariant::fromValue(QDBusVariant(sURLPipeOut)); 

    cDBus.call(cMessage, QDBus::NoBlock);
}


/**
 * prints the module definition in a human-readable format to the target output stream
 *
 * @param   cTarget             the stream to write to
 */
void module::dump(std::ostream & cTarget) const {
    
    cTarget << "Module '" << m_sProcessImage << "'," << std::endl
            << "\twith configuration: '" << m_sConfiguration << "'" << std::endl
            << "\tdbus name: '" << m_sDBusServiceName << "'" << std::endl
            << "\tlogging path: '" << m_sLogFile << "'" << std::endl
            << "\t" << (m_bAlice ? "(alice)" : "(bob)") << std::endl;
}


/**
 * try to locate and check the full path of a module's executable
 * 
 * @return  a valid path object if found
 */
boost::filesystem::path module::executable() const {
    
    boost::filesystem::path cExecutable;
    
    try {
        cExecutable = qkd::utility::environment::find_executable(process_image());
    } 
    catch (const boost::filesystem::filesystem_error & filesystem_error) {
        dump(std::cerr);
        std::cerr << "Exception was thrown while trying to locate executable "
                  << process_image() << std::endl
                  << filesystem_error.what() << std::endl;
    }
    
    return cExecutable;
}


/**
 * get the module via a process ID
 * 
 * @param   sPID        the process id of the running module
 */
module module::get_by_pid(std::string const & sPID) {
    
    module cModule;
    int nTries = 50;
    do {

        qkd::utility::investigation cInvestigation = qkd::utility::investigation::investigate();
        for (auto const & p : cInvestigation.modules()) {

            if (p.second.at("process_id") == sPID) {
                cModule = module(p.second.at("dbus"));
                break;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        nTries--;

    } while (!cModule.is_valid() && (nTries > 0));
    
    return cModule;
}


/**
 * check if this is a valid module definition
 *
 * @return  true, if we have a valid module at hand
 */
bool module::is_valid() const {
    return !m_sDBusServiceName.empty();
}


/**
 * parse a single module XML node
 * 
 * the found values are inserted into the global
 * pipeline variable (g_cPipeline)
 * 
 * @param   cModuleElement      the module XML element
 * @return  0 for success, else errorcode as for main()
 */
int module::parse(QDomElement const & cModuleElement) {
    
    clear();
    
    if (cModuleElement.tagName() != "module") {
        std::cerr << "failed to parse configuration: element 'module' for a single module not found." << std::endl;
        return 1;
    }
    
    if (!cModuleElement.hasAttribute("path")) {
        std::cerr << "module lacks 'path' attribute which is mandatory." << std::endl;
        return 1;
    }
    m_sProcessImage = cModuleElement.attribute("path").toStdString();
    
    for (QDomNode cNode = cModuleElement.firstChild(); !cNode.isNull(); cNode = cNode.nextSibling()) {
        
        if (!cNode.isElement()) continue;
        
        QDomElement cDomElement = cNode.toElement();
        if (cDomElement.tagName() == "config") {
            if (cDomElement.hasAttribute("path")) {
                m_sConfiguration = cDomElement.attribute("path").toStdString();    
            }
        }
        else
            
        if (cDomElement.tagName() == "role") {
            if (cDomElement.hasAttribute("value")) {
                std::string sModuleRole = cDomElement.attribute("value").toStdString();
                if (sModuleRole == "alice") {
                    m_bAlice = true;
                }
                else
                if (sModuleRole == "bob") {
                    m_bAlice = false;
                }
                else {
                    std::cerr << "module: '" 
                            << m_sProcessImage 
                            << "' - ignoring role value '" << 
                            sModuleRole << "'." << 
                            std::endl;
                }
            }
        }
        else
            
        if (cDomElement.tagName() == "args") {
            if (cDomElement.hasAttribute("value")) {
                m_cArgs.push_back(cDomElement.attribute("value").toStdString());
            }
            if (!cDomElement.text().isEmpty()) {
                m_cArgs.push_back(cDomElement.text().toStdString());
            }
        }
        else
            
        if (cDomElement.tagName() == "log") {
            if (cDomElement.hasAttribute("path")) m_sLogFile = cDomElement.attribute("path").toStdString();    
        }
        
        else {
            std::cerr << "module: '" 
                    << m_sProcessImage 
                    << "' - ignoring unknown tag '" 
                    << cDomElement.tagName().toStdString() 
                    << std::endl;
        }
    }
    
    return 0;
}


/**
 * start the module
 */
void module::start() {
    
    if (!is_valid()) return;
    
    QDBusConnection cDBus = qkd::utility::dbus::qkd_dbus();
    std::cout << m_sDBusServiceName << "..." << std::endl;

    QDBusMessage cMessage;
    QDBusMessage cReply;

    cMessage = QDBusMessage::createMethodCall(
            QString::fromStdString(m_sDBusServiceName), 
            "/Module", 
            "at.ac.ait.qkd.module",
            "run");
    cReply = cDBus.call(cMessage);
    wait_for_module_state(qkd::module::STATE_READY);

    cMessage = QDBusMessage::createMethodCall(
            QString::fromStdString(m_sDBusServiceName), 
            "/Module", 
            "at.ac.ait.qkd.module",
            "resume");
    cReply = cDBus.call(cMessage);
    wait_for_module_state(qkd::module::STATE_RUNNING);
}


/**
 * waits until the module reached a certain state
 *
 * @param   eState                  module state to wait for
 */
bool module::wait_for_module_state(qkd::module::module_state eState) {
    
    bool res = false;

    // timeout: 50 * 100 millisec --> 5 sec
    QDBusConnection cDBus = qkd::utility::dbus::qkd_dbus();
    int nTries = 50;
    do {

        QDBusMessage cMessage = QDBusMessage::createMethodCall(
                QString::fromStdString(m_sDBusServiceName), 
                "/Module", 
                "org.freedesktop.DBus.Properties", 
                "Get");
        cMessage << "at.ac.ait.qkd.module" << "state";
        QDBusReply<QDBusVariant> cReply = cDBus.call(cMessage);
        int nModuleState = cReply.value().variant().toInt();

        if ((int)eState == nModuleState) {
            res = true;
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        nTries--;

    } while (!res && (nTries > 0));

    return res;
}

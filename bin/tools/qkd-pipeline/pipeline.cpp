/*
 * pipeline.cpp
 * 
 * declares a pipeline to be loaded by the qkd-pipeline tool
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

#include <fstream>
#include <iostream>
#include <thread>

#include <signal.h>

#include <boost/filesystem.hpp>

#include <QtCore/QFile>
#include <QtCore/QUrl>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusReply>
#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>

#include <qkd/utility/environment.h>
#include <qkd/utility/dbus.h>
#include <qkd/utility/investigation.h>

#include "pipeline.h"


// ------------------------------------------------------------
// decl


/**
 * test if the given URL can be worked with
 *
 * @param   sURL        url to be worked with
 * @return  true, if URL is ok
 */
static bool ensure_writeable(std::string const & sURL);


/**
 * read child's PID from file
 *
 * @param   cPath       the file to read
 * @return  child's pid (as string number)
 */
static std::string read_child_pid(boost::filesystem::path const & cPath);


/**
 * write current PID into file
 *
 * @param   cPath       the file to write
 */
static void write_current_pid(boost::filesystem::path const & cPath);


// ------------------------------------------------------------
// code


/**
 * autoconnect modules
 *
 * @return  true for success
 */
bool pipeline::autoconnect_modules() {

    if (m_cModules.empty()) return false;

    // our ipc sockets will be placed in ${TMP}/qkd
    boost::filesystem::path cSocketPath = qkd::utility::environment::temp_path() / "qkd";

    QString sNextModulePipeIn = QString::fromStdString(m_sURLPipeOut);

    // interconnect modules in reverse order
    for (auto iter = m_cModules.rbegin(); iter != m_cModules.rend(); ++iter) {

        if (!(*iter).is_valid()) continue;
        
        (*iter).dbus_call_pause();

        boost::filesystem::path cPipeInPath = cSocketPath / (*iter).dbus_service_name();
        QString sURLPipeIn = "ipc://" + QString::fromStdString(cPipeInPath.string());
        
        (*iter).dbus_set_url_pipe_in(sURLPipeIn);
        (*iter).dbus_set_url_pipe_out(sNextModulePipeIn);

        sNextModulePipeIn = sURLPipeIn;
    }


    // all done -> fix pipeline entry point

    set_pipeline_entry();
    
    return true;
}


/**
 * parse the given XML config file
 * 
 * @param   sPipelineConfiguration      path to configuration file
 * @return  0 for success, else errorcode as for main()
 */
int pipeline::parse(std::string const & sPipelineConfiguration) {
    
    QFile cFile(QString::fromStdString(sPipelineConfiguration));
    if (!cFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        std::cerr << "failed to open '" << sPipelineConfiguration << "'." << std::endl;
        return 1;
    }
    
    QDomDocument cDomDoc;
    QString sDomParsingError;
    int nDomParsingErrorLine;
    int nDomParsingErrorColumn;
    if (!cDomDoc.setContent(&cFile, &sDomParsingError, &nDomParsingErrorLine, &nDomParsingErrorColumn)) {
        std::cerr << "error parsing pipeline configuration file.\n \
             error: '" << sDomParsingError.toStdString() << "'\n \
              line: " << nDomParsingErrorLine << "\n \
            column: " << nDomParsingErrorColumn << std::endl;
        return 1;
    }
    
    QDomElement cRootElement = cDomDoc.documentElement();
    if (cRootElement.tagName() != "pipeline") {
        std::cerr << "failed to parse configuration: root element 'pipeline' not found." << std::endl;
        return 1;
    }
    
    if (!cRootElement.hasAttribute("name")) {
        std::cerr << "pipeline tag element has no 'name' attribute which is mandatory." << std::endl;
        return 1;
    }
    m_sName = cRootElement.attribute("name").toStdString();
    
    if (cRootElement.hasAttribute("autoconnect")) {
        m_bAutoConnect = (cRootElement.attribute("autoconnect") == "true");
    }
    
    if (cRootElement.hasAttribute("pipein")) {
        m_sURLPipeIn = cRootElement.attribute("pipein").toStdString();
        if (!ensure_writeable(m_sURLPipeIn)) {
            std::cerr << "cannot deal with pipein '" << m_sURLPipeIn << "'" << std::endl;
        }
    }

    if (cRootElement.hasAttribute("pipeout")) {
        m_sURLPipeOut = cRootElement.attribute("pipeout").toStdString();
        if (!ensure_writeable(m_sURLPipeOut)) {
            std::cerr << "cannot deal with pipeout '" << m_sURLPipeOut << "'" << std::endl;
        }
    }

    int nModuleErrorCode = 0;
    for (QDomNode cNode = cRootElement.firstChild(); !cNode.isNull() && (nModuleErrorCode == 0); cNode = cNode.nextSibling()) {
        if (cNode.isElement()) {
            module cModule;
            if ((nModuleErrorCode = cModule.parse(cNode.toElement())) == 0) {
                m_cModules.push_back(cModule);
            }
        }
    }
    
    if (nModuleErrorCode == 0) {
        std::cout << "modules found: " << m_cModules.size() << std::endl;
    }
    
    return nModuleErrorCode;
}


/**
 * retrieves the pipeline entry and exit URLs
 *
 * @param   sURLPipeIn      pipeline entry
 * @param   sURLPipeOut     pipeline exit
 */
void pipeline::pipeline_pipes(std::string & sURLPipeIn, std::string & sURLPipeOut) const {

    sURLPipeIn = std::string();
    sURLPipeOut = std::string();

    if (m_cModules.empty()) return;
    
    // pipein is url_pipe_in of the first module
    // pipeout is url_pipe_out of the last valid module
    
    sURLPipeIn = m_cModules.front().dbus_get_url_pipe_in().toStdString();
    
    for (auto iter = m_cModules.rbegin(); iter != m_cModules.rend(); ++iter) {
        if ((*iter).is_valid()) {
            sURLPipeOut = (*iter).dbus_get_url_pipe_out().toStdString();
            break;
        }
    }
}


/**
 * set the pipeline entry socket
 */
void pipeline::set_pipeline_entry() {

    if (m_sURLPipeIn.empty()) return;

    auto cModule = m_cModules.front();
    if (!cModule.is_valid()) {
        std::cerr << "first module in pipeline is invalid - refused to set pipeline entry point" << std::endl;
        return;
    }
    
    cModule.dbus_set_url_pipe_in(QString::fromStdString(m_sURLPipeIn));
}


/**
 * start the pipeline
 * 
 * @return  0 for success, else errorcode as for main()
 */
int pipeline::start() {

    if (m_sLogFolder.size()) {
        if (!verify_log_folder()) return 1;
    }
    boost::filesystem::path cLogFolder(m_sLogFolder);
        
    std::cout << "starting modules ..." << std::endl;
    
    int nModulesLaunched = 0;
    for (auto & cModule : m_cModules) {
        
        // try to locate the executable
        boost::filesystem::path cExecutable = cModule.executable();
        if (!cExecutable.string().size()) {
            std::cerr << "module: '"
                      << cModule.process_image()
                      << "' - error: failed to locate executable '"
                      << cModule.process_image() << "'"
                      << std::endl;
            continue;
        }

        // write current pid into pid file 
        boost::filesystem::path cPIDFileName = qkd::utility::environment::temp_path() / "qkd-pipeline.autoconnect.module.pid";

        // fork and daemonize
        if (!fork()) {
            
            // this is within a new child
            // now daemon() does another fork
            // so we have to get a hold on
            // the child's child PID --> we write it into a file
            // since stdin and stdout are lost now

            if (daemon(1, 0) == -1) {
                std::cerr << "module: '" 
                        << cModule.process_image() 
                        << "' - error: failed to daemonize subprocess." 
                        << std::endl;
            }
            else {

                // write actual PID into tmp file to be read
                // by the qkd-pipeline tool again to find DBus service name
                // of current module
                write_current_pid(cPIDFileName);

                // redirect to log file
                if (m_sLogFolder.size() && cModule.log_file().size()) {
                    
                    boost::filesystem::path cLogFile = cLogFolder / boost::filesystem::path(cModule.log_file());
                    if (!freopen(cLogFile.string().c_str(), "a+", stderr)) {
                        std::cerr << "module: '"
                                  << cModule.process_image()
                                  << "' - error: failed to redirect stderr (" << strerror(errno) << ")"
                                  << std::endl;
                        exit(1);
                    }
                }
                
                char * argv[1024];
                cModule.command_line(argv);
                if (execv(argv[0], argv) == -1) {
                    
                    // if we end up here execv failed
                    int nError = errno;
                    std::cerr << "module: '" 
                            << cModule.process_image()
                            << "' - error: failed to start subprocess: " 
                            << strerror(nError) 
                            << " (" 
                            << nError 
                            << ")"  
                            << std::endl;
                }
                
                // we reach this point: fail!
                exit(1);
                
            }
        }
        else {
            
            std::cout << "started module: " << cModule.process_image() << " ";
            std::string sChildPID = read_child_pid(cPIDFileName);
            cModule = module::get_by_pid(sChildPID);
            if (!cModule.is_valid()) {
                std::cout << " --- failed" << std::endl;
                std::cerr << "unable to get module PID or DBus service name - is module running?" << std::endl;
                continue;
            }

            std::cout << "DBus: " << cModule.dbus_service_name() << std::endl;
            nModulesLaunched++;
        }
    }

    if (nModulesLaunched == 0) {
        std::cerr << "failed to start a single module - this is futile" << std::endl;
        return 1;
    }

    if (m_bAutoConnect) {
        if (!autoconnect_modules()) {
            std::cerr << "failed to autoconnect modules" << std::endl;
        }
    }

    std::string sURLPipeIn;
    std::string sURLPipeOut;
    pipeline_pipes(sURLPipeIn, sURLPipeOut);
    std::cout << "pipeline entry point: " << sURLPipeIn << std::endl;
    std::cout << "pipeline exit point: " << sURLPipeOut << std::endl;
    
    start_modules();
    std::cout << "starting modules... done" << std::endl;
    
    return 0;
}


/**
 * start the modules of the pipeline
 */
void pipeline::start_modules() {

    std::cout << "starting modules..." << std::endl;

    if (m_cModules.empty()) return;
    for(module & m : m_cModules) {
        m.start();
    }
}


/**
 * stop the pipeline
 * 
 * @return  0 for success, else errorcode as for main()
 */
int pipeline::stop() {
    
    std::cout << "stopping modules..." << std::endl;
    
    qkd::utility::investigation cInvestigation = qkd::utility::investigation::investigate();
    QDBusConnection cDBus = qkd::utility::dbus::qkd_dbus();
    
    // iterate over the module definitions
    for (auto const & cModule : m_cModules) {
        
        // try to find this module in our module spec
        // to find we check:
        //      
        //      pipeline must be the same as our definition
        //      process_image must end in module-path
        //      role must be as our definition
        
        for (auto const & cModuleFound : cInvestigation.modules()) {
            
            // pipeline name?
            if (cModuleFound.second.at("pipeline") != m_sName) continue;
            
            // process image?
            std::string sProcessImage = cModuleFound.second.at("process_image");
            if (sProcessImage.size() < cModule.process_image().size()) continue;
            unsigned int nPos = sProcessImage.size() - cModule.process_image().size();
            sProcessImage = sProcessImage.substr(nPos, sProcessImage.size());
            if (sProcessImage != cModule.process_image()) continue;
            
            // role?
            std::string sRoleName = cModuleFound.second.at("role_name");
            if (cModule.is_alice() && (sRoleName != "alice")) continue;
            if (!cModule.is_alice() && (sRoleName != "bob")) continue;

            // found: all filter above apply.
            std::cout << "terminating module: " << cModuleFound.second.at("dbus") << std::endl;
            module m(cModuleFound.second.at("dbus"));
            m.dbus_call_terminate();
        }
    }

    std::cout << "stopping modules... done" << std::endl;
    
    return 0;
}


/**
 * verifies that the log folder exists and is writeable
 * 
 * @return  true, if we have a good log folder setting
 */
bool pipeline::verify_log_folder() const {
    
    if (!m_sLogFolder.size()) return false;
    
    boost::filesystem::path cLogFolder(m_sLogFolder);
    if (!boost::filesystem::exists(cLogFolder)) {
        boost::filesystem::create_directory(cLogFolder);
        if (!boost::filesystem::exists(cLogFolder)) {
            std::cerr << "failed to create log folder '" << m_sLogFolder << "'" << std::endl;
            return false;
        }
    }
    
    if (!boost::filesystem::is_directory(cLogFolder)) {
        std::cerr << "path '" << m_sLogFolder << "' is not a directory" << std::endl;
        return false;
    }
    
    return true;
}


/**
 * test if the given URL can be worked with
 *
 * @param   sURL        url to be worked with
 * @return  true, if URL is ok
 */
bool ensure_writeable(std::string const & sURL) {

    // void URLs are read-/writeable
    if (sURL.empty()) return true;

    QUrl cURL(QString::fromStdString(sURL));
    if (cURL.scheme() == "tcp") return true;
    if (cURL.scheme() != "ipc") return false;

    std::string sPath = cURL.path().toStdString();
    boost::filesystem::path cPath(sPath);

    // this is 'mkdir -p $(dirname sURL)'
    boost::filesystem::path p;
    for (auto d = cPath.begin(); d != --cPath.end(); ++d) {
        p = p / (*d);
        if (boost::filesystem::is_directory(p)) continue;
        if (boost::filesystem::exists(p)) return false;
        if (!boost::filesystem::create_directory(p)) return false;
    }

    bool res = boost::filesystem::exists(cPath);
    if (!res) {

        // test if we can create the file
        std::ofstream f(sPath);
        f << "";
        f.close();
        res = boost::filesystem::exists(cPath);
        boost::filesystem::remove(cPath);
    }

    return res;
}


/**
 * read child's PID from file
 *
 * @param   cPath       the file to read
 * @return  child's pid (as string number)
 */
std::string read_child_pid(boost::filesystem::path const & cPath) {

    std::string sChildPID;

    // timeout: 50 * 100 millisec --> 5 sec
    for (int i = 0; i < 50; ++i) {

        if (!boost::filesystem::exists(cPath)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        if (boost::filesystem::file_size(cPath) == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        // read and check if the PID exists
        sChildPID = "";
        std::ifstream cPIDFile;
        cPIDFile.open(cPath.string());
        cPIDFile >> sChildPID;
        cPIDFile.close();

        if (sChildPID.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        pid_t nChildPID = std::stoi(sChildPID);
        if (kill(nChildPID, 0) != 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            sChildPID = "";
            continue;
        }

        break;
    }

    if (!boost::filesystem::exists(cPath)) {
        return "";
    }
    boost::filesystem::remove(cPath);

    return sChildPID;
}


/**
 * write current PID into file
 *
 * @param   cPath       the file to write
 */
void write_current_pid(boost::filesystem::path const & cPath) {
    
    std::ofstream cPIDFile;
    cPIDFile.open(cPath.string());
    cPIDFile << getpid();
    cPIDFile.flush();
    cPIDFile.close();
}

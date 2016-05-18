/*
 * main.cpp
 * 
 * This is the qkd pipeline tool
 *
 * Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *
 * Copyright (C) 2013-2016 AIT Austrian Institute of Technology
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

#include <sys/types.h>
#include <signal.h>

#include <fstream>
#include <iostream>
#include <thread>
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

// Qt
#include <QtCore/QCoreApplication>
#include <QtCore/QFile>
#include <QtCore/QUrl>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusReply>
#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>

// ait
#include <qkd/module/module.h>
#include <qkd/utility/dbus.h>
#include <qkd/utility/environment.h>
#include <qkd/utility/investigation.h>


// ------------------------------------------------------------
// decl


/**
 * a single module definition as found in the configuration file
 */
struct module_definition {

    
    std::string sPath;                  /**< path to module binary */
    std::string sConfiguration;         /**< path to module's configuration file */
    bool bAlice;                        /**< alice role (or bob if false) */
    std::list<std::string> sArgs;       /**< additional arguments to pass on the command line */
    std::string sLog;                   /**< path to log file */
    std::string sDBusServiceName;       /**< DBus service name of started module */
        
    /**
     * clear the module values
     */
    void clear() {
        sPath = "";
        sConfiguration = "",
        bAlice = true;
        sLog = "";
    };
   

    /**
     * check if this is a valid module definition
     *
     * @return  true, if we have a valid module at hand
     */
    bool valid() const {
        return !sDBusServiceName.empty();
    }
};


/**
 * the pipeline definition found in the pipeline config xml
 */
struct {
    
    std::string sName;                          /**< pipeline name */
    std::string sLogFolder;                     /**< log folder */
    std::list<module_definition> cModules;      /**< list of modules */

    bool bAutoConnect = false;                  /**< autoconnect modules */
    std::string sURLPipeIn;                     /**< input URL of whole pipeline */
    std::string sURLPipeOut;                    /**< output URL of whole pipeline */

} g_cPipeline;


/**
 * autoconnect listened modules
 *
 * @return  true for success
 */
static bool autoconnect_modules();


/**
 * test if the given URL can be worked with
 *
 * @param   sURL        url to be worked with
 * @return  true, if URL is ok
 */
static bool ensure_writeable(std::string const & sURL);


/**
 * searches for the DBus service name of a module
 *
 * @param   sPID        process ID of module
 * @return  DBus Service Name of module
 */
static std::string get_dbus_service_name(std::string const & sPID);


/**
 * retrieves the pipeline entry and exit URLs
 *
 * @param   sURLPipeIn      pipeline entry
 * @param   sURLPipeOut     pipeline exit
 */
static void get_pipeline_pipes(std::string & sURLPipeIn, std::string & sURLPipeOut);


/**
 * parse the given XML config file
 * 
 * the found values are inserted into the global
 * pipeline variable (g_cPipeline)
 * 
 * @param   sPipelineConfiguration      path to configuration file
 * @return  0 for success, else errorcode as for main()
 */
static int parse(std::string const & sPipelineConfiguration);


/**
 * parse a single module XML node
 * 
 * the found values are inserted into the global
 * pipeline variable (g_cPipeline)
 * 
 * @param   cModuleElement      the module XML element
 * @return  0 for success, else errorcode as for main()
 */
static int parse_module(QDomElement const & cModuleElement);


/**
 * read child's PID from file
 *
 * @param   cPath       the file to read
 * @return  child's pid (as string number)
 */
static std::string read_child_pid(boost::filesystem::path const & cPath);


/**
 * set the pipeline entry socket
 */
static void set_pipeline_entry();


/**
 * start the pipeline
 * 
 * starts all modules specified in the global
 * pipeline variable (g_cPipeline)
 * 
 * @return  0 for success, else errorcode as for main()
 */
static int start();


/**
 * start the modules of the pipeline
 */
static void start_modules();


/**
 * stop the pipeline
 * 
 * stops all modules specified in the global
 * pipeline variable (g_cPipeline)
 * 
 * @return  0 for success, else errorcode as for main()
 */
static int stop();


/**
 * waits until a module reached a certain state
 *
 * @param   sDBusServiceName        service name of the module
 * @param   eState                  module state to wait for
 */
static bool wait_for_module_state(std::string const & sDBusServiceName, qkd::module::module_state eState);


/**
 * write current PID into file
 *
 * @param   cPath       the file to write
 * @return  true for success
 */
static void write_current_pid(boost::filesystem::path const & cPath);


// ------------------------------------------------------------
// code

/**
 * prints the module definition in a human-readable format to the target output stream
 *
 * @param   cTarget             the stream to write to
 * @param   cModuleDefinition   the module definition to print
 */
void print_module_details(std::ostream& cTarget, const module_definition& cModuleDefinition) {
    cTarget << "Module '" << cModuleDefinition.sPath << "'," << std::endl
    << "\twith configuration: '" << cModuleDefinition.sConfiguration << "'" << std::endl
    << "\tdbus name: '" << cModuleDefinition.sDBusServiceName << "'" << std::endl
    << "\tlogging path: '" << cModuleDefinition.sLog << "'" << std::endl
    << "\t" << (cModuleDefinition.bAlice ? "(alice)" : "(bob)") << std::endl;
}


/**
 * autoconnect listened modules
 *
 * @return  true for success
 */
bool autoconnect_modules() {

    if (g_cPipeline.cModules.empty()) return false;

    // our ipc sockets will be placed in ${TMP}/qkd
    boost::filesystem::path cSocketPath = qkd::utility::environment::temp_path() / "qkd";

    QString sNextModulePipeIn = QString::fromStdString(g_cPipeline.sURLPipeOut);

    // interconnect modules in reverse order
    QDBusConnection cDBus = qkd::utility::dbus::qkd_dbus();
    for (auto iter = g_cPipeline.cModules.rbegin(); iter != g_cPipeline.cModules.rend(); ++iter) {

        if (!(*iter).valid()) continue;

        QDBusMessage cMessage;

        cMessage = QDBusMessage::createMethodCall(
                QString::fromStdString((*iter).sDBusServiceName), 
                "/Module", 
                "at.ac.ait.qkd.module",
                "pause");
        cDBus.call(cMessage, QDBus::NoBlock);

        boost::filesystem::path cPipeInPath = cSocketPath / (*iter).sDBusServiceName;
        QString sURLPipeIn = "ipc://" + QString::fromStdString(cPipeInPath.string());

        cMessage = QDBusMessage::createMethodCall(
                QString::fromStdString((*iter).sDBusServiceName), 
                "/Module", 
                "org.freedesktop.DBus.Properties", 
                "Set");

        cMessage 
                << "at.ac.ait.qkd.module" 
                << "url_pipe_in" 
                << QVariant::fromValue(QDBusVariant(sURLPipeIn)); 

        cDBus.call(cMessage, QDBus::NoBlock);

        cMessage = QDBusMessage::createMethodCall(
                QString::fromStdString((*iter).sDBusServiceName), 
                "/Module", 
                "org.freedesktop.DBus.Properties", 
                "Set");

        cMessage 
                << "at.ac.ait.qkd.module" 
                << "url_pipe_out" 
                << QVariant::fromValue(QDBusVariant(sNextModulePipeIn)); 

        cDBus.call(cMessage, QDBus::NoBlock);

        sNextModulePipeIn = sURLPipeIn;
    }


    // all done -> fix pipeline entry point

    set_pipeline_entry();
    
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
 * searches for the DBus service name of a module
 *
 * @param   sPID        process ID of module
 * @return  DBus Service Name of module
 */
std::string get_dbus_service_name(std::string const & sPID) {

    std::string res = "";
    int nTries = 50;
    do {

        qkd::utility::investigation cInvestigation = qkd::utility::investigation::investigate();
        for (auto const & p : cInvestigation.modules()) {

            if (p.second.at("process_id") == sPID) {
                res = p.second.at("dbus");
                break;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        nTries--;

    } while (res.empty() && (nTries > 0));
    
    return res;
}


/**
 * retrieves the pipeline entry and exit URLs
 *
 * @param   sURLPipeIn      pipeline entry
 * @param   sURLPipeOut     pipeline exit
 */
void get_pipeline_pipes(std::string & sURLPipeIn, std::string & sURLPipeOut) {

    sURLPipeIn = std::string();
    sURLPipeOut = std::string();

    if (g_cPipeline.cModules.empty()) return;

    QDBusConnection cDBus = qkd::utility::dbus::qkd_dbus();
    QDBusMessage cMessage;

    std::string sFirstModuleServiceName = g_cPipeline.cModules.front().sDBusServiceName;
    cMessage = QDBusMessage::createMethodCall(
            QString::fromStdString(sFirstModuleServiceName), 
            "/Module", 
            "org.freedesktop.DBus.Properties", 
            "Get");
    cMessage << "at.ac.ait.qkd.module" << "url_pipe_in";
    QDBusReply<QDBusVariant> cReplyPipeIn = cDBus.call(cMessage);
    sURLPipeIn = cReplyPipeIn.value().variant().toString().toStdString();

    std::string sLastModuleServiceName;
    for (auto iter = g_cPipeline.cModules.rbegin(); iter != g_cPipeline.cModules.rend(); ++iter) {
        if ((*iter).valid()) {
            sLastModuleServiceName = (*iter).sDBusServiceName;
            break;
        }
    }
    if (sLastModuleServiceName.empty()) return;

    cMessage = QDBusMessage::createMethodCall(
            QString::fromStdString(sLastModuleServiceName), 
            "/Module", 
            "org.freedesktop.DBus.Properties", 
            "Get");
    cMessage << "at.ac.ait.qkd.module" << "url_pipe_out";
    QDBusReply<QDBusVariant> cReplyPipeOut = cDBus.call(cMessage);
    sURLPipeOut = cReplyPipeOut.value().variant().toString().toStdString();
}


/**
 * start
 * 
 * @param   argc        as usual
 * @param   argv        as usual
 * @return  as usual
 */
int main(int argc, char ** argv) {
    
    std::string sApplication = std::string("qkd-pipeline - AIT QKD Key Pipeline Tool V") + VERSION;
    std::string sDescription = std::string("\nThis tool lets you start/stop/restart a full QKD pipeline.\n\nCopyright 2013-2016 AIT Austrian Institute of Technology GmbH");
    std::string sSynopsis = std::string("Usage: ") + argv[0] + " [OPTIONS] COMMAND PIPELINE-CONFIG";
    
    boost::program_options::options_description cOptions(sApplication + "\n" + sDescription + "\n\n\t" + sSynopsis + "\n\nAllowed Options");
    cOptions.add_options()("help,h", "this page");
    cOptions.add_options()("log,l", boost::program_options::value<std::string>(), "path to log folder");
    cOptions.add_options()("version,v", "print version string");
    
    boost::program_options::options_description cArgs("Arguments");
    
    cArgs.add_options()("COMMAND", "COMMAND is either 'start', 'stop' or 'restart'.");
    cArgs.add_options()("PIPELINE-CONFIG", "PIPELINE-CONFIG is the path to the pipeline configuration XML.");
    
    boost::program_options::positional_options_description cPositionalDescription; 
    cPositionalDescription.add("COMMAND", 1);
    cPositionalDescription.add("PIPELINE-CONFIG", 1);
    
    boost::program_options::options_description cCmdLineOptions("Command Line");
    cCmdLineOptions.add(cOptions);
    cCmdLineOptions.add(cArgs);
    
    boost::program_options::variables_map cVariableMap;
    try {
        boost::program_options::command_line_parser cParser(argc, argv);
        boost::program_options::store(cParser.options(cCmdLineOptions).positional(cPositionalDescription).run(), cVariableMap);
        boost::program_options::notify(cVariableMap);        
    }
    catch (std::exception & cException) {
        std::cerr << "error parsing command line: " 
                << cException.what() 
                << "\ntype '--help' for help" 
                << std::endl;        
        return 1;
    }
    
    if (cVariableMap.count("help")) {
        std::cout << cOptions << std::endl;
        std::cout << cArgs.find("COMMAND", false).description() << "\n";      
        std::cout << cArgs.find("PIPELINE-CONFIG", false).description() << "\n" << std::endl;      
        return 0;
    }
    
    if (cVariableMap.count("version")) {
        std::cout << sApplication << std::endl;
        return 0;
    }
    
    if (cVariableMap.count("log")) {
        g_cPipeline.sLogFolder = cVariableMap["log"].as<std::string>();
        if (!boost::filesystem::exists(boost::filesystem::path(g_cPipeline.sLogFolder))) {
            std::cerr << "cannot access log folder '" << g_cPipeline.sLogFolder << "'." << std::endl;
            return 1;
        }
    }
    
    if (!cVariableMap.count("COMMAND")) {
        std::cerr << "no pipeline command.\ntype '--help' for information." << std::endl;
        return 1;
    }
    std::string sPipelineCommand = cVariableMap["COMMAND"].as<std::string>();
    if (!sPipelineCommand.size()) {
        std::cerr 
                << "neither 'start', 'stop' nor 'restart' specified.\nchoose one command - type '--help' for help." 
                << std::endl;
        return 1;
    }
    
    bool bStart = (sPipelineCommand == "start");
    bool bStop = (sPipelineCommand == "stop");
    bool bRestart = (sPipelineCommand == "restart");
    if (!bStart && !bStop && !bRestart) {
        std::cerr << "command '" 
                << sPipelineCommand 
                << "' unknown.\nchoose one command - type '--help' for help." 
                << std::endl;
        return 1;
    }

    if (!cVariableMap.count("PIPELINE-CONFIG")) {
        std::cerr << "no pipeline-config specified.\ntype '--help' for information." << std::endl;
        return 1;
    }
    std::string sPipelineConfiguration = cVariableMap["PIPELINE-CONFIG"].as<std::string>();
    
    QCoreApplication cApp(argc, argv);

    int nConfigErrorCode = parse(sPipelineConfiguration);
    if (nConfigErrorCode != 0) return nConfigErrorCode;
    
    int nStartErrorCode = 0;
    int nStopErrorCode = 0;
    if (bStop || bRestart) nStopErrorCode = stop();
    if (bStart || bRestart) nStartErrorCode = start();
    
    return std::max<int>(nStartErrorCode, nStopErrorCode);
}


/**
 * parse the given XML config file
 * 
 * the found values are inserted into the global
 * pipeline variable (g_cPipeline)
 * 
 * @param   sPipelineConfiguration      path to configuration file
 * @return  0 for success, else errorcode as for main()
 */
int parse(std::string const & sPipelineConfiguration) {
    
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
    g_cPipeline.sName = cRootElement.attribute("name").toStdString();
    
    if (cRootElement.hasAttribute("autoconnect")) {
        g_cPipeline.bAutoConnect = (cRootElement.attribute("autoconnect") == "true");
    }
    
    if (cRootElement.hasAttribute("pipein")) {
        g_cPipeline.sURLPipeIn = cRootElement.attribute("pipein").toStdString();
        if (!ensure_writeable(g_cPipeline.sURLPipeIn)) {
            std::cerr << "cannot deal with pipein '" << g_cPipeline.sURLPipeIn << "'" << std::endl;
        }
    }

    if (cRootElement.hasAttribute("pipeout")) {
        g_cPipeline.sURLPipeOut = cRootElement.attribute("pipeout").toStdString();
        if (!ensure_writeable(g_cPipeline.sURLPipeOut)) {
            std::cerr << "cannot deal with pipeout '" << g_cPipeline.sURLPipeOut << "'" << std::endl;
        }
    }

    int nModuleErrorCode = 0;
    for (QDomNode cNode = cRootElement.firstChild(); !cNode.isNull() && (nModuleErrorCode == 0); cNode = cNode.nextSibling()) {
        if (cNode.isElement()) {
            nModuleErrorCode = parse_module(cNode.toElement());
        }
    }
    
    if (nModuleErrorCode == 0) {
        std::cout << "modules found: " << g_cPipeline.cModules.size() << std::endl;
    }
    
    return nModuleErrorCode;
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
int parse_module(QDomElement const & cModuleElement) {
    
    module_definition cModule;
    cModule.clear();
    
    if (cModuleElement.tagName() != "module") {
        std::cerr << "failed to parse configuration: element 'module' for a single module not found." << std::endl;
        return 1;
    }
    
    if (!cModuleElement.hasAttribute("path")) {
        std::cerr << "module lacks 'path' attribute which is mandatory." << std::endl;
        return 1;
    }
    cModule.sPath = cModuleElement.attribute("path").toStdString();
    
    for (QDomNode cNode = cModuleElement.firstChild(); !cNode.isNull(); cNode = cNode.nextSibling()) {
        
        if (!cNode.isElement()) continue;
        
        QDomElement cDomElement = cNode.toElement();
        if (cDomElement.tagName() == "config") {
            if (cDomElement.hasAttribute("path")) {
                cModule.sConfiguration = cDomElement.attribute("path").toStdString();    
            }
        }
        else
            
        if (cDomElement.tagName() == "role") {
            if (cDomElement.hasAttribute("value")) {
                std::string sModuleRole = cDomElement.attribute("value").toStdString();
                if (sModuleRole == "alice") {
                    cModule.bAlice = true;
                }
                else
                if (sModuleRole == "bob") {
                    cModule.bAlice = false;
                }
                else {
                    std::cerr << "module: '" 
                            << cModule.sPath 
                            << "' - ignoring role value '" << 
                            sModuleRole << "'." << 
                            std::endl;
                }
            }
        }
        else
            
        if (cDomElement.tagName() == "args") {
            if (cDomElement.hasAttribute("value")) {
                cModule.sArgs.push_back(cDomElement.attribute("value").toStdString());
            }
            if (!cDomElement.text().isEmpty()) {
                cModule.sArgs.push_back(cDomElement.text().toStdString());
            }
        }
        else
            
        if (cDomElement.tagName() == "log") {
            if (cDomElement.hasAttribute("path")) cModule.sLog = cDomElement.attribute("path").toStdString();    
        }
        
        else {
            std::cerr << "module: '" 
                    << cModule.sPath 
                    << "' - ignoring unknown tag '" 
                    << cDomElement.tagName().toStdString() 
                    << std::endl;
        }
    }
    
    g_cPipeline.cModules.push_back(cModule);
    
    return 0;
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
 * set the pipeline entry socket
 */
void set_pipeline_entry() {

    if (g_cPipeline.sURLPipeIn.empty()) return;

    auto cModule = g_cPipeline.cModules.front();
    if (!cModule.valid()) {
        std::cerr << "first module in pipeline is invalid - refused to set pipeline entry point" << std::endl;
        return;
    }

    QDBusConnection cDBus = qkd::utility::dbus::qkd_dbus();
    std::string sFirstModuleServiceName = cModule.sDBusServiceName;
    QString sFirstModulePipeIn = QString::fromStdString(g_cPipeline.sURLPipeIn);

    QDBusMessage cMessage = QDBusMessage::createMethodCall(
            QString::fromStdString(sFirstModuleServiceName), 
            "/Module", 
            "org.freedesktop.DBus.Properties", 
            "Set");

    cMessage 
            << "at.ac.ait.qkd.module" 
            << "url_pipe_in" 
            << QVariant::fromValue(QDBusVariant(sFirstModulePipeIn)); 

    cDBus.call(cMessage, QDBus::NoBlock);
}


/**
 * start the pipeline
 * 
 * starts all modules specified in the global
 * pipeline variable (g_cPipeline)
 * 
 * @return  0 for success, else errorcode as for main()
 */
int start() {

    if (g_cPipeline.sLogFolder.size()) {
        
        boost::filesystem::path cLogFolder(g_cPipeline.sLogFolder);
        if (!boost::filesystem::exists(cLogFolder)) {
            boost::filesystem::create_directory(cLogFolder);
            if (!boost::filesystem::exists(cLogFolder)) {
                std::cerr << "failed to create log folder '" << g_cPipeline.sLogFolder << "'" << std::endl;
                return 1;
            }
        }
        
        if (!boost::filesystem::is_directory(cLogFolder)) {
            std::cerr << "path '" << g_cPipeline.sLogFolder << "' is not a directory" << std::endl;
            return 1;
        }
    }
        
    std::cout << "starting modules ..." << std::endl;
    
    int nModulesLaunched = 0;
    for (auto & cModule : g_cPipeline.cModules) {
        
        // try to locate the executable
        boost::filesystem::path cExecutable;
        try {
            cExecutable = qkd::utility::environment::find_executable(cModule.sPath);
        } catch (const boost::filesystem::filesystem_error& filesystem_error) {
            print_module_details(std::cerr, cModule);
            std::cerr << "Exception was thrown while trying to locate executable "
                      << cModule.sPath << std::endl
                      << filesystem_error.what() << std::endl;
            exit(1);
        }

        if (!cExecutable.string().size()) {
            std::cerr << "module: '"
                      << cModule.sPath
                      << "' - error: failed to locate executable '"
                      << cModule.sPath << "'"
                      << std::endl;
            continue;
        }

        cModule.sPath = cExecutable.string();

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
                        << cModule.sPath 
                        << "' - error: failed to daemonize subprocess." 
                        << std::endl;
            }
            else {

                try {
                    // write actual PID into tmp file to be read
                    // by the qkd-pipeline tool again to find DBus service name
                    // of current module
                    write_current_pid(cPIDFileName);
                } catch (const boost::filesystem::filesystem_error& filesystem_error) {
                    print_module_details(std::cerr, cModule);
                    std::cerr << "Exception was thrown while trying to write the module's PID to a file." << std::endl
                              << "The path to the PID file was: " << cPIDFileName.string() << std::endl
                              << filesystem_error.what() << std::endl;
                    exit(1);
                }

                // redirect to log file
                if (g_cPipeline.sLogFolder.size() && cModule.sLog.size()) {
                    try {
                        boost::filesystem::path cLogFile(g_cPipeline.sLogFolder);
                        cLogFile /= boost::filesystem::path(cModule.sLog);
                        if (!freopen(cLogFile.string().c_str(), "a+", stderr)) {
                            std::cerr << "module: '"
                            << cModule.sPath
                            << "' - error: failed to redirect stderr."
                            << std::endl;
                        }
                    } catch (const boost::filesystem::filesystem_error& filesystem_error) {
                        print_module_details(std::cerr, cModule);
                        std::cerr << "Exception was thrown while trying to redirect logging data to a file" << std::endl
                                  << "The path to the log file was: " << cModule.sLog << std::endl
                                  << "The log folder is: " << g_cPipeline.sLogFolder << std::endl
                                  << filesystem_error.what() << std::endl;
                        exit(1);
                    }
                }
                
                char * argv[1024];
                unsigned int nArg = 0;
                
                argv[nArg++] = strdup(cModule.sPath.c_str());
                if (!cModule.bAlice) argv[nArg++] = strdup("--bob");
                argv[nArg++] = strdup("--config");
                argv[nArg++] = strdup(cModule.sConfiguration.c_str());
                for (auto const s : cModule.sArgs) {
                    argv[nArg++] = strdup(s.c_str());
                    if (nArg == 1024) break;
                }
                
                argv[nArg++] = nullptr;
                if (execv(argv[0], argv) == -1) {
                    
                    // if we end up here execv failed
                    int nError = errno;
                    std::cerr << "module: '" 
                            << cModule.sPath 
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
            
            std::cout << "started module: " << cModule.sPath << " ";
            std::string sChildPID = read_child_pid(cPIDFileName);
            cModule.sDBusServiceName = get_dbus_service_name(sChildPID);
            if (!cModule.valid()) {
                std::cout << " --- failed" << std::endl;
                std::cerr << "unable to get module PID or DBus service name - is module running?" << std::endl;
                continue;
            }

            std::cout << "DBus: " << cModule.sDBusServiceName << std::endl;
            nModulesLaunched++;
        }
    }

    if (nModulesLaunched == 0) {
        std::cerr << "failed to start a single module - this is futile" << std::endl;
        return 1;
    }


    if (g_cPipeline.bAutoConnect) {
        if (!autoconnect_modules()) {
            std::cerr << "failed to autoconnect modules" << std::endl;
        }
    }

    std::string sURLPipeIn;
    std::string sURLPipeOut;
    get_pipeline_pipes(sURLPipeIn, sURLPipeOut);
    std::cout << "pipeline entry point: " << sURLPipeIn << std::endl;
    std::cout << "pipeline exit point: " << sURLPipeOut << std::endl;
    start_modules();
    std::cout << "starting modules ... done" << std::endl;
    
    return 0;
}


/**
 * start the modules of the pipeline
 */
void start_modules() {

    if (g_cPipeline.cModules.empty()) return;

    std::cout << "starting modules..." << std::endl;

    QDBusConnection cDBus = qkd::utility::dbus::qkd_dbus();
    for (auto iter = g_cPipeline.cModules.rbegin(); iter != g_cPipeline.cModules.rend(); ++iter) {

        if (!(*iter).valid()) continue;
        std::cout << (*iter).sDBusServiceName << "..." << std::endl;

        QDBusMessage cMessage;
        QDBusMessage cReply;

        cMessage = QDBusMessage::createMethodCall(
                QString::fromStdString((*iter).sDBusServiceName), 
                "/Module", 
                "at.ac.ait.qkd.module",
                "run");
        cReply = cDBus.call(cMessage);
        wait_for_module_state((*iter).sDBusServiceName, qkd::module::STATE_READY);

        cMessage = QDBusMessage::createMethodCall(
                QString::fromStdString((*iter).sDBusServiceName), 
                "/Module", 
                "at.ac.ait.qkd.module",
                "resume");
        cReply = cDBus.call(cMessage);
        wait_for_module_state((*iter).sDBusServiceName, qkd::module::STATE_RUNNING);
    }
}


/**
 * stop the pipeline
 * 
 * stops all modules specified in the global
 * pipeline variable (g_cPipeline)
 * 
 * @return  0 for success, else errorcode as for main()
 */
int stop() {
    
    std::cout << "stopping modules ..." << std::endl;
    
    qkd::utility::investigation cInvestigation = qkd::utility::investigation::investigate();
    QDBusConnection cDBus = qkd::utility::dbus::qkd_dbus();
    
    // iterate over the module definitions
    for (auto const & cModuleDefined : g_cPipeline.cModules) {
        
        // try to find this module in our module spec
        // to find we check:
        //      
        //      pipeline must be the same as our definition
        //      process_image must end in module-path
        //      role must be as our definition
        
        for (auto const & cModuleFound : cInvestigation.modules()) {
            
            if (cModuleFound.second.at("pipeline") != g_cPipeline.sName) continue;
            
            std::string sProcessImage = cModuleFound.second.at("process_image");
            if (sProcessImage.size() < cModuleDefined.sPath.size()) continue;
            unsigned int nPos = sProcessImage.size() - cModuleDefined.sPath.size();
            sProcessImage = sProcessImage.substr(nPos, sProcessImage.size());
            if (sProcessImage != cModuleDefined.sPath) continue;
            
            std::string sRoleName = cModuleFound.second.at("role_name");
            if (cModuleDefined.bAlice && (sRoleName != "alice")) continue;
            if (!cModuleDefined.bAlice && (sRoleName != "bob")) continue;

            // found
            std::cout << "terminating module: " << cModuleFound.second.at("dbus") << std::endl;
            
            QDBusMessage cMessage = QDBusMessage::createMethodCall(
                    QString::fromStdString(cModuleFound.second.at("dbus")), 
                    "/Module", 
                    "at.ac.ait.qkd.module", 
                    "terminate");
            cDBus.call(cMessage, QDBus::NoBlock);
        }
    }

    std::cout << "stopping modules ... done" << std::endl;
    
    return 0;
}


/**
 * waits until a module reached a certain state
 *
 * @param   sDBusServiceName        service name of the module
 * @param   eState                  module state to wait for
 */
bool wait_for_module_state(std::string const & sDBusServiceName, qkd::module::module_state eState) {

    bool res = false;

    // timeout: 50 * 100 millisec --> 5 sec
    QDBusConnection cDBus = qkd::utility::dbus::qkd_dbus();
    int nTries = 50;
    do {

        QDBusMessage cMessage = QDBusMessage::createMethodCall(
                QString::fromStdString(sDBusServiceName), 
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


/**
 * write current PID into file
 *
 * @param   cPath       the file to write
 * @return  true for success
 */
void write_current_pid(boost::filesystem::path const & cPath) {
    std::ofstream cPIDFile;
    cPIDFile.open(cPath.string());
    cPIDFile << getpid();
    cPIDFile.flush();
    cPIDFile.close();
}
/*
 * main.cpp
 * 
 * This is the qkd pipeline tool
 *
 * Autor: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
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
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>

// ait
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
    bool bStart;                        /**< start module immediately */
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
        bStart = false;
        sConfiguration = "",
        bAlice = true;
        sLog = "";
    };
    
};


/**
 * the pipeline definition found in the pipeline config xml
 */
struct {
    
    std::string sName;                          /**< pipeline name */
    std::string sLogFolder;                     /**< log folder */
    std::list<module_definition> cModules;      /**< list of modules */
    bool bAutoConnect = false;                  /**< autoconnect modules */

} g_cPipeline;


/**
 * autoconnect listened modules
 *
 * @return  true for success
 */
static bool autoconnect_modules();


/**
 * searches for the DBus service name of a module
 *
 * @param   sPID        process ID of module
 * @return  DBus Service Name of module
 */
static std::string get_dbus_service_name(std::string const & sPID);


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
 * read childs PID from file
 *
 * @param   cPath       the file to read
 * @return  child's pid (as string number)
 */
static std::string read_child_pid(boost::filesystem::path const & cPath);


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
 * stop the pipeline
 * 
 * stops all modules specified in the global
 * pipeline variable (g_cPipeline)
 * 
 * @return  0 for success, else errorcode as for main()
 */
static int stop();


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
 * autoconnect listened modules
 *
 * @return  true for success
 */
bool autoconnect_modules() {

    return true;
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
 * start
 * 
 * @param   argc        as usual
 * @param   argv        as usual
 * @return  as usual
 */
int main(int argc, char ** argv) {
    
    // create the command line header
    std::string sApplication = std::string("qkd-pipeline - AIT QKD Key Pipeline Tool V") + VERSION;
    std::string sDescription = std::string("\nThis tools let start/stop/restart a full QKD pipeline.\n\nCopyright 2013-2015 AIT Austrian Institute of Technology GmbH");
    std::string sSynopsis = std::string("Usage: ") + argv[0] + " [OPTIONS] COMMAND PIPELINE-CONFIG";
    
    // define program options
    boost::program_options::options_description cOptions(sApplication + "\n" + sDescription + "\n\n\t" + sSynopsis + "\n\nAllowed Options");
    cOptions.add_options()("help,h", "this page");
    cOptions.add_options()("log,l", boost::program_options::value<std::string>(), "path to log folder");
    cOptions.add_options()("version,v", "print version string");
    
    // arguments
    boost::program_options::options_description cArgs("Arguments");
    
    cArgs.add_options()("COMMAND", "COMMAND is either 'start', 'stop' or 'restart'.");
    cArgs.add_options()("PIPELINE-CONFIG", "PIPELINE-CONFIG is the path to the pipeline configuration XML.");
    
    boost::program_options::positional_options_description cPositionalDescription; 
    cPositionalDescription.add("COMMAND", 1);
    cPositionalDescription.add("PIPELINE-CONFIG", 1);
    
    // construct overall options
    boost::program_options::options_description cCmdLineOptions("Command Line");
    cCmdLineOptions.add(cOptions);
    cCmdLineOptions.add(cArgs);
    
    // option variable map
    boost::program_options::variables_map cVariableMap;
    
    try {
        // parse action
        boost::program_options::command_line_parser cParser(argc, argv);
        boost::program_options::store(cParser.options(cCmdLineOptions).positional(cPositionalDescription).run(), cVariableMap);
        boost::program_options::notify(cVariableMap);        
    }
    catch (std::exception & cException) {
        std::cerr << "error parsing command line: " << cException.what() << "\ntype '--help' for help" << std::endl;        
        return 1;
    }
    
    // check for "help" set
    if (cVariableMap.count("help")) {
        std::cout << cOptions << std::endl;
        std::cout << cArgs.find("COMMAND", false).description() << "\n";      
        std::cout << cArgs.find("PIPELINE-CONFIG", false).description() << "\n" << std::endl;      
        return 0;
    }
    
    // check for "version" set
    if (cVariableMap.count("version")) {
        std::cout << sApplication << std::endl;
        return 0;
    }
    
    // log folder
    if (cVariableMap.count("log")) {
        g_cPipeline.sLogFolder = cVariableMap["log"].as<std::string>();
        if (!boost::filesystem::exists(boost::filesystem::path(g_cPipeline.sLogFolder))) {
            std::cerr << "cannot access log folder '" << g_cPipeline.sLogFolder << "'." << std::endl;
            return 1;
        }
    }
    
    // pipeline command
    if (!cVariableMap.count("COMMAND")) {
        std::cerr << "no pipeline command.\ntype '--help' for information." << std::endl;
        return 1;
    }
    std::string sPipelineCommand = cVariableMap["COMMAND"].as<std::string>();
    if (!sPipelineCommand.size()) {
        std::cerr << "neither 'start', 'stop' nor 'restart' specified.\nchoose one command - type '--help' for help." << std::endl;
        return 1;
    }
    
    // check for valid command values
    bool bStart = (sPipelineCommand == "start");
    bool bStop = (sPipelineCommand == "stop");
    bool bRestart = (sPipelineCommand == "restart");
    if (!bStart && !bStop && !bRestart) {
        std::cerr << "command '" << sPipelineCommand << "' unknown.\nchoose one command - type '--help' for help." << std::endl;
        return 1;
    }

    // pipeline-configuration
    if (!cVariableMap.count("PIPELINE-CONFIG")) {
        std::cerr << "no pipeline-config specified.\ntype '--help' for information." << std::endl;
        return 1;
    }
    std::string sPipelineConfiguration = cVariableMap["PIPELINE-CONFIG"].as<std::string>();
    
    // from here one we have really work to do
    QCoreApplication cApp(argc, argv);

    // make the steps
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
    
    // open config file
    QFile cFile(QString::fromStdString(sPipelineConfiguration));
    if (!cFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        std::cerr << "failed to open '" << sPipelineConfiguration << "'." << std::endl;
        return 1;
    }
    
    // read in
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
    
    // check root XML element
    QDomElement cRootElement = cDomDoc.documentElement();
    if (cRootElement.tagName() != "pipeline") {
        std::cerr << "failed to parse configuration: root element 'pipeline' not found." << std::endl;
        return 1;
    }
    
    // the 'pipeline' MUST have a name attribute
    if (!cRootElement.hasAttribute("name")) {
        std::cerr << "pipeline tag element has no 'name' attribute which is mandatory." << std::endl;
        return 1;
    }
    g_cPipeline.sName = cRootElement.attribute("name").toStdString();
    
    // the 'pipeline' MIGHT have a autoconnect attribute
    if (!cRootElement.hasAttribute("autoconnect")) {
        g_cPipeline.bAutoConnect = (cRootElement.attribute("autoconnect") == "true");
    }
    
    // iterate over the module nodes
    int nModuleErrorCode = 0;
    for (QDomNode cNode = cRootElement.firstChild(); !cNode.isNull() && (nModuleErrorCode == 0); cNode = cNode.nextSibling()) {
        
        // we just pick the xml elements
        if (cNode.isElement()) nModuleErrorCode = parse_module(cNode.toElement());
    }
    
    // state what we've found
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
    
    // sanity check for module node
    if (cModuleElement.tagName() != "module") {
        std::cerr << "failed to parse configuration: element 'module' for a single module not found." << std::endl;
        return 1;
    }
    
    // the 'module' MUST have a path attribute
    if (!cModuleElement.hasAttribute("path")) {
        std::cerr << "module lacks 'path' attribute which is mandatory." << std::endl;
        return 1;
    }
    cModule.sPath = cModuleElement.attribute("path").toStdString();
    
    // start attribute value
    std::string sStartAttribute = "no";
    if (cModuleElement.hasAttribute("start")) sStartAttribute = cModuleElement.attribute("start").toStdString();    
    if (sStartAttribute == "no") {
        cModule.bStart = false;
    }
    else 
    if (sStartAttribute == "yes") {
        cModule.bStart = true;
    }
    else {
        std::cerr << "module: '" << cModule.sPath << "' - failed to parse 'start' attribute value." << std::endl;
        return 1;
    }
    
    // iterate over the childs
    for (QDomNode cNode = cModuleElement.firstChild(); !cNode.isNull(); cNode = cNode.nextSibling()) {
        
        // we just pick the xml elements
        if (!cNode.isElement()) continue;
        
        QDomElement cDomElement = cNode.toElement();
        
        // config tag
        if (cDomElement.tagName() == "config") {
            if (cDomElement.hasAttribute("path")) cModule.sConfiguration = cDomElement.attribute("path").toStdString();    
        }
        else
            
        // config role
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
                    std::cerr << "module: '" << cModule.sPath << "' - ignoring role value '" << sModuleRole << "'." << std::endl;
                }
            }
        }
        else
            
        // config args
        if (cDomElement.tagName() == "args") {
            if (cDomElement.hasAttribute("value")) cModule.sArgs.push_back(cDomElement.attribute("value").toStdString());
            if (!cDomElement.text().isEmpty()) cModule.sArgs.push_back(cDomElement.text().toStdString());
        }
        else
            
        // config logs
        if (cDomElement.tagName() == "log") {
            if (cDomElement.hasAttribute("path")) cModule.sLog = cDomElement.attribute("path").toStdString();    
        }
        
        // ... unknown config tag
        else {
            std::cerr << "module: '" << cModule.sPath << "' - ignoring unknown tag '" << cDomElement.tagName().toStdString() << std::endl;
        }
    }
    
    // add module to our pipeline setting
    g_cPipeline.cModules.push_back(cModule);
    
    return 0;
}


/**
 * read childs PID from file
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
 * start the pipeline
 * 
 * starts all modules specified in the global
 * pipeline variable (g_cPipeline)
 * 
 * @return  0 for success, else errorcode as for main()
 */
int start() {

    // set up log folder
    if (g_cPipeline.sLogFolder.size()) {
        
        boost::filesystem::path cLogFolder(g_cPipeline.sLogFolder);
        if (!boost::filesystem::exists(cLogFolder)) {
            boost::filesystem::create_directory(cLogFolder);
            if (!boost::filesystem::exists(cLogFolder)) {
                std::cerr << "failed to create log folder '" << g_cPipeline.sLogFolder << "'" << std::endl;
                return 1;
            }
        }
        
        // a directory?
        if (!boost::filesystem::is_directory(cLogFolder)) {
            std::cerr << "path '" << g_cPipeline.sLogFolder << "' is not a directory" << std::endl;
            return 1;
        }
    }
        
    std::cout << "starting modules ..." << std::endl;
    
    for (auto & cModule : g_cPipeline.cModules) {
        
        // try to locate the executable
        boost::filesystem::path cExecutable = qkd::utility::environment::find_executable(cModule.sPath);
        if (!cExecutable.string().size()) {
            std::cerr << "module: '" << cModule.sPath << "' - error: failed to locate executable '" << cModule.sPath << "'" << std::endl;
            continue;
        }

        cModule.sPath = cExecutable.string();
        
        // write current pid into pid file 
        boost::filesystem::path cPIDFileName = qkd::utility::environment::temp_path() / "qkd-pipeline.autoconnect.module.pid";

        // fork and daemonize
        if (!fork()) {
            
            // this is within a new child
            // now daemon() does another fork
            // so we have to get a holdon to 
            // the child's child PID --> we write it into a file
            // since stdin and stdout are lost now

            if (daemon(1, 0) == -1) {
                std::cerr << "module: '" << cModule.sPath << "' - error: failed to daemonize subprocess." << std::endl;
            }
            else {

                // write actual PID into tmp file to be read
                // by the qkd-pipeline tool again to find DBus service name
                // of current module
                write_current_pid(cPIDFileName);

                // redirect to log file
                if (g_cPipeline.sLogFolder.size() && cModule.sLog.size()) {
                    boost::filesystem::path cLogFile(g_cPipeline.sLogFolder);
                    cLogFile /= boost::filesystem::path(cModule.sLog);
                    if (!freopen(cLogFile.string().c_str(), "a+", stderr)) {
                        std::cerr << "module: '" 
                                << cModule.sPath 
                                << "' - error: failed to redirect stderr." 
                                << std::endl;
                    }
                }
                
                char * argv[1024];
                unsigned int nArg = 0;
                
                argv[nArg++] = strdup(cModule.sPath.c_str());
                if (cModule.bStart) argv[nArg++] = strdup("--run");
                if (!cModule.bAlice) argv[nArg++] = strdup("--bob");
                argv[nArg++] = strdup("--config");
                argv[nArg++] = strdup(cModule.sConfiguration.c_str());
                for (auto const s : cModule.sArgs) {
                    argv[nArg++] = strdup(s.c_str());
                    if (nArg == 1024) break;
                }
                
                argv[nArg++] = nullptr;

                // launch process
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
            if (sChildPID.empty() && g_cPipeline.bAutoConnect) {
                std::cout << std::endl;
                if (g_cPipeline.bAutoConnect) {
                    std::cerr << "unable to fetch module's process ID - can't autoconnect" << std::endl;
                }
                continue;
            }

            std::cout << "PID: " << sChildPID << " ";

            cModule.sDBusServiceName = get_dbus_service_name(sChildPID);
            std::cout << "DBus: " << cModule.sDBusServiceName << std::endl;
        }
    }

    if (g_cPipeline.bAutoConnect) {
        if (!autoconnect_modules()) {
            std::cerr << "failed to autoconnect modules" << std::endl;
        }
    }

    std::cout << "starting modules ... done" << std::endl;
    
    return 0;
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
    
    // investigate the system
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
        // any other ideas
        // 
        
        // iterate over the modules of the system
        for (auto const & cModuleFound : cInvestigation.modules()) {
            
            // pipeline
            if (cModuleFound.second.at("pipeline") != g_cPipeline.sName) continue;
            
            // process_image
            std::string sProcessImage = cModuleFound.second.at("process_image");
            if (sProcessImage.size() < cModuleDefined.sPath.size()) continue;
            unsigned int nPos = sProcessImage.size() - cModuleDefined.sPath.size();
            sProcessImage = sProcessImage.substr(nPos, sProcessImage.size());
            if (sProcessImage != cModuleDefined.sPath) continue;
            
            // role check
            std::string sRoleName = cModuleFound.second.at("role_name");
            if (cModuleDefined.bAlice && (sRoleName != "alice")) continue;
            if (!cModuleDefined.bAlice && (sRoleName != "bob")) continue;

            // found
            std::cout << "terminating module: " << cModuleFound.second.at("dbus") << std::endl;
            
            // invoke 'terminate' on module via DBus
            QDBusMessage cMessage = QDBusMessage::createMethodCall(QString::fromStdString(cModuleFound.second.at("dbus")), "/Module", "at.ac.ait.qkd.module", "terminate");
            cDBus.call(cMessage, QDBus::NoBlock);
        }
    }

    std::cout << "stopping modules ... done" << std::endl;
    
    return 0;
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


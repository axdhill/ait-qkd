/*
 * node.cpp
 *
 * implement the Q3P Node
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

 
// ------------------------------------------------------------
// incs

#include "config.h"

#include <fstream>

#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#include <boost/program_options/detail/config_file.hpp>

// time system headers
#ifdef HAVE_SYS_TIME_H
#   include <sys/time.h>
#endif

#ifdef HAVE_SYS_TIMES_H
#   include <sys/times.h>
#endif

#ifdef HAVE_TIME_H
#   include <time.h>
#endif

// Qt
#include <QtCore/QCoreApplication>

// ait
#include <qkd/utility/dbus.h>
#include <qkd/utility/investigation.h>
#include <qkd/utility/syslog.h>

#include "node.h"
#include "node_dbus.h"


using namespace qkd;
using namespace qkd::q3p;


// ------------------------------------------------------------
// decls


// log callback
static void log_callback(std::string const & sLog);



// ------------------------------------------------------------
// vars


/**
 * the main node instance
 */
static node * g_cNode = nullptr;


// ------------------------------------------------------------
// code


/**
 * ctor
 * 
 * @param   sId                 ID of the node
 * @param   sConfigFileURL      URL of the config file
 */
node::node(QString const & sId, QString const & sConfigFileURL) : QObject(), m_sConfigFile(sConfigFileURL), m_sId(sId) {
    
    if (!g_cNode) g_cNode = this;
    
    qkd::utility::debug::set_callback(log_callback);
    
    qkd::utility::syslog::info() << "launching Q3P node \"" << sId.toStdString() << "\"";
    
    struct timeval cTV;
    struct timezone cTZ;
    gettimeofday(&cTV, &cTZ);
    
    m_nStartTimeStamp = cTV.tv_sec;
    m_cRandom = qkd::utility::random_source::create("");
    
    qkd::utility::syslog::info() << "connecting to DBus:" << getenv("DBUS_SESSION_BUS_ADDRESS");
    
    QDBusConnection cDBus = qkd::utility::dbus::qkd_dbus();
    QString sServiceName = QString("at.ac.ait.q3p.node-") + id();
    bool bSuccess = cDBus.registerService(sServiceName);
    if (!bSuccess) {
        QString sMessage = QString("Failed to register DBus service \"") + sServiceName + "\""; 
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << sMessage.toStdString();
    }
    
    qkd::utility::syslog::info() << "connected to DBus:" << getenv("DBUS_SESSION_BUS_ADDRESS") << " as \"" << sServiceName.toStdString() << "\"";

    new NodeAdaptor(this);
    bSuccess = cDBus.registerObject("/Node", this);
    if (!bSuccess) {
        QString sMessage = QString("Failed to register DBus object \"/Node\""); 
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << sMessage.toStdString();
    }
    else {
        qkd::utility::syslog::info() << "node registered on DBus as \"" << sServiceName.toStdString() << "\"";
    }
    
    QTimer::singleShot(0, this, SLOT(apply_config_file()));
}


/**
 * dtor
 */
node::~node() {
}


/**
 * fetch a config file and apply values
 */
void node::apply_config_file() {
    
    qkd::utility::properties cConfig;
    load_config_file(cConfig);
    if (cConfig.size() == 0) return;
    
    std::map<std::string, qkd::utility::properties> cLinkConfig;
    extract_link_config(cLinkConfig, cConfig);
    
    if (cLinkConfig.empty()) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ 
                << ": " << "no link configuration found - did you miss a [link.NAME] line?";
    }
    else {
    
        if (qkd::utility::debug::enabled()) {
            
            qkd::utility::debug() << "parsed config data:";        
            for (auto & link_iter : cLinkConfig) {
            
                qkd::utility::debug() << "\tlink config identifier: " << link_iter.first;        
                for (auto & iter : link_iter.second) {
                    
                    // do not print the shared secret in plain on debug out
                    if (iter.first != "secret") qkd::utility::debug() << "\t\t" << iter.first << " = " << iter.second;
                    else qkd::utility::debug() << "\t\t" << iter.first << " = <XXXXXXXXXXXXXXXXXXXXXXXX>";
                }
            }
        }

        for (auto & link_iter : cLinkConfig) {
            apply_link_config(link_iter.first, link_iter.second);
        }
    }
}


/**
 * apply a link config
 * 
 * @param   sLinkIdentifier     the link identifier
 * @param   cConfig             a key-->value map for the link
 */
void node::apply_link_config(std::string const & sLinkIdentifier, qkd::utility::properties const & cConfig) {
    
    qkd::utility::debug() << "applying values for config setting '" << sLinkIdentifier << "'";
    
    struct {
       
        std::string sSection;           /**< sectionname in config file */
        std::string sId;                /**< id of link */
        std::string sDb;                /**< db config */
        std::string sMaster;            /**< master/slave config */
        std::string sListenURI;         /**< listen URI config */
        std::string sPeerURI;           /**< peer URI config */
        std::string sSecret;            /**< secret data config */
        std::string sSecretFile;        /**< filename of secret data config */
        std::string sIPSec;             /**< IPSec setting in config */
        std::string sInject;            /**< inject file in config */
        
    } cLinkConfig;
    
    cLinkConfig.sSection    = sLinkIdentifier;
    cLinkConfig.sId         = (cConfig.find("id")           != cConfig.end() ? cConfig.at("id") : "");
    cLinkConfig.sDb         = (cConfig.find("db")           != cConfig.end() ? cConfig.at("db") : "");
    cLinkConfig.sMaster     = (cConfig.find("master")       != cConfig.end() ? cConfig.at("master") : "");
    cLinkConfig.sListenURI  = (cConfig.find("listen.uri")   != cConfig.end() ? cConfig.at("listen.uri") : "");
    cLinkConfig.sPeerURI    = (cConfig.find("peer.uri")     != cConfig.end() ? cConfig.at("peer.uri") : "");
    cLinkConfig.sSecret     = (cConfig.find("secret")       != cConfig.end() ? cConfig.at("secret") : "");
    cLinkConfig.sSecretFile = (cConfig.find("secret_file")  != cConfig.end() ? cConfig.at("secret_file") : "");
    cLinkConfig.sIPSec      = (cConfig.find("ipsec")        != cConfig.end() ? cConfig.at("ipsec") : "");
    cLinkConfig.sInject     = (cConfig.find("inject")       != cConfig.end() ? cConfig.at("inject") : "");
    
    if (cLinkConfig.sId.empty()) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ 
                << ": " << "failed to setup link for '" << sLinkIdentifier << "': missing value for 'id'";
        return;
    }
    
    QString sId = QString::fromStdString(cConfig.at("id"));
    if (!create_link(QString::fromStdString(cLinkConfig.sId))) return;
    
    qkd::q3p::engine cEngine = qkd::q3p::engine_instance::get(sId);
    if (cEngine.get() == nullptr) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ 
                << ": " << "failed to setup link: engine created but unable to fetch unstance. this mus not happen. this is a bug.";
        return;
    }
    
    apply_link_config_master(cEngine, cLinkConfig.sMaster);
    apply_link_config_db(cEngine, cLinkConfig.sDb);
    apply_link_config_inject(cEngine, cLinkConfig.sInject);
    
    QByteArray cSharedSecret;
    if (!cLinkConfig.sSecret.empty() && !cLinkConfig.sSecretFile.empty()) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ 
                << ": " << "parsing config for '" << sLinkIdentifier << "': both 'secret' AND 'secret_file' given - 'secret' takes precedence.";
    }
    if (!cLinkConfig.sSecret.empty()) {
        cSharedSecret = load_link_config_secret(cLinkConfig.sSecret);
    }
    else
    if (!cLinkConfig.sSecretFile.empty()) {
        cSharedSecret = load_link_config_secret_file(cLinkConfig.sSecretFile);
    }
    if (cSharedSecret.size() == 0) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ 
                << ": " << "config for '" << sLinkIdentifier << "': I don't have a shared secret to start with - unable to proceed.";
        return;
    }
    
    apply_link_config_ipsec(cEngine, cLinkConfig.sIPSec);
    
    if (cLinkConfig.sListenURI.size()) {
        cEngine->listen(QString::fromStdString(cLinkConfig.sListenURI), cSharedSecret);
    }
    else {
        qkd::utility::syslog::info() << "config for '" << sLinkIdentifier << "': insufficient listener-config - not going to listen.";
    }

    if (cLinkConfig.sPeerURI.size()) {
        cEngine->connect(QString::fromStdString(cLinkConfig.sPeerURI), cSharedSecret);
    }
    else {
        qkd::utility::syslog::info() << "config for '" << sLinkIdentifier << "': insufficient peer-config - not going to connect peer.";
    }
}


/**
 * apply a link config: "db"
 * 
 * @param   cEngine             the link instance
 * @param   sValue              the value for "db"
 */
void node::apply_link_config_db(qkd::q3p::engine & cEngine, std::string const & sValue) const {
    
    if (sValue.empty()) {
        return;
    }
    
    cEngine->open_db(QString::fromStdString(sValue));
    if (!cEngine->db_opened()) {
        qkd::utility::syslog::warning() << "failed to open keystore DB with: " << sValue;
    }
}


/**
 * apply a link config: "inject"
 * 
 * @param   cEngine             the link instance
 * @param   sValue              the value for "inject"
 */
void node::apply_link_config_inject(qkd::q3p::engine & cEngine, std::string const & sValue) const {
    
    if (sValue.empty()) {
        return;
    }
    
    if (!cEngine->db_opened()) {
        return;
    }
    
    if (cEngine->common_store()->count() == 0) {
        
        std::string sInjectFile = sValue;
        QUrl cInjectFileUrl(QString::fromStdString(sValue));
        if (!cInjectFileUrl.scheme().isEmpty()) sInjectFile = cInjectFileUrl.toLocalFile().toStdString();

        boost::filesystem::path cInjectFilePath(sInjectFile);
        if (!boost::filesystem::exists(cInjectFilePath)) {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "cannot access given inject file: '" << cInjectFilePath.string() << "'";
        }
        else {
            
            if (!boost::filesystem::is_regular_file(cInjectFilePath)) {
                qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ 
                        << ": " << "given inject file: '" << cInjectFilePath.string() << "' seems not to be a regular file";
            }
            else {
                
                QFile cInjectFile(QString::fromStdString(cInjectFilePath.string()));
                if (!cInjectFile.open(QIODevice::ReadOnly)) {
                    qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ 
                            << ": " << "failed to open inject file: '" << cInjectFilePath.string() << "'";
                }
                else {
                    cEngine->inject(cInjectFile.readAll());
                }
            }
        }

    }
}


/**
 * apply a link config: "ipsec"
 * 
 * @param   cEngine             the link instance
 * @param   sValue              the value for "ipsec"
 */
void node::apply_link_config_ipsec(qkd::q3p::engine & cEngine, std::string const & sValue) const {
    
    if (sValue.empty()) {
        return;
    }
    
    cEngine->configure_ipsec(QString::fromStdString(sValue));
}


/**
 * apply a link config: "master"
 * 
 * @param   cEngine             the link instance
 * @param   sValue              the value for "master"
 */
void node::apply_link_config_master(qkd::q3p::engine & cEngine, std::string const & sValue) const {
    
    if (sValue.empty()) {
        return;
    }
    
    std::set<std::string> cTrueValues = { "1", "y", "yes", "true" };
    std::set<std::string> cFalseValues = { "0", "n", "no", "false" };
    
    if (cTrueValues.find(sValue) != cTrueValues.end()) {
        cEngine->set_master(true);
        cEngine->set_slave(false);
    }
    else
    if (cFalseValues.find(sValue) != cFalseValues.end()) {
        cEngine->set_master(false);
        cEngine->set_slave(true);
    }
    else {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ 
                << ": " << "failed to parse value for '" << cEngine->id().toStdString() << "': don't know how to interpret value of 'master': '" << sValue << "'";
    }
}


/**
 * create a set of config file hints
 * 
 * @return  an ordered list of config file hints
 */
std::list<std::string> node::config_file_hints() const {
    
    std::list<std::string> cConfigFileHints;
    
    std::string sNode = id().toStdString();
    QString sConfigFile = m_sConfigFile;
    
    if (!sConfigFile.isEmpty()) {
        
        QUrl cConfigFileUrl(sConfigFile);
        if (!cConfigFileUrl.scheme().isEmpty()) sConfigFile = cConfigFileUrl.toLocalFile();

        boost::filesystem::path cConfigFilePath(sConfigFile.toStdString());
        if (!boost::filesystem::exists(cConfigFilePath)) {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "cannot access given config file: '" << cConfigFilePath.string() << "'";
        }
        else {
            
            if (!boost::filesystem::is_regular_file(cConfigFilePath)) {
                qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ 
                        << ": " << "given config file: '" << cConfigFilePath.string() << "' seems not to be a regular file";
            }
            else {
                cConfigFileHints.push_back(boost::filesystem::absolute(cConfigFilePath).string());
            }
        }
    }
    
    cConfigFileHints.push_back(qkd::utility::environment::process_image_path().string() + "/" + sNode + ".conf");
    cConfigFileHints.push_back(qkd::utility::environment::data_path("q3p").string() + "/" + sNode + ".conf");
    cConfigFileHints.push_back(qkd::utility::environment::prefix_path().string() + "/etc/q3p/" + sNode + ".conf");
    cConfigFileHints.push_back(qkd::utility::environment::config_path().string() + "/q3p/" + sNode + ".conf");
    
    if (qkd::utility::debug::enabled()) {
        qkd::utility::debug() << "these are the config files I'll try to load ...";
        int nHint = 0;
        for (auto & sFileHint : cConfigFileHints) {
            qkd::utility::debug() << "configfile file hint #" << ++nHint << ": " << sFileHint;
        }
    }
    
    return cConfigFileHints;
}


/**
 * create a link instance
 * 
 * @param   sName       name of the link
 */
bool node::create_link(QString const & sName) {
    
    try {
        qkd::q3p::engine cEngine = qkd::q3p::engine_instance::create(id(), sName);
        qkd::utility::syslog::info() << "created and registered Q3P engine: \"" << cEngine->link_id().toStdString() << "\"";
        emit link_created(sName);
        return true;
    }
    catch (std::exception const & cException) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ 
                << ": " << "failed to register new Q3P engine: \"" << cException.what() << "\" - already registered";
    }
    catch (...) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ 
                << ": " << "failed to register new Q3P engine: \"" << sName.toStdString() << "\" - unknown reason";
    }

    return false; 
}


/**
 * extract the link configurations based on a set of configuration entries
 * 
 * @param   cLinkConfig         [out] the found link configurations
 * @param   cConfig             the loaded configuration values
 */
void node::extract_link_config(std::map<std::string, qkd::utility::properties> & cLinkConfig, qkd::utility::properties const & cConfig) {
    
    for (auto & iter : cConfig) {
        
        std::vector<std::string> sTokens;
        boost::split(sTokens, iter.first, boost::is_any_of("."));
        if (sTokens.size() < 3) continue;
        
        // first particle should be "link"
        if (sTokens[0] != "link") continue;
        
        // the link INI identifier must be present
        if (sTokens[1].size() == 0) continue;
        
        // the key must be present
        if (sTokens[2].size() == 0) continue;

        std::stringstream ssKey;
        ssKey << sTokens[2];
        for (auto key_iter = sTokens.begin() + 3; key_iter != sTokens.end(); key_iter++) {
            ssKey << "." << (*key_iter);
        }
        std::string sKey = ssKey.str();
        
        const std::set<std::string> cValidKeyNames = { "db", "id", "listen.uri", "master", "peer.uri", "secret", "secret_file", "ipsec", "inject" };
        if (cValidKeyNames.find(sKey) == cValidKeyNames.end()) {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ 
                    << ": " << "parsed config file: '" << m_sConfigFile.toStdString() 
                    << "', section [" << sTokens[0] << "." << sTokens[1] << "]: detected unknown key '" << sKey << "' - dropping";
            continue;
        }
                
        cLinkConfig[sTokens[1]][sKey] = iter.second;
    }
}


/**
 * get the list of links
 * 
 * @return  a list of links
 */
QStringList node::links() {
    
    QStringList cList;
    
    qkd::q3p::engine_map const & cEngines = qkd::q3p::engine_instance::engines();
    for (auto & iter : cEngines) {
        cList << iter.second->link_id();
    }
    
    return cList;
}


/**
 * load the config file
 * 
 * @param   cConfig         [out] the found proerties inside the config file
 */
void node::load_config_file(qkd::utility::properties & cConfig) {

    std::list<std::string> cConfigFileHints = config_file_hints();
    m_sConfigFile = "";
    
    for (auto & sFileHint : cConfigFileHints) {

        std::ifstream cConfigFile(sFileHint);
        if (!cConfigFile.is_open()) continue;
        
        m_sConfigFile = QString::fromStdString(sFileHint);
        qkd::utility::syslog::info() << "found config file: " << sFileHint << ", taking values from there ...";

        std::set<std::string> cOptions;
        cOptions.insert("*");
        
        try {
            boost::program_options::detail::config_file_iterator cConfigIter(cConfigFile, cOptions);
            boost::program_options::detail::config_file_iterator cEOF;
            
            for (; cConfigIter != cEOF; cConfigIter++) {
                boost::program_options::option cOption = *cConfigIter;
                cConfig[cOption.string_key] = cOption.value[0];
            }
        }
        catch (boost::program_options::invalid_syntax const & cErrInvalidSyntax) {
            qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ 
                    << ": " << "failed to parse config file: " << sFileHint << " invalid syntax at: '" << cErrInvalidSyntax.tokens() << "'";
            exit(1);
        }
        catch (std::exception const & cException) {
            qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ 
                    << ": " << "failed to parse config file: " << sFileHint << " exception: " << cException.what();
            exit(1);
        }
        
        // we take the very first found file and exit here the search for files
        // otherwise config options will be overwritten by config files found later
        break;
    }
    
    if (m_sConfigFile.isEmpty()) {
        qkd::utility::syslog::info() << "no config file found, starting with default/empty values";
        return;
    }
    
    if (cConfig.size() == 0) {
        if (!m_sConfigFile.isEmpty()) {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ 
                    << ": " << "found config file: " << m_sConfigFile.toStdString() << " but didn't find any option - is this intended?";
        }
        return;
    }
}


/**
 * load the secret specified by link config: "secret"
 * 
 * @param   sValue              the value for "secret"
 * @return  the loaded secret
 */
QByteArray node::load_link_config_secret(std::string const & sValue) const {
    
    QByteArray res;
    
    if (sValue.empty()) {
        return res;
    }
    
    res = QByteArray(sValue.data(), sValue.length());
    
    return res;
}


/**
 * load the secret specified by link config: "secret_file"
 * 
 * @param   sValue              the value for "secret"
 * @return  the loaded secret
 */
QByteArray node::load_link_config_secret_file(std::string const & sValue) const {
    
    QByteArray res;
    
    if (sValue.empty()) {
        return res;
    }

    QFile cSecretFile(QString::fromStdString(sValue));
    if (!cSecretFile.open(QIODevice::ReadOnly)) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ 
                << ": " << "failed to open shared secret_file: '" << sValue << "'";
    }
    else res = cSecretFile.readAll();
        
    return res;
}

/**
 * get the current present modules on the node
 * 
 * The return list is a series of string each one of
 * the format:
 * 
 *      ID;STATE;NODE;PIPELINE;HINT;URL_LISTEN;URL_PEER;URL_PIPE_IN;URL_PIPE_OUT;
 * 
 * All fields are separated with a semicolon ';'
 * 
 *      ID ............. The id of the module
 *      PID ............ The process ID of the module
 *      STATE .......... The current state of the module
 *      NODE ........... The id of this node been asked
 *      PIPELINE ....... The id of the pipeline the module
 *                       is currently in
 *      HINT ........... Any user supplied information to 
 *                       the module
 *      URL_LISTEN ..... The public available listen URL of
 *                       the module
 *      URL_PEER ....... The peer URL the module is connected to
 *      URL_PIPE_IN .... The pipe IN URL
 *      URL_PIPE_OUT ... The pipe OUT URL
 * 
 * Hence: the node gives a s**t what each module
 * is doing and if it is indeed connected to one
 * of its links or not. If it is present, its listed.
 * Point.
 * 
 * @return  a list of running modules
 */
QStringList node::modules() {

    QStringList cModules;
    
    // check what is currently on the system
    qkd::utility::investigation cInvestigation = qkd::utility::investigation::investigate();

    // collect the modules
    for (auto const & cModule : cInvestigation.modules()) {
        
        std::stringstream ss;
        ss << cModule.second.at("id") << ";" 
            << cModule.second.at("process_id") << ";"
            << cModule.second.at("state") << ";"
            << m_sId.toStdString() << ";"
            << cModule.second.at("pipeline") << ";"
            << cModule.second.at("hint") << ";"
            << cModule.second.at("url_listen") << ";"
            << cModule.second.at("url_peer") << ";"
            << cModule.second.at("url_pipe_in") << ";"
            << cModule.second.at("url_pipe_out") << ";";
        
        cModules << QString::fromStdString(ss.str());
    }
    
    return cModules;
}


/**
 * turn down the node
 */
void node::quit() {
    qkd::utility::syslog::info() << "received quit signal. shutting down ...";
    qkd::q3p::engine_instance::close_all();
    qApp->quit();
}


/**
 * set the url of the random value source
 *
 * @param   sRandomUrl      the new url of the random value source
 */
void node::set_random_url(QString sRandomUrl) {
    
    try {
        
        qkd::utility::random r = qkd::utility::random_source::create(sRandomUrl.toStdString());
        m_cRandom = r;
        m_sRandomUrl = sRandomUrl;
        
        qkd::utility::syslog::info() << "new random source: '" << sRandomUrl.toStdString() << "'";
        
        emit random_url_changed(sRandomUrl);
    }
    catch (...) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ 
                << ": " << "failed to set new random source: '" << sRandomUrl.toStdString() << "'";
    }
}


/**
 * trigger a new log entry
 * 
 * @param   sLog        the log message
 */
void node::trigger_log(QString const & sLog) {
    emit log(sLog);
}


/**
 * number of seconds this key store is up
 * 
 * @return  number of seconds this key store is up and running
 */
qulonglong node::uptime() {
    
    struct timeval cTV;
    struct timezone cTZ;
    gettimeofday(&cTV, &cTZ);
    
    return (cTV.tv_sec - m_nStartTimeStamp);
}


/**
 * the log callback
 * 
 * @param   sLog        the new log message
 */
void log_callback(std::string const & sLog) {
    if (!g_cNode) return;
    g_cNode->trigger_log(QString::fromStdString(sLog));
}

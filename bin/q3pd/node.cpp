/*
 * node.cpp
 *
 * implement the Q3P Node
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
#include <qkd/q3p/engine.h>
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
    
    // install the main node instance
    if (!g_cNode) g_cNode = this;
    
    // install log callback 
    qkd::utility::debug::set_callback(log_callback);
    
    // syslog
    qkd::utility::syslog::info() << "launching Q3P node \"" << sId.toStdString() << "\"";
    
    struct timeval cTV;
    struct timezone cTZ;
    gettimeofday(&cTV, &cTZ);
    
    m_nStartTimeStamp = cTV.tv_sec;
    m_cRandom = qkd::utility::random_source::create("");
    
    // syslog
    qkd::utility::syslog::info() << "connecting to DBus:" << getenv("DBUS_SESSION_BUS_ADDRESS");
    
    // register Service on DBus
    QDBusConnection cDBus = qkd::utility::dbus::qkd_dbus();
    QString sServiceName = QString("at.ac.ait.q3p.node-") + id();
    bool bSuccess = cDBus.registerService(sServiceName);
    if (!bSuccess) {
        QString sMessage = QString("Failed to register DBus service \"") + sServiceName + "\""; 
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << sMessage.toStdString();
    }
    
    // syslog
    qkd::utility::syslog::info() << "connected to DBus:" << getenv("DBUS_SESSION_BUS_ADDRESS") << " as \"" << sServiceName.toStdString() << "\"";

    // register Object on DBus
    new NodeAdaptor(this);
    bSuccess = cDBus.registerObject("/Node", this);
    if (!bSuccess) {
        QString sMessage = QString("Failed to register DBus object \"/Node\""); 
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << sMessage.toStdString();
    }
    else {
        qkd::utility::syslog::info() << "node registered on DBus as \"" << sServiceName.toStdString() << "\"";
    }
    
    // the right next thing is to apply the config file (if any)
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
    
    // create a list of potential config files
    std::list<std::string> cConfigFileHints = config_file_hints();
    m_sConfigFile = "";
    
    // this will take all options found
    qkd::utility::properties cConfig;
    
    // find and load
    for (auto & sFileHint : cConfigFileHints) {

        // open file
        std::ifstream cConfigFile(sFileHint);
        if (!cConfigFile.is_open()) continue;
        
        // found a config file
        m_sConfigFile = QString::fromStdString(sFileHint);
        qkd::utility::syslog::info() << "found config file: " << sFileHint << ", taking values from there ...";

        // read in the options
        std::set<std::string> cOptions;
        cOptions.insert("*");
        
        try {
            boost::program_options::detail::config_file_iterator cConfigIter(cConfigFile, cOptions);
            boost::program_options::detail::config_file_iterator cEOF;
            
            // collect all found options
            for (; cConfigIter != cEOF; cConfigIter++) {
                
                // get the next option
                boost::program_options::option cOption = *cConfigIter;
                cConfig[cOption.string_key] = cOption.value[0];
            }
        }
        catch (boost::program_options::invalid_syntax const & cErrInvalidSyntax) {
            qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to parse config file: " << sFileHint << " invalid syntax at: '" << cErrInvalidSyntax.tokens() << "'";
            exit(1);
        }
        catch (std::exception const & cException) {
            qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to parse config file: " << sFileHint << " exception: " << cException.what();
            exit(1);
        }
        
        // we take the very first found file and exit here the search for files
        // otherwise config options will be overwritten by config files found later
        break;
    }
    
    // found anything?
    if (m_sConfigFile.isEmpty()) {
        qkd::utility::syslog::info() << "no config file found, starting with default/empty values";
        return;
    }
    
    // check if we have anything at all
    if (cConfig.size() == 0) {
        if (!m_sConfigFile.isEmpty()) qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "found config file: " << m_sConfigFile.toStdString() << " but didn't find any option - is this intended?";
        return;
    }

    // this will take all options found - per link
    std::map<std::string, qkd::utility::properties> cLinkConfig;
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

        // reconstruct the key: all the rest key value with a '.' in between 
        std::stringstream ssKey;
        ssKey << sTokens[2];
        for (auto key_iter = sTokens.begin() + 3; key_iter != sTokens.end(); key_iter++) {
            ssKey << "." << (*key_iter);
        }
        std::string sKey = ssKey.str();
        
        // the key must be known
        const std::set<std::string> cValidKeyNames = { "db", "id", "listen.uri", "master", "peer.uri", "secret", "secret_file", "ipsec" };
        if (cValidKeyNames.find(sKey) == cValidKeyNames.end()) {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "parsed config file: '" << m_sConfigFile.toStdString() << "', section [" << sTokens[0] << "." << sTokens[1] << "]: detected unknown key '" << sKey << "' - dropping";
            continue;
        }
                
        // "link INI identifier" --> "key" = "value"
        cLinkConfig[sTokens[1]][sKey] = iter.second;
    }
    
    // check if we do have a link config at all
    if (cLinkConfig.empty()) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "no link configuration found - did you miss a [link.NAME] line?";
    }
    else {
    
        // debug out
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

        // apply the values
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
    
    if (qkd::utility::debug::enabled()) qkd::utility::debug() << "applying values for config setting '" << sLinkIdentifier << "'";
    
    // grab the id
    if (cConfig.find("id") == cConfig.end()) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to setup link for '" << sLinkIdentifier << "': missing value for 'id'";
        return;
    }
    
    // extract the id and create the link
    QString sId = QString::fromStdString(cConfig.at("id"));
    if (!create_link(sId)) return;
    
    // get the link
    qkd::q3p::engine cEngine = qkd::q3p::engine_instance::get(sId);
    if (cEngine.get() == nullptr) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to setup link: engine created but unable to fetch unstance. this mus not happen. this is a bug.";
        return;
    }
    
    // check for role
    if (cConfig.find("master") != cConfig.end()) {
        
        std::set<std::string> cTrueValues = { "1", "y", "yes", "true" };
        std::set<std::string> cFalseValues = { "0", "n", "no", "false" };
        
        if (cTrueValues.find(cConfig.at("master")) != cTrueValues.end()) {
            cEngine->set_master(true);
            cEngine->set_slave(false);
        }
        else
        if (cFalseValues.find(cConfig.at("master")) != cFalseValues.end()) {
            cEngine->set_master(false);
            cEngine->set_slave(true);
        }
        else {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to parse value for '" << sLinkIdentifier << "': don't know how to interpret value of 'master': '" << cConfig.at("master") << "'";
        }
    }
    
    // set up DB
    if (cConfig.find("db") != cConfig.end()) cEngine->open_db(QString::fromStdString(cConfig.at("db")));
    
    // check the shared secret
    if ((cConfig.find("secret") != cConfig.end()) && (cConfig.find("secret_file") != cConfig.end())) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "parsing config for '" << sLinkIdentifier << "': both 'secret' AND 'secret_file' given - 'secret' takes precedence.";
    }

    // get the secret
    QByteArray cSharedSecret;
    if (cConfig.find("secret") != cConfig.end()) cSharedSecret = QByteArray(cConfig.at("secret").data(), cConfig.at("secret").length());
    else
    if (cConfig.find("secret_file") != cConfig.end()) {
        
        // read in the shared secret from a file
        QFile cSecretFile(QString::fromStdString(cConfig.at("secret_file")));
        if (!cSecretFile.open(QIODevice::ReadOnly)) qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to open shared secret_file: '" << cConfig.at("secret_file") << "'";
        else cSharedSecret = cSecretFile.readAll();
    }
    
    // do we have a secret?
    if (cSharedSecret.size() == 0) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "config for '" << sLinkIdentifier << "': I don't have a shared secret to start with - unable to proceed.";
        return;
    }
    
    // set up IPSec
    if (cConfig.find("ipsec") != cConfig.end()) cEngine->configure_ipsec(QString::fromStdString(cConfig.at("ipsec")));
    

    // start a listener?
    if (cConfig.find("listen.uri") != cConfig.end()) {
        cEngine->listen(QString::fromStdString(cConfig.at("listen.uri")), cSharedSecret);
    }
    else {
        qkd::utility::syslog::info() << "config for '" << sLinkIdentifier << "': insufficient listener-config - not going to listen.";
    }

    // connect?
    if (cConfig.find("peer.uri") != cConfig.end()) {
        cEngine->connect(QString::fromStdString(cConfig.at("peer.uri")), cSharedSecret);
    }
    else {
        qkd::utility::syslog::info() << "config for '" << sLinkIdentifier << "': insufficient peer-config - not going to connect peer.";
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
    
    // add an already noted config file
    if (!sConfigFile.isEmpty()) {
        
        // deduce proper settings
        QUrl cConfigFileUrl(sConfigFile);
        if (!cConfigFileUrl.scheme().isEmpty()) sConfigFile = cConfigFileUrl.toLocalFile();

        // deal with a file ...
        boost::filesystem::path cConfigFilePath(sConfigFile.toStdString());
        if (!boost::filesystem::exists(cConfigFilePath)) {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "cannot access given config file: '" << cConfigFilePath.string() << "'";
        }
        else {
            
            // is this a file?
            if (!boost::filesystem::is_regular_file(cConfigFilePath)) {
                qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "given config file: '" << cConfigFilePath.string() << "' seems not to be a regular file";
            }
            else {
                
                // a good file, make it absolute
                cConfigFileHints.push_back(boost::filesystem::absolute(cConfigFilePath).string());
            }
        }
    }
    
    // add default searches
    cConfigFileHints.push_back(qkd::utility::environment::process_image_path().string() + "/" + sNode + ".conf");
    cConfigFileHints.push_back(qkd::utility::environment::data_path("q3p").string() + "/" + sNode + ".conf");
    cConfigFileHints.push_back(qkd::utility::environment::prefix_path().string() + "/etc/q3p/" + sNode + ".conf");
    cConfigFileHints.push_back(qkd::utility::environment::config_path().string() + "/q3p/" + sNode + ".conf");
    
    // debug out
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
    catch (qkd::q3p::engine_instance::engine_already_registered & cException) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to register new Q3P engine: \"" << sName.toStdString() << "\" - already registered";
    }
    catch (qkd::q3p::engine_instance::engine_invalid_id & cException) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to register new Q3P engine: \"" << sName.toStdString() << "\" - invalid id name";
    }
    catch (...) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to register new Q3P engine: \"" << sName.toStdString() << "\" - unknown reason";
    }

    return false; 
}


/**
 * get the list of links
 * 
 * @return  a list of links
 */
QStringList node::links() {
    
    QStringList cList;
    
    // walk over the known engines
    qkd::q3p::engine_map const & cEngines = qkd::q3p::engine_instance::engines();
    for (auto & iter : cEngines) {
        cList << iter.second->link_id();
    }
    
    return cList;
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
 *      PIPELINE ....... The id of the pipeline the modue 
 *                       is currently in
 *      HINT ........... Any user supplied information to 
 *                       the module
 *      URL_LISTEN ..... The public availbale listen URL of 
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
    
    // syslog
    qkd::utility::syslog::info() << "received quit signal. shutting down ...";

    // close them all
    qkd::q3p::engine_instance::close_all();
    
    // end "application"
    qApp->quit();
}


/**
 * set the url of the random value source
 *
 * @param   sRandomUrl      the new url of the random value source
 */
void node::set_random_url(QString sRandomUrl) {
    
    try {
        
        // the next line may fail, when an invalid URL is specified
        qkd::utility::random r = qkd::utility::random_source::create(sRandomUrl.toStdString());
        
        // good. apply!
        m_cRandom = r;
        m_sRandomUrl = sRandomUrl;
        
        // syslog
        qkd::utility::syslog::info() << "new random source: '" << sRandomUrl.toStdString() << "'";
        
        emit random_url_changed(sRandomUrl);
    }
    catch (...) {
        // syslog
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to set new random source: '" << sRandomUrl.toStdString() << "'";
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


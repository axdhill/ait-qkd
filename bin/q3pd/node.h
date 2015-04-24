/*
 * node.h
 * 
 * the Q3P Node definition
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

 
#ifndef __QKD_Q3P_NODE_H_
#define __QKD_Q3P_NODE_H_


// ------------------------------------------------------------
// incs

#include <exception>
#include <string>
#include <boost/shared_ptr.hpp>

// Qt
#include <QtCore/QObject>
#include <QtDBus/QtDBus>

// ait
#include <qkd/utility/debug.h>
#include <qkd/utility/environment.h>
#include <qkd/utility/properties.h>
#include <qkd/utility/random.h>


// ------------------------------------------------------------
// decls


namespace qkd {
    
namespace q3p {    


/**
 * This is a Q3P Node.
 * 
 * All the Key Store needs is an ID. Then it starts up and
 * connects it to its current session DBus. The ID must
 * be unique among all Q3P KeyStores connected to the
 * same DBus.
 * 
 * Once connected, the node can be accessed by the DBus address:
 * 
 *          at.ac.ait.q3p.node-ID
 * 
 *           (with ID substiuted)
 * 
 * On this session DBus it offers properties and methods under "/Node"
 * 
 *      DBus Interface: "at.ac.ait.q3p.node"
 * 
 * Properties of at.ac.ait.q3p.node
 * 
 *      -name-          -read/write-    -description-
 * 
 *      config_file         R           the config file the node found and configured itself
 * 
 *      debug               R/W         enable/disable debug output on stderr
 * 
 *      id                  R           ID of the node
 * 
 *      process_id          R           PID of the KeyStore process within the operating system
 * 
 *      process_image       R           Full path to the Q3P KeyStore program launched
 * 
 *      random_url          R/W         The random URL used to gain random values
 * 
 *      start_time          R           UNIX epoch timestamp of Q3P KeyStore launch
 * 
 * 
 * Methods of at.ac.ait.q3p.node
 * 
 *      -name-                  -description-
 * 
 *      create_link()           create a link instance
 * 
 *      links()                 return the known links
 * 
 *      modules()               get a list of current present modules on the node
 * 
 *      quit()                  shut down KeyStore
 * 
 *      uptime()                return seconds since launch (see: "start_time")
 * 
 * 
 * Signals of at.ac.ait.q3p.node
 * 
 *      -name-                  -description-
 * 
 *      debug_changed           the debug property has a new value
 * 
 *      link_created            a link has been created
 * 
 *      log                     a log message
 * 
 *      random_url_changed      the random url property has a new value
 * 
 * 
 * Note: a UNIX epoch timestamp counts the seconds since 1/1/1970.
 */
class node : public QObject {
    
    
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "at.ac.ait.q3p.node")
    
    Q_PROPERTY(QString config_file READ config_file)
    Q_PROPERTY(bool debug READ debug WRITE set_debug)
    Q_PROPERTY(QString id READ id)
    Q_PROPERTY(unsigned int process_id READ process_id)
    Q_PROPERTY(QString process_image READ process_image)
    Q_PROPERTY(QString random_url READ random_url WRITE set_random_url)
    Q_PROPERTY(qulonglong start_time READ start_time)
    
    
public:


    /**
     * ctor
     * 
     * @param   sId                 ID of the node
     * @param   sConfigFileURL      URL of the config file
     */
    node(QString const & sId, QString const & sConfigFileURL = "");
    
    
    /**
     * dtor
     */
    ~node();
    
    
    /**
     * the config file we found and loaded
     *
     * @return  the config file we found and loaded
     */
    QString config_file() const { return m_sConfigFile; };
    
    
    /**
     * check if we are in debug mode
     * 
     * @return  true, if debug messages ought to go to stderr
     */
    bool debug() const { return qkd::utility::debug::enabled(); };
    
    
    /**
     * get the id of the key store
     *
     * @return  the id of the key store
     */
    QString id() const { return m_sId; };
    
    
    /**
     * get the process id of the key store
     * 
     * @return  the operating system process id of the key store
     */
    unsigned int process_id() const { return qkd::utility::environment::process_id(); };
    
    
    /**
     * get the process image path of the key store
     * 
     * @return  the operating system process image of this key store
     */
    QString process_image() const { return QString::fromStdString(qkd::utility::environment::process_image_path().string()); };
    
    
    /**
     * get the url of the random value source
     *
     * @return  the url of the random value source
     */
    QString random_url() const { return m_sRandomUrl; };
    
    
    /**
     * set the debug flag
     * 
     * @param   bDebug      new debug value
     */
    void set_debug(bool bDebug) { qkd::utility::debug::enabled() = bDebug; emit debug_changed(bDebug); };
    
    
    /**
     * set the url of the random value source
     *
     * @param   sRandomUrl      the new url of the random value source
     */
    void set_random_url(QString sRandomUrl);
    
    
    /**
     * UNIX epoch timestamp of launch
     * 
     * Seconds since 1/1/1970 when this instance
     * has been launched.
     * 
     * @return  UNIX epoch timestamp of key-store launch
     */
    unsigned long start_time() const { return m_nStartTimeStamp; };
    
    
    /**
     * trigger a new log entry
     * 
     * @param   sLog        the log message
     */
    void trigger_log(QString const & sLog);
    
    
public slots:
    
    
    /**
     * create a link instance
     * 
     * @param   sName       name of the link
     */
    bool create_link(QString const & sName);
    
    
    /**
     * get the list of links
     * 
     * @return  a list of links
     */
    QStringList links();
    
    
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
    QStringList modules();
    
    
    /**
     * turn down the key store
     */
    Q_NOREPLY void quit();
    
    
    /**
     * number of seconds this key store is up
     * 
     * @return  number of seconds this key store is up and running
     */
    qulonglong uptime();
    
    
signals:
    
    
    /**
     * debug property changed
     * 
     * @param   bEnabled        new value of debug
     */
    void debug_changed(bool bEnabled);
    
    
    /**
     * a link has been created
     * 
     * @param   sLink           name of new link
     */
    void link_created(QString sLink);
    
    
    /**
     * a log message
     * 
     * @param   sMessage        the log message
     */
    void log(QString sMessage);
    
    
    /**
     * random_url property changed
     * 
     * @param   sRandomUrl      new value of random_url
     */
    void random_url_changed(QString sRandomUrl);
    
    
private slots:
    
    
    /**
     * fetch a config file and apply values
     */
    void apply_config_file();
    

private:
    
    
    /**
     * apply a link config
     * 
     * @param   sLinkIdentifier     the link identifier
     * @param   cConfig             a key-->value map for the link
     */
    void apply_link_config(std::string const & sLinkIdentifier, qkd::utility::properties const & cConfig);
    

    /**
     * create a set of config file hints
     * 
     * @return  an ordered list of config file hints
     */
    std::list<std::string> config_file_hints() const;
    

    /**
     * the config file we loaded
     */
    QString m_sConfigFile;
    
    
    /**
     * id of key store
     */
    QString m_sId;
    
    
    /**
     * the random source
     */
    qkd::utility::random m_cRandom;
    
    
    /**
     * the random url
     */
    QString m_sRandomUrl;
    
    
    /**
     * init UNIX epoch: time of birth
     */
    unsigned long m_nStartTimeStamp;
    
};
  

}

}


#endif

/*
 * module.h
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

 
#ifndef __QKD_QKD_PIPELINE_MODULE_H_
#define __QKD_QKD_PIPELINE_MODULE_H_


// ------------------------------------------------------------
// incs

#include <list>
#include <string>

#include <QtXml/QDomElement>

#include <qkd/module/module.h>

// ------------------------------------------------------------
// decl


/**
 * This class holds a module definiton for the qkd-pipeline tool
 */
class module  {
    

public:
    
    
    /**
     * ctor
     */
    module();
    
    
    /**
     * ctor
     * 
     * @param   sDBusServiceName        the known DBus service name of the module
     */
    module(std::string const & sDBusServiceName);
    
    
    /**
     * returns the module's program arguments
     * 
     * @return  the command line arguments for the module
     */
    std::list<std::string> const & arguments() const { return m_cArgs; }

    
    /**
     * clear the module values
     */
    void clear();
    
    
    /**
     * create the command line for this module
     * 
     * @param   cArgs       the command line arguments
     * @return  the number of command line arguments
     */
    unsigned int command_line(char * cArgs[1024]);
    
    
    /**
     * returns the path to the configuration file
     * 
     * @return  the path to the configuration for this module
     */
    std::string const & configuration_file() { return m_sConfiguration; }
    
    
    /**
     * "pause()" on the DBus object
     */
    void dbus_call_pause();
    
    
    /**
     * "terminate()" on the DBus object
     */
    void dbus_call_terminate();
    
    
    /**
     * get the "url_pipe_in" property on the DBus object
     * 
     * @return  the DBus property of URL pipe in
     */
    QString dbus_get_url_pipe_in() const;
    
    
    /**
     * get the "url_pipe_out" property on the DBus object
     * 
     * @return  the DBus property of URL pipe out
     */
    QString dbus_get_url_pipe_out() const;
    
    
    /**
     * returns the DBus name handle of this module
     * 
     * @return  the DBus Service name on which to get this module
     */
    std::string dbus_service_name() const { return m_sDBusServiceName; }
   

    /**
     * set the "url_pipe_in" property on the DBus object
     * 
     * @param   sURLPipeIn      the new URL pipe in
     */
    void dbus_set_url_pipe_in(QString sURLPipeIn);
    
    
    /**
     * set the "url_pipe_out" property on the DBus object
     * 
     * @param   sURLPipeOut     the new URL pipe out
     */
    void dbus_set_url_pipe_out(QString sURLPipeOut);
    
    
    /**
     * return the found path to the module's executable
     * 
     * @return  the executable for this module
     */
    boost::filesystem::path executable() const;
   
   
    /**
     * prints the module definition in a human-readable format to the target output stream
     *
     * @param   cTarget             the stream to write to
     */
    void dump(std::ostream & cTarget) const;
    
    
    /**
     * get the module via a process ID
     * 
     * @param   sPID        the process id of the running module
     */
    static module get_by_pid(std::string const & sPID);
    
    
    /**
     * check if this is to be run as 'alice'
     *
     * @return  true, if this module should start as 'alice'
     */
    bool is_alice() const { return m_bAlice; }
    
    
    /**
     * check if this is a valid module definition
     *
     * @return  true, if we have a valid module at hand
     */
    bool is_valid() const;
    
    
    /**
     * returns the path to the module's log file
     * 
     * @return  the path to the module's log file
     */
    std::string log_file() const { return m_sLogFile; }
    
    
    /**
     * parse a single module XML node
     * 
     * the found values are inserted into the global
     * pipeline variable (g_cPipeline)
     * 
     * @param   cModuleElement      the module XML element
     * @return  0 for success, else errorcode as for main()
     */
    int parse(QDomElement const & cModuleElement);
    
    
    /**
     * get the path to the module's process image as it is configured
     * 
     * @return  the path to the module's program file as configured
     */
    std::string process_image() const { return m_sProcessImage; }
    
    
    /**
     * sets the DBus name handle of this module
     * 
     * @param   sDBusServiceName        the new DBus Service name of the module
     */
    void set_dbus_service_name(std::string const & sDBusServiceName) { m_sDBusServiceName = sDBusServiceName; }
   

    /**
     * start the module
     */
    void start();
    
    
private:    


    /**
     * waits until the module reached a certain state
     *
     * @param   eState                  module state to wait for
     */
    bool wait_for_module_state(qkd::module::module_state eState);


    std::string m_sProcessImage;            /**< path to module binary */
    std::string m_sConfiguration;           /**< path to module's configuration file */
    bool m_bAlice;                          /**< alice role (or bob if false) */
    std::list<std::string> m_cArgs;         /**< additional arguments to pass on the command line */
    std::string m_sLogFile;                 /**< path to log file */
    std::string m_sDBusServiceName;         /**< DBus service name of started module */
    
        
};


#endif

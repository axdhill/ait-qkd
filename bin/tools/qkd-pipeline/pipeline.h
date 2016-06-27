/*
 * pipeline.h
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

 
#ifndef __QKD_QKD_PIPELINE_PIPELINE_H_
#define __QKD_QKD_PIPELINE_PIPELINE_H_


// ------------------------------------------------------------
// incs

#include <list>
#include <string>

#include "module.h"


// ------------------------------------------------------------
// decl


/**
 * This class holds a pipeline definiton for the qkd-pipeline tool
 */
class pipeline  {
    

public:

    
    /**
     * autoconnect modules
     *
     * @return  true for success
     */
    bool autoconnect_modules();
    
    
    /**
     * return the path of the pipeline's log folder
     * 
     * @return  the path to the log folder for the pipeline
     */
    std::string const & log_folder() const { return m_sLogFolder; }
    
    
    /**
     * parse the given XML config file
     * 
     * @param   sPipelineConfiguration      path to configuration file
     * @return  0 for success, else errorcode as for main()
     */
    int parse(std::string const & sPipelineConfiguration);
    
    
    /**
     * sets the path of the pipeline's log folder
     * 
     * @param   sLogFolder      the new path to the log folder for the pipeline
     */
    void set_log_folder(std::string const & sLogFolder) { m_sLogFolder = sLogFolder; }
    
    
    /**
     * start the pipeline
     * 
     * @return  0 for success, else errorcode as for main()
     */
    int start();
    
    
    /**
     * stop the pipeline
     * 
     * @return  0 for success, else errorcode as for main()
     */
    int stop();
    
    
private:

    
    /**
     * retrieves the pipeline entry and exit URLs
     *
     * @param   sURLPipeIn      pipeline entry
     * @param   sURLPipeOut     pipeline exit
     */
    void pipeline_pipes(std::string & sURLPipeIn, std::string & sURLPipeOut) const;
    
    
    /**
     * start the modules of the pipeline
     */
    void start_modules();
    
    
    /**
     * set the pipeline entry socket
     */
    void set_pipeline_entry();


    /**
     * verifies that the log folder exists and is writeable
     * 
     * @return  true, if we have a good log folder setting
     */
    bool verify_log_folder() const;
    
    
    std::string m_sName;                    /**< pipeline name */
    std::string m_sLogFolder;               /**< log folder */
    std::list<module> m_cModules;           /**< list of modules */

    bool m_bAutoConnect = false;            /**< autoconnect modules */
    std::string m_sURLPipeIn;               /**< input URL of whole pipeline */
    std::string m_sURLPipeOut;              /**< output URL of whole pipeline */
    

    
};


#endif

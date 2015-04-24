/*
 * investigation.h
 * 
 * An investigator makes a deep system inspection and 
 * returns a current snapshot of a running QKD machine.
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

 
#ifndef __QKD_UTILITY_INVESTIGATION_H_
#define __QKD_UTILITY_INVESTIGATION_H_


// ------------------------------------------------------------
// incs

#include <chrono>
#include <map>
#include <set>
#include <string>

// ait
#include <qkd/utility/properties.h>


// ------------------------------------------------------------
// decls


namespace qkd {
    
namespace utility {    
    
    
/**
 * the result of a single investigation
 */    
struct investigation_result {
    
    std::map<std::string, qkd::utility::properties> cLinks;         /**< all links discovered */
    std::map<std::string, qkd::utility::properties> cModules;       /**< all modules discovered */
    std::map<std::string, qkd::utility::properties> cNodes;         /**< all nodes discovered */
    std::map<std::string, qkd::utility::properties> cPipelines;     /**< all pipelines discovered */
};  


/**
 * a single element in the pipeline graph
 */
struct pipeline_element {
    
    std::string sModuleId;                      /**< module id */
    std::set<std::string> cPredecessors;        /**< set of previous modules */
    std::set<std::string> cSuccessors;          /**< set of following modules */
    
    uint64_t nLevel;                            /**< module level */
};


/**
 * this class makes a deep system investigation and returns any found QKD relevant information
 * 
 * Basically this class is a container for found properties of a current
 * QKD system investigation. Hence, due to the highly parallel and distributed nature
 * of the overall system, we might indeed get an invalid snapshot. This is as information
 * id gathered about entities and their relationship to each other right in the middle of
 * action.
 * 
 * In order to make a real valid snapshot, one has to apply system snapshot generation
 * patterns, but currently this should be sufficient.
 */
class investigation {


public:
    
    
    /**
     * copy ctor
     * 
     * @param   rhs     right hand side
     */
    investigation(investigation const & rhs) : m_cResult(rhs.m_cResult), m_cTimestampEnd(rhs.m_cTimestampEnd), m_cTimestampStart(rhs.m_cTimestampStart) {};
    
    
    /**
     * dump found result to some stream
     * 
     * @param   cStream     the stream to dump to
     */
    void dump(std::ostream & cStream) const;
    
    
    /**
     * inspect the system
     * 
     * @return  the investigation results (as an instance of this class)
     */
    static investigation investigate();
    
    
    /**
     * return the found links
     * 
     * @return  the found links
     */
    inline std::map<std::string, qkd::utility::properties> const & links() const { return m_cResult.cLinks; };
    
    
    /**
     * return the found modules
     * 
     * @return  the found modules
     */
    inline std::map<std::string, qkd::utility::properties> const & modules() const { return m_cResult.cModules; };
    
    
    /**
     * return the found nodes
     * 
     * @return  the found nodes
     */
    inline std::map<std::string, qkd::utility::properties> const & nodes() const { return m_cResult.cNodes; };
    
    
    /**
     * return the found pipelines
     * 
     * @return  the found pipelines
     */
    inline std::map<std::string, qkd::utility::properties> const & pipelines() const { return m_cResult.cPipelines; };
    
    
    /**
     * return timestamp of investigation
     * 
     * @return  time of system snapshot
     */
    inline std::chrono::system_clock::time_point const & timestamp() const { return m_cTimestampEnd; };


    /**
     * return timestamp of investigation
     * 
     * @return  time of system snapshot
     */
    inline std::chrono::system_clock::duration duration() const { return (m_cTimestampEnd - m_cTimestampStart); };


private:


    /**
     * ctor
     */
    investigation() : m_cTimestampStart(std::chrono::system_clock::now()) {};
    
    
    // ------------------------------------------------------------
    // data
    
    
    /**
     * the result of the investigation
     */
    investigation_result m_cResult;
    
    
    /**
     * timestamp of investigation end
     */
    std::chrono::system_clock::time_point m_cTimestampEnd;        

    
    /**
     * timestamp of investigation start
     */
    std::chrono::system_clock::time_point m_cTimestampStart;        

    
};



}

}

#endif


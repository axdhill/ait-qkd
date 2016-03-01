/*
 * json_output.h
 *
 * This is shows the current QKD system snapshot
 *
 * Author: Manuel Warum, <manuel.warum@ait.ac.at>
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


#ifndef QKD_VIEW_JSON_OUTPUT_H
#define QKD_VIEW_JSON_OUTPUT_H


// ------------------------------------------------------------
// incs


#include <iostream>

#include <boost/format.hpp>

#include <qkd/utility/investigation.h>
#include <qkd/utility/properties.h>

#include "output_format.h"


// ------------------------------------------------------------
// decl


/**
 * An output_format implementation that specifically deals with JSON-formatted output.
 */
class json_output : public output_format {
    
    
public:
    
    
    /**
     * ctor
     * 
     * @param  cProgramOptions  formatting-related runtime options
     */
    json_output(configuration_options const & cProgramOptions);
    

    /**
     * Writes investigation results to the specified output stream.
     * 
     * @param  cOut            the stream to write to.
     * @param  cInvestigation  the particular investigation instance to format.
     */
    void write(std::ostream & cOut, qkd::utility::investigation const & cInvestigation);
    

protected:
    
    
    /**
     * Writes the basic investigation details to the specified stream.
     * 
     * @param  cOut            the stream to write to.
     * @param  cInvestigation  the particular investigation instance to format.
     */
    void dump_investigation_details(std::ostream & cOut, qkd::utility::investigation const & cInvestigation) const;
    

    /**
     * Utility method that dumps a specified set of properties to the specified output stream.
     *
     * @param  cOut            the stream to write to.
     * @param  cFields         a vector of all the fields we are interested in.
     * @param  cSource          the investigation's data container.
     */
    inline void dump_json_array(std::ostream & cOut,
                                std::vector<std::string> const & cFields,
                                std::map<std::string, qkd::utility::properties> const & cSource) const;

                                
    /**
     * Writes link information to the specified stream.
     * 
     * @param  cOut            the stream to write to.
     * @param  cLinkMap        the particular link data to format.
     */
    void dump_links(std::ostream & cOut, std::map<std::string, qkd::utility::properties> const & cLinkMap) const;

    
    /**
     * Writes module information to the specified stream.
     * 
     * @param  cOut            the stream to write to.
     * @param  cModuleMap      the particular module data to format.
     */
    void dump_modules(std::ostream & cOut, std::map<std::string, qkd::utility::properties> const & cModuleMap) const;

    
    /**
     * Writes node information to the specified stream.
     * 
     * @param  cOut            the stream to write to.
     * @param  cNodeMap        the particular node data to format.
     */
    void dump_nodes(std::ostream & cOut, std::map<std::string, qkd::utility::properties> const & cNodeMap) const;

    
    /**
     * Utility method that provides a format instance for the specified fields.
     *
     * @param  cFields         a vector of all the fields we are interested in.
     * @return a format instance that concatenates all fields into a JSON object.
     */
    inline boost::format simple_format(std::vector<std::string> const & cFields) const;

    
    bool m_bPrintModuleIO = false;
    
    
};


#endif

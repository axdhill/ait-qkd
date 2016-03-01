/*
 * tabular_output.h
 *
 * This is shows the current QKD system snapshot
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

#ifndef QKD_VIEW_TABULAR_OUTPUT_H
#define QKD_VIEW_TABULAR_OUTPUT_H


// ------------------------------------------------------------
// incs


#include <ostream>

#include <qkd/utility/investigation.h>
#include <qkd/utility/properties.h>

#include "output_format.h"


// ------------------------------------------------------------
// decl


/**
 * An output_format implementation that specifically deals with tabular formatted output.
 */
class tabular_output : public output_format {
    
    
public:
    
    
    /**
     * ctor
     * 
     * @param  cProgramOptions  formatting-related runtime options
     */
    tabular_output(configuration_options const & cProgramOptions);
    

    /**
     * Writes investigation results to the specified output stream.
     * 
     * @param  cOut            the stream to write to.
     * @param  cInvestigation  the particular investigation instance to format.
     */
    void write(std::ostream & cOut, qkd::utility::investigation const & cInvestigation);

    
private:

    
    /**
     * Writes the basic investigation details to the specified stream.
     * 
     * @param  cOut            the stream to write to.
     * @param  cInvestigation  the particular investigation instance to format.
     */
    void dump_investigation_details(std::ostream & cOut, qkd::utility::investigation const & cInvestigation) const;

    
    /**
     * Writes node information to the specified stream.
     * 
     * @param  cOut            the stream to write to.
     * @param  cNodeMap        the particular node data to format.
     */
    void dump_nodes(std::ostream & cOut, const std::map<std::string, qkd::utility::properties> & cNodeMap);

    
    /**
     * Writes link information to the specified stream.
     * 
     * @param  cOut            the stream to write to.
     * @param  cLinkMap        the particular link data to format.
     */
    void dump_links(std::ostream & cOut, const std::map<std::string, qkd::utility::properties> & cLinkMap);
    

    /**
     * Writes module information to the specified stream.
     * 
     * @param  cOut            the stream to write to.
     * @param  cModuleMap      the particular module data to format.
     */
    void dump_modules(std::ostream & cOut, const std::map<std::string, qkd::utility::properties> & cModuleMap);
    
    
    /**
     * Writes full module information to the specified stream.
     * 
     * @param  cOut            the stream to write to.
     * @param  cModuleMap      the particular module data to format.
     */
    void dump_modules_full(std::ostream & cOut, const std::map<std::string, qkd::utility::properties> & cModuleMap);
    
    
    /**
     * Writes io module information to the specified stream.
     * 
     * @param  cOut            the stream to write to.
     * @param  cModuleMap      the particular module data to format.
     */
    void dump_modules_io(std::ostream & cOut, const std::map<std::string, qkd::utility::properties> & cModuleMap);
    
    
    /**
     * Writes short module information to the specified stream.
     * 
     * @param  cOut            the stream to write to.
     * @param  cModuleMap      the particular module data to format.
     */
    void dump_modules_short(std::ostream & cOut, const std::map<std::string, qkd::utility::properties> & cModuleMap);
    
    
    bool bPrintHeader = true;               /**< pringt header in output */
    bool bPrintModuleIO = false;            /**< print only module connection addresses */
    bool bPrintShort = false;               /**< print only important data */
    
};


#endif

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

#ifndef QKD_TABULAR_OUTPUT_H
#define QKD_TABULAR_OUTPUT_H

#include <ostream>

#include <qkd/utility/investigation.h>
#include <qkd/utility/properties.h>

#include "output_format.h"

class tabular_output : public output_format {
public:
    bool bPrintHeader = true;
    bool bPrintModuleIO = false;
    bool bPrintShort = false;

    void initialize(configuration_options const &cProgramOptions);
    void write(std::ostream &cOut, qkd::utility::investigation &cInvestigation);

protected:
    typedef std::map<std::string, std::string::size_type> column_width;

    struct {
        column_width cLink;         /**< width of link columns */
        column_width cModule;       /**< width of module columns */
        column_width cNode;         /**< width of node columns */
    } cColumnWidth;

    void set_column_width(column_width &cColumnWidth, qkd::utility::properties const &cProperties);

    void dump_investigation_details(std::ostream &cOut, qkd::utility::investigation &cInvestigation) const;
    void dump_nodes(std::ostream &cOut, const std::map<std::string, qkd::utility::properties> &cNodeMap);
    void dump_links(std::ostream &cOut, const std::map<std::string, qkd::utility::properties> &cLinkMap);
    void dump_modules(std::ostream &cOut, const std::map<std::string, qkd::utility::properties> & cModuleMap);
};


#endif //QKD_TABULAR_OUTPUT_H

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

#ifndef QKD_JSON_OUTPUT_H
#define QKD_JSON_OUTPUT_H

#include <iostream>

#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string/replace.hpp>

#include <qkd/utility/debug.h>
#include <qkd/utility/investigation.h>
#include <qkd/utility/properties.h>

#include "output_format.h"

class json_output : public output_format {
public:
    bool bPrintModuleIO = false;

    void initialize(boost::program_options::variables_map & cProgramOptions);
    void write(qkd::utility::investigation & cInvestigation);

protected:
    void dump_investigation_details(qkd::utility::investigation & cInvestigation) const;
    void dump_nodes(const std::map<std::string, qkd::utility::properties> &cNodeMap) const;
    void dump_links(const std::map<std::string, qkd::utility::properties> &cLinkMap) const;
    void dump_modules(const std::map<std::string, qkd::utility::properties> & cModuleMap) const;

    inline void dump_json_array(const std::vector<std::string> &cFields, const std::map<std::string, qkd::utility::properties> &source)const;
    inline boost::format simple_format(std::vector<std::string> const &cFields)const;
};


#endif //QKD_JSON_OUTPUT_H

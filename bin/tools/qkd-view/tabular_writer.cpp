/*
 * tabular_writer.cpp
 *
 * Write qkd::utility::properties as tabular
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

#include <iostream>
#include <sstream>

#include <boost/format.hpp>

#include "tabular_writer.h"


// ------------------------------------------------------------
// code


    /**
     * ctor
     * 
     * @param   cOut            the ostream to write to
     * @param   cTable          the properties to write
     * @param   cFields         the fields of interest within the properties
     * @param   bHeader         print the header  
     * @param   sIndent         indent string of each line
     */
tabular_writer::tabular_writer(std::ostream & cOut, 
                               std::map<std::string, qkd::utility::properties> const & cTable, 
                               std::list<std::string> const & cFields, 
                               bool bHeader, 
                               std::string const & sIndent) : 
        m_cFields(cFields), m_bHeader(bHeader), m_sIndent(sIndent), m_cTable(cTable) {
            
    set_column_width();
    set_format_string();
    write(cOut);
}


/**
 * get the correct format string.
 *
 * @return  a boost::format compliant format string
 */
void tabular_writer::set_format_string() {
    
    static std::string const sDelimiter = "    ";
    
    std::stringstream ss;
    ss << "%-" << m_sIndent.length() << "s" << sDelimiter;
    for (auto const & s: m_cFields) {
        ss << "%-" << m_cColumnWidth[s] << "s" << sDelimiter;
    }
    
    m_sFormatString = ss.str();
}


/**
 * Set the maximum column width for the properties.
 */
void tabular_writer::set_column_width() {
    
    for (auto const & cEntry : m_cTable) {
        
        for (auto const & cColumn : cEntry.second) {
            
            if (m_cColumnWidth.find(cColumn.first) == m_cColumnWidth.end()) {
                m_cColumnWidth[cColumn.first] = cColumn.first.length();
            }

            m_cColumnWidth[cColumn.first] = std::max(m_cColumnWidth[cColumn.first], cColumn.second.length());
        }
    }
}


/**
 * Writes the properties to the specified output stream.
 * 
 * @param  cOut            the stream to write to.
 */
void tabular_writer::write(std::ostream & cOut) const {
    
    write_header(cOut);
    
    for (auto const & cEntry: m_cTable) {
        
        boost::format cFormat(m_sFormatString);
        cFormat % m_sIndent;
        for (auto const & f: m_cFields) {
            cFormat % cEntry.second.at(f);
        }

        cOut << cFormat.str() << std::endl;
    }
}


/**
 * Writes the header.
 * 
 * @param  cOut            the stream to write to.
 */
void tabular_writer::write_header(std::ostream & cOut) const {
    
    if (!m_bHeader) return;

    boost::format cFormat(m_sFormatString);
    cFormat % "";
    for (auto const & s: m_cFields) {
        cFormat % s;
    }

    cOut << cFormat.str() << std::endl;
}

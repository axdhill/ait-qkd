/*
 * tabular_writer.h
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

#ifndef QKD_VIEW_TABULAR_WRITER_H
#define QKD_VIEW_TABULAR_WRITER_H


// ------------------------------------------------------------
// incs

#include <list>
#include <map>
#include <ostream>
#include <string>

#include <qkd/utility/properties.h>


// ------------------------------------------------------------
// decl


/**
 * This class writes qkd::utility::properties in tabular form to an output stream.
 */
class tabular_writer {
    
    
public:
    
    
    /**
     * ctor
     * 
     * @param   cOut            the ostream to write to
     * @param   cTable          the properties to write
     * @param   cFields         the fields of interest within the properties
     * @param   bHeader         print the header  
     * @param   sIndent         indent string of each line
     */
    tabular_writer(std::ostream & cOut, 
                   std::map<std::string, qkd::utility::properties> const & cTable, 
                   std::list<std::string> const & cFields, 
                   bool bHeader, 
                   std::string const & sIndent);
    

private:
    
    
    /**
     * Set the maximum column width for the properties.
     */
    void set_column_width();

    
    /**
     * set the boost::format compliant format string.
     */
    void set_format_string();

    
    /**
     * Writes the properties to the specified output stream.
     * 
     * @param  cOut            the stream to write to.
     */
    void write(std::ostream & cOut) const;

    
    /**
     * Writes the header.
     * 
     * @param  cOut            the stream to write to.
     */
    void write_header(std::ostream & cOut) const;
    
    
    /**
     * holds the maximum width of each column.
     */
    std::map<std::string, std::string::size_type> m_cColumnWidth;
    
    
    /**
     * fields of interest within the properties.
     */
    std::list<std::string> const & m_cFields;
    
    
    /**
     * the boost::format compliant format string.
     */
    std::string m_sFormatString;
    
    
    /**
     * print the header of each column.
     */
    bool m_bHeader;
    
    
    /**
     * indent of each line.
     */
    std::string m_sIndent;
    
    
    /**
     * the properties to write.
     */
    std::map<std::string, qkd::utility::properties> const & m_cTable;

    
};


#endif

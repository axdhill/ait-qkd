/*
 * ptree.cpp
 *
 * declare some usefull functions for boost::property_tree::ptree
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


// ------------------------------------------------------------
// incs

#include <sstream>

// ait
#include <qkd/utility/ptree.h>


// ------------------------------------------------------------
// fwd


/**
 * recursive dumping function for ptree::dump
 *
 * @param   cPT         the property_tree to dump
 * @param   nIndent     number of indents
 * @return  a string describing the memory footprint of the property_tree
 */
std::string dump(boost::property_tree::ptree const & cPT, int nIndent);


// ------------------------------------------------------------
// code


/**
 * return a string describing the memory footprint of a ptree
 *
 * @param   cPT         the property_tree to dump
 * @return  a string describing the memory footprint of the property_tree
 */
std::string qkd::utility::ptree_dump(boost::property_tree::ptree const & cPT) {
    return dump(cPT, 0);
}


/**
 * recursive dumping function for ptree::dump
 *
 * @param   cPT         the property_tree to dump
 * @param   nIndent     number of indents
 * @return  a string describing the memory footprint of the property_tree
 */
std::string dump(boost::property_tree::ptree const & cPT, int nIndent) {
    
    std::stringstream ss;
    
    std::stringstream ssIndent;
    for (int i = 0; i < nIndent; ++i) ssIndent << "    ";
    
    for (auto const & cNode: cPT) {
        
        ss << ssIndent.str() << "node.first=" << cNode.first << std::endl;
        ss << ssIndent.str() << "node.second.empty()=" << (cNode.second.empty() ? "true" : "false") << std::endl;
        ss << ssIndent.str() << "node.second.count('')=" << cNode.second.count("") << std::endl;
        ss << ssIndent.str() << "node.second.data=" << cNode.second.get_value<std::string>() << std::endl;
        
        ss << dump(cNode.second, nIndent + 1);
    }
    
    return ss.str();
}

/*
 * ptree.h
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


#ifndef __QKD_UTILITY_PTREE_H
#define __QKD_UTILITY_PTREE_H


// ------------------------------------------------------------
// incs

#include <string>

#include <boost/property_tree/ptree.hpp>


// ------------------------------------------------------------
// decls


namespace qkd {
    
namespace utility {    
    

/**
 * return a string describing the memory footprint of a ptree
 *
 * @param   cPT         the property_tree to dump
 * @return  a string describing the memory footprint of the property_tree
 */
std::string ptree_dump(boost::property_tree::ptree const & cPT);


}

}

#endif


/* 
 * version.cpp
 * 
 * version implementation file 
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


// ------------------------------------------------------------
// incs

// ait
#include <qkd/version.h>

#define MAKE_VERSION(x, y, z)   ((x) << 24 | (y) << 16 | (z))


// ------------------------------------------------------------
// code


/**
 * version check for 9.9999 (pre R10)
 * 
 * any compiler and linker may link against the qkd library
 * requesting this funtcion to ensure availablity
 * 
 * @return  value, representing the version
 */
uint32_t qkd_version_9_9999() {
    return MAKE_VERSION(9, 9999, 0);
}


/**
 * version check for 9.9999.1 (R10 Developer Snapshot)
 * 
 * any compiler and linker may link against the qkd library
 * requesting this funtcion to ensure availablity
 * 
 * @return  value, representing the version
 */
uint32_t qkd_version_9_9999_1() {
    return MAKE_VERSION(9, 9999, 1);
}


/**
 * version check for 9.9999.2 
 * 
 * any compiler and linker may link against the qkd library
 * requesting this funtcion to ensure availablity
 * 
 * @return  value, representing the version
 */
uint32_t qkd_version_9_9999_2() {
    return MAKE_VERSION(9, 9999, 2);
}


/**
 * version check for 9.9999.3 
 * 
 * any compiler and linker may link against the qkd library
 * requesting this funtcion to ensure availablity
 * 
 * @return  value, representing the version
 */
uint32_t qkd_version_9_9999_3() {
    return MAKE_VERSION(9, 9999, 3);
}


/**
 * version check for 9.9999.4 
 * 
 * any compiler and linker may link against the qkd library
 * requesting this funtcion to ensure availablity
 * 
 * @return  value, representing the version
 */
uint32_t qkd_version_9_9999_4() {
    return MAKE_VERSION(9, 9999, 4);
}


/**
 * version check for 9.9999.5 
 * 
 * any compiler and linker may link against the qkd library
 * requesting this funtcion to ensure availablity
 * 
 * @return  value, representing the version
 */
uint32_t qkd_version_9_9999_5() {
    return MAKE_VERSION(9, 9999, 5);
}


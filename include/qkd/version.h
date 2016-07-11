/*
 * version.h
 *
 * version information file
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


#ifndef __QKD_VERSION_H
#define __QKD_VERSION_H


// ------------------------------------------------------------
// incs

#include <inttypes.h>
#include <string>


// ------------------------------------------------------------
// defs

#define QKD_VERSION             "9.9999.8"


// ------------------------------------------------------------
// decls


/**
 * version check for 9.9999 (pre R10)
 * 
 * any compiler and linker may link against the qkd library
 * requesting this function to ensure availability
 * 
 * @return  value, representing the version
 */
uint32_t qkd_version_9_9999();


/**
 * version check for 9.9999.1 (R10 Developer Snapshot)
 * 
 * any compiler and linker may link against the qkd library
 * requesting this function to ensure availability
 * 
 * @return  value, representing the version
 */
uint32_t qkd_version_9_9999_1();


/**
 * version check for 9.9999.2 
 * 
 * any compiler and linker may link against the qkd library
 * requesting this function to ensure availability
 * 
 * @return  value, representing the version
 */
uint32_t qkd_version_9_9999_2();


/**
 * version check for 9.9999.3 
 * 
 * any compiler and linker may link against the qkd library
 * requesting this function to ensure availability
 * 
 * @return  value, representing the version
 */
uint32_t qkd_version_9_9999_3();


/**
 * version check for 9.9999.4 
 * 
 * any compiler and linker may link against the qkd library
 * requesting this function to ensure availability
 * 
 * @return  value, representing the version
 */
uint32_t qkd_version_9_9999_4();


/**
 * version check for 9.9999.5 
 * 
 * any compiler and linker may link against the qkd library
 * requesting this function to ensure availability
 * 
 * @return  value, representing the version
 */
uint32_t qkd_version_9_9999_5();


/**
 * version check for 9.9999.6
 * 
 * any compiler and linker may link against the qkd library
 * requesting this function to ensure availability
 * 
 * @return  value, representing the version
 */
uint32_t qkd_version_9_9999_6();


/**
 * version check for 9.9999.7
 * 
 * any compiler and linker may link against the qkd library
 * requesting this function to ensure availability
 * 
 * @return  value, representing the version
 */
uint32_t qkd_version_9_9999_7();


/**
 * version check for 9.9999.8
 *
 * any compiler and linker may link against the qkd library
 * requesting this function to ensure availability
 *
 * @return  value, representing the version
 */
uint32_t qkd_version_9_9999_8();



namespace qkd {
   
    
/**
 * returns a version string including the git branch, commit and change
 * 
 * ... if git was located on the system
 * 
 * The returned string should look like
 * 
 *      9.9999.7 (develop: 3f86ba8615af7d05316733d58a3c472b88fe9f83 *)
 * 
 * Meaning: this is version 9.9999.7 at the branch "develop" with the
 * git commit 3f86ba8615af7d05316733d58a3c472b88fe9f83 as HEAD. The
 * final '*' indicates that the current working directory has changed.
 * 
 * @return  a string holding the current QKD version (including GIT infos)
 */
std::string version();
    
    
}


#endif


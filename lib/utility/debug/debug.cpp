/*
 * debug.cpp
 * 
 * Implements the qkd debug class
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

#include <iostream>
#include <mutex>

// ait
#include <qkd/utility/debug.h>


using namespace qkd::utility;


// ------------------------------------------------------------
// vars


/**
 * global debug flag
 */
static bool g_bDebug = false;


/**
 * debug callback
 */
void (*g_fLog)(std::string const &) = nullptr;


/**
 * sync output
 */
static std::mutex g_cMutex;


// ------------------------------------------------------------
// code


/**
 * the debug flag
 * 
 * @return  the reference to the global wide debug flag
 */
bool & debug::enabled() {
    return g_bDebug;
}


/**
 * write to log
 */
void debug::flush() {
    
    // don't to any debug if not necessary
    if ((!g_bDebug) && (!m_bForceOutput)) return;
    
    // dump
    std::string sLine = str();

    {
        // output must be synced
        std::lock_guard<std::mutex> cLock(g_cMutex);
        std::cerr << sLine << std::endl;
    }
    
    // invoke callback
    if (g_fLog) (* g_fLog)(sLine);
}


/**
 * sets the callback function for new logs
 * 
 * @param   fLog        the callback function
 */
void debug::set_callback(void (* fLog)(std::string const & )) {
    g_fLog = fLog;
}

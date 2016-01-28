/*
 * syslog.cpp
 * 
 * Implements the qkd syslog interface
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

#include <mutex>

// ait
#include <qkd/utility/debug.h>
#include <qkd/utility/syslog.h>


using namespace qkd::utility;


// ------------------------------------------------------------
// vars


/**
 * sync output
 */
static std::mutex g_cMutex;


// ------------------------------------------------------------
// code


/**
 * write to log
 */
void syslog::flush() {
    
    {
        // sync output
        std::lock_guard<std::mutex> cLock(g_cMutex);
        ::syslog(m_nPriority, "%s", str().c_str());
    }
    
    qkd::utility::debug() << str();
}


/**
 * init log
 */
void syslog::init() {
    
    // don't init twice
    static bool bInitialized = false;
    if (bInitialized) return;
    
    // open log
    openlog(nullptr, LOG_PID, LOG_USER);
    bInitialized = true;
}



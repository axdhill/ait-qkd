/*
 * syslog.h
 *
 * declare a neat syslog interface
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


#ifndef __QKD_UTILITY_SYSLOG_H_
#define __QKD_UTILITY_SYSLOG_H_


// ------------------------------------------------------------
// incs

#include <sstream>

#include <syslog.h>


// ------------------------------------------------------------
// decls


namespace qkd {
    
namespace utility {    
    

/**
 * The syslog class provides a neat interface 
 * to write to a syslog in a standard fashion.
 * 
 * One creates a syslog object dedicated to a certain 
 * log level, then stuffs in what the message is about. 
 * 
 * When the syslog object is removed, the destructor 
 * automatically flushes the logline to the system's syslog.
 * 
 * That is:
 * 
 *      ...
 *      qkd::utility::syslog::info() << "This is a INFO line " << "with " << "a number " << 3.1415;
 *      ...
 * 
 * Easy, ha?
 * 
 * Hence: if debug has been enabled (qkd::utility::debug) then all syslog messages
 * will automatically also go to the debug channel output.
 */
class syslog : public std::stringstream {

    
public:
    
    
    /**
     * copy ctor
     * 
     * @param   rhs             right hand side
     */
    syslog(syslog const & rhs) : std::basic_ios<char>(), std::basic_stringstream<char>(), m_nPriority(rhs.m_nPriority) {};
    
    
    /**
     * dtor
     */
    virtual ~syslog() { flush(); }
    
    
    /**
     * return the crit syslog stream
     * 
     * @return  the syslog instance for critical messages
     */
    static syslog crit() { init(); return syslog(LOG_CRIT); }

    
    /**
     * write to log
     */
    void flush();
    
    
    /**
     * return the info syslog stream
     * 
     * @return  the syslog instance for info messages
     */
    static syslog info() { init(); return syslog(LOG_INFO); }
    

    /**
     * return the warning syslog stream
     * 
     * @return  the syslog instance for warning messages
     */
    static syslog warning() { init(); return syslog(LOG_WARNING); }
    

private:


    /**
     * ctor
     */
    syslog() {};
    
    
    /**
     * ctor
     * 
     * @param   nPriority       priority level of the syslog object
     */
    syslog(int nPriority) : m_nPriority(nPriority) {};
    
    
    /**
     * inits the syslog subsystem
     */
    static void init();
    
    
    /**
     * syslog priority level
     */
    int m_nPriority;
    
};


}
    
}


#endif


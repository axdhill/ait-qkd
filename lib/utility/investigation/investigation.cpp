/*
 * investigation.cpp
 * 
 * investigation implementation
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

// ait
#include <qkd/utility/investigation.h>

#include "investigation_dbus.h"


using namespace qkd::utility;


// ------------------------------------------------------------
// code


/**
 * dump found result to some stream
 * 
 * @param   cStream     the stream to dump to
 */
void investigation::dump(std::ostream & cStream) const {
    
    // timestamp
    //
    // actually we need a std::put_time(localtime(&cTimeStamp)) here
    // ... but my gcc is TOOOOOO old to have this method in <iomanip>
    //      *sigh*
    //                  - Oliver
    //
    // I frickle around a "solution" in plain old rusty C ... :(
    //
    time_t cTimestamp = std::chrono::system_clock::to_time_t(m_cTimestampEnd);
    tm * cTm = localtime(&cTimestamp);
    char sTimestamp[32];
    snprintf(sTimestamp, 32, "%04d-%02d-%02d %02d:%02d:%02d", cTm->tm_year + 1900, cTm->tm_mon, cTm->tm_mday, cTm->tm_hour, cTm->tm_min, cTm->tm_sec);
    cStream << "qkd system investigataion dump of timestamp: " << sTimestamp << std::endl;
    
    // all the nodes
    cStream << "\"nodes\": {\n";
    for (auto const & cNode : m_cResult.cNodes) cNode.second.write(cStream, "\t");
    cStream << "}\n";
    
    // all the links
    cStream << "\"links\": {\n";
    for (auto const & cLink : m_cResult.cLinks) cLink.second.write(cStream, "\t");
    cStream << "}\n";
    
    // all the modules
    cStream << "\"modules\": {\n";
    for (auto const & cModule : m_cResult.cModules) cModule.second.write(cStream, "\t");
    cStream << "}\n";
    
    // all the pipelines
    cStream << "\"pipelines\": {\n";
    for (auto const & cPipeline : m_cResult.cPipelines) cPipeline.second.write(cStream, "\t");
    cStream << "}\n";
}


/**
 * inspect the system
 * 
 * @return  the investigation results (as an instance of this class)
 */
investigation investigation::investigate() {
    
    // prepare a investigation object
    investigation cInvestigation;
    
    // instantiate an investigation via DBus worker
    investigation_dbus cInvestigationDBus;
    
    // inspect system!
    cInvestigationDBus.investigate(cInvestigation.m_cResult);
    
    // capture end point
    cInvestigation.m_cTimestampEnd = std::chrono::system_clock::now();
    
    return cInvestigation;
}


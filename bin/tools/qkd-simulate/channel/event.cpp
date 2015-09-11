/*
 * event.cpp
 * 
 * Implementation of event class
 * 
 * Author: Oliver Maurhart
 *
 * Copyright (C) 2013-2015 AIT Austrian Institute of Technology
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
#include "event.h"
#include "channel_event_handler.h"


using namespace qkd::simulate;


// -----------------------------------------------------------------
// code


/**
 * string representation of this event in JSON syntax
 * 
 * @return  a string representation of this event
 */
std::string event::str() const {
    
    std::stringstream ss;
    
    ss << "{ ";
    ss << "\"id\": " << nId << ", ";
    ss << "\"priority\": \"" << event_priority_str[ePriority] << "\", ";
    ss << "\"type\": \"" << event_type_str[eType] << "\", ";
    ss << "\"destination\": \"" << (cDestination ? cDestination->get_name() : "") << "\", ";
    ss << "\"source\": \"" << (cSource ? cSource->get_name() : "") << "\", ";
    ss << "\"time\": " << nTime << ", ";
    
    ss << "\"data\": { ";
    ss << "\"photon pair id\": " << cData.m_nPhotonPairId << ", ";
    ss << "\"photon state\": \"" << photon_state_str[cData.m_ePhotonState] << "\", ";
    ss << "\"detect time\": " << cData.m_nDetectTime << "\", ";
    ss << "\"alice\": " << cData.m_bAlice << "\", ";
    ss << "\"down\": " << cData.m_bDown << "\" ";
    ss << "} ";
    
    ss << "} ";
    
    return ss.str();
}


/*
 * channel_event_handler.cpp
 * 
 * Implementation of a channel event handler
 * 
 * Autor: Philipp Grabenweger
 * Autor: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
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

#include <assert.h>

// ait
#include <qkd/utility/debug.h>

#include "channel_event_handler.h"
#include "channel_event_manager.h"


using namespace qkd::simulate;


// -----------------------------------------------------------------
// code


/**
 * function that initializes the channel event handler. 
 * 
 * @param   cParent                 the parent channel event handler
 * @param   cManager              the event manager
 * @param   cPhotonPairManager      the photon pair manager
 */
void channel_event_handler::init(channel_event_handler * cParent, channel_event_manager * cManager, photon_pair_manager * cPhotonPairManager) {
    
    assert(cManager != nullptr);
    m_cParent = cParent;
    m_cManager = cManager;
    m_cPhotonPairManager = cPhotonPairManager;
    
    m_cManager->add_event_handler(this);
}

/*
 * fiber.cpp
 *
 * implementation of a quantum fiber system
 *
 * Author: Mario Kahn
 *         Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *         Philipp Grabenweger
 *
 * Copyright (C) 2013-2016 AIT Austrian Institute of Technology
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

#include <fstream>
#include <stdexcept>

#include <math.h>

// ait
#include "fiber.h"
#include "channel_event_manager.h"


using namespace qkd::simulate;


// -----------------------------------------------------------------
// code


/** 
 * ctor
 */
fiber::fiber() {
    set_absorption_coefficient(1.0);
    set_photon_delay(0.0);
    set_sync_delay(0.0);
    set_length(1.0);
    set_loss(false);
    set_noise_photon_rate(0.0);
}


/**
 * handle a channel event. 
 * 
 * @param   cEvent      the channel event to be handled
 */
void fiber::handle(event const & cEvent) {

    switch (cEvent.eType) {
        
    case event::type::PHOTON: 
        
        if (cEvent.cSource == parent()) { 
            
            // incoming photon event (originally coming from EPR source)
            event cEventNew;
            
            cEventNew.ePriority = event::priority::NORMAL;
            cEventNew.eType = event::type::PHOTON;
            cEventNew.cDestination = &m_fiber_quantum;
            cEventNew.cSource = this;
            cEventNew.nTime = manager()->get_time();
            cEventNew.cData.m_nPhotonPairId = cEvent.cData.m_nPhotonPairId;
            
            // forward event to quantum fiber
            manager()->add_event(cEventNew); 
        }
        else 
        if (cEvent.cSource == &m_fiber_quantum) { 
            
            // photon coming out of quantum fiber
            event cEventNew;
            
            cEventNew.ePriority = event::priority::NORMAL;
            cEventNew.eType = event::type::PHOTON;
            cEventNew.cDestination = &m_delay_line;
            cEventNew.cSource = this;
            cEventNew.nTime = manager()->get_time();
            cEventNew.cData.m_nPhotonPairId = cEvent.cData.m_nPhotonPairId;
            
            // forward event to delay line
            manager()->add_event(cEventNew); 
        }
        else 
        if (cEvent.cSource == &m_noise_photon_source) { 
            
            // photon coming out of noise photon source
            event cEventNew;
            
            cEventNew.ePriority = event::priority::NORMAL;
            cEventNew.eType = event::type::PHOTON;
            cEventNew.cDestination = &m_delay_line;
            cEventNew.cSource = this;
            cEventNew.nTime = manager()->get_time();
            cEventNew.cData.m_nPhotonPairId = cEvent.cData.m_nPhotonPairId;
            
            // forward event to delay line
            manager()->add_event(cEventNew); 
        }
        else 
        if (cEvent.cSource == &m_delay_line) { 
            
            // photon coming out of delay line
            event cEventNew;
            
            cEventNew.ePriority = event::priority::NORMAL;
            cEventNew.eType = event::type::PHOTON;
            cEventNew.cDestination = parent();
            cEventNew.cSource = this;
            cEventNew.nTime = manager()->get_time();
            cEventNew.cData.m_nPhotonPairId = cEvent.cData.m_nPhotonPairId;
            
            // forward event to channel
            manager()->add_event(cEventNew); 
        }
    
        break;
        
    case event::type::SYNC_PULSE: {
        
        if (cEvent.cSource == parent()) { 
            
            // incoming sync pulse (coming from detector at Alice side)
            event cEventNew;
                
            cEventNew.ePriority = event::priority::NORMAL;
            cEventNew.eType = event::type::SYNC_PULSE;
            cEventNew.cDestination = &m_fiber_sync;
            cEventNew.cSource = this;
            cEventNew.nTime = manager()->get_time();
            
            // forward event to sync transmission fiber
            manager()->add_event(cEventNew); 
        }
        else 
        if (cEvent.cSource == &m_fiber_sync) { 
            
            // sync pulse coming out of sync transmission fiber
            event cEventNew;
                
            cEventNew.ePriority = event::priority::NORMAL;
            cEventNew.eType = event::type::SYNC_PULSE;
            cEventNew.cDestination = &m_delay_line_sync;
            cEventNew.cSource = this;
            cEventNew.nTime = manager()->get_time();
            
            // forward event to sync delay line
            manager()->add_event(cEventNew); 
        }
        else 
        if (cEvent.cSource == &m_delay_line_sync) { 
            
            // sync pulse coming out of sync delay line
            event cEventNew;
                
            cEventNew.ePriority = event::priority::NORMAL;
            cEventNew.eType = event::type::SYNC_PULSE;
            cEventNew.cDestination = parent();
            cEventNew.cSource = this;
            cEventNew.nTime = manager()->get_time();
            
            // forward event to channel
            manager()->add_event(cEventNew); 
        }
    }
    break;
    
    default:
        break;
    }
}


/**
 * function that initializes the channel event handler. 
 * 
 * @param   cParent                 the parent channel event handler
 * @param   cManager              the event manager
 * @param   cPhotonPairManager      the photon pair manager
 */
void fiber::init(channel_event_handler * cParent, channel_event_manager * cManager, photon_pair_manager * cPhotonPairManager) {

    m_delay_line.set_name("DelayLine");
    m_delay_line_sync.set_name("DelayLineSync");
    m_fiber_quantum.set_name("FiberQuantum");
    m_fiber_sync.set_name("FiberSync");
    m_noise_photon_source.set_name("NoisePhotonSource");

    channel_event_handler::init(cParent, cManager, cPhotonPairManager);
    m_delay_line.init(this, cManager, cPhotonPairManager);
    m_delay_line_sync.init(this, cManager, cPhotonPairManager);
    m_fiber_quantum.init(this, cManager, cPhotonPairManager);
    m_fiber_sync.init(this, cManager, cPhotonPairManager);
    m_noise_photon_source.init(this, cManager, cPhotonPairManager);
}

    
/**
 * set fiber absorption coefficient in [0-10 dB/km]
 *
 * @param  nAbsorptionCoefficient       the new fiber absorption coefficient in [db/km]
 */
void fiber::set_absorption_coefficient(double nAbsorptionCoefficient) throw(std::out_of_range) { 
    if (nAbsorptionCoefficient < 0.0 || nAbsorptionCoefficient > 10.0) throw std::out_of_range("fiber::set_absorption_coefficient: nAbsorptionCoefficient"); 
    m_nAbsorptionCoefficient = nAbsorptionCoefficient;
    update_absorption_coefficient();
}


/**
 * set fiber length in [0 - 500 km]
 *
 * @param  nLength          the new fiber length
 */
void fiber::set_length(double nLength) throw(std::out_of_range) { 
    if (nLength < 0.0 || nLength > 500.0) throw std::out_of_range("fiber::set_length: nLength"); 
    m_nLength = nLength;
    m_fiber_quantum.set_length(m_nLength);
}


/**
 * set fiber noise photon rate in 1/s
 *
 * @param  nNoisePhotonRate         the new fiber noise photon rate in 1/s
 */
void fiber::set_noise_photon_rate(double nNoisePhotonRate) throw(std::out_of_range) { 
    if (nNoisePhotonRate < 0.0) throw std::out_of_range("fiber::set_noise_photon_rate: nNoisePhotonRate"); 
    m_noise_photon_source.set_noise_photon_rate(nNoisePhotonRate);
}

/**
 * set the photon delay time
 * 
 * @param   nDelay      photon delay time in ns
 */
void fiber::set_photon_delay(double nDelay) throw(std::out_of_range) {
    if (nDelay < 0.0) throw std::out_of_range("fiber::set_photon_delay: nDelay");
    m_delay_line.set_delay_time(nDelay);
}


/**
 * set the sync pulse delay time
 * 
 * @param   nDelay      sync pulse delay time in ns 
 */
void fiber::set_sync_delay(double nDelay) throw(std::out_of_range) {
    if (nDelay < 0.0) throw std::out_of_range("fiber::set_sync_delay: nDelay");
    m_delay_line_sync.set_delay_time(nDelay);
}


/**
 * write out all parameters of this event handler and its child event handlers
 * 
 * @param   cStream     the stream to write to
 */
void fiber::write_parameters(std::ofstream & cStream) {

    cStream << "NAME: " << get_name() << std::endl;
    cStream << "m_nAbsorptionCoefficient: " << m_nAbsorptionCoefficient << std::endl;
    cStream << "m_nLength: " << m_nLength << std::endl;
    cStream << "m_bLoss: " << m_bLoss << std::endl;
    cStream << std::endl;
    m_delay_line.write_parameters(cStream);
    m_delay_line_sync.write_parameters(cStream);
    m_fiber_quantum.write_parameters(cStream);
    m_fiber_sync.write_parameters(cStream);
    m_noise_photon_source.write_parameters(cStream);
}

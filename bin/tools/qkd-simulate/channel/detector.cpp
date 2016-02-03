/*
 * detector.cpp
 *
 * implementation of a quantum channel detector
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

// ait
#include <qkd/utility/debug.h>

#include "channel.h"
#include "detector.h"

#define EVENT_TABLE_LEN_MAX     8192                /**< maximum size of an event table */


using namespace qkd::simulate;


// -----------------------------------------------------------------
// code


/** 
 * ctor
 * 
 * @param   cChannel    the channel of this detector
 * @param   bAlice      state if the detector is alice (or bob)
 */
detector::detector(qkd::simulate::channel * cChannel, bool bAlice) : 

    m_bAlice(bAlice),   
    m_cChannel(cChannel),
    m_cSyncPulseGenerator(nullptr),
    m_cSyncPulseReceiver(nullptr) {
            
    m_cDetectorOptics.set_alice(bAlice);
    
    if (bAlice) {
        m_cSyncPulseGenerator = new sync_pulse_generator;
    }
    else {
        m_cSyncPulseReceiver = new sync_pulse_receiver;
    }
    
    // setup default values
    set_dark_counts(false);
    set_dark_count_rate(100.0);
    set_detection_mode(detection_mode::FREE_RUNNING);
    set_down_time(10.0);
    set_efficiency(50.0);
    set_event_table_size(32);
    set_jitter(false);
    set_loss(false);
    set_loss_rate(0.0);
    set_photon_time_delay(5.0);
    set_photon_time_stnd_deviation(1.0);
    set_sync_delay(5.0);
    set_sync_stnd_deviation(1.0);
    set_time_slot_width(30.0);
            
    // we MUST have a channel
    assert(cChannel != nullptr);
}


/** 
 * dtor
 */
detector::~detector() {
    
    if (m_cSyncPulseGenerator) {
        delete m_cSyncPulseGenerator;
        m_cSyncPulseGenerator = nullptr;
    }
    if (m_cSyncPulseReceiver) {
        delete m_cSyncPulseReceiver;
        m_cSyncPulseReceiver = nullptr;
    }
}


/**
 * handle a channel event. 
 * 
 * @param   cEvent      the channel event to be handled
 */
void detector::handle(event const & cEvent) {

    switch (cEvent.eType) {
        
    case event::type::DOWN_END:  
        
        // end of some detection element's down time
        if (m_detection_mode != detection_mode::FREE_RUNNING) { 
            
            // if detector is not in free running mode, 
            // the event must be forwarded to sync pulse generator/receiver
            event cEventNew;
                
            cEventNew.ePriority = event::priority::SUPERHIGH;
            cEventNew.eType = event::type::DOWN_END;
            
            if (m_bAlice) {
                cEventNew.cDestination = m_cSyncPulseGenerator;
            }
            else {
                cEventNew.cDestination = m_cSyncPulseReceiver;
            }
            
            cEventNew.cSource = this;
            cEventNew.nTime = manager()->get_time();
            
            if (cEvent.cSource == &m_cDetectionElementH) {
                cEventNew.cData.m_ePhotonState = photon_state::HORIZONTAL;
            }
            else 
            if (cEvent.cSource == &m_cDetectionElementV) {
                cEventNew.cData.m_ePhotonState = photon_state::VERTICAL;
            }
            else 
            if (cEvent.cSource == &m_cDetectionElementP) {
                cEventNew.cData.m_ePhotonState = photon_state::PLUS;
            }
            else 
            if (cEvent.cSource == &m_cDetectionElementM) {
                cEventNew.cData.m_ePhotonState = photon_state::MINUS;
            }
            
            manager()->add_event(cEventNew);
        }
        break;
        
    case event::type::PHOTON: 
        
        if (cEvent.cSource == parent()) { 
            
            // incoming photon event
            event cEventNew;
            
            cEventNew.ePriority = event::priority::NORMAL;
            cEventNew.eType = event::type::PHOTON;
            cEventNew.cDestination = &m_cDetectorOptics;
            cEventNew.cSource = this;
            cEventNew.nTime = manager()->get_time();
            cEventNew.cData.m_nPhotonPairId = cEvent.cData.m_nPhotonPairId;
            
            // forward to detector optics
            manager()->add_event(cEventNew); 
        }
        else 
        if (cEvent.cSource == &m_cDetectorOptics) { 
            
            // photon coming out of detector optics
            event cEventNew;
            
            cEventNew.ePriority = event::priority::NORMAL;
            cEventNew.eType = event::type::PHOTON;
            
            switch (cEvent.cData.m_ePhotonState) {
                
            case photon_state::HORIZONTAL:
                cEventNew.cDestination = &m_cDetectionElementH;
                break;
                
            case photon_state::VERTICAL:
                cEventNew.cDestination = &m_cDetectionElementV;
                break;
            
            case photon_state::PLUS:
                cEventNew.cDestination = &m_cDetectionElementP;
                break;
                
            case photon_state::MINUS:
                cEventNew.cDestination = &m_cDetectionElementM;
                break;
                
            default:
                break;
            }
            
            cEventNew.cSource = this;
            cEventNew.nTime = manager()->get_time();
            
            // forward photon event to respective detection 
            // element depending on photon polarization
            manager()->add_event(cEventNew); 
        }
        break;
        
    case event::type::PULSE: 
    
        { 
        
            // electrical pulse coming out of some detection element
            event cEventNew;
                
            cEventNew.ePriority = event::priority::NORMAL;
            cEventNew.eType = event::type::DETECTOR_PULSE;
            cEventNew.cDestination = parent();
            cEventNew.cSource = this;
            cEventNew.nTime = manager()->get_time();
            cEventNew.cData.m_nDetectTime = cEvent.cData.m_nDetectTime;
            
            if (cEvent.cSource == &m_cDetectionElementH) {
                cEventNew.cData.m_ePhotonState = photon_state::HORIZONTAL;
            }
            else 
            if (cEvent.cSource == &m_cDetectionElementV) {
                cEventNew.cData.m_ePhotonState = photon_state::VERTICAL;
            }
            else 
            if (cEvent.cSource == &m_cDetectionElementP) {
                cEventNew.cData.m_ePhotonState = photon_state::PLUS;
            }
            else 
            if (cEvent.cSource == &m_cDetectionElementM) {
                cEventNew.cData.m_ePhotonState = photon_state::MINUS;
            }
            
            if (m_detection_mode == detection_mode::FREE_RUNNING) { 
                // if detector is free running
                // forward event to channel, will be passed on to ttm for logging
                manager()->add_event(cEventNew); 
            }
            else { 
                
                // if detector is not free running
                // forward event to event buffer
                cEventNew.cDestination = &m_cEventBuffer;
                manager()->add_event(cEventNew); 
                
                cEventNew.ePriority = event::priority::HIGH;
                if (m_bAlice) {
                    cEventNew.cDestination = m_cSyncPulseGenerator;
                }
                else {
                    cEventNew.cDestination = m_cSyncPulseReceiver;
                }
                
                cEventNew.cData.m_bDown = cEvent.cData.m_bDown;
                
                // forward event to sync pulse generator/receiver
                manager()->add_event(cEventNew); 
            }
        }
        break;
        
    case event::type::SYNC_PULSE: 
        
        // sync pulse coming out of sync pulse generator
        if (m_bAlice) { 
            
            event cEventNew;
            
            cEventNew.ePriority = event::priority::NORMAL;
            cEventNew.eType = event::type::SYNC_PULSE;
            cEventNew.cDestination = parent();
            cEventNew.cSource = this;
            cEventNew.nTime = manager()->get_time();
            
            // forward event to channel
            manager()->add_event(cEventNew); 
            
            cEventNew.ePriority = event::priority::HIGH;
            cEventNew.cDestination = &m_cWindowGenerator;
            // forward event to window generator
            manager()->add_event(cEventNew); 
            
        }
        else {
            
            if (cEvent.cSource == parent()) { 
                
                // sync pulse coming from transmission fiber
                event cEventNew;
                
                cEventNew.ePriority = event::priority::NORMAL;
                cEventNew.eType = event::type::SYNC_PULSE;
                cEventNew.cDestination = m_cSyncPulseReceiver;
                cEventNew.cSource = this;
                cEventNew.nTime = manager()->get_time();
                // forward event to sync pulse receiver
                manager()->add_event(cEventNew); 
                
            }
            else 
            if (cEvent.cSource == m_cSyncPulseReceiver) { 
                
                // sync pulse detected by sync pulse receiver
                event cEventNew;
                
                cEventNew.ePriority = event::priority::NORMAL;
                cEventNew.eType = event::type::SYNC_PULSE;
                cEventNew.cDestination = &m_cWindowGenerator;
                cEventNew.cSource = this;
                cEventNew.nTime = manager()->get_time();
                
                // forward event to window generator
                manager()->add_event(cEventNew); 
            }
        }
        break;
        
    case event::type::SYNC_PULSE_BAD: 
        
        if (cEvent.cSource == m_cSyncPulseReceiver) { 
            
            // bad sync pulse received by sync pulse receiver while some detection elements were down
            event cEventNew;
                    
            cEventNew.ePriority = event::priority::NORMAL;
            cEventNew.eType = event::type::SYNC_PULSE_BAD;
            cEventNew.cDestination = &m_cWindowGenerator;
            cEventNew.cSource = this;
            cEventNew.nTime = manager()->get_time();
            
            // forward event to window generator
            manager()->add_event(cEventNew); 
        }
        else 
        if (cEvent.cSource == &m_cWindowGenerator) { 
            
            // bad sync pulse event coming from window generator
            event cEventNew;
                    
            cEventNew.ePriority = event::priority::NORMAL;
            cEventNew.eType = event::type::SYNC_PULSE_BAD;
            cEventNew.cDestination = &m_cEventBuffer;
            cEventNew.cSource = this;
            cEventNew.nTime = manager()->get_time();
            
            // forward event to event buffer
            manager()->add_event(cEventNew); 
        }
        break;
        
    case event::type::WINDOW_END: 
        
        {
            // window end event coming from window generator
            event cEventNew;
            
            cEventNew.ePriority = event::priority::SUPERHIGH;
            cEventNew.eType = event::type::WINDOW_END;
            cEventNew.cDestination = &m_cEventBuffer;
            cEventNew.cSource = this;
            cEventNew.nTime = manager()->get_time();
            
            // forward event to event buffer
            manager()->add_event(cEventNew); 
            
            if (m_bAlice) { 
                // on Alice side
                cEventNew.cDestination = m_cSyncPulseGenerator;
                // forward event to window generator
                manager()->add_event(cEventNew); 
            }
            else { 
                
                // on Bob side, disable events are sent to all detection elements
                cEventNew.eType = event::type::DISABLE;
                cEventNew.cDestination = &m_cDetectionElementH;
                manager()->add_event(cEventNew);
                
                cEventNew.cDestination = &m_cDetectionElementV;
                manager()->add_event(cEventNew);
                
                cEventNew.cDestination = &m_cDetectionElementP;
                manager()->add_event(cEventNew);
                
                cEventNew.cDestination = &m_cDetectionElementM;
                manager()->add_event(cEventNew);
            }
        }
    
        break;
    
    
    case event::type::WINDOW_END_BAD: 
        
        { 
            // bad window end event coming from window generator on Bob side
            event cEventNew;  

            cEventNew.ePriority = event::priority::SUPERHIGH;
            cEventNew.eType = event::type::WINDOW_END_BAD;
            cEventNew.cDestination = &m_cEventBuffer;
            cEventNew.cSource = this;
            cEventNew.nTime = manager()->get_time();
            
            // forward event to event buffer
            manager()->add_event(cEventNew); 

            // send disable events to all detection elements
            cEventNew.eType = event::type::DISABLE;
            cEventNew.cDestination = &m_cDetectionElementH;
            manager()->add_event(cEventNew);
            
            cEventNew.cDestination = &m_cDetectionElementV;
            manager()->add_event(cEventNew);
            
            cEventNew.cDestination = &m_cDetectionElementP;
            manager()->add_event(cEventNew);
            
            cEventNew.cDestination = &m_cDetectionElementM;
            manager()->add_event(cEventNew);
        }
        break;
        
    case event::type::WINDOW_START: 
    
        { 
            // window start event coming from window generator
            event cEventNew;
            
            cEventNew.ePriority = event::priority::HIGH;
            cEventNew.eType = event::type::WINDOW_START;
            cEventNew.cDestination = &m_cEventBuffer;
            cEventNew.cSource = this;
            cEventNew.nTime = manager()->get_time();
            
            // forward event to event buffer
            manager()->add_event(cEventNew); 
            
            if (!m_bAlice) { 
                
                // on Bob side, enable events are sent to all detection elements
                cEventNew.eType = event::type::ENABLE;
                cEventNew.cDestination = &m_cDetectionElementH;
                manager()->add_event(cEventNew);
                
                cEventNew.cDestination = &m_cDetectionElementV;
                manager()->add_event(cEventNew);
                
                cEventNew.cDestination = &m_cDetectionElementP;
                manager()->add_event(cEventNew);
                
                cEventNew.cDestination = &m_cDetectionElementM;
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
void detector::init(channel_event_handler * cParent, channel_event_manager * cManager, photon_pair_manager * cPhotonPairManager) {

    m_cDetectorOptics.set_name("DetectorOptics");
    m_cDetectionElementH.set_name("DetectionElementH");
    m_cDetectionElementV.set_name("DetectionElementV");
    m_cDetectionElementP.set_name("DetectionElementP");
    m_cDetectionElementM.set_name("DetectionElementM");
    m_cEventBuffer.set_name("EventBuffer");
    
    if (m_bAlice) {
        m_cSyncPulseGenerator->set_name("SyncPulseGenerator");
    }
    else {
        m_cSyncPulseReceiver->set_name("SyncPulseReceiver");
    }
    m_cWindowGenerator.set_name("WindowGenerator");

    channel_event_handler::init(cParent, cManager, cPhotonPairManager);
    m_cDetectorOptics.init(this, cManager, cPhotonPairManager);
    m_cDetectionElementH.init(this, cManager, cPhotonPairManager);
    m_cDetectionElementV.init(this, cManager, cPhotonPairManager);
    m_cDetectionElementP.init(this, cManager, cPhotonPairManager);
    m_cDetectionElementM.init(this, cManager, cPhotonPairManager);
    m_cEventBuffer.init(this, cManager, cPhotonPairManager);
    
    if (m_bAlice) {
        m_cSyncPulseGenerator->init(this, cManager, cPhotonPairManager);
    }
    else {
        m_cSyncPulseReceiver->init(this, cManager, cPhotonPairManager);
    }
    m_cWindowGenerator.init(this, cManager, cPhotonPairManager);
}


/**
 * sets the detector dark count rate in [0 - 10 000 Hz]
 * 
 * @param   nDarkCountRate      the new detector dark count rate  in [Hz]
 */
void detector::set_dark_count_rate(double nDarkCountRate) throw(std::out_of_range) {
    if (nDarkCountRate < 0.0 || nDarkCountRate > 10000.0) throw std::out_of_range("detector::set_dark_count_rate: nDarkCountRate");
    m_nDarkCountRate = nDarkCountRate;
    update_dark_count_rate();
}


/**
 * sets the detection mode
 * 
 * @param   eDetectionMode      the detection mode
 */
void detector::set_detection_mode(detection_mode eDetectionMode) throw(std::out_of_range) {

    if ((!m_bAlice) && eDetectionMode == detection_mode::SYNC_INITIATOR_READY) throw std::out_of_range("detector::set_detection_mode: p_detection_mode");
    
    m_detection_mode = eDetectionMode;
    if (m_bAlice || eDetectionMode == detection_mode::FREE_RUNNING) {
        
        m_cDetectionElementH.set_init_enabled(true);
        m_cDetectionElementV.set_init_enabled(true);
        m_cDetectionElementP.set_init_enabled(true);
        m_cDetectionElementM.set_init_enabled(true);
    }
    else {
        
        m_cDetectionElementH.set_init_enabled(false);
        m_cDetectionElementV.set_init_enabled(false);
        m_cDetectionElementP.set_init_enabled(false);
        m_cDetectionElementM.set_init_enabled(false);
    }
    
    if (m_bAlice) {
        m_cSyncPulseGenerator->set_detection_mode(eDetectionMode);
    }
    else {
        m_cSyncPulseReceiver->set_detection_mode(eDetectionMode);
    }
}


/**
 * sets detector down time  in [0 - 10000 ns]
 * 
 * @param   nDownTime           new detector down time in [ns]
 */
void detector::set_down_time(double nDownTime) throw(std::out_of_range) {
    if (nDownTime < 0.0 || nDownTime > 10000.0) throw std::out_of_range("detector::set_down_time: nDownTime");
    m_nDownTime = nDownTime;
    update_detection_loss();
}


/**
 * sets the detection efficiency in [0 - 100%]
 * 
 * @param   nEfficiency         the new detector efficiency in [%]
 */
void detector::set_efficiency(double nEfficiency) throw(std::out_of_range) {
    if (nEfficiency < 0.0 || nEfficiency > 100.0) throw std::out_of_range("detector::set_efficiency: nEfficiency");
    m_nEfficiency = nEfficiency / 100.0; // P.G.: changed nEfficiency to nEfficiency / 100.0
    update_detection_loss();
}


/**
 * set the event table size
 * 
 * @param   nEventTableSize     the new event table size
 */
void detector::set_event_table_size(uint64_t nEventTableSize) throw(std::out_of_range) {
    if (nEventTableSize > EVENT_TABLE_LEN_MAX) throw std::out_of_range("detector::set_event_table_size: nEventTableSize");
    m_nEventTableSize = nEventTableSize; 
    m_cEventBuffer.set_buffer_size(nEventTableSize); 
}


/**
 * sets the distance independent loss [0 - 30 dB]
 * 
 * @param   nLossRate           the new loss in [dB]
 */
void detector::set_loss_rate(double nLossRate) throw(std::out_of_range) {
    if (nLossRate < 0.0 || nLossRate > 30.0) throw std::out_of_range("detector::set_loss: nLossRate");
    m_nLossRate = nLossRate;
    update_detection_loss();
}


/**
 * sets photon detection delay time in [ns]
 * 
 * @param   nPhotonTimeDelay    new photon detection time delay in [ns]
 */
void detector::set_photon_time_delay(double nPhotonTimeDelay) throw(std::out_of_range) {
    if (nPhotonTimeDelay < 0.0) throw std::out_of_range("detector::set_photon_time_delay: nPhotonTimeDelay");
    m_nPhotonTimeDelay = nPhotonTimeDelay;
    update_jitter();
}


/**
 * sets standard deviation of photon time distribution in [0 - 100 ns]
 * 
 * @param   nPhotonTimeStndDeviation    new standard deviation of photon time distribution in [ns]
 */
void detector::set_photon_time_stnd_deviation(double nPhotonTimeStndDeviation) throw(std::out_of_range) {
    if (nPhotonTimeStndDeviation < 0.0 || nPhotonTimeStndDeviation > 100.0) throw std::out_of_range("detector::set_photon_time_stnd_deviation: nPhotonTimeStndDeviation");
    m_nPhotonTimeStndDeviation = nPhotonTimeStndDeviation;
    update_jitter();
}


/**
 * sets the time slot width used as coincidence window in [0 - 1000 ns]
 *
 * @param   nTimeSlotWidth      the new time slot width [ns]
 */
void detector::set_time_slot_width(double nTimeSlotWidth) throw(std::out_of_range) {
    if (nTimeSlotWidth < 0.0 || nTimeSlotWidth > 1000.0) throw std::out_of_range("detector::set_time_slot_width: nTimeSlotWidth");
    m_nTimeSlotWidth = nTimeSlotWidth;
    m_cWindowGenerator.set_window_width(nTimeSlotWidth);
}


/**
 * update the dark count rate on each detector element
 */
void detector::update_dark_count_rate() {
    if (m_bDarkCounts) {
        m_cDetectionElementH.set_dark_count_rate(m_nDarkCountRate);
        m_cDetectionElementV.set_dark_count_rate(m_nDarkCountRate);
        m_cDetectionElementP.set_dark_count_rate(m_nDarkCountRate);
        m_cDetectionElementM.set_dark_count_rate(m_nDarkCountRate);
    }
    else {
        m_cDetectionElementH.set_dark_count_rate(0.0);
        m_cDetectionElementV.set_dark_count_rate(0.0);
        m_cDetectionElementP.set_dark_count_rate(0.0);
        m_cDetectionElementM.set_dark_count_rate(0.0);
    }
}


/**
 * update efficiency and detector down time on each detector element
 */
void detector::update_detection_loss() {
    
    if (m_bLoss) {
        m_cDetectorOptics.set_loss(m_nLossRate);
        m_cDetectorOptics.set_efficiency(m_nEfficiency);
        m_cDetectionElementH.set_down_time(m_nDownTime);
        m_cDetectionElementV.set_down_time(m_nDownTime);
        m_cDetectionElementP.set_down_time(m_nDownTime);
        m_cDetectionElementM.set_down_time(m_nDownTime);
    }
    else {
        m_cDetectorOptics.set_loss(0.0);
        m_cDetectorOptics.set_efficiency(1.0);
        m_cDetectionElementH.set_down_time(0.0);
        m_cDetectionElementV.set_down_time(0.0);
        m_cDetectionElementP.set_down_time(0.0);
        m_cDetectionElementM.set_down_time(0.0);
    }
}


/**
 * apply time delay and deviation on each detector element
 */
void detector::update_jitter() {
    
    if (m_bJitter) {
        m_cDetectionElementH.set_delay(m_nPhotonTimeDelay);
        m_cDetectionElementV.set_delay(m_nPhotonTimeDelay);
        m_cDetectionElementP.set_delay(m_nPhotonTimeDelay);
        m_cDetectionElementM.set_delay(m_nPhotonTimeDelay);
        
        m_cDetectionElementH.set_jitter(m_nPhotonTimeStndDeviation);
        m_cDetectionElementV.set_jitter(m_nPhotonTimeStndDeviation);
        m_cDetectionElementP.set_jitter(m_nPhotonTimeStndDeviation);
        m_cDetectionElementM.set_jitter(m_nPhotonTimeStndDeviation);
    }
    else {
        m_cDetectionElementH.set_delay(0.0);
        m_cDetectionElementV.set_delay(0.0);
        m_cDetectionElementP.set_delay(0.0);
        m_cDetectionElementM.set_delay(0.0);
        
        m_cDetectionElementH.set_jitter(0.0);
        m_cDetectionElementV.set_jitter(0.0);
        m_cDetectionElementP.set_jitter(0.0);
        m_cDetectionElementM.set_jitter(0.0);
    }
}


/**
 * write out all parameters of this event handler and its child event handlers
 * 
 * @param   cStream     the stream to write to
 */
void detector::write_parameters(std::ofstream & cStream) {

    cStream << "NAME: " << get_name() << std::endl;
    cStream << "m_bAlice: " << m_bAlice << std::endl;
    cStream << "m_bDarkCounts: " << m_bDarkCounts << std::endl;
    cStream << "m_nDarkCountRate: " << m_nDarkCountRate << std::endl;
    cStream << "m_nDownTime: " << m_nDownTime << std::endl;
    cStream << "m_nEfficiency: " << m_nEfficiency << std::endl;
    cStream << "m_nEventTableSize: " << m_nEventTableSize << std::endl;
    cStream << "m_bJitter: " << m_bJitter << std::endl;
    cStream << "m_bLoss: " << m_bLoss << std::endl;
    cStream << "m_nLossRate: " << m_nLossRate << std::endl;
    cStream << "m_nPhotonTimeDelay: " << m_nPhotonTimeDelay << std::endl;
    cStream << "m_nPhotonTimeStndDeviation: " << m_nPhotonTimeStndDeviation << std::endl;
    cStream << "m_nTimeSlotWidth: " << m_nTimeSlotWidth << std::endl;
    cStream << "m_detection_mode: " << static_cast<int>(m_detection_mode) << std::endl;
    cStream << std::endl;
    
    m_cDetectorOptics.write_parameters(cStream);
    m_cDetectionElementH.write_parameters(cStream);
    m_cDetectionElementV.write_parameters(cStream);
    m_cDetectionElementP.write_parameters(cStream);
    m_cDetectionElementM.write_parameters(cStream); 
    if (m_bAlice) {
        m_cSyncPulseGenerator->write_parameters(cStream);
    }
    else {
        m_cSyncPulseReceiver->write_parameters(cStream);
    } 
    m_cWindowGenerator.write_parameters(cStream);
    m_cEventBuffer.write_parameters(cStream);
}



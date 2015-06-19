/*
 * channel.cpp
 *
 * implementation of a quantum channel
 *
 * Autor: Mario Kahn
 * Autor: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 * Autor: Philipp Grabenweger
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

#include "config.h"

#include <iostream>

#include <pthread.h>
#include <signal.h>

// 0MQ
#include <zmq.h>

// ait
#include "channel.h"
#include "measurement_bb84.h"
#include "detector/detection_modes.h"


using namespace qkd::simulate;


// -----------------------------------------------------------------
// code


/** 
 * ctor
 */
channel::channel() : m_cPipeAlice(nullptr), m_cPipeBob(nullptr) {
    
    m_cDetectorAlice = new detector(this, true);
    m_cDetectorBob = new detector(this, false);
    
    // Default values for channel models
    set_sync_stnd_deviation(1.0);
    set_sim_end_time(1000.0);
    set_timeslot_center_shift(0.0);
    m_bDetectorThreadRun = false;
    
    m_cPhotonPairManager.set_manager(&m_cManager);
    init(nullptr, &m_cManager, &m_cPhotonPairManager);
    
    // setup the 0MQ context
    m_cZMQContext = zmq_ctx_new();
    assert(m_cZMQContext != nullptr);
}


/** 
 * dtor
 */
channel::~channel() {
    
    // interrupt detector thread in the case it is running
    interrupt_thread();
    
    if (m_cDetectorAlice) {
        delete m_cDetectorAlice;
        m_cDetectorAlice = nullptr;
    }
    if (m_cDetectorBob) {
        delete m_cDetectorBob;
        m_cDetectorBob = nullptr;
    }
    
    if (m_cPipeAlice ) {
        zmq_close(m_cPipeAlice);
        m_cPipeAlice  = nullptr;
    }
    if (m_cPipeBob ) {
        zmq_close(m_cPipeBob);
        m_cPipeBob  = nullptr;
    }
    if (m_cZMQContext) {
        zmq_ctx_term(m_cZMQContext);    
        m_cZMQContext = nullptr;
    }
}


/**
 * remove output files
 */
void channel::delete_files() {
    
    // enfore deleting of output files
    unlink(get_file_alice().c_str());
    unlink(get_file_bob().c_str());
}


/**
 * detector thread method
 */
void channel::detector_thread() {
    
    m_nRound = 0;
    do {
        
        // do a measurment
        measurement cMeasurement = measure();
        measurement_bb84 * cMeasurementBB84 = dynamic_cast<qkd::simulate::measurement_bb84 *>(cMeasurement.get());
        if (!cMeasurementBB84->free_running()) flush_measurment(cMeasurement);
        
        // yet another round
        m_nRound++;
        
    } while (is_looping() && is_simulation_running());
    
    m_bDetectorThreadRun = false;
}


/**
 * this writes the event tables to the targets
 * 
 * @param   cMeasurement    the measurement made
 */
void channel::flush_measurment(qkd::simulate::measurement const & cMeasurement) {
    
    // only proceed if we are entitled to run
    if (!is_simulation_running()) return;
    
    
    // pipe?
    if (is_piping()) {
        
        // pipe: alice
        if (!m_cPipeAlice) return;
        
        qkd::utility::buffer cBufferAlice;
        cBufferAlice << cMeasurement->key_alice();

        // send
        int nSentAlice = zmq_send(m_cPipeAlice, cBufferAlice.get(), cBufferAlice.size(), 0);
        if (nSentAlice == -1) {
            std::stringstream ss;
            ss << "failed to send key to alice: " << strerror(zmq_errno());
            throw std::runtime_error(ss.str());
        }
            
        // pipe: bob
        if (!m_cPipeBob) return;
        
        qkd::utility::buffer cBufferBob;
        cBufferBob << cMeasurement->key_bob();

        // send
        int nSentBob = zmq_send(m_cPipeBob, cBufferBob.get(), cBufferBob.size(), 0);
        if (nSentBob == -1) {
            std::stringstream ss;
            ss << "failed to send key to bob: " << strerror(zmq_errno());
            throw std::runtime_error(ss.str());
        }
        
    }
    else {
        
        // file: alice
        std::ofstream cFileAlice(get_file_alice(), std::ios_base::out | std::ios_base::app);
        if (cFileAlice.good()) cFileAlice << cMeasurement->key_alice();
        cFileAlice.close();
        
        // file: bob
        std::ofstream cFileBob(get_file_bob(), std::ios_base::out | std::ios_base::app);
        if (cFileBob.good()) cFileBob << cMeasurement->key_alice();
        cFileBob.close();
    }
}


/**
 * handle a channel event. 
 * 
 * @param   cEvent          the channel event to be handled
 */
void channel::handle(event const & cEvent) {
    
    switch (cEvent.eType) {
        
    case event::type::PHOTON: 
        
        if (cEvent.cSource == &m_cSource) { 
            
            // photon generation event triggered by EPR photon source
            
            event cEventNew;
            
            cEventNew.ePriority = event::priority::NORMAL;
            cEventNew.eType = event::type::PHOTON;
            cEventNew.cDestination = m_cDetectorAlice;
            cEventNew.cSource = this;
            cEventNew.nTime = manager()->get_time();
            cEventNew.cData.m_nPhotonPairId = cEvent.cData.m_nPhotonPairId;
            
            // forward event to detector at Alice side
            manager()->add_event(cEventNew); 
            
            cEventNew.ePriority = event::priority::SUBNORMAL;
            cEventNew.cDestination = &m_cFiber;
            
            // forward event to fiber
            manager()->add_event(cEventNew); 
            
        }
        else 
        if (cEvent.cSource == &m_cFiber) { 
            
            // photon coming out of fiber
            event cEventNew;
            
            cEventNew.ePriority = event::priority::NORMAL;
            cEventNew.eType = event::type::PHOTON;
            cEventNew.cDestination = m_cDetectorBob;
            cEventNew.cSource = this;
            cEventNew.nTime = manager()->get_time();
            cEventNew.cData.m_nPhotonPairId = cEvent.cData.m_nPhotonPairId;
            
            // forward event to detector at Bob side
            manager()->add_event(cEventNew); 
        }
        break;
    
    case event::type::DETECTOR_PULSE:  
        
        {
            // detector pulse event coming from detector at Alice/Bob sides
            event cEventNew;
                
            cEventNew.ePriority = event::priority::NORMAL;
            cEventNew.eType = event::type::DETECTOR_PULSE;
            cEventNew.cDestination = &m_cTTM;
            cEventNew.cSource = this;
            cEventNew.nTime = manager()->get_time();
            cEventNew.cData.m_nDetectTime = cEvent.cData.m_nDetectTime;
            cEventNew.cData.m_ePhotonState = cEvent.cData.m_ePhotonState;
            cEventNew.cData.m_bAlice = (cEvent.cSource == m_cDetectorAlice);
            
            // forward event to TTM module
            manager()->add_event(cEventNew); 
        }
        break;
    
    case event::type::SYNC_PULSE: 
        
        if (cEvent.cSource == m_cDetectorAlice) { 
            
            // sync pulse coming from detector at Alice side
            event cEventNew;
                
            cEventNew.ePriority = event::priority::NORMAL;
            cEventNew.eType = event::type::SYNC_PULSE;
            cEventNew.cDestination = &m_cFiber;
            cEventNew.cSource = this;
            cEventNew.nTime = manager()->get_time();
            
            // forward event to fiber
            manager()->add_event(cEventNew); 
        }
        else 
        if (cEvent.cSource == &m_cFiber) { 
            
            // sync pulse coming out of fiber after transmission
            event cEventNew;
                
            cEventNew.ePriority = event::priority::NORMAL;
            cEventNew.eType = event::type::SYNC_PULSE;
            cEventNew.cDestination = m_cDetectorBob;
            cEventNew.cSource = this;
            cEventNew.nTime = manager()->get_time();
            
            // forward event to detector at Bob side
            manager()->add_event(cEventNew); 
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
void channel::init(channel_event_handler * cParent, channel_event_manager * cManager, photon_pair_manager * cPhotonPairManager) {
    
    set_name("Channel");
    m_cSource.set_name("Source");
    m_cFiber.set_name("Fiber");
    m_cDetectorAlice->set_name("DetectorAlice");
    m_cDetectorBob->set_name("DetectorBob");
    m_cTTM.set_name("TTM");

    channel_event_handler::init(cParent, cManager, cPhotonPairManager);   
    m_cSource.init(this, cManager, cPhotonPairManager);
    m_cFiber.init(this, cManager, cPhotonPairManager);
    m_cDetectorAlice->init(this, cManager, cPhotonPairManager);
    m_cDetectorBob->init(this, cManager, cPhotonPairManager);
    m_cTTM.init(this, cManager, cPhotonPairManager);
}


/**
 * interrupt detector thread
 */
void channel::interrupt_thread() {
    
    // do we have a detector thread at all? 
    if (m_cDetectorThread.get_id() == std::thread::id()) return;
    
    // interrupt detector thread
    m_bDetectorThreadRun = false;
    pthread_kill(m_cDetectorThread.native_handle(), SIGCHLD);
    
    // join detector thread
    m_cDetectorThread.join();
}


/**
 * start the detector thread
 */
void channel::launch_detector_thread() {
    
    // interrupt detector thread in the case it is running
    interrupt_thread();
    
    // launch thread
    m_bDetectorThreadRun = true;
    m_cDetectorThread = std::thread([this](){ detector_thread(); });
}


/**
 * perform a measurement
 * 
 * @return  the measurment
 */
measurement channel::measure() {
    
    using namespace std::chrono;
    
    high_resolution_clock::time_point cCurrent;
    high_resolution_clock::time_point cNext;
    
    // create coincidences
    high_resolution_clock::time_point cStart = high_resolution_clock::now();
    measurement cMeasurement = measure_internal();
    high_resolution_clock::time_point cEnd = cStart + duration_cast<high_resolution_clock::duration>(nanoseconds((uint64_t)cMeasurement->acquisition_duration()));
    
    // wait until simulated time has passed
    while (m_bDetectorThreadRun) {
        
        cCurrent = std::chrono::high_resolution_clock::now();
        cNext = cCurrent + duration_cast<high_resolution_clock::duration>(milliseconds(100));
        
        if (cNext < cEnd) {
            std::this_thread::sleep_until(cNext);
        }
        else {
            std::this_thread::sleep_until(cEnd);
            break;
        }
    }
    
    return cMeasurement;
}


/**
 * sets a pipe out 
 * 
 * @param   cSocket         the socket to set
 * @param   sPipe           the new pipe out url
 */
void channel::set_pipe(void * & cSocket, std::string const & sPipe) {

    if (cSocket) {
        zmq_close(cSocket);
        cSocket = nullptr;
    }
    
    cSocket = zmq_socket(m_cZMQContext, ZMQ_PUSH);

    int nHighWaterMark = 1000;
    if (zmq_setsockopt(cSocket, ZMQ_SNDHWM, &nHighWaterMark, sizeof(nHighWaterMark)) == -1) {
        std::stringstream ss;
        ss << "failed to set high water mark on socket: " << strerror(zmq_errno());
        throw std::runtime_error(ss.str());
    }
            
    int nTimeoutPipe = -1;
    if (zmq_setsockopt(cSocket, ZMQ_SNDTIMEO, &nTimeoutPipe, sizeof(nTimeoutPipe)) == -1) {
        std::stringstream ss;
        ss << "failed to set timeout on socket: " << strerror(zmq_errno());
        throw std::runtime_error(ss.str());
    }
            
    int nLinger = 0;
    if (zmq_setsockopt(cSocket, ZMQ_LINGER, &nLinger, sizeof(nLinger)) == -1) {
        std::stringstream ss;
        ss << "failed to set linger on socket: " << strerror(zmq_errno());
        throw std::runtime_error(ss.str());
    }

    zmq_connect(cSocket, sPipe.c_str());
}


/**
 * sets the pipe out url for alice
 * 
 * @param   sPipe           the new pipe out url
 */
void channel::set_pipe_alice(std::string const & sPipe) {
    set_pipe(m_cPipeAlice, sPipe);
    
}


/**
 * sets the pipe out url for bob
 * 
 * @param   sPipe           the new pipe out url
 */
void channel::set_pipe_bob(std::string const & sPipe) {
    set_pipe(m_cPipeBob, sPipe);
}


/**
 * set standard deviation for sync signal in [0 - 100 ns]
 *  
 * @param   nStndDeviation          the new standard deviation for sync signal in [ns]
 */
void channel::set_sync_stnd_deviation(double nStndDeviation) throw(std::out_of_range) { 
    
    if (nStndDeviation >= 0.0 && nStndDeviation <= 100.0) {
        
        m_nStndSyncDeviation = nStndDeviation;
        m_cDetectorBob->set_sync_stnd_deviation(nStndDeviation);
        m_cDetectorBob->set_sync_delay(5.0 * nStndDeviation);
        update_delay_times();
    }
    else throw std::out_of_range("channel_bb84::set_sync_stnd_deviation: nStndDeviation");
}


/**
 * function to update quantum/sync fiber delay times
 */
void channel::update_delay_times() {
    
    if (m_cDetectorBob->get_detection_mode() == detection_mode::FREE_RUNNING) {
        
        m_cFiber.set_photon_delay(0.0);
        m_cFiber.set_sync_delay(0.0);
    }
    else {
        
        double delay = 5.0 * m_nStndSyncDeviation + 0.5 * m_cDetectorBob->time_slot_width() + m_nTimeslotCenterShift;
        if (delay >= 0.0) {
            m_cFiber.set_photon_delay(delay);
            m_cFiber.set_sync_delay(0.0);
        }
        else {
            m_cFiber.set_photon_delay(0.0);
            m_cFiber.set_sync_delay(-delay);
        }
    }
}


/**
 * write out all parameters of this event handler and its child event handlers
 * 
 * @param   cStream     the stream to write to
 */
void channel::write_parameters(std::ofstream & cStream) {
    
    cStream << "NAME: " << get_name() << std::endl;
    cStream << "m_nStndSyncDeviation: " << m_nStndSyncDeviation << std::endl;
    cStream << "m_nTimeslotCenterShift: " << m_nTimeslotCenterShift << std::endl;
    cStream << std::endl;
    
    m_cSource.write_parameters(cStream);
    m_cFiber.write_parameters(cStream);
    m_cDetectorAlice->write_parameters(cStream);
    m_cDetectorBob->write_parameters(cStream);
}

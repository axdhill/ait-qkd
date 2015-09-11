/*
 * channel.h
 * 
 * definition of a quantum channel
 *
 * Author: Mario Kahn
 *         Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *         Philipp Grabenweger
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

 
#ifndef __QKD_QKD_SIMULATE_CHANNEL_H_
#define __QKD_QKD_SIMULATE_CHANNEL_H_


// ------------------------------------------------------------
// incs

#include <atomic>
#include <fstream>
#include <thread>

// ait
#include "detector.h"
#include "event.h"
#include "fiber.h"
#include "measurement.h"
#include "source.h"

#include "channel_event_manager.h"
#include "photon_pair_manager.h"
#include "ttm.h"


// -----------------------------------------------------------------
// decl


namespace qkd {
    
namespace simulate {

    
/**
 * an abstract quantum channel
 * 
 * channel is the abstract interface for the optical 
 * quantum channel and simulates raw key generation 
 * based on entangled photons
 * 
 * a channel is the container object which contains:
 * 
 *  - a source
 *  - two detectors
 *  - a fiber
 *  - a TTM
 */
class channel : public channel_event_handler {
    
    
public:
    

    /** 
     * ctor
     */
    explicit channel();
    
    
    /**
     * copy ctor
     */
    channel(channel const & rhs) = delete;

    
    /** 
     * dtor
     */
    virtual ~channel();

    
    /**
     * get the alice detector
     * 
     * @return  alice detector
     */
    detector * & alice() { return m_cDetectorAlice; };
    
    
    /**
     * get the alice detector
     * 
     * @return  alice detector
     */
    detector const * alice() const { return m_cDetectorAlice; };
    
    
    /**
     * get the bob detector
     * 
     * @return  bob detector
     */
    detector * & bob() { return m_cDetectorBob; };
    
    
    /**
     * get the bob detector
     * 
     * @return  bob detector
     */
    detector const * bob() const { return m_cDetectorBob; };
    
    
    /**
     * Convert power from dB to values
     * 
     * @param   x       dB10 value
     * @return  resulting value
     */
    static double db10(double x) { return pow(10.0, x / 10.0); };


    /**
     * Convert power from dB to values
     * 
     * @param   x       dB20 value
     * @return  resulting value
     */
    static double db20(double x) { return pow(10.0, x / 20.0); };
    
    
    /**
     * remove output files
     */
    void delete_files();
    
    
    /**
     * gets the file out of alice
     * 
     * @return  where the alice output is written to
     */
    std::string const & get_file_alice() const { return m_sFileNameAlice; };
    
    
    /**
     * gets the file out of bob
     * 
     * @return  where the bob output is written to
     */
    std::string const & get_file_bob() const { return m_sFileNameBob; };
    
    
    /**
     * get the transmission medium
     * 
     * @return  the transmission medium
     */
    qkd::simulate::fiber & fiber() { return m_cFiber; };

    
    /**
     * get the transmission medium
     * 
     * @return  the transmission medium
     */
    qkd::simulate::fiber const & fiber() const { return m_cFiber; };
    

    /**
     * handle a channel event. 
     * 
     * @param   cEvent      the channel event to be handled
     */
    void handle(event const & cEvent);

    
    /**
     * function that initializes the channel event handler. 
     * 
     * @param   cParent                 the parent channel event handler
     * @param   cManager              the event manager
     * @param   cPhotonPairManager      the photon pair manager
     */
    void init(channel_event_handler * cParent, channel_event_manager * cManager, photon_pair_manager * cPhotonPairManager);  
    
    
    /**
     * interrupt detector thread
     */
    void interrupt_thread();
    
    
    /**
     * check if teh simulatioj is looping
     * 
     * @return  true if the simultion is run infinite
     */
    bool is_looping() const { return m_bLoop; };
    
    
    /**
     * check if sync pulse events are piped
     * 
     * @return  true if the key stream is forward into a pipe
     */
    bool is_piping() const { return m_bPipe; };
    

    /**
     * check if we do have a running simulation
     * 
     * @return  true, if a simulation is currently onging
     */
    bool is_simulation_running() const { return m_bDetectorThreadRun; };
    
    
    /**
     * start the detector thread
     */
    void launch_detector_thread();
    
    
    /**
     * perform a measurement
     * 
     * @return  the measurment
     */
    measurement measure();
    
    
    /**
     * get current round number
     * 
     * @return  current simulation round number
     */
    uint64_t round() const { return m_nRound; };
    
    
    /**
     * sets the file out url for alice
     * 
     * @param   sFile           the filename to write to
     */
    void set_file_alice(std::string const & sFile) { m_sFileNameAlice = sFile; };
    
    
    /**
     * sets the file out url for bob
     * 
     * @param   sFile           the filename to write to
     */
    void set_file_bob(std::string const & sFile)  { m_sFileNameBob = sFile; };
    
    
    /**
     * set inifinit simulation loop
     * 
     * @param   bLoop       new inifinite simulation loop flag
     */
    void set_looping(bool bLoop) { m_bLoop = bLoop; };
    
    
    /**
     * sets the pipe out url for alice
     * 
     * @param   sPipe           the new pipe out url
     */
    void set_pipe_alice(std::string const & sPipe);
    
    
    /**
     * sets the pipe out url for bob
     * 
     * @param   sPipe           the new pipe out url
     */
    void set_pipe_bob(std::string const & sPipe);
    
    
    /**
     * set new piping flag for sync pulse events
     * 
     * @param   bPipe           new piping flag
     */
    void set_piping(bool bPipe) { m_bPipe = bPipe; };
    
    
    /**
     * set the simulation end time
     * 
     * @param   nSimEndTime     the simulation end time in [us]
     */
    void set_sim_end_time(double nSimEndTime) throw(std::out_of_range) { 
        if (nSimEndTime >= 0.0) m_cManager.set_sim_end_time(static_cast<int64_t>(nSimEndTime / (1e6 * ttm::RESOLUTION)));
        else throw std::out_of_range("channel_bb84::set_sim_end_time: nSimEndTime");
    }
    
    
    /**
     * set standard deviation for sync signal in [0 - 100 ns]
     *  
     * @param   nStndDeviation          the new standard deviation for sync signal in [ns]
     */
    void set_sync_stnd_deviation(double nStndDeviation) throw(std::out_of_range);
    
    
    /**
     * set the timeslot center shift
     * 
     * @param   nTimeslotCenterShift        timeslot center shift in ns
     */
    void set_timeslot_center_shift(double nTimeslotCenterShift ) throw (std::out_of_range) {
        m_nTimeslotCenterShift = nTimeslotCenterShift;
        update_delay_times();
    }
    
    
    /**
     * get the simulation end time
     * 
     * @return  the simulation end time in [us]
     */
    double sim_end_time() { return static_cast<double>(m_cManager.get_sim_end_time()) * (1e6 * ttm::RESOLUTION); }
    

    /**
     * get the photon source
     * 
     * @return  the photon source
     */
    qkd::simulate::source & source() { return m_cSource; };
    
    
    /**
     * get the photon source
     * 
     * @return  the photon source
     */
    qkd::simulate::source const & source() const { return m_cSource; };
    

    /**
     * get standard deviation for sync signal in [0 - 100 ns]
     *
     * @return  standard deviation for sync signal
     */
    double sync_stnd_deviation() const { return m_nStndSyncDeviation; }

    
    /**
     * get the timeslot center shift
     * 
     * @return timeslot center shift in ns
     */
    double timeslot_center_shift() const { return m_nTimeslotCenterShift; }
    
    
    /**
     * get the ttm
     * 
     * @return  the ttm
     */
    qkd::simulate::ttm & ttm() { return m_cTTM; };
    
    
    /**
     * get the ttm
     * 
     * @return  the ttm
     */
    qkd::simulate::ttm const & ttm() const { return m_cTTM; };
    
    
    /**
     * function to update quantum/sync fiber delay times
     */
    void update_delay_times();
    

    /**
     * write out all parameters of this event handler and its child event handlers
     * 
     * @param   cStream     the stream to write to
     */
    void write_parameters(std::ofstream & cStream);

    
protected:
    
    
    /**
     * the channel event manager
     */
    channel_event_manager m_cManager;
    
    
    /**
     * the photon pair manager
     */
    photon_pair_manager m_cPhotonPairManager;
    
    
private:
    
    
    /**
     * detector thread method
     */
    void detector_thread();   
    
    
    /**
     * this writes the event tables to the targets
     * 
     * @param   cMeasurement     the measurement made
     */
    void flush_measurment(qkd::simulate::measurement const & cMeasurement);
    
    
    /**
     * sets a pipe out 
     * 
     * @param   cSocket         the socket to set
     * @param   sPipe           the new pipe out url
     */
    void set_pipe(void * & cSocket, std::string const & sPipe);


    /**
     * perform a measurement 
     * to be overwritten by derived classes
     * 
     * @return  the measurment
     */
    virtual measurement measure_internal() = 0;
    
    
    /**
     * thread running flag 
     */        
    std::atomic<bool> m_bDetectorThreadRun;
    
    
    /**
     * Standard deviation for the gaussian sync signal  stored in unit [ns], range [0-100 ns]
     */
    double m_nStndSyncDeviation;


    /**
     * alice detector
     */
    detector * m_cDetectorAlice;
    

    /**
     * bob detector
     */
    detector * m_cDetectorBob;
    
    
    /**
     * the transmission medium
     */
    qkd::simulate::fiber m_cFiber;
    
    
    /**
     * output stream for alice file
     */
    std::ofstream m_cFileAlice;
    
    
    /**
     * output filename for alice
     */
    std::string m_sFileNameAlice;
    
    
    /**
     * output stream for bob file
     */
    std::ofstream m_cFileBob;
    
    
    /**
     * output filename for bob
     */
    std::string m_sFileNameBob;
    
    
    /**
     * infinite simulation loop flag
     */
    std::atomic<bool> m_bLoop;
    
    
    /**
     * push sync pulse events to pipe flag
     */
    bool m_bPipe;
    
    
    /**
     * outgoing 0MQ socket of the pipe for Alice
     */
    void * m_cPipeAlice;
    
    
    /**
     * outgoing 0MQ socket of the pipe for Bob
     */
    void * m_cPipeBob;
    
    
    /**
     * current simulation round number
     */
    std::atomic<uint64_t> m_nRound;
    
    
    /**
     * quantum source
     */
    qkd::simulate::source m_cSource;
    
    
    /**
     * detector thread object 
     */
    std::thread m_cDetectorThread;
    
    
    /**
     * our ZMQ context used
     */
    void * m_cZMQContext;
    
    
    /**
     * timeslot center shift in [ns]
     */
    double m_nTimeslotCenterShift;
    
    
    /**
     * the TTM module
     */
    qkd::simulate::ttm m_cTTM;
};


}
}

#endif

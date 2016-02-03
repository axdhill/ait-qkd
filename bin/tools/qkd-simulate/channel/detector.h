/*
 * detector.h
 * 
 * definition of a quantum channel detector
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

 
#ifndef __QKD_QKD_SIMULATE_DETECTOR_H_
#define __QKD_QKD_SIMULATE_DETECTOR_H_


// ------------------------------------------------------------
// incs

#include <stdexcept>

// ait
#include "channel_event_handler.h"
#include "detector/detector_optics.h"
#include "detector/detection_element.h"
#include "detector/detection_modes.h"
#include "detector/event_buffer.h"
#include "detector/sync_pulse_generator.h"
#include "detector/sync_pulse_receiver.h"
#include "detector/window_generator.h"


// -----------------------------------------------------------------
// decl


// fwd
namespace zmq { class context_t; class socket_t; }


namespace qkd {
    
namespace simulate {
    
    
// fwd
class channel;


/**
 * an abstract quantum channel detector: detect photons
 */
class detector : public channel_event_handler {
    

public:
    

    /** 
     * ctor
     * 
     * @param   cChannel    the channel of this detector
     * @param   bAlice      state if the detector is alice (or bob)
     */
    explicit detector(qkd::simulate::channel * cChannel, bool bAlice);

    
    /** 
     * copy ctor
     * 
     * @param   rhs         right hand side
     */
    detector(detector const & rhs) = delete;

    
    /** 
     * dtor
     */
    virtual ~detector();
    
    
    /**
     * get the channel used
     * 
     * @return  the channel
     */
    qkd::simulate::channel * channel() { return m_cChannel; }
    
    
    /**
     * get the dark counts enabled flag
     *
     * @return  the detector dark count enabled flags
     */
    bool dark_counts() const { return m_bDarkCounts; };
    
    
    /**
     * get the detector dark count rate  in [0 - 10 000 Hz]
     *
     * @return  the detector dark count rate in [Hz]
     */
    double dark_count_rate() const { return m_nDarkCountRate; };
    
    
    /**
     * get the detector down time in [0 - 10000 ns]
     *
     * @return  detector down time in [ns]
     */
    double down_time() const { return m_nDownTime; };


    /**
     * get the detection efficiency in [0 - 100%]
     *
     * @return  detection efficiency in [%]
     */
    double efficiency() const { return m_nEfficiency * 100.0; };
    
    
    /**
     * get the event table size
     * 
     * @return  the event table size in bytes
     */
    uint64_t event_table_size() const { return m_nEventTableSize; };


    /**
     * get the event buffer
     * 
     * @return  the event buffer
     */
    unsigned char * get_buffer() { return m_cEventBuffer.get_buffer(); }
    
    
    /**
     * get the detection mode
     *
     * @return  detection mode
     */
    detection_mode get_detection_mode() const { return m_detection_mode; }    
    
    
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
     * check if we are alice
     * 
     * the alice detector is identified with 
     * the name been "alice" (lowercase)
     * 
     * @return  true, if this is the alice detector
     */
    bool is_alice() const { return m_bAlice; };
    
    
    /**
     * test if event buffer is full
     * 
     * @return  true if event buffer is full, false otherwise
     */
    bool is_buffer_full() { return m_cEventBuffer.is_buffer_full(); }
    
    
    /**
     * check if jitter is enabled
     * 
     * @return  jitter flag
     */
    bool jitter() const { return m_bJitter; };
     
     
    /**
     * check if loss is enabled
     * 
     * @return  loss flag
     */
    bool loss() const { return m_bLoss; };
     
     
    /**
     * get the distance independent loss in [0 - 30 dB]
     * 
     * @return  the distance independent loss in in [0 - 30 dB]
     */
    double loss_rate() const { return m_nLossRate; };
     
     
    /**
     * return the name of this detector
     * 
     * @return  the name of this detector
     */
    std::string name() const { if (is_alice()) return "alice"; else return "bob"; };
    
    
    /**
     * get the photon detection time delay in [ns]
     *
     * @return  photon detection delay time in [ns]
     */
    double photon_time_delay() const { return m_nPhotonTimeDelay; };
    
    
    /**
     * get the standard deviation of the photon detection time in [0 - 100 ns]
     *
     * @return  standard deviation of photon distribution in [ns]
     */
    double photon_time_stnd_deviation() const { return m_nPhotonTimeStndDeviation; };
    

    /**
     * set the dark counts enabled flag
     *
     * @param   bDarkCounts         the new dark counts enabled flags
     */
    void set_dark_counts(bool bDarkCounts) { m_bDarkCounts = bDarkCounts; update_dark_count_rate(); };
    
    
    /**
     * sets the detector dark count rate in [0 - 10 000 Hz]
     * 
     * @param   nDarkCountRate      the new detector dark count rate  in [Hz]
     */
    void set_dark_count_rate(double nDarkCountRate) throw(std::out_of_range);

    
    /**
     * sets the detection mode
     * 
     * @param   eDetectionMode      the detection mode
     */
    void set_detection_mode(detection_mode eDetectionMode) throw(std::out_of_range);
    
    
    /**
     * sets detector down time  in [0 - 10000 ns]
     * 
     * @param   nDownTime           new detector down time in [ns]
     */
    void set_down_time(double nDownTime) throw(std::out_of_range);

    
    /**
     * sets the detection efficiency in [0 - 100 %]
     * 
     * @param   nEfficiency         the new detector efficiency in [%]
     */
    void set_efficiency(double nEfficiency) throw(std::out_of_range);
   

    /**
     * set the event table size
     * 
     * @param   nEventTableSize     the new event table size
     */
    void set_event_table_size(uint64_t nEventTableSize) throw(std::out_of_range);


    /**
     * set jitter flag  
     * 
     * @param   bJitter     new jitter flag
     */
    void set_jitter(bool bJitter) { m_bJitter = bJitter; update_jitter(); };
     
     
    /**
     * set loss flag  
     * 
     * @param   bLoss     new loss flag
     */
    void set_loss(bool bLoss) { m_bLoss = bLoss; update_detection_loss(); };
     
     
    /**
     * sets the distance independent loss [0 - 30 dB]
     * 
     * @param   nDistanceIndepLoss      the new loss in [dB]
     */
    void set_loss_rate(double nLossRate) throw(std::out_of_range);
    

    /**
     * sets delay of photon detection in ns
     * 
     * @param   nPhotonTimeDelay    new photon detection delay time in [ns]
     */
    void set_photon_time_delay(double nPhotonTimeDelay) throw(std::out_of_range);
    
    
    /**
     * sets standard deviation of photon time distribution in [0 - 100 ns]
     * 
     * @param   nPhotonTimeStndDeviation    new standard deviation of photon time distribution in [ns]
     */
    void set_photon_time_stnd_deviation(double nPhotonTimeStndDeviation) throw(std::out_of_range);
    

    /**
     * set the sync delay time
     * 
     * @param   nSyncDelay      the sync delay time in ns
     */
    void set_sync_delay(double nSyncDelay) throw(std::out_of_range) {
        if (!m_bAlice) {
            if (nSyncDelay < 0.0) throw std::out_of_range("detector::set_sync_delay: nSyncDelay");
            m_cSyncPulseReceiver->set_delay(nSyncDelay);
        }
    }
    
    
    /**
     * set the sync detection jitter standard deviation
     * 
     * @param   nSyncStdnDeviation      the sync detection jitter standard deviation in [ns]
     */
    void set_sync_stnd_deviation(double nSyncStdnDeviation) throw(std::out_of_range) { 
        if (!m_bAlice) {
            if (nSyncStdnDeviation < 0.0) throw std::out_of_range("detector::set_sync_stnd_deviation: nSyncStdnDeviation");
            m_cSyncPulseReceiver->set_jitter(nSyncStdnDeviation);
        }
    }
    

    /**
     * sets the time slot width used as coincidence window in [0 - 1000 ns]
     *
     * @param   nTimeSlotWidth      the new time slot width [ns]
     */
    void set_time_slot_width(double nTimeSlotWidth) throw(std::out_of_range);
    
    
    /**
     * setup the pipe-put (this should be run within the detector thread)
     */
    void setup_pipe_out();
    

    /**
     * get the time slot width used as coincidence window in [0 - 1000 ns]
     *
     * @return  the detector time slot width (coincidence window)
     */
    double time_slot_width() const { return m_nTimeSlotWidth; };


    /**
     * write out all parameters of this event handler and its child event handlers
     * 
     * @param   cStream     the stream to write to
     */
    void write_parameters(std::ofstream & cStream);
    

private:
    
    
    /**
     * update the dark count rate on each detector element
     */
    void update_dark_count_rate();
    

    /**
     * update efficiency and detector down time on each detector element
     */
    void update_detection_loss();
    
    
    /**
     * apply time delay and deviation on each detector element
     */
    void update_jitter();
    
    
    bool m_bAlice;                              /**< state if we are alice */
    bool m_bDarkCounts;                         /**< dark counts enabled */
    detection_mode m_detection_mode;            /**< detection mode for this detector */
    double m_nDarkCountRate;                    /**< dark count rate in 1/s */
    double m_nDownTime;                         /**< down time in ns */
    double m_nEfficiency;                       /**< efficiency [0 - 1]*/  // P.G.: added [0 - 1]
    uint64_t m_nEventTableSize;                 /**< event table size in bytes */
    bool m_bJitter;                             /**< jitter enabled */
    bool m_bLoss;                               /**< loss enabled */
    double m_nLossRate;                         /**< distance independent loss in dB */
    double m_nPhotonTimeDelay;                  /**< photon detection time delay in ns */
    double m_nPhotonTimeStndDeviation;          /**< photon time standard deviation in ns */
    double m_nTimeSlotWidth;                    /**< time slot width == coincidence window in ns */
    
    
    /**
     * the channel
     */
    qkd::simulate::channel * m_cChannel;
    
    
    /**
     * the detector optics
     */
    detector_optics m_cDetectorOptics;
    
    
    /**
     * detection element for horizontal polarization
     */
    detection_element m_cDetectionElementH;
    
    
    /**
     * detection element for vertical polarization
     */
    detection_element m_cDetectionElementV;
    
    
    /**
     * detection element for "plus" polarization
     */
    detection_element m_cDetectionElementP;
    
    
    /**
     * detection element for "minus" polarization
     */
    detection_element m_cDetectionElementM;
    
    
    /**
     * event buffer
     */
    event_buffer m_cEventBuffer;
    
    
    /**
     * sync pulse generator (only used at Alice's side)
     */
    sync_pulse_generator * m_cSyncPulseGenerator;
    
    
    /**
     * sync pulse receiver (only used at Bob's side)
     */
    sync_pulse_receiver * m_cSyncPulseReceiver;
    
    
    /**
     * window generator
     */
    window_generator m_cWindowGenerator;
};


}
}

#endif

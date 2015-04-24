/*
 * frame.h
 * 
 * A cascade frame holds a key plus associated methods 
 * relevant for cascade
 * 
 * Autor: Philipp Grabenweger
 *        Christoph Pacher, <christoph.pacher@ait.ac.at>
 *        Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *
 * Copyright (C) 2014-2015 AIT Austrian Institute of Technology
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


#ifndef __QKD_MODULE_QKD_CASCADE_FRAME_H
#define __QKD_MODULE_QKD_CASCADE_FRAME_H


// ------------------------------------------------------------
// incs

#include <set>

// ait
#include <qkd/key/key.h>


// ------------------------------------------------------------
// decl


// fwd
class parity_checker;


/**
 * A cascade frame holds the key along with necessary cascade relevant data and methods
 */
class frame {   


public:


    /**
     * ctor
     *
     * @param   cKey        the keys we operate on
     */
    frame(qkd::key::key & cKey) : m_cKey(cKey), m_nTransmittedMessages(0), m_nTransmittedParities(0) {}

    
    /**
     * copy ctor
     */
    frame(frame const & rhs) = delete;


    /**
     * dtor
     */
    virtual ~frame();

   
    /**
     * add a parity checker that should be notified of changes of this frame
     *
     * @param   cChecker            the parity checker to add
     */
    void add_checker(parity_checker * cChecker);


     /**
     * add a number of transmitted messages
     *
     * @param   n       number of new messages transmitted
     */
    void add_transmitted_messages(uint64_t n) { m_nTransmittedMessages += n; }
   

    /**
     * add a number of transmitted parities
     *
     * @param   n       number of new parities transmitted
     */
    void add_transmitted_parities(uint64_t n) { m_nTransmittedParities += n; }


    /**
     * return checkers of this frame
     *
     * @return  the checkers of this frame
     */
    std::vector<parity_checker *> & checkers() { return m_cCheckers; };
    
   
    /**
     * get indices of surely correct bits inside the frame
     * 
     * @return  set of indices of surely correct frame bits
     */
    std::set<uint64_t> & correct_bits() { return m_cCorrectBits; }


    /**
     * get indices of surely correct bits inside the frame
     * 
     * @return  set of indices of surely correct frame bits
     */
    std::set<uint64_t> const & correct_bits() const { return m_cCorrectBits; }


    /**
     * get indices of corrected bits inside the frame
     * 
     * @return  set of indices of corrected frame bits
     */
    std::set<uint64_t> & corrected_bits() { return m_cCorrectedBits; }


    /**
     * get indices of corrected bits inside the frame
     * 
     * @return  set of indices of correct frame bits
     */
    std::set<uint64_t> const & corrected_bits() const { return m_cCorrectedBits; }


    /**
     * invert a bit in the frame
     *
     * @param   pos     position of the bit
     */
    void flip_bit(uint64_t pos);
    

    /**
     * get a bit of the frame
     * 
     * @param   pos     position of the bit
     * @return  value of the bit
     */
    inline bool get_bit(uint64_t pos) const { return m_cKey.get_bit(pos); };   


    /**
     * get the key included
     *
     * @return  key included
     */
    qkd::key::key & key() { return m_cKey; };


    /**
     * get the key included
     *
     * @return  key included
     */
    qkd::key::key const & key() const { return m_cKey; };


    /**
     * notify all checkers of a bit correction, but without changing the bit in this frame
     *
     * @param   pos         bit position
     */
    void notify_bit_change_remote(uint64_t pos);
    

    /**
     * notify this frame and all checkers of a correct bit in this frame
     *
     * @param   pos         bit position
     */
    void notify_correct_bit(uint64_t pos);
   

    /**
     * remove a parity checker from the frame's change notification list
     *
     * @param   cChecker        checker to remove
     */
    void remove_checker(parity_checker * cChecker);
    

    /**
     * change a bit in the frame to a specified value
     *
     * @param   pos     position of bit
     * @param   bit     bit value
     */
    void set_bit(uint64_t pos, bool bit);
    
 
    /**
     * get number of transmitted messages
     * 
     * @return  number of transmitted messages
     */
    uint64_t transmitted_messages() const { return m_nTransmittedMessages; }
   

    /**
     * get number of transmitted parities
     * 
     * @return  number of transmitted parities
     */
    uint64_t transmitted_parities() const { return m_nTransmittedParities; }

 
private:

     
    /**
     * our key we operate on
     */
    qkd::key::key & m_cKey;


    /**
     * set of parity checkers to notify of frame changes 
     */
    std::vector<parity_checker *> m_cCheckers;     


    /**
     *  set containing the indices of all frame bits that are known to be correct 
     */
    std::set<uint64_t> m_cCorrectBits; 


    /**
     *  set containing the indices of all frame bits that have been corrected
     */
    std::set<uint64_t> m_cCorrectedBits; 


    /**
     *  number of transmitted messages 
     */    
    uint64_t m_nTransmittedMessages; 


    /**
     * number of transmitted parities 
     */    
    uint64_t m_nTransmittedParities; 

};

#endif


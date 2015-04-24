/*
 * frame.cpp
 * 
 * Implementation of additinal cascade key 
 * frame methods
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


// ------------------------------------------------------------
// incs

// ait
#include <qkd/utility/syslog.h>

#include "frame.h"
#include "parity-checker.h"


// ------------------------------------------------------------
// code


/**
 * dtor
 */
frame::~frame() {
    // free checkers
    for (auto cChecker : m_cCheckers) delete cChecker;
}


/**
 * add a parity checker that should be notified of changes of this frame
 *
 * @param   cChecker            the parity checker to add
 */
void frame::add_checker(parity_checker * cChecker) {

    // sanity check
    if (!cChecker) return;

    // insert
    m_cCheckers.push_back(cChecker);
}


/**
 * invert a bit in the frame
 *
 * @param   pos     position of the bit
 */
void frame::flip_bit(uint64_t pos) { 

    // sanity check
    if (pos >= m_cKey.size() * 8) return;

    set_bit(pos, !get_bit(pos)); 

    // we fliped the bit as bob, assuming 
    // now having a correct bit here
    m_cCorrectedBits.insert(pos);
} 


/**
 * change a bit in the frame to a specified value
 *
 * @param   pos     position of bit
 * @param   bit     bit value
 */
void frame::set_bit(uint64_t pos, bool bit) { 

    // sanity check
    if (pos >= m_cKey.size() * 8) return;

    // do not act on no-change
    if (m_cKey.get_bit(pos) == bit) return;

    // change bit and notify all parity checkers of change
    m_cKey.set_bit(pos, bit);
    for (auto cChecker : m_cCheckers) {
        cChecker->notify_bit_change_local(pos);
    }
}


/**
 * notify all checkers of a bit correction, but without changing the bit in this frame
 *
 * @param   pos         bit position
 */
void frame::notify_bit_change_remote(uint64_t pos) {

    // sanity check
    if (pos >= m_cKey.size() * 8) return;

    m_cCorrectedBits.insert(pos);
    for (auto cChecker : m_cCheckers) {
        cChecker->notify_bit_change_remote(pos);
    }
}


/**
 * notify this frame and all checkers of a correct bit in this frame
 *
 * @param   pos         bit position
 */
void frame::notify_correct_bit(uint64_t pos) {

    // sanity check
    if (pos >= m_cKey.size() * 8) return;

    m_cCorrectBits.insert(pos);
    
    for (auto cChecker : m_cCheckers) {
        cChecker->notify_correct_bit(pos);
    }
}


/**
 * remove a parity checker from the frame's change notification list
 *
 * @param   cChecker        checker to remove
 */
void frame::remove_checker(parity_checker * cChecker) {

    // sanity check
    if (!cChecker) return;
    for (auto it = m_cCheckers.begin(); it != m_cCheckers.end(); ++it) {
        if ((*it) == cChecker) {
            m_cCheckers.erase(it);
            break;
        }
    }
    delete cChecker;
}


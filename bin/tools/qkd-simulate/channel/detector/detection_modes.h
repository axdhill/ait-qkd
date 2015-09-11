/*
 * detection_modes.h
 * 
 * Definition of detection modes
 *
 * Author: Philipp Grabenweger
 *         Oliver Maurhart, <oliver.maurhart@ait.ac.at>
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


#ifndef __QKD_QKD_DETECTION_MODES_H_
#define __QKD_QKD_DETECTION_MODES_H_


// -----------------------------------------------------------------
// decl


namespace qkd {
    
namespace simulate {

/**
 * detection modes
 */
enum detection_mode : uint8_t {
    
    FREE_RUNNING,               /**< detector is free running (no sync pulse generation/receive, only TTM records) */
    SYNC,                       /**< sync pulse generation/receive irrespective of detector down times */
    SYNC_INITIATOR_READY,       /**< mode valid only for Alice side: sync pulse generation, but only if last sync pulse initiator is not down anymore */
    SYNC_ALL_READY              /**< meaning at Alice side: sync pulse generation, but only if no detection element is down.
                                 *  meaning at Bob side:   sync pulse receive, but if one or more detection elements are down, a zero entry is stored in event buffer */
};


}
}

#endif

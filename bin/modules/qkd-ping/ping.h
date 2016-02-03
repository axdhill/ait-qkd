/*
 * ping.h
 * 
 * The ping mechanism itself
 * 
 * Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *
 * Copyright (C) 2014-2016 AIT Austrian Institute of Technology
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


#ifndef __QKD_MODULE_PING_H_
#define __QKD_MODULE_PING_H_


// ------------------------------------------------------------
// incs

// ait
#include <qkd/module/communicator.h>


// ------------------------------------------------------------
// decl


/**
 * do ping as alice
 *
 * @param   cModuleComm         the module communicator
 * @param   nPackageSize        size of packet to send
 * @return  true, if successfully pinged
 */
bool ping_alice(qkd::module::communicator & cModuleComm, uint64_t nPackageSize);


/**
 * do ping as bob
 *
 * @param   cModuleComm         the module communicator
 * @param   nPackageSize        size of packet to send
 * @return  true, if successfully pinged
 */
bool ping_bob(qkd::module::communicator & cModuleComm, uint64_t nPackageSize);


#endif


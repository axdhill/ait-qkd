/*
 * nic_win32.cpp
 *
 * implement the network interface q3p "card" on win32 systems
 * 
 * Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *
 * Copyright (C) 2012-2016 AIT Austrian Institute of Technology
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


#if defined(__WIN32__)


// ------------------------------------------------------------
// incs

#include <qkd/q3p/nic.h>

using namespace qkd::q3p;

#define NIC_OS_DETAIL_IMPLEMENTATION
#include "nic_common.cpp"


// ------------------------------------------------------------
// code


/**
 * ctor
 * 
 * @param   cEngine     the parent engine
 * @throws  mq_no_engine
 */
nic_instance::nic_instance(qkd::q3p::engine_instance * cEngine) : QObject(), m_cEngine(cEngine) {
#error "Windows port not implemented yet"
}


/**
 * dtor
 */
nic_instance::~nic_instance() {
#error "Windows port not implemented yet"
}


/**
 * adds the IP4 route to the kernel
 * 
 * @return  true, if successully added
 */
bool nic_instance::add_ip4_route() {
#error "Windows port not implemented yet"
}


/**
 * assign local IP4
 * 
 * @return  true, if successully assigned
 */
bool nic_instance::assign_local_ip4() {
#error "Windows port not implemented yet"
}


/**
 * the reader thread
 * 
 * read data from local user applications and send them
 * to the peer instance
 */
void nic_instance::reader() {
#error "Windows port not implemented yet"
}


/**
 * write data to the device, thus sending it to the kernel
 * 
 * this is used to send data which have been received by
 * the TUN/TAP to local user applications
 * 
 * @param   cData       the data to write
 */
void nic_instance::write(qkd::utility::memory const & cData) {
#error "Windows port not implemented yet"
}



#endif

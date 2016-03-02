/*
 * nic_common.cpp
 *
 * common methods for any operating system to implement the network interface q3p "card" 
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


#ifndef NIC_OS_DETAIL_IMPLEMENTATION
#error "nic_common.cpp may not be directly compiled. It is to be included into the nic_[OS].cpp files."
#endif


// ------------------------------------------------------------
// code


/**
 * set the local IP4 address of the NIC
 * 
 * @param   sIP4        the new local address of the NIC
 */
void nic_instance::set_ip4_local(QString sIP4) {
    
    std::string s = sIP4.toStdString();
    if (m_cEngine->nic_ip4_local() != s) {
        m_cEngine->set_nic_ip4_local(s);
        return;
    }
    
    m_sIP4Local = s;
    setup_networking();
}


/**
 * set the remote IP4 address of the NIC
 * 
 * @param   sIP4        the new remote address of the NIC
 */
void nic_instance::set_ip4_remote(QString sIP4) {
    
    std::string s = sIP4.toStdString();
    if (m_cEngine->nic_ip4_remote() != s) {
        m_cEngine->set_nic_ip4_remote(s);
        return;
    }
    
    m_sIP4Remote = s;
    setup_networking();
}


/**
 * apply IP4 address and routing
 */
void nic_instance::setup_networking() {

    if (assign_local_ip4()) {
        emit ip4_changed();
        if (add_ip4_route()) {
        }
    }
}


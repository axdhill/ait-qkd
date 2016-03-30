/*
 * nic.h
 * 
 * this file describes the Q3P network interface card handling
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

 
#ifndef __QKD_Q3P_NIC_H_
#define __QKD_Q3P_NIC_H_


// ------------------------------------------------------------
// incs

#include <exception>
#include <memory>
#include <string>

#include <inttypes.h>

// Qt
#include <QtCore/QObject>
#include <QtDBus/QtDBus>

// ait
#include <qkd/utility/memory.h>


// ------------------------------------------------------------
// decls


namespace qkd {
    
namespace q3p {    

// fwd
class engine_instance;
    
class nic_instance;
typedef std::shared_ptr<nic_instance> nic;


/**
 * This is the network interface q3p "card" object.
 * 
 * On behalf of the TUN/TAP we create a q3p0 (or q3p1, or q3p2, 
 * or ... you got the picture) interface which is capable of
 * handling IP packets.
 * 
 * =)
 * 
 * 
 * For a process creating a TUN/TAP device on Linux 
 * it needs the CAP_NET_ADMIN capability.
 * 
 * One can set this capability with:
 * 
 *      $ sudo setcap cap_net_admin=ep /PATH/TO/PROCESS
 * 
 * 
 * The offered DBus interface is
 * 
 *      DBus Interface: "at.ac.ait.q3p.nic"
 * 
 * 
 * Properties of at.ac.ait.q3p.nic
 * 
 *      -name-          -read/write-    -description-
 * 
 *      name                R           name of the NIC
 * 
 * Signals emitted by at.ac.ait.q3p.nic
 * 
 *      device_ready(QString)           emitted when created the TUN/TAP device
 * 
 */
class nic_instance : public QObject {
    
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "at.ac.ait.q3p.nic")
    
    Q_PROPERTY(QString ip4_local READ ip4_local WRITE set_ip4_local)        /**< local ip4 address of the NIC */
    Q_PROPERTY(QString ip4_remote READ ip4_remote WRITE set_ip4_remote)     /**< remote ip4 address of the NIC */
    Q_PROPERTY(QString name READ name)                                      /**< network interface name */
    
public:
    
    
    /**
     * ctor
     * 
     * @param   cEngine     the parent engine
     * @throws  nic_no_engine
     */
    nic_instance(qkd::q3p::engine_instance * cEngine);
    

    /**
     * dtor
     */
    virtual ~nic_instance();
    
    
    /**
     * return the Q3P engine
     * 
     * @return  the Q3P engine associated
     */
    inline qkd::q3p::engine_instance * engine() { return m_cEngine; }
    
    
    /**
     * return the Q3P engine
     * 
     * @return  the Q3P engine associated
     */
    inline qkd::q3p::engine_instance const * engine() const { return m_cEngine; }
    
    
    /**
     * return the local IP4 address assigned to the NIC
     * 
     * @return  the local IP4 address used
     */
    inline QString ip4_local() const { return QString::fromStdString(m_sIP4Local); }
    
    
    /**
     * return the remote IP4 address assigned to the NIC
     * 
     * @return  the remote IP4 address used
     */
    inline QString ip4_remote() const { return QString::fromStdString(m_sIP4Remote); }
    
    
    /**
     * name of the message queue
     * 
     * @return  the name of the message queue
     */
    inline QString name() const { return QString::fromStdString(m_sName); }
    
    
    /**
     * set the local IP4 address of the NIC
     * 
     * @param   sIP4        the new local address of the NIC
     */
    void set_ip4_local(QString sIP4);
    
    
    /**
     * set the remote IP4 address of the NIC
     * 
     * @param   sIP4        the new remote address of the NIC
     */
    void set_ip4_remote(QString sIP4);
    
    
    /**
     * write data to the device, thus sending it to the kernel
     * 
     * @param   cData       the data to write
     */
    void write(qkd::utility::memory const & cData);

    
signals:    
    
    
    /**
     * emitted when we have a TUN/TAP device created
     * 
     * @param   sDevice     name of the device
     */
    void device_ready(QString sDevice);
    
    
    /**
     * signaled whenever the address of the associated NIC changed
     */
    void ip4_changed();
    
    
    /**
     * signaled whenever we have a route to our peer
     */
    void route_added();
    
    
    /**
     * signaled whenever we lost a route to our peer
     */
    void route_deleted();
    
    
private:
    
    
    /**
     * adds the IP4 route to the kernel
     * 
     * @return  true, if successully added
     */
    bool add_ip4_route();
    
    
    /**
     * assign local IP4
     * 
     * @return  true, if successully assigned
     */
    bool assign_local_ip4();
    
    
    /**
     * removes the IP4 route from the kernel
     * 
     * @return  true, if successully removed
     */
    bool del_ip4_route();
    
    
    /**
     * the reader thread
     */
    void reader();
    
    
    /**
     * apply IP4 address and routing
     */
    void setup_networking();
    
    
    /**
     * the Q3P engine
     */
    qkd::q3p::engine_instance * m_cEngine;
    
    
    /**
     * local IP4 address
     */
    std::string m_sIP4Local;
    
    
    /**
     * local IP4 address
     */
    std::string m_sIP4Remote;
    
    
    /**
     * name of the device
     */
    std::string m_sName;


    // pimpl
    class nic_data;
    std::shared_ptr<nic_data> d;
};
  

}

}


#endif


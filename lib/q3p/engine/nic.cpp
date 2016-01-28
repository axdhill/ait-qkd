/*
 * nic.cpp
 *
 * implement the network interface q3p "card"
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

 
// ------------------------------------------------------------
// incs

#include <atomic>
#include <thread>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>

// this is currentl Linux only code
// TODO: find a way to make this portable.
//       there is TUN/TAP support on other operating
//       systems like Mac OSX ...
#if defined(__linux__)

#   include <linux/if.h>
#   include <linux/if_tun.h>
#   include <sys/ioctl.h>

#else

#   error "Currently no other operating system than Linux supported. Sorry. Vist: http://sqt.ait.ac.at/software or mail oliver.maurhart@ait.ac.at for support on this."

#endif


// Qt
#include <QtCore/QSocketNotifier>

// ait
#include <qkd/q3p/engine.h>
#include <qkd/q3p/nic.h>
#include <qkd/utility/debug.h>
#include <qkd/utility/syslog.h>


using namespace qkd::q3p;


// ------------------------------------------------------------
// decl


/**
 * the nic pimpl
 */
class qkd::q3p::nic_instance::nic_data {
    
    
public:
    
    
    /**
     * ctor
     */
    nic_data(): nFD(0) {};
    
    int nFD;                        /**< tun/tap file descriptor */
    std::atomic<bool> bRun;         /**< run flag */
    std::thread cReaderThread;      /**< the reader thread */
};


// ------------------------------------------------------------
// code


/**
 * ctor
 * 
 * @param   cEngine     the parent engine
 * @throws  mq_no_engine
 */
nic_instance::nic_instance(qkd::q3p::engine_instance * cEngine) : QObject(), m_cEngine(cEngine) {
    
    // engine
    if (!m_cEngine) throw qkd::q3p::nic_instance::nic_no_engine();
    
    // pimpl
    d = boost::shared_ptr<qkd::q3p::nic_instance::nic_data>(new qkd::q3p::nic_instance::nic_data());    
    d->nFD = 0;
    
    // get up q3pX
    init_tun();
}


/**
 * dtor
 */
nic_instance::~nic_instance() {

    // stop reader thread
    if (d->cReaderThread.get_id() != std::thread::id()) {
    
        // interrupt reader thread and join
        d->bRun = false;
        pthread_kill(d->cReaderThread.native_handle(), SIGCHLD);
        if (d->cReaderThread.joinable()) d->cReaderThread.join();
    }
    
    // close the TUN/TAP device
    close(d->nFD);
}


/**
 * get up the tun (from tun/tap) device
 */
void nic_instance::init_tun() {
    
    // open /dev/net/tun
    d->nFD = ::open("/dev/net/tun", O_RDWR);
    if (d->nFD < 0) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "could not access /dev/net/tun: " << strerror(errno);
        return;
    }
    
    // make an ioctl on the file descriptor with IFF_TUN set
    struct ifreq cIFReq;
    memset(&cIFReq, 0, sizeof(cIFReq));
    cIFReq.ifr_flags = IFF_TUN;
    
    // prepare the name
    strncpy(cIFReq.ifr_name, "q3p%d", IFNAMSIZ);
    
    // create the device
    if (ioctl(d->nFD, TUNSETIFF, (void *)&cIFReq) == -1) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "could not create TUN device: " << strerror(errno);
        close(d->nFD);
        d->nFD = 0;
        return;
    }
    
    // we are up
    m_sName = cIFReq.ifr_name;
    
    // create reader thread
    d->bRun = true;
    d->cReaderThread = std::thread([this]{ reader(); });
    
    // tell environemnt
    emit device_ready(QString::fromStdString(m_sName));
    qkd::utility::syslog::info() << "created TUN device: " << m_sName;
}


/**
 * the reader thread
 * 
 * read data from local user applications and send them
 * to the peer instance
 */
void nic_instance::reader() {

    char cBuffer[1024 * 64];
    uint64_t nSize;
    while (d->bRun && ((nSize = read(d->nFD, cBuffer, 1024 * 64)) > 0 )) {

        // ignore errors
        if (nSize == (uint64_t)-1) continue;
        
        // wrap data and send it
        qkd::utility::memory cPayload = qkd::utility::memory::wrap((qkd::utility::memory::value_t *)cBuffer, nSize);
        m_cEngine->send_data(cPayload);
    }
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

    // don't write if we ain't got an interface
    if (d->nFD <= 0) {
        if (qkd::utility::debug::enabled()) qkd::utility::debug() << "failed to write " << cData.size() << " bytes to TUN/TAP: no device present.";
        return;
    }
    
    // pass data to the kernel
    uint64_t nSize = ::write(d->nFD, cData.get(), cData.size());
    if (nSize != cData.size()) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "nic in trouble: failed to pass received data to the kernel";
    }
}



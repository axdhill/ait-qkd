/*
 * nic_linux.cpp
 *
 * implement the network interface q3p "card" on Linux
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

 
/*
 * Netlink interaction inspired by 
 * http://stackoverflow.com/questions/3288065/getting-gateway-to-use-for-a-given-ip-in-ansi-c#3288983
 */


// this is only Linux code
#if defined(__linux__)


// ------------------------------------------------------------
// incs

#include <stddef.h>

#include <atomic>
#include <thread>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <arpa/inet.h>
#include <asm/types.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h> 
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <boost/format.hpp>

// Qt
#include <QtCore/QSocketNotifier>

// ait
#include <qkd/q3p/engine.h>
#include <qkd/q3p/nic.h>
#include <qkd/utility/debug.h>
#include <qkd/utility/syslog.h>

using namespace qkd::q3p;

#define NIC_OS_DETAIL_IMPLEMENTATION
#include "nic_common.cpp"


// ------------------------------------------------------------
// vars


/**
 * netlink message sequence number
 */
static unsigned int g_nNetlinkMessageNumber = 0;


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


/**
 * check if the device flags are point-to-point
 * 
 * @param   sDevice         the device
 * @return  true, if the device flags are ok
 */
bool check_device_flags(std::string const & sDevice);


/**
 * get current IP4 address of device
 * 
 * @param   sDevice         the device
 * @return  the inetrnet IP4 address of the device
 */
in_addr_t get_current_ip4(std::string const & sDevice);


/**
 * quick and dirty in_addr_t to string
 * 
 * (because standard POSIX stuff is plain stupid with all
 * that struct inaddr, in_addr_t, s_addr, ... what a mess!)
 * 
 * @param   nIP4        the IP4 address
 * @return  string in the 4 digit-dot notation
 */
std::string inet_addr_to_string(in_addr_t nIP4);


/**
 * get up the tun (from tun/tap) device
 * 
 * @param   nDeviceFD       device file descriptor
 * @param   sDeviceName     name of new device
 * @return  true, if device has been successully setup
 */
bool init_tun(int & nDeviceFD, std::string & sDeviceName);


/**
 * checks if the given string contains an IP4 dotted address
 * 
 * @return  true, if inet_aton accepts it (or similar)
 */
bool is_ip4_string(std::string const & sIP4);


/**
 * receive from the netlink layer
 * 
 * The buffer has to be allocated prior with a proper buffer size. On success the buffer
 * will be filled with nlmsghdr structs linear. On success the number of bytes written
 * to the buffer is returned, else -1.
 * 
 * @param   nSocket             the netlink socket
 * @param   cBuffer             the buffer which receives the netlink messages
 * @param   nMessageNumber      the message number to receive
 * @return  number of bytes received (-1 in case of error)
 */
int netlink_recv(int nSocket, char * cBuffer, ssize_t nBufferSize, int nMessageNumber);
    
    
/**
 * send a netlink message to the kernel
 * 
 * @param   nSocket             the netlink socket
 * @param   cNetlinkMessage     the message to be sent
 * @return  true on successul sent
 */
bool netlink_send(int nSocket, struct nlmsghdr * cNetlinkMessage);


/**
 * set current IP4 address of device
 * 
 * @param   sDevice         the device
 * @param   nIP4            the new IP4 address
 * @return  true, if successully assigned
 */
bool set_current_ip4(std::string const & sDevice, in_addr_t nIP4);


/**
 * apply network device flags properly
 * 
 * @param   sDevice         the device
 */
void set_device_flags(std::string const & sDevice);


// ------------------------------------------------------------
// code


/**
 * ctor
 * 
 * @param   cEngine     the parent engine
 * @throws  mq_no_engine
 */
nic_instance::nic_instance(qkd::q3p::engine_instance * cEngine) : QObject(), m_cEngine(cEngine) {
    
    if (!m_cEngine) {
        throw std::invalid_argument("nic instance with NULL engine");
    }
    
    d = std::shared_ptr<qkd::q3p::nic_instance::nic_data>(new qkd::q3p::nic_instance::nic_data());    
    d->nFD = 0;
    
    // get up q3pX
    if (init_tun(d->nFD, m_sName)) {
    
        d->bRun = true;
        d->cReaderThread = std::thread([this]{ reader(); });
        
        emit device_ready(QString::fromStdString(m_sName));
    }
}


/**
 * dtor
 */
nic_instance::~nic_instance() {

    if (d->cReaderThread.get_id() != std::thread::id()) {
        d->bRun = false;
        pthread_kill(d->cReaderThread.native_handle(), SIGCHLD);
        if (d->cReaderThread.joinable()) d->cReaderThread.join();
    }
    
    close(d->nFD);
}


/**
 * adds the IP4 route to the kernel
 * 
 * @return  true, if successully added
 */
bool nic_instance::add_ip4_route() {
    
    if (m_sName.empty()) {
        return false;
    }

    if (m_sIP4Local.empty() || m_sIP4Remote.empty()) {
        return false;
    }
    
    if (!is_ip4_string(m_sIP4Local)) {
        qkd::utility::debug() << "Failed to translate local IP4 address: '" << m_sIP4Local << "'";
        return false;
    }
    
    if (!is_ip4_string(m_sIP4Remote)) {
        qkd::utility::debug() << "Failed to translate remote IP4 address: '" << m_sIP4Remote << "'";
        return false;
    }
    
    return false;
}


/**
 * assign local IP4
 * 
 * @return  true, if successully assigned
 */
bool nic_instance::assign_local_ip4() {

    if (m_sName.empty()) {
        return false;
    }

    if (m_sIP4Local.empty()) {
        return false;
    }
    
    if (!is_ip4_string(m_sIP4Local)) {
        qkd::utility::debug() << "Failed to translate local IP4 address: '" << m_sIP4Local << "'";
        return false;
    }
    
    in_addr_t nIP4 = inet_addr(m_sIP4Local.c_str());
    if (get_current_ip4(m_sName) == nIP4) {
        return true;
    }
    
    if (!set_current_ip4(m_sName, nIP4)) {
        return false;
    }
    
    if (!check_device_flags(m_sName)) {
        set_device_flags(m_sName);
    }
    
    qkd::utility::syslog::info() << "assigned " << m_sIP4Local << " to device " << m_sName;
    
    return false;
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

        if (nSize == (uint64_t)-1) continue;
        
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

    if (d->nFD <= 0) {
        if (qkd::utility::debug::enabled()) qkd::utility::debug() << "failed to write " << cData.size() << " bytes to TUN/TAP: no device present.";
        return;
    }
    
    uint64_t nSize = ::write(d->nFD, cData.get(), cData.size());
    if (nSize != cData.size()) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "nic in trouble: failed to pass received data to the kernel";
    }
}


/**
 * check if the device flags are point-to-point
 * 
 * @param   sDevice         the device
 * @return  true, if the device flags are ok
 */
bool check_device_flags(std::string const & sDevice) {
    
    bool res = false;
    
    int nExitCode = 0;
    
    int s = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
    ifreq cIFReq;
    memset(&cIFReq, 0, sizeof(cIFReq));
    strncpy(cIFReq.ifr_name, sDevice.c_str(), IFNAMSIZ);
    
    nExitCode = ioctl(s, SIOCGIFFLAGS, &cIFReq);
    close(s);
    if (nExitCode != 0) {
        return false;
    }
    
    short nFlags;
    memcpy((char *)&nFlags, (char *)&cIFReq + offsetof(struct ifreq, ifr_flags), sizeof(nFlags));
    
    res = (nFlags == (IFF_UP | IFF_POINTOPOINT | IFF_RUNNING | IFF_NOARP | IFF_MULTICAST));
    
    return res;
}


/**
 * get current IP4 address of device
 * 
 * @param   sDevice         the device
 * @return  the inetrnet IP4 address of the device
 */
in_addr_t get_current_ip4(std::string const & sDevice) {
    
    in_addr_t res = -1;
    
    int nExitCode = 0;
    
    int s = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
    ifreq cIFReq;
    memset(&cIFReq, 0, sizeof(cIFReq));
    strncpy(cIFReq.ifr_name, sDevice.c_str(), IFNAMSIZ);
    
    nExitCode = ioctl(s, SIOCGIFADDR, &cIFReq);
    close(s);
    if (nExitCode != 0) {
        return res;
    }
    
    sockaddr_in cAddr;
    memcpy((char *)&cAddr, (char *)&cIFReq + offsetof(struct ifreq, ifr_addr), sizeof(cAddr));
    res = cAddr.sin_addr.s_addr;

    return res;
}


/**
 * receive from the netlink layer
 * 
 * The buffer has to be allocated prior with a proper buffer size. On success the buffer
 * will be filled with nlmsghdr structs linearly. On success the number of bytes written
 * to the buffer is returned, else -1.
 * 
 * @param   nSocket             the netlink socket
 * @param   cBuffer             the buffer which receives the netlink messages
 * @param   nMessageNumber      the message number to receive
 * @return  number of bytes received (-1 in case of error)
 */
int netlink_recv(int nSocket, char * cBuffer, ssize_t nBufferSize, unsigned int nMessageNumber) {
    
    unsigned int nProcessId = getpid();
    int nRead = 0;
    int nMessageLen = 0;

    struct nlmsghdr * cNetlinkMessage = nullptr;
    
    do {
        
        nRead = recv(nSocket, cBuffer, nBufferSize - nMessageLen, 0);
        if (nRead < 0) {
            qkd::utility::debug() << "Failed to read from netlink socket. Error: " << strerror(errno);
            return -1;
        }

        // TODO: check if the message number and the destination pid is ok
        
        cNetlinkMessage = (struct nlmsghdr *)cBuffer;

        if ((NLMSG_OK(cNetlinkMessage, nRead) == 0) || (cNetlinkMessage->nlmsg_type == NLMSG_ERROR)) {
            qkd::utility::debug() << "Error in received netlink message. Error: " << strerror(errno);
            return -1;
        }

        if (cNetlinkMessage->nlmsg_type == NLMSG_DONE) {
            break;
        } 
        else {
            cBuffer += nRead;
            nMessageLen += nRead;
        }

        if ((cNetlinkMessage->nlmsg_flags & NLM_F_MULTI) == 0) {
            // no multipart message --> no further recv necessary
            break;
        }
        
    } while ((cNetlinkMessage->nlmsg_seq != nMessageNumber) || (cNetlinkMessage->nlmsg_pid != nProcessId));
    
    return nMessageLen;
}


/**
 * send a netlink message to the kernel
 * 
 * @param   nSocket             the netlink socket
 * @param   cNetlinkMessage     the message to be sent
 * @return  true on successul sent
 */
bool netlink_send(int nSocket, struct nlmsghdr * cNetlinkMessage) {
    
    if (nSocket == -1) {
        qkd::utility::debug() << "Refused to send netlink message on invalid socket.";
        return false;
    }
    if (cNetlinkMessage == nullptr) {
        qkd::utility::debug() << "Refused to send NULL netlink message.";
        return false;
    }
    
    cNetlinkMessage->nlmsg_seq = g_nNetlinkMessageNumber++;
    cNetlinkMessage->nlmsg_pid = getpid();
    
    if (send(nSocket, cNetlinkMessage, cNetlinkMessage->nlmsg_len, 0) < 0) {
        qkd::utility::debug() << "Failed to send netlink message. Error: " << strerror(errno);
        return false;
    }
    
    return true;
}


/**
 * checks if the given string contains an IP4 dotted address
 * 
 * @return  true, if inet_aton accepts it (or similar)
 */
bool is_ip4_string(std::string const & sIP4) {

    in_addr a;
    if (inet_aton(sIP4.c_str(), &a) == 0) {
        return false;
    }
    
    return true;
}


/**
 * set current IP4 address of device
 * 
 * @param   sDevice         the device
 * @param   nIP4            the new IP4 address
 * @return  true, if successully assigned
 */
bool set_current_ip4(std::string const & sDevice, in_addr_t nIP4) {
    
    int nExitCode = 0;    
    int s = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
    ifreq cIFReq;
    
    memset(&cIFReq, 0, sizeof(cIFReq));
    strncpy(cIFReq.ifr_name, sDevice.c_str(), IFNAMSIZ);
    
    sockaddr_in cAddr;
    memset(&cAddr, 0, sizeof(cAddr));
    cAddr.sin_family = AF_INET;
    cAddr.sin_port = 0;
    cAddr.sin_addr.s_addr = nIP4;
    memcpy((char *)&cIFReq + offsetof(struct ifreq, ifr_addr), (char *)&cAddr, sizeof(struct sockaddr));

    nExitCode = ioctl(s, SIOCSIFADDR, &cIFReq);
    close(s);
    if (nExitCode != 0) {
        qkd::utility::syslog::warning() << "Failed to assign IP4 '" << inet_addr_to_string(nIP4) << "' to interface " << sDevice << ": error code = " << errno << " - " << strerror(errno);
        return false;
    }

    return true;
}


/**
 * quick and dirty in_addr_t to string
 * 
 * (because standard POSIX stuff is plain stupid with all
 * that struct inaddr, in_addr_t, s_addr, ... what a mess!)
 * 
 * @param   nIP4        the IP4 address
 * @return  string in the 4 digit-dot notation
 */
std::string inet_addr_to_string(in_addr_t nIP4) {
    
    std::stringstream ss;
    
    char * c = (char *)&nIP4;
    ss << (int)c[0] << "." << (int)c[1] << "." << (int)c[2] << "." << (int)c[3];
    
    return ss.str();
}


/**
 * get up the tun (from tun/tap) device
 * 
 * @param   nDeviceFD       device file descriptor
 * @param   sDeviceName     name of new device
 * @return  true, if device has been successully setup
 */
bool init_tun(int & nDeviceFD, std::string & sDeviceName) {
    
    nDeviceFD = ::open("/dev/net/tun", O_RDWR);
    if (nDeviceFD < 0) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "could not access /dev/net/tun: " << strerror(errno);
        return false;
    }
    
    struct ifreq cIFReq;
    memset(&cIFReq, 0, sizeof(cIFReq));
    cIFReq.ifr_flags = IFF_TUN;
    
    strncpy(cIFReq.ifr_name, "q3p%d", IFNAMSIZ);
    if (ioctl(nDeviceFD, TUNSETIFF, (void *)&cIFReq) == -1) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "could not create TUN device: " << strerror(errno);
        close(nDeviceFD);
        nDeviceFD = 0;
        return false;
    }
    
    sDeviceName = cIFReq.ifr_name;
    qkd::utility::syslog::info() << "created TUN device: " << sDeviceName;
    
    return true;
}


/**
 * apply network device flags properly
 * 
 * @param   sDevice         the device
 */
void set_device_flags(std::string const & sDevice) {

    int nExitCode = 0;    
    int s = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
    ifreq cIFReq;
    
    memset(&cIFReq, 0, sizeof(cIFReq));
    strncpy(cIFReq.ifr_name, sDevice.c_str(), IFNAMSIZ);
    
    short nFlags = (IFF_UP | IFF_POINTOPOINT | IFF_RUNNING | IFF_NOARP | IFF_MULTICAST);
    memcpy((char *)&cIFReq + offsetof(struct ifreq, ifr_flags), (char *)&nFlags, sizeof(short));

    nExitCode = ioctl(s, SIOCSIFFLAGS, &cIFReq);
    close(s);
    if (nExitCode != 0) {
        qkd::utility::syslog::warning() << "Failed to set device flags to interface " << sDevice << ": error code = " << errno << " - " << strerror(errno);
        return;
    }
    
}

#endif

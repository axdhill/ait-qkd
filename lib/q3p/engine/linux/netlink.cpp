/*
 * netlink.cpp
 * 
 * interface to Linux kernel netlink
 *
 * Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *
 * Copyright (C) 2016 AIT Austrian Institute of Technology
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


// this is only Linux code
#if defined(__linux__)
 

/*
 * Netlink interaction inspired by 
 * http://stackoverflow.com/questions/3288065/getting-gateway-to-use-for-a-given-ip-in-ansi-c#3288983
 */


// ------------------------------------------------------------
// incs


#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#include <boost/format.hpp>

// ait
#include <qkd/utility/debug.h>
#include <qkd/utility/memory.h>
#include <qkd/common_macros.h>

#include "netlink.h"
#include "netlink_base.h"
#include "netlink_ifinfomsg.h"
#include "netlink_message.h"
#include "netlink_nlmsghdr.h"
#include "netlink_rtattr.h"
#include "netlink_rtmsg.h"

using namespace qkd::q3p;


// ------------------------------------------------------------
// vars


/**
 * static netlink debug flag
 */
bool netlink::m_bDebug = true;


/**
 * debug netlink kernel blobs send/recv
 */
bool netlink::m_bDebugMessageBlobs = true;


// ------------------------------------------------------------
// decl


/**
 * receive from the netlink layer
 * 
 * @param   cSocket             the netlink socket to use
 * @param   nPort               port number (usually PID of current process)
 * @param   nMessageNumber      the message number to receive
 * @param   cBuffer             the buffer which receives the netlink messages
 * @param   nBufferSize         size of the buffer to write to
 * @return  number of bytes received (-1 in case of error)
 */
static int netlink_recv(qkd::q3p::netlink::socket cSocket, unsigned int nPort, uint32_t nMessageNumber, char * cBuffer, size_t nBufferSize);

    
/**
 * receive from the netlink layer
 * 
 * @param   cSocket             the netlink socket to use
 * @param   nPort               port number (usually PID of current process)
 * @param   nMessageNumber      the message number to receive
 * @param   cMessage            the message object to be filled
 * @return  number of bytes received (-1 in case of error)
 */
static int netlink_recv(qkd::q3p::netlink::socket cSocket, unsigned int nPort, uint32_t nMessageNumber, qkd::q3p::netlink_message & cMessage);

    
/**
 * send a netlink message to the kernel
 * 
 * @param   cSocket             the netlink socket to use
 * @param   cNetlinkMessage     the message to be sent
 * @return  message number sent (or 0 in case or error)
 */
UNUSED static __u32 netlink_send(qkd::q3p::netlink::socket cSocket, void * cNetlinkMessage);


/**
 * send a netlink message to the kernel
 * 
 * @param   cSocket             the netlink socket to use
 * @param   cMessage            the message to be sent
 * @return  message number sent (or 0 in case or error)
 */
static uint32_t netlink_send(qkd::q3p::netlink::socket cSocket, qkd::q3p::netlink_message const & cMessage);


/**
 * parse a netlink route answer
 * 
 * @param   cNetlinkMessage         the netlink message holding the route info
 * @param   cRoute                  [out] netlink route information parsed
 * @return  true, if succesfully parsed
 */
UNUSED static bool parse_netlink_route(nlmsghdr * cNetlinkMessage, qkd::q3p::netlink::route & cRoute);


/**
 * pick all routing attributes in a netlink routing message and place them in a table
 * 
 * @param   cRoutingAttributeTable          [out] the table to place attributes to
 * @param   nTableSize                      maximum table size
 * @param   cRoutingAttribute               pointer to first routing attribute of a netlink message
 * @param   nMessageSize                    size of netlink message (counted from the first routing attribute)
 */
UNUSED static void pick_routing_attributes(rtattr * cRoutingAttributeTable[], int nTableSize, struct rtattr * cRoutingAttribute, int nMessageSize);


// ------------------------------------------------------------
// code


/**
 * describe route as string
 * 
 * @return  a human readble string for the route
 */
std::string netlink::route::str() const {
    boost::format cFormat("from: %-15s to: %-15s gw: %-15s dev: %s");
    cFormat % inet_ntoa(cSrcAddress) % inet_ntoa(cDstAddress) % inet_ntoa(cGateway) % sInterface;
    return cFormat.str();
}


/**
 * ctor
 */
netlink::netlink() {
    
    m_cNetlinkRouteSocket.nSocket = ::socket(PF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE);
    if (m_cNetlinkRouteSocket.nSocket == -1) {
        qkd::utility::debug(netlink::debug()) << "Failed to create netlink route socket. Error: " << strerror(errno);
    }
    m_cNetlinkRouteSocket.nSequenceNumber = 0;
}


/**
 * dtor
 */
netlink::~netlink() {
    if (m_cNetlinkRouteSocket.nSocket == -1) {
        close(m_cNetlinkRouteSocket.nSocket);
    }
}


/**
 * add a route to the kernel routing table
 * 
 * @param   cRoute      the route to add
 * @return  true on success
 */
bool netlink::add_route(UNUSED route const & cRoute) {
    
    UNUSED netlink::routing_table cRoutingTable = get_routing_table();
    
    return false;
}


/**
 * remove a route from the kernel routing table
 * 
 * @param   cRoute      the route to remove
 * @return  true on success
 */
bool netlink::del_route(UNUSED route const & cRoute) {
    
    return false;
}


/**
 * get the current kernel main routing table
 * 
 * @return  the current routing table
 */
netlink::routing_table netlink::get_routing_table() {
    
    netlink::routing_table res;
    
    // ---- send netlink request ----
    
    netlink_message cQuery;
    
    netlink_nlmsghdr cNetlinkMessageHeader;
    cNetlinkMessageHeader->nlmsg_type = RTM_GETROUTE;
    cNetlinkMessageHeader->nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST;
    cQuery.add(cNetlinkMessageHeader);
    
    netlink_ifinfomsg cInterfaceInfo;
    cInterfaceInfo->ifi_family = AF_INET;
    cQuery.add(cInterfaceInfo);
    
    netlink_rtattr cRouteAttribute(cNetlinkMessageHeader->nlmsg_type, sizeof(uint32_t));
    cRouteAttribute->rta_type = IFLA_EXT_MASK;
    *((uint32_t *)cRouteAttribute.value()) = RTEXT_FILTER_VF;
    cQuery.add(cRouteAttribute);
    
    uint32_t nMessageId = netlink_send(m_cNetlinkRouteSocket, cQuery);
    if (!nMessageId) {
        return res;
    }
    
    // ---- recv netlink answer ----
    
    netlink_message cAnswer;
    netlink_recv(m_cNetlinkRouteSocket, cNetlinkMessageHeader->nlmsg_pid, nMessageId, cAnswer);
    
//     char cRecvBuffer[16384];
//     do {
//     
//         int nRead = netlink_recv(m_cNetlinkRouteSocket, cNetlinkMessageHeader->nlmsg_pid, nMessageId, cRecvBuffer, sizeof(cRecvBuffer));
//         if (nRead <= 0) {
//             return netlink::routing_table();
//         }
//         
//         nlmsghdr * cNetlinkMessage = (struct nlmsghdr*)cRecvBuffer;
//         while (NLMSG_OK(cNetlinkMessage, nRead)) {
//             
//             if (cNetlinkMessage->nlmsg_type == NLMSG_DONE) {
//                 break;
//             }
//             
//             if (cNetlinkMessage->nlmsg_type == NLMSG_ERROR) {
//                 
//                 nlmsgerr * cError = (nlmsgerr *)NLMSG_DATA(cNetlinkMessage);
//                 if (cNetlinkMessage->nlmsg_len < NLMSG_LENGTH(sizeof(struct nlmsgerr))) {
//                     qkd::utility::debug(netlink::debug()) << "ERROR truncated";
//                 } 
//                 else {
//                     qkd::utility::debug(netlink::debug()) << "Error in parsing netlink message. Error: " << strerror(-cError->error);
//                 }
//             }
//             else {
// 
//                 netlink::route cRoute;
//                 if (parse_netlink_route(cNetlinkMessage, cRoute)) {
// qkd::utility::debug(true) << __DEBUG_LOCATION__ << "cRoute=" << cRoute.str();
//                 }
//             }
//             
//             cNetlinkMessage = NLMSG_NEXT(cNetlinkMessage, nRead);
//         }
//    
//    } while (true);
    
    
    return res;
}


/**
 * get the netlink singelton
 * 
 * @return  the netlink singelton instance
 */
netlink & netlink::instance() {
    static netlink cNetlink;
    return cNetlink;
}


/**
 * receive from the netlink layer
 * 
 * @param   cSocket             the netlink socket to use
 * @param   nPort               port number (usually PID of current process)
 * @param   nMessageNumber      the message number to receive
 * @param   cBuffer             the buffer which receives the netlink messages
 * @param   nBufferSize         size of the buffer to write to
 * @return  number of bytes received (-1 in case of error)
 */
int netlink_recv(qkd::q3p::netlink::socket cSocket, unsigned int nPort, uint32_t nMessageNumber, char * cBuffer, size_t nBufferSize) {
    
    if (cSocket.nSocket == -1) {
        qkd::utility::debug(netlink::debug()) << "Refused to receive netlink message on invalid socket.";
        return 0;
    }
    
    int nMessageLen = 0;
    char cReceiveBuffer[16384];
    
    iovec cIOV;
    cIOV.iov_base = cReceiveBuffer ;
    cIOV.iov_len = sizeof(cReceiveBuffer);
    
    sockaddr_nl cNlAddr;
    msghdr cMsg;
    cMsg.msg_name = &cNlAddr;
    cMsg.msg_namelen = sizeof(cNlAddr);
    cMsg.msg_iov = &cIOV;
    cMsg.msg_iovlen = 1;
    
    char * cDestinationBuffer = cBuffer;

qkd::utility::debug() << __DEBUG_LOCATION__;
    
    while (true) {
        
qkd::utility::debug() << __DEBUG_LOCATION__;

        int nRead = recvmsg(cSocket.nSocket, &cMsg, 0);
qkd::utility::debug() << __DEBUG_LOCATION__ << "nRead=" << nRead;
        
        if (nRead < 0) {
            
            if (errno == EINTR || errno == EAGAIN) {
                continue;
            }
            qkd::utility::debug(netlink::debug()) << "Failed to read from netlink socket. Error: " << strerror(errno);
            if (errno == ENOBUFS) {
                continue;
            }
            return -1;
        }
        
        if (nRead == 0) {
            qkd::utility::debug(netlink::debug()) << "EOF on netlink";
            return nMessageLen;
        }
        
        if ((unsigned int)nRead < sizeof(nlmsghdr)) {
            qkd::utility::debug(netlink::debug()) << "Netlink receive message is less than minimum nlmsghdr";
            return -1;
        }

        nlmsghdr * cNetlinkMessage = (nlmsghdr *)cReceiveBuffer;
        if ((NLMSG_OK(cNetlinkMessage, nRead) == 0) || (cNetlinkMessage->nlmsg_type == NLMSG_ERROR)) {
            qkd::utility::debug(netlink::debug()) << "Error in received netlink message. Error: " << strerror(errno);
            return -1;
        }
        
        if ((cNetlinkMessage->nlmsg_seq != nMessageNumber) || (cNetlinkMessage->nlmsg_pid != nPort)) {
            qkd::utility::debug(netlink::debug()) << "Dropping kernel packet for wrong sequence number and/or wrong port id";
            continue;
        }
        
        if ((size_t)nRead >= nBufferSize) {
            qkd::utility::debug(netlink::debug()) << "Receiver buffer too small";
            return -1;
        }
        memcpy(cDestinationBuffer, cReceiveBuffer, nRead);
        cDestinationBuffer += nRead;
        nBufferSize -= nRead;
        nMessageLen += nRead;
        
        if (cNetlinkMessage->nlmsg_type == NLMSG_DONE) {
            break;
        }
        
        if ((cNetlinkMessage->nlmsg_flags & NLM_F_MULTI) == 0) {
            break;
        }
    }
    
    if (netlink::debug_message_blobs()) {
        qkd::utility::memory cRecvMessage = qkd::utility::memory::wrap((qkd::utility::memory::value_t *)cBuffer, nMessageLen);
        qkd::utility::debug(netlink::debug()) << "netlink recv (" << nMessageLen << " bytes):\n" 
                << cRecvMessage.canonical("    ");
    }
    
    return nMessageLen;
}
        

/**
 * receive from the netlink layer
 * 
 * @param   cSocket             the netlink socket to use
 * @param   nPort               port number (usually PID of current process)
 * @param   nMessageNumber      the message number to receive
 * @param   cMessage            the message object to be filled
 * @return  number of bytes received (-1 in case of error)
 */
int netlink_recv(qkd::q3p::netlink::socket cSocket, unsigned int nPort, uint32_t nMessageNumber, UNUSED qkd::q3p::netlink_message & cMessage) {
    
    static uint32_t const nBufferSize = 16384;
    char cBuffer[nBufferSize];
    int res = netlink_recv(cSocket, nPort, nMessageNumber, cBuffer, nBufferSize);
    
    return res;
}

    
/**
 * send a netlink message to the kernel
 * 
 * @param   nSocket             the netlink socket to use
 * @param   cNetlinkMessage     the message to be sent
 * @return  message number sent (or 0 in case or error)
 */
__u32 netlink_send(qkd::q3p::netlink::socket cSocket, void * cNetlinkMessage) {
    
    if (cSocket.nSocket == -1) {
        qkd::utility::debug(netlink::debug()) << "Refused to send netlink message on invalid socket.";
        return 0;
    }
    if (cNetlinkMessage == nullptr) {
        qkd::utility::debug(netlink::debug()) << "Refused to send NULL netlink message.";
        return 0;
    }
    
    cSocket.nSequenceNumber++;
    if (!cSocket.nSequenceNumber) ++cSocket.nSequenceNumber;
    
    nlmsghdr * cNtMsg = (nlmsghdr *)cNetlinkMessage;
    cNtMsg->nlmsg_seq = cSocket.nSequenceNumber;
    
    // nlmsg_pid may not be getpid()... 
    // this is only true for the first netlink socket of the process
    cNtMsg->nlmsg_pid = getpid();
    
    if (::send(cSocket.nSocket, cNtMsg, cNtMsg->nlmsg_len, 0) < 0) {
        qkd::utility::debug(netlink::debug()) << "Failed to send netlink message. Error: " << strerror(errno);
        return 0;
    }
    
    if (netlink::debug_message_blobs()) {
        qkd::utility::memory cSentMessage = qkd::utility::memory::wrap((qkd::utility::memory::value_t *)cNtMsg, cNtMsg->nlmsg_len);
        qkd::utility::debug(netlink::debug()) << "netlink sent (" << cNtMsg->nlmsg_len << " bytes):\n" 
                << cSentMessage.canonical("    ");
    }
    
    return cSocket.nSequenceNumber;
}


/**
 * send a netlink message to the kernel
 * 
 * @param   cSocket             the netlink socket to use
 * @param   cMessage            the message to be sent
 * @return  message number sent (or 0 in case or error)
 */
uint32_t netlink_send(qkd::q3p::netlink::socket cSocket, qkd::q3p::netlink_message const & cMessage) {
    
    if (cSocket.nSocket == -1) {
        qkd::utility::debug(netlink::debug()) << "Refused to send netlink message on invalid socket.";
        return 0;
    }
    if (cMessage.size() == 0) {
        qkd::utility::debug(netlink::debug()) << "Refused to send empty netlink message.";
        return 0;
    }
    
    ++cSocket.nSequenceNumber;
    if (!cSocket.nSequenceNumber) ++cSocket.nSequenceNumber;
    
    netlink_nlmsghdr * cMessageHeader = (netlink_nlmsghdr *)cMessage.front().get();
    if (cMessageHeader->name() != "nlmsghdr") {
        throw std::runtime_error("no nlmsghdr as first argument of netlink message");
    }
    (*cMessageHeader)->nlmsg_seq = cSocket.nSequenceNumber;
    
    // nlmsg_pid may not be getpid()... 
    // this is only true for the first netlink socket of the process
    // nlmsg_pid is really "port id" not "process id"
    // thus, this may fail, if we use several sockets for the same port id...
    (*cMessageHeader)->nlmsg_pid = getpid();
    
    std::unique_ptr<iovec[]> cIOVec(new iovec[cMessage.size()]);
    int i = 0;
    for (auto const & p : cMessage) {
        cIOVec.get()[i].iov_base = p->data();
        cIOVec.get()[i].iov_len = p->size();
        ++i;
    }
    
    msghdr cMsgHdr;
    memset(&cMsgHdr, 0, sizeof(cMsgHdr));
    cMsgHdr.msg_iov = cIOVec.get();
    cMsgHdr.msg_iovlen = cMessage.size();
    
    if (::sendmsg(cSocket.nSocket, &cMsgHdr, 0) < 0) {
        qkd::utility::debug(netlink::debug()) << "Failed to send netlink message. Error: " << strerror(errno);
        return 0;
    }
    
    if (netlink::debug_message_blobs()) {
        qkd::utility::debug(netlink::debug()) << cMessage.str();
    }

    return cSocket.nSequenceNumber; 
}


/**
 * parse a netlink route answer
 * 
 * @param   cNetlinkMessage         the netlink message holding the route info
 * @param   cRoute                  [out] netlink route information parsed
 * @return  true, if succesfully parsed
 */
bool parse_netlink_route(nlmsghdr * cNetlinkMessage, UNUSED qkd::q3p::netlink::route & cRoute) {
    
qkd::utility::debug(true) << __DEBUG_LOCATION__;    

    if (cNetlinkMessage == nullptr) return false;
    
    if ((cNetlinkMessage->nlmsg_type != RTM_NEWROUTE) && (cNetlinkMessage->nlmsg_type != RTM_DELROUTE)) {
        qkd::utility::debug(netlink::debug()) << "While parsing netlink route info: not a route netlink message";
        return false;
    }
    
    rtmsg * cRoutingMessage = (rtmsg *)NLMSG_DATA(cNetlinkMessage);
    int nMessageSize = cNetlinkMessage->nlmsg_len - NLMSG_LENGTH(sizeof(*cRoutingMessage));
    
    rtattr * cRoutingAttribute[RTA_MAX + 1];
    pick_routing_attributes(cRoutingAttribute, RTA_MAX + 1, RTM_RTA(cRoutingMessage), nMessageSize);
    
    if (cRoutingAttribute[RTA_DST]) {
        cRoute.cDstAddress.s_addr = *(unsigned int const *)RTA_DATA(cRoutingAttribute[RTA_DST]);
    }
    if (cRoutingAttribute[RTA_SRC]) {
        cRoute.cSrcAddress.s_addr = *(unsigned int const *)RTA_DATA(cRoutingAttribute[RTA_SRC]);
    }
    if (cRoutingAttribute[RTA_GATEWAY]) {
        cRoute.cGateway.s_addr = *(unsigned int const *)RTA_DATA(cRoutingAttribute[RTA_GATEWAY]);
    }
    if (cRoutingAttribute[RTA_OIF]) {
        char sIFName[128];
        if_indextoname(*((int *)RTA_DATA(cRoutingAttribute[RTA_OIF])), sIFName);
        cRoute.sInterface = std::string(sIFName);
    }
    
qkd::utility::debug(true) << __DEBUG_LOCATION__;    

    return true;
}


/**
 * pick all routing attributes in a netlink routing message and place them in a table
 * 
 * @param   cRoutingAttributeTable          [out] the table to place attributes to
 * @param   nTableSize                      maximum table size
 * @param   cRoutingAttribute               pointer to first routing attribute of a netlink message
 * @param   nMessageSize                    size of netlink message (counted from the first routing attribute)
 */
void pick_routing_attributes(rtattr * cRoutingAttributeTable[], int nTableSize, struct rtattr * cRoutingAttribute, int nMessageSize) {
    
    memset(cRoutingAttributeTable, 0, sizeof(rtattr *) * nTableSize);
    while (RTA_OK(cRoutingAttribute, nMessageSize)) {
        
        if (cRoutingAttribute->rta_type <= nTableSize) {
            cRoutingAttributeTable[cRoutingAttribute->rta_type] = cRoutingAttribute;
        }
        
        cRoutingAttribute = RTA_NEXT(cRoutingAttribute, nMessageSize);
    }
}    


#endif

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

// ait
#include <qkd/utility/debug.h>
#include <qkd/utility/memory.h>
#include <qkd/utility/syslog.h>
#include <qkd/common_macros.h>

#include "netlink.h"
#include "netlink_base.h"
#include "netlink_ifinfomsg.h"
#include "netlink_message.h"
#include "netlink_nlmsghdr.h"
#include "netlink_parser.h"
#include "netlink_rtattr.h"
#include "netlink_rtmsg.h"

using namespace qkd::q3p;


// ------------------------------------------------------------
// vars


/**
 * static netlink debug flag
 */
bool netlink::m_bDebug = false;


/**
 * debug netlink kernel blobs send/recv
 */
bool netlink::m_bDebugMessageBlobs = false;


// ------------------------------------------------------------
// decl

    
/**
 * receive from the netlink layer
 * 
 * @param   cSocket             the netlink socket to use
 * @param   nPort               port number (usually PID of current process)
 * @param   nMessageNumber      the message number to receive
 * @param   cMessage            the message object to be filled
 * @return  number of bytes received (-1 in case of error)
 */
static int netlink_recv(qkd::q3p::netlink::socket & cSocket, unsigned int nPort, uint32_t nMessageNumber, qkd::q3p::netlink_message & cMessage);

    
/**
 * send a netlink message to the kernel
 * 
 * @param   cSocket             the netlink socket to use
 * @param   cMessage            the message to be sent
 * @return  message number sent (or 0 in case or error)
 */
static uint32_t netlink_send(qkd::q3p::netlink::socket & cSocket, qkd::q3p::netlink_message const & cMessage);


// ------------------------------------------------------------
// code


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
bool netlink::add_route(route const & cRoute) {

    // ---- send netlink add route request ----
    
    netlink_message cQuery;
    
    netlink_nlmsghdr cNetlinkMessageHeader;
    cNetlinkMessageHeader->nlmsg_type = RTM_NEWROUTE;
    cNetlinkMessageHeader->nlmsg_flags = NLM_F_EXCL | NLM_F_CREATE | NLM_F_REQUEST | NLM_F_ACK;
    cNetlinkMessageHeader->nlmsg_pid = getpid();
    cQuery.add(cNetlinkMessageHeader);
    
    netlink_rtmsg cRoutingMessage;
    cRoutingMessage->rtm_family = AF_INET;
    cRoutingMessage->rtm_dst_len = 32;
    cRoutingMessage->rtm_src_len = 0;
    cRoutingMessage->rtm_table = RT_TABLE_MAIN;
    cRoutingMessage->rtm_protocol = RTPROT_STATIC;
    cRoutingMessage->rtm_scope = RT_SCOPE_LINK;
    cRoutingMessage->rtm_type = RTN_UNICAST;
    cQuery.add(cRoutingMessage);
    
    netlink_rtattr cRtAttrDst(RTM_NEWROUTE, sizeof(in_addr));
    cRtAttrDst->rta_type = RTA_DST;
    memcpy(cRtAttrDst.value(), &cRoute.m_cDstAddress, sizeof(in_addr));
    cQuery.add(cRtAttrDst);
    
    netlink_rtattr cRtAttrPrefSrc(RTM_NEWROUTE, sizeof(in_addr));
    cRtAttrPrefSrc->rta_type = RTA_GATEWAY;
    memcpy(cRtAttrPrefSrc.value(), &cRoute.m_cSrcAddress, sizeof(in_addr));
    cQuery.add(cRtAttrPrefSrc);
    
    uint32_t nMessageId = netlink_send(m_cNetlinkRouteSocket, cQuery);
    if (!nMessageId) {
        return false;
    }
    
    // ---- recv netlink answer ----
    
    netlink_message cAnswer;
    if (netlink_recv(m_cNetlinkRouteSocket, cNetlinkMessageHeader->nlmsg_pid, nMessageId, cAnswer) > 0) {
        
        int nError = cAnswer.error();
        if (nError == 0) {
            qkd::utility::debug(netlink::debug()) << "Added route: " << cRoute.str();
            return true;
        }
        
        if (nError > 0) {
            qkd::utility::debug(netlink::debug()) << "Failed to set new route: received unknown netlink message answer (internal error).";
        }
        if (nError < 0) {
            qkd::utility::debug(netlink::debug()) << "Failed to add new route: netlink error code: " << nError;
        }
    }        
    
    return false;
}


/**
 * remove a route from the kernel routing table
 * 
 * @param   cRoute      the route to remove
 * @return  true on success
 */
bool netlink::del_route(route const & cRoute) {
    
    // ---- send netlink del route request ----
    
    netlink_message cQuery;
    
    netlink_nlmsghdr cNetlinkMessageHeader;
    cNetlinkMessageHeader->nlmsg_type = RTM_DELROUTE;
    cNetlinkMessageHeader->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    cNetlinkMessageHeader->nlmsg_pid = getpid();
    cQuery.add(cNetlinkMessageHeader);
    
    netlink_rtmsg cRoutingMessage;
    cRoutingMessage->rtm_family = AF_INET;
    cRoutingMessage->rtm_dst_len = 32;
    cRoutingMessage->rtm_src_len = 0;
    cRoutingMessage->rtm_table = RT_TABLE_MAIN;
    cRoutingMessage->rtm_scope = RT_SCOPE_NOWHERE;
    cQuery.add(cRoutingMessage);
    
    netlink_rtattr cRtAttrDst(RTM_DELROUTE, sizeof(in_addr));
    cRtAttrDst->rta_type = RTA_DST;
    memcpy(cRtAttrDst.value(), &cRoute.m_cDstAddress, sizeof(in_addr));
    cQuery.add(cRtAttrDst);
    
    netlink_rtattr cRtAttrPrefSrc(RTM_DELROUTE, sizeof(in_addr));
    cRtAttrPrefSrc->rta_type = RTA_GATEWAY;
    memcpy(cRtAttrPrefSrc.value(), &cRoute.m_cSrcAddress, sizeof(in_addr));
    cQuery.add(cRtAttrPrefSrc);

    uint32_t nMessageId = netlink_send(m_cNetlinkRouteSocket, cQuery);
    if (!nMessageId) {
        return false;
    }
    
    // ---- recv netlink answer ----
    
    netlink_message cAnswer;
    if (netlink_recv(m_cNetlinkRouteSocket, cNetlinkMessageHeader->nlmsg_pid, nMessageId, cAnswer) > 0) {
        
        int nError = cAnswer.error();
        if (nError == 0) {
            qkd::utility::debug(netlink::debug()) << "Removed route: " << cRoute.str();
            return true;
        }
        
        if (nError > 0) {
            qkd::utility::debug(netlink::debug()) << "Failed to delete route: received unknown netlink message answer (internal error).";
        }
        if (nError < 0) {
            qkd::utility::debug(netlink::debug()) << "Failed to delete route: netlink error code: " << nError;
        }
    }        
    
    return false;
}


/**
 * get the current kernel main routing table
 * 
 * @return  the current routing table
 */
routing_table netlink::get_routing_table() {
    
    routing_table res;
    
    // ---- send netlink request ----
    
    netlink_message cQuery;
    
    netlink_nlmsghdr cNetlinkMessageHeader;
    cNetlinkMessageHeader->nlmsg_type = RTM_GETROUTE;
    cNetlinkMessageHeader->nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST;
    cNetlinkMessageHeader->nlmsg_pid = getpid();
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
    if (netlink_recv(m_cNetlinkRouteSocket, cNetlinkMessageHeader->nlmsg_pid, nMessageId, cAnswer) > 0) {

        route cRoute;
        
        netlink_rtmsg * cRtMsg = nullptr;
        netlink_rtattr * cRtAttr = nullptr;
            
        for (auto const & m : cAnswer) {
            
            if (m->name() == "nlmsghdr") {
                if (!cRoute.empty()) {
                    res.push_back(cRoute);
                    cRoute = route();
                }
            }
            
            if (m->name() == "rtmsg") {
                cRtMsg = (netlink_rtmsg *)m.get();
                cRoute.m_nDstHostLen = (*cRtMsg)->rtm_dst_len;
                cRoute.m_nSrcHostLen = (*cRtMsg)->rtm_src_len;
            }
            
            if (m->name() == "rtattr") {
                
                char sIf[IF_NAMESIZE];
                
                cRtAttr = (netlink_rtattr *)m.get();
                switch ((*cRtAttr)->rta_type) {
                    
                case RTA_SRC:
                case RTA_PREFSRC:
                    cRoute.m_cSrcAddress = *((in_addr *)cRtAttr->value());
                    break;
                    
                case RTA_DST:
                    cRoute.m_cDstAddress = *((in_addr *)cRtAttr->value());
                    break;
                        
                case RTA_GATEWAY:
                    cRoute.m_cGateway = *((in_addr *)cRtAttr->value());
                    break;
                    
                case RTA_IIF:
                case RTA_OIF:
                    cRoute.m_nInterface = *((int *)cRtAttr->value());
                    cRoute.m_sInterface = if_indextoname(cRoute.m_nInterface, sIf);
                    break;
                    
                case RTA_PRIORITY:
                    cRoute.m_nPriority = *((int *)cRtAttr->value());
                    break;
                    
                case RTA_METRICS:
                    cRoute.m_nMetrics = *((int *)cRtAttr->value());
                    break;
                }
            }
        }
        
        if (!cRoute.empty()) {
            res.push_back(cRoute);
        }
    }
    
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
 * @param   cMessage            the message object to be filled
 * @return  number of bytes received (-1 in case of error)
 */
int netlink_recv(qkd::q3p::netlink::socket & cSocket, unsigned int nPort, uint32_t nMessageNumber, UNUSED qkd::q3p::netlink_message & cMessage) {
    
    if (cSocket.nSocket == -1) {
        qkd::utility::debug(netlink::debug()) << "Refused to receive netlink message on invalid socket.";
        return 0;
    }
    
    int nMessageLen = 0;
    static uint32_t const nBufferSize = 16384;
    char cBuffer[16384];
    
    iovec cIOV;
    cIOV.iov_base = cBuffer ;
    cIOV.iov_len = nBufferSize;
    
    sockaddr_nl cNlAddr;
    msghdr cMsg;
    cMsg.msg_name = &cNlAddr;
    cMsg.msg_namelen = sizeof(cNlAddr);
    cMsg.msg_iov = &cIOV;
    cMsg.msg_iovlen = 1;
    
    std::shared_ptr<netlink_parser> cParser;
    
    while (true) {
        
        int nRead = recvmsg(cSocket.nSocket, &cMsg, 0);
        
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

        nlmsghdr * cNlMsgHdr = (nlmsghdr *)cBuffer;
        netlink_nlmsghdr cNetlinkMessage(*cNlMsgHdr);
        
        if (NLMSG_OK(cNlMsgHdr, nRead) == 0) {
            qkd::utility::debug(netlink::debug()) << "Error in received netlink message. Error: " << strerror(errno);
            return -1;
        }
        if ((cNetlinkMessage->nlmsg_seq != nMessageNumber) || (cNetlinkMessage->nlmsg_pid != nPort)) {
            qkd::utility::debug(netlink::debug()) << "Dropping kernel packet for wrong sequence number and/or wrong port id";
            continue;
        }
        
        nMessageLen += nRead;
        
        if (cNetlinkMessage->nlmsg_type == NLMSG_ERROR) {
            netlink_parser::create(NLMSG_ERROR)->parse(cMessage, cBuffer, nRead);
            nMessageLen = nRead;
            break;
        }
        
        if (!cParser) {
            cParser = netlink_parser::create(cNetlinkMessage->nlmsg_type);
        }
        cParser->parse(cMessage, cBuffer, nRead);
        
        if (cNetlinkMessage->nlmsg_type == NLMSG_DONE) {
            break;
        }
        
        if ((cNetlinkMessage->nlmsg_flags & NLM_F_MULTI) == 0) {
            break;
        }
    }
    
    if (netlink::debug_message_blobs()) {
        qkd::utility::debug(netlink::debug()) << "netlink recv: " << cMessage.str();
    }
    
    return nMessageLen;
}

    
/**
 * send a netlink message to the kernel
 * 
 * @param   cSocket             the netlink socket to use
 * @param   cMessage            the message to be sent
 * @return  message number sent (or 0 in case or error)
 */
uint32_t netlink_send(qkd::q3p::netlink::socket & cSocket, qkd::q3p::netlink_message const & cMessage) {
    
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
    uint32_t nTotalSize = 0;
    for (auto const & p : cMessage) {
        cIOVec.get()[i].iov_base = p->data();
        cIOVec.get()[i].iov_len = p->size();
        nTotalSize += p->size();
        ++i;
    }
    (*cMessageHeader)->nlmsg_len = nTotalSize;
    
    msghdr cMsgHdr;
    memset(&cMsgHdr, 0, sizeof(cMsgHdr));
    cMsgHdr.msg_iov = cIOVec.get();
    cMsgHdr.msg_iovlen = cMessage.size();
    
    if (::sendmsg(cSocket.nSocket, &cMsgHdr, 0) < 0) {
        qkd::utility::debug(netlink::debug()) << "Failed to send netlink message. Error: " << strerror(errno);
        return 0;
    }
    
    if (netlink::debug_message_blobs()) {
        qkd::utility::debug(netlink::debug()) << "netlink sent: " << cMessage.str();
    }

    return cSocket.nSequenceNumber; 
}


#endif

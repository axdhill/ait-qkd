/* 
 * module_internal.cpp
 * 
 * QKD module internal implementation
 *
 * Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *
 * Copyright (C) 2015 AIT Austrian Institute of Technology
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


#include <boost/format.hpp>

#include <qkd/module/module.h>
#include <qkd/utility/syslog.h>

#include "module_internal.h"

using namespace qkd::module;


// ------------------------------------------------------------
// code


/**
 * ctor
 * 
 * @param   cParentModule       the parent module of this inner module
 * @param   sId                 module id
 */
module::module_internal::module_internal(module * cParentModule, std::string sId) : cModule(cParentModule), sId(sId), nStartTimeStamp(0), cStash(nullptr) { 
    
    // default values
    
    eRole = module_role::ROLE_ALICE;
    eType = module_type::TYPE_OTHER;
    
    cRandom = qkd::utility::random_source::source();
    
    cLastProcessedKey = std::chrono::system_clock::now() - std::chrono::hours(1);
    cModuleBirth = std::chrono::high_resolution_clock::now();
    bProcessing = false;
    bDebugMessageFlow = false;

    nTerminateAfter = 0;
    
    cConListen = new qkd::module::connection(connection_type::LISTEN);
    cConPeer = new qkd::module::connection(connection_type::PEER);
    cConPipeIn = new qkd::module::connection(connection_type::PIPE_IN);
    cConPipeOut = new qkd::module::connection(connection_type::PIPE_OUT);

    cConPipeIn->add("stdin://");
    cConPipeOut->add("stdout://");
    
    cStash = new qkd::module::stash(cParentModule);
}


/**
 * dtor
 */
module::module_internal::~module_internal() {
    if (cStash != nullptr) delete cStash;
    cStash = nullptr;
    if (cConPipeOut != nullptr) delete cConPipeOut;
    cConPipeOut = nullptr;
    if (cConPipeIn != nullptr) delete cConPipeIn;
    cConPipeIn = nullptr;
    if (cConPeer != nullptr) delete cConPeer;
    cConPeer = nullptr;
    if (cConListen != nullptr) delete cConListen;
    cConListen = nullptr;
}


/**
 * add key statistics for incoming
 * 
 * @param   cKey        new key arrived
 */
void module::module_internal::add_stats_incoming(qkd::key::key const & cKey) {

    std::lock_guard<std::recursive_mutex> cLock(cStat.cMutex);

    cStat.nKeysIncoming++;
    cStat.nKeyBitsIncoming += cKey.size() * 8;
    cStat.nDisclosedBitsIncoming += cKey.meta().nDisclosedBits;
    cStat.cKeysIncomingRate << cStat.nKeysIncoming;
    cStat.cKeyBitsIncomingRate << cStat.nKeyBitsIncoming;
    cStat.cDisclosedBitsIncomingRate << cStat.nDisclosedBitsIncoming;
}


/**
 * add key statistics for outgoing
 * 
 * @param   cKey        key sent
 */
void module::module_internal::add_stats_outgoing(qkd::key::key const & cKey) {

    std::lock_guard<std::recursive_mutex> cLock(cStat.cMutex);

    cStat.nKeysOutgoing++;
    cStat.nKeyBitsOutgoing += cKey.size() * 8;
    cStat.nDisclosedBitsOutgoing += cKey.meta().nDisclosedBits;
    cStat.cKeysOutgoingRate << cStat.nKeysOutgoing;
    cStat.cKeyBitsOutgoingRate << cStat.nKeyBitsOutgoing;
    cStat.cDisclosedBitsOutgoingRate << cStat.nDisclosedBitsOutgoing;
}


/**
 * connect to remote instance
 * 
 * @param   sPeerURL        the remote instance URL
 */
void module::module_internal::connect(std::string sPeerURL) {
    cConPeer->clear();
    cConPeer->add(sPeerURL, 1000, cModule->id().toStdString(), "listen");
}


/**
 * dump current module config
 */
void module::module_internal::debug_config() {

    qkd::utility::debug() << "current module config:";
    qkd::utility::debug() << "              role: " << cModule->role_name().toStdString();
    qkd::utility::debug() << "       url_pipe_in: " << cModule->url_pipe_in().toStdString();
    qkd::utility::debug() << "      url_pipe_out: " << cModule->url_pipe_out().toStdString();
    qkd::utility::debug() << "          url_peer: " << cModule->url_peer().toStdString();
    qkd::utility::debug() << "        url_listen: " << cModule->url_listen().toStdString();
    qkd::utility::debug() << "          pipeline: " << cModule->pipeline().toStdString();
    qkd::utility::debug() << "              hint: " << cModule->hint().toStdString();
    qkd::utility::debug() << "        random_url: " << cModule->random_url().toStdString();
    qkd::utility::debug() << "  synchronize_keys: " << (cModule->synchronize_keys() ? "true" : "false");
    qkd::utility::debug() << "   synchronize_ttl: " << cModule->synchronize_ttl();
    qkd::utility::debug() << "   terminate_after: " << cModule->terminate_after();
}


/**
 * dump a message to stderr
 *
 * @param   bSent       message has been sent
 * @param   cMessage    message itself
 */
void module::module_internal::debug_message(bool bSent, qkd::module::message const & cMessage) {

    if (!bDebugMessageFlow) return;
    if (bSent) {
        qkd::utility::debug() << "<MOD-SENT>" << cMessage.string("          ");
    }
    else {
        qkd::utility::debug() << "<MOD-RECV>" << cMessage.string("          ");
    }
 }


/**
 * dump key PULL to stderr
 *
 * @param   cKey        key to dump
 */
void module::module_internal::debug_key_pull(qkd::key::key const & cKey) {

    boost::format cLineFormater = 
            boost::format("key-PULL [%015ums] id: %010u bits: %010u err: %6.4f dis: %010u crc: %08x state: %-13s");
    
    auto cTimePoint = std::chrono::duration_cast<std::chrono::milliseconds>(cModule->age());
    cLineFormater % cTimePoint.count();
    cLineFormater % cKey.id();
    cLineFormater % (cKey.size() * 8);
    cLineFormater % cKey.meta().nErrorRate;
    cLineFormater % cKey.meta().nDisclosedBits;
    cLineFormater % cKey.data().crc32();
    cLineFormater % cKey.state_string();
    
    qkd::utility::debug() << cLineFormater.str();
}


/**
 * dump key PUSH  to stderr
 *
 * @param   cKey        key to dump
 */
void module::module_internal::debug_key_push(qkd::key::key const & cKey) {

    boost::format cLineFormater = 
            boost::format("key-PUSH [%015ums] id: %010u bits: %010u err: %6.4f dis: %010u crc: %08x state: %-13s dur: %012u ns (%06u ms)");

    auto cTimePoint = std::chrono::duration_cast<std::chrono::milliseconds>(cModule->age());
    cLineFormater % cTimePoint.count();
    cLineFormater % cKey.id();
    cLineFormater % (cKey.size() * 8);
    cLineFormater % cKey.meta().nErrorRate;
    cLineFormater % cKey.meta().nDisclosedBits;
    cLineFormater % cKey.data().crc32();
    cLineFormater % cKey.state_string();
    auto cNanoSeconds =  std::chrono::duration_cast<std::chrono::nanoseconds>(cKey.dwell());
    cLineFormater % cNanoSeconds.count();
    cLineFormater % (uint64_t)(std::floor(cNanoSeconds.count() / 1000000.0 + 0.5));
    
    qkd::utility::debug() << cLineFormater.str();
}


/**
 * get the current module state
 * 
 * @return  the current module state
 */
module_state module::module_internal::get_state() const {
    std::unique_lock<std::mutex> cLock(cStateMutex);
    return eState;
}


/**
 * clean any resources left
 */
void module::module_internal::release() {
    
    set_state(module_state::STATE_TERMINATING);
    
    cConListen->reset();
    cConPeer->reset();
    cConPipeIn->reset();
    cConPipeOut->reset();
    
    set_state(module_state::STATE_TERMINATED);
}


/**
 * set a new module state
 * 
 * the working thread will be notified (if waiting)
 * 
 * @param   eNewState       the new module state
 */
void module::module_internal::set_state(module_state eNewState) {
    std::unique_lock<std::mutex> cLock(cStateMutex);
    eState = eNewState;
    cStateCondition.notify_all();
}


/**
 * wait for state change
 * 
 * this method waits for any state change caused by another
 * thread but the working one
 * 
 * This method returns if we have a new state but eWorkingState
 * 
 * @param   eWorkingState       current working state
 * @return  the new module state
 */
module_state module::module_internal::wait_for_state_change(module_state eWorkingState) const {
    std::unique_lock<std::mutex> cLock(cStateMutex);
    while (eWorkingState == eState) cStateCondition.wait(cLock);
    return eState;
}

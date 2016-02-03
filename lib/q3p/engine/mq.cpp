/*
 * mq.cpp
 *
 * implement the message queue (aka "Key Pump")
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

#include <fstream>

#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

// ait
#include <qkd/q3p/engine.h>
#include <qkd/q3p/mq.h>
#include <qkd/utility/debug.h>
#include <qkd/utility/syslog.h>


using namespace qkd::q3p;


// ------------------------------------------------------------
// defs


/**
 * maximum of keys in a queue
 */
#define MAX_KEYS_IN_QUEUE           100ul


/**
 * maximum of keysize in a queue
 */
#define MAX_KEYSIZE_IN_QUEUE        8192ul


// ------------------------------------------------------------
// decl


/**
 * the mq pimpl
 */
class qkd::q3p::mq_instance::mq_data {
    
    
public:
    
    
    /**
     * ctor
     */
    mq_data() : nMaxKey(0), nMaxKeySize(0) { };
    
    
    /**
     * the message queue descriptor handle
     */
    mqd_t nMQDescriptor;
    
    
    /**
     * maximum number of keys in the queue
     */
    uint64_t nMaxKey;
   

    /**
     * maximum size of a key in the queue
     */
    uint64_t nMaxKeySize;
};


// ------------------------------------------------------------
// code


/**
 * ctor
 * 
 * @param   cEngine     the parent engine
 * @throws  mq_no_engine
 */
mq_instance::mq_instance(qkd::q3p::engine_instance * cEngine) : QObject(), m_cEngine(cEngine), m_bPaused(true) {
    
    // engine
    if (!m_cEngine) throw qkd::q3p::mq_instance::mq_no_engine();
    
    // pimpl
    d = boost::shared_ptr<qkd::q3p::mq_instance::mq_data>(new qkd::q3p::mq_instance::mq_data());    
    
    // create the message queue object
    m_sName = "/" + engine()->link_id().toStdString();
    
    // get the maximum number of messages in the queue from the operating system
    std::ifstream cProcSysFsMqueueMsgMaxFile("/proc/sys/fs/mqueue/msg_max", std::ios::in);
    if (cProcSysFsMqueueMsgMaxFile.is_open()) {
        std::string sNumber;
        cProcSysFsMqueueMsgMaxFile >> sNumber;
        std::istringstream(sNumber) >> d->nMaxKey;
    }
    
    // get the maximum size of messages in the queue from the operating system
    std::ifstream cProcSysFsMqueueMsgSizeMaxFile("/proc/sys/fs/mqueue/msgsize_max", std::ios::in);
    if (cProcSysFsMqueueMsgSizeMaxFile.is_open()) {
        std::string sNumber;
        cProcSysFsMqueueMsgSizeMaxFile >> sNumber;
        std::istringstream(sNumber) >> d->nMaxKeySize;
    }
    
    // fix upper bounds on values
    d->nMaxKey = std::min(d->nMaxKey, (uint64_t)MAX_KEYS_IN_QUEUE);
    d->nMaxKeySize = std::min(d->nMaxKeySize, (uint64_t)MAX_KEYSIZE_IN_QUEUE);
    
    // key size must be a multiple of application_buffer()->quantum()
    d->nMaxKeySize -= (d->nMaxKeySize % engine()->application_buffer()->quantum());
    
    // define the mq properties
    struct mq_attr cAttr;
    memset(&cAttr, 0, sizeof(cAttr));
    cAttr.mq_flags = O_NONBLOCK;
    cAttr.mq_maxmsg = d->nMaxKey;
    cAttr.mq_msgsize = d->nMaxKeySize;
    
    // open the MQ
    d->nMQDescriptor = mq_open(m_sName.c_str(), O_WRONLY | O_CREAT | O_NONBLOCK, 0666, &cAttr);
    if (d->nMQDescriptor == -1) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to init MQ '" << m_sName << "': " << strerror(errno);
    }
}


/**
 * pause message queue fillment
 */
void mq_instance::pause() {
    m_bPaused = true;
    emit mode_changed(m_bPaused);
}


/**
 * fill message queue with keys
 */
void mq_instance::produce() {

    if (m_bPaused) return;
    
    // check how many keys we should produce
    struct mq_attr cAttr;
    memset(&cAttr, 0, sizeof(cAttr));
    mq_getattr(d->nMQDescriptor, &cAttr);
    
    uint64_t nKeysToProduce = d->nMaxKey - cAttr.mq_curmsgs;
    if (nKeysToProduce == 0) return;
    
    // TODO: sync ourselves with the peer based on roles and key-ids

    // place them into the MQ
    uint64_t nKeysConsumed = 0;
    for (uint64_t i = 0; i < nKeysToProduce; i++) {
        
        // fetch as much keys from the application
        // buffer and send them to the queue
        qkd::key::key_vector cKeys = engine()->application_buffer()->find_valid(d->nMaxKeySize, 1);
        
        // if we didn't find any keys left: bail out
        if (cKeys.empty()) break;
        
        // compile the keys from the buffer into a large key "message"
        qkd::key::key_ring cMQKeyRing(d->nMaxKeySize);
        for (auto & nKeyId : cKeys) {
            qkd::key::key cKey = engine()->application_buffer()->get(nKeyId);
            cMQKeyRing << cKey;
        }

        // place in MQ
        int nError = mq_send(d->nMQDescriptor, (char const *)cMQKeyRing.at(0).data().get(), cMQKeyRing.at(0).size(), 0);
        if (nError) {
            
            // placing into the queue failed ... unmark the keys
            engine()->application_buffer()->set_key_count(cKeys, 0);
        }
        else {
            
            // we shipped a key into the MQ: delete them from the buffer
            engine()->application_buffer()->del(cKeys);
            nKeysConsumed += cKeys.size();
        }
    }

    // tell environment
    if (nKeysConsumed) {
        engine()->application_buffer()->emit_charge_change(0, nKeysConsumed);
        if (qkd::utility::debug::enabled()) {
            qkd::utility::debug() << "consumed " << nKeysConsumed << " keys for MQ named '" << m_sName << "'";
            qkd::utility::debug() << "current charges: " << engine()->charge_string();            
        }
    }
}


/**
 * purge the message queue
 */
void mq_instance::purge() {
 
    // temporary suspend production
    bool bOldPaused = paused();
    m_bPaused = true;
    
    // reopen the MQ in read mode
    mqd_t nMQD = mq_open(m_sName.c_str(), O_RDONLY | O_NONBLOCK);
    if (nMQD == -1) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to purge message queue, while reopen the message queue I got:" << strerror(errno);
        m_bPaused = bOldPaused;
        return;
    }
    
    // as long as there is a message ...
    struct mq_attr cAttr;
    do {
        
        // read out from the MQ as long as we 
        memset(&cAttr, 0, sizeof(cAttr));
        mq_getattr(nMQD, &cAttr);
        
        if (cAttr.mq_curmsgs) {
    
            // prepare space to dump the next message
            char * cMsg = new char[cAttr.mq_msgsize];
            
            // maximum timespan to wait for a read: 1 sec
            struct timespec cTimespan;
            clock_gettime(CLOCK_MONOTONIC, &cTimespan);
            cTimespan.tv_sec++;
            
            // extract message
            unsigned nMsgPriority = 0;
            if (mq_timedreceive(nMQD, cMsg, cAttr.mq_msgsize, &nMsgPriority, &cTimespan) == -1) {
                qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to purge MQ '" << m_sName << "': " << strerror(errno);
                break;
            }

            // destroy ...
            delete [] cMsg;
        }
         
    } while (cAttr.mq_curmsgs);
    
    // close the reading mq again
    mq_close(nMQD);
    
    // reactivate production again (if necessary)
    m_bPaused = bOldPaused;
    
    // tell environment
    emit purged();
}


/**
 * resume message queue fillment
 */
void mq_instance::resume() {
    m_bPaused = false;
    emit mode_changed(m_bPaused);
    produce();
}


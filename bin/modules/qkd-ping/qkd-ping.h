/*
 * qkd-ping.h
 * 
 * The qkd-ping sends a series of messages back and forth 
 * to test remote module to module interconnection
 * 
 * Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *
 * Copyright (C) 2012-2015 AIT Austrian Institute of Technology
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


#ifndef __QKD_MODULE_QKD_PING_H_
#define __QKD_MODULE_QKD_PING_H_


// ------------------------------------------------------------
// incs

// ait
#include <qkd/module/module.h>


// ------------------------------------------------------------
// decl


/**
 * The qkd-ping sends messages back and forth to test module
 * to module interconnection capabilities
 * 
 * The qkd-ping QKD module supports the "at.ac.ait.qkd.ping" Interface.
 * 
 * Properties of at.ac.ait.qkd.ping
 * 
 *      -name-                  -read/write-    -description-
 *
 *      max_roundtrip               R/W         maximum number of rountrips
 *      payload_size                R/W         amount of bytes to send/receive
 *      roundtrips                  R           number of roundntrips so far
 *      sleep_time                  R/W         sleep time between two consecutive calls
 */
class qkd_ping : public qkd::module::module {
    
    
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "at.ac.ait.qkd.ping")

    Q_PROPERTY(qulonglong max_roundtrip READ max_roundtrip WRITE set_max_roundtrip)     /**< the maximum number of roundntrips */
    Q_PROPERTY(qulonglong payload_size READ payload_size WRITE set_payload_size)        /**< get/set the size of the package sent */
    Q_PROPERTY(qulonglong roundtrips READ roundtrips)                                   /**< the number of roundtrips so far */
    Q_PROPERTY(qulonglong sleep_time READ sleep_time WRITE set_sleep_time)              /**< get/set number of milliseconds to wait between two consecutive roundntrips */
    
    
public:


    /**
     * ctor
     */
    qkd_ping();
    
    
    /**
     * returns the maximum number of roundtrips to do
     * 
     * @return  the maximum number of roundtrips to do
     */
    qulonglong max_roundtrip() const; 
    
    
    /**
     * returns the number of bytes of the payload sent back and forth
     * 
     * @return  the number of bytes sent each roundtrip
     */
    qulonglong payload_size() const; 
    
    
    /**
     * returns the number of current rounndtrips
     * 
     * @return  the number of current roundtrips
     */
    qulonglong roundtrips() const; 
    
    
    /**
     * set a new maximum number of roundtrips to do
     * 
     * @param   nMaxRoundtrip       the new maximum number of roundtrips to do
     */
    void set_max_roundtrip(qulonglong nMaxRoundtrip); 
    
    
    /**
     * set a new number of bytes to send back and forth
     * 
     * @param   nPayloadSize    the new number of bytes to send and recv
     */    
    void set_payload_size(qulonglong nPayloadSize);


    /**
     * set a new number of milliseconds to wait between a roundtrip
     * this number must be a multple of timeout()
     * 
     * @param   nSleepTime      the new number of milliseconds to sleep
     */    
    void set_sleep_time(qulonglong nSleepTime);


    /**
     * returns the number of milliseconds to wait between a roundtrip
     * 
     * @return  the time to sleep between calls
     */
    qulonglong sleep_time() const; 
    

private:
    
    
    /**
     * module work
     * 
     * @param   cKey                    not used
     * @param   cIncomingContext        incoming crypto context
     * @param   cOutgoingContext        outgoing crypto context
     * @return  always false (we never forward keys)
     */
    virtual bool process(UNUSED qkd::key::key & cKey, UNUSED qkd::crypto::crypto_context & cIncomingContext, UNUSED qkd::crypto::crypto_context & cOutgoingContext) { 
        if (is_alice()) process_alice();
        else process_bob();
        return false;
    };
    
    
    /**
     * module work as ALICE
     */
    void process_alice();
    
    
    /**
     * module work as BOB
     */
    void process_bob();
    
    
    // pimpl
    class qkd_ping_data;
    boost::shared_ptr<qkd_ping_data> d;
    
};


#endif


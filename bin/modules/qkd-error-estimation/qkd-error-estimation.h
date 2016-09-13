/*
 * qkd-error-estimation.h
 * 
 * The qkd-error-estimation discloses a portion of bits for error estimation
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


#ifndef __QKD_MODULE_QKD_ERROR_ESTIMATION_H_
#define __QKD_MODULE_QKD_ERROR_ESTIMATION_H_


// ------------------------------------------------------------
// incs

// ait
#include <qkd/module/module.h>


// ------------------------------------------------------------
// decl


/**
 * The qkd-error-estimation discloses a small portion of bits for error estimation
 * 
 * The error estimation discloses a number of bits of the key bypassing. The disclosed keybits
 * are discarded and the error bits in the are set accordingly.
 * 
 * E.g.
 * 
 *      original key bits = 1000
 *      percentage disclosed for error estimation = 15%
 * 
 *      number of bits disclosed = 150
 *      detected errors = 6
 *      detected error rate = 4%
 * 
 *      new key length = 850
 *      number of error bits set in new key = 34 (==> 4% of 850)
 * 
 * 
 * The qkd-error-estimation QKD module supports the "at.ac.ait.qkd.errorestimation" Interface.
 * 
 * Properties of at.ac.ait.qkd.errorestimation
 * 
 *      -name-                  -read/write-    -description-
 * 
 *      average_error               R           current average error estimation value
 * 
 *      disclose                   R/W          disclosed ratio for error estimation (0.0 - 1.0)
 * 
 *      last_error                  R           last error estimation value 
 * 
 */
class qkd_error_estimation : public qkd::module::module {
    
    
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "at.ac.ait.qkd.errorestimation")

    Q_PROPERTY(double average_error READ average_error)                         /**< current average of estimation  */
    Q_PROPERTY(double disclose READ disclose WRITE set_disclose)                /**< get/set ratio of disclosed bits */
    Q_PROPERTY(double last_error READ last_error)                               /**< last error estimation  */
    

public:


    /**
     * ctor
     */
    qkd_error_estimation();
    
    
    /**
     * return the average error over the last keys
     * 
     * @return  the average error over the last keys
     */
    double average_error() const;

    
    /**
     * return the last error estimation
     * 
     * @return  the last error estimation
     */
    double last_error() const;

    
    /**
     * return the ratio of disclosed bits
     * 
     * @return  the ratio of disclosed bits
     */
    double disclose() const;

    
    /**
     * sets the ratio of disclosed bits
     * 
     * @param   nRatio      the new ratio of disclosed bits
     */
    void set_disclose(double nRatio);
    
    
protected:
    
    
    /**
     * accept a key for processing
     * 
     * We accept keys of shared secret bit data encoding and which
     * have not been yet disclosed (hint: we do change the state and
     * therefore a once disclosed key will drop this state information).
     * 
     * @param   cKey            the key to check
     * @return  true, if the key should be processed by this module
     */
    bool accept(qkd::key::key const & cKey) const { 
        return (cKey.encoding() == qkd::key::ENCODING_SHARED_SECRET_BITS) && 
                (cKey.state() != qkd::key::key_state::KEY_STATE_DISCLOSED);
    }
    
    
    /**
     * apply the loaded key value map to the module
     * 
     * @param   sURL            URL of config file loaded
     * @param   cConfig         map of key --> value
     */
    void apply_config(std::string const & sURL, qkd::utility::properties const & cConfig);
    

private:
    
    
    /**
     * module work
     * 
     * @param   cKey                    key with errors to estimate
     * @param   cIncomingContext        incoming crypto context
     * @param   cOutgoingContext        outgoing crypto context
     * @return  always true
     */
    virtual bool process(qkd::key::key & cKey, qkd::crypto::crypto_context & cIncomingContext, qkd::crypto::crypto_context & cOutgoingContext);

    
    /**
     * module work as alice
     * 
     * @param   cKey                    key with errors to estimate
     * @param   cIncomingContext        incoming crypto context
     * @param   cOutgoingContext        outgoing crypto context
     * @return  always true
     */
    bool process_alice(qkd::key::key & cKey, qkd::crypto::crypto_context & cIncomingContext, qkd::crypto::crypto_context & cOutgoingContext);

    
    /**
     * module work as bob
     * 
     * @param   cKey                    key with errors to estimate
     * @param   cIncomingContext        incoming crypto context
     * @param   cOutgoingContext        outgoing crypto context
     * @return  always true
     */
    bool process_bob(qkd::key::key & cKey, qkd::crypto::crypto_context & cIncomingContext, qkd::crypto::crypto_context & cOutgoingContext);

    
    // pimpl
    class qkd_error_estimation_data;
    std::shared_ptr<qkd_error_estimation_data> d;
    
};


#endif

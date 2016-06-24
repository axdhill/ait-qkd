/*
 * qkd-privacy-amplification.h
 * 
 * This is the QKD PRIVACY AMPLIFICATION definition
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


#ifndef __QKD_MODULE_QKD_PRIVACY_AMPLIFICATION_H_
#define __QKD_MODULE_QKD_PRIVACY_AMPLIFICATION_H_


// ------------------------------------------------------------
// incs

// ait
#include <qkd/module/module.h>


// ------------------------------------------------------------
// decl


/**
 * how do we calculate the final key size
 */
enum calculation_procedure : uint8_t {
    
    CALCULATE_SECURITY_BITS = 0,        /**< do calculation with security bits */
    CALCULATE_REDUCTION_RATE = 1,       /**< do calculation based on reduction rate */
};


/**
 * The qkd-privacy-amplification runs the QKD privacy amplification
 * to reduce Eve's knowledge by the information leaked from
 * the public discussion of the QKD postprocessing
 * 
 * The qkd-buffer QKD module supports the "at.ac.ait.qkd.privacyamplification" Interface.
 * 
 * Properties of at.ac.ait.qkd.privacyamplification
 * 
 *      -name-                  -read/write-    -description-
 * 
 *      calculation                  R          current calculation procedure
 * 
 *      reduction_rate              R/W         reduction of key: 0.0 => no final key, 1.0 => no reduction
 * 
 *      security_bits               R/W         number of security bits introduced into privacy amplification
 * 
 */
class qkd_privacy_amplification : public qkd::module::module {
    
    
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "at.ac.ait.qkd.privacyamplification")

    Q_PROPERTY(qulonglong calculation READ calculation)                                     /**< get current calculate procedure */
    Q_PROPERTY(double reduction_rate READ reduction_rate WRITE set_reduction_rate)          /**< get/set reduction rate */
    Q_PROPERTY(qulonglong security_bits READ security_bits WRITE set_security_bits)         /**< get/set number of security bits */


public:


    /**
     * ctor
     */
    qkd_privacy_amplification();
    
    
    /**
     * get the current calculation procedure
     * 
     *  0 ==> work on security bits
     *  1 ==> work on reduction rate
     *
     * @return  the current calculate procedure
     */
    qulonglong calculation() const;
    
    
    /**
     * get the reduction rate of the key
     * 
     * the size of the key is shrunk by this rate value
     * 
     * rate: 0.0 ==> no final key
     *       1.0 ==> no reduction
     *
     * @return  the reduction rate
     */
    double reduction_rate() const;
    
    
    /**
     * get the number of security bits
     * 
     * @return  number of security bits introduced into privacy amplification
     */
    qulonglong security_bits() const;
    
    
    /**
     * set the reduction rate of the key
     * 
     * the size of the key is shrunk by this rate value
     * 
     * rate: 0.0 ==> no final key
     *       1.0 ==> no reduction
     *
     * @param   nRate       the new reduction rate of the key
     */
    void set_reduction_rate(double nRate);
    
    
    /**
     * set the new number of security bits introduced into privacy amplification
     * 
     * @param   nBits       the new number of security bits 
     */
    void set_security_bits(qulonglong nBits);
    
    
protected:
    
    
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
     * @param   cKey                    the key just read from the input pipe
     * @param   cIncomingContext        incoming crypto context
     * @param   cOutgoingContext        outgoing crypto context
     * @return  always true
     */
    bool process(qkd::key::key & cKey, qkd::crypto::crypto_context & cIncomingContext, qkd::crypto::crypto_context & cOutgoingContext);

    
    // pimpl
    class qkd_privacy_amplification_data;
    std::shared_ptr<qkd_privacy_amplification_data> d;
    
};


#endif

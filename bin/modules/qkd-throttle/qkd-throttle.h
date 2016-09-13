/*
 * qkd-throttle.h
 * 
 * The qkd-throttle slows down the key stream bypassing (handy for development)
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


#ifndef __QKD_MODULE_QKD_THROTTLE_H_
#define __QKD_MODULE_QKD_THROTTLE_H_


// ------------------------------------------------------------
// incs

// ait
#include <qkd/module/module.h>


// ------------------------------------------------------------
// decl


/**
 * The qkd-throttle slows down the bypassing keystream.
 * 
 * The qkd-throttle QKD module supports the "at.ac.ait.qkd.throttle" Interface.
 * 
 * Properties of at.ac.ait.qkd.throttle
 * 
 *      -name-                  -read/write-    -description-
 * 
 *      bits_per_second              R          current bits per second
 * 
 *      keys_per_second              R          current keys per second
 * 
 *      max_bits_per_second         R/W         maximum key-bits per second allowed (0 == no maximum)
 * 
 *      max_keys_per_second         R/W         maximum keys per second allowed (0 == no maximum)
 * 
 */
class qkd_throttle : public qkd::module::module {
    
    
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "at.ac.ait.qkd.throttle")

    Q_PROPERTY(double bits_per_second READ bits_per_second)                                             /**< get the current key-bits per second */
    Q_PROPERTY(double keys_per_second READ keys_per_second)                                             /**< get the current keys per second */
    Q_PROPERTY(double max_bits_per_second READ max_bits_per_second WRITE set_max_bits_per_second)       /**< get/set the maximum key-bits per second */
    Q_PROPERTY(double max_keys_per_second READ max_keys_per_second WRITE set_max_keys_per_second)       /**< get/set the maximum keys per second */
    
    
public:


    /**
     * ctor
     */
    qkd_throttle();
    
    
    /**
     * return the current bits per second
     * 
     * @return  the current bits per second
     */
    double bits_per_second() const;

    
    /**
     * return the current keys per second
     * 
     * @return  the current keys per second
     */
    double keys_per_second() const;

    
    /**
     * return the maximum bits per second
     * 
     * @return  the maximum bits per second
     */
    double max_bits_per_second() const;

    
    /**
     * return the maximum keys per second
     * 
     * @return  the maximum keys per second
     */
    double max_keys_per_second() const;

    
    /**
     * sets the maximum bits per second
     * 
     * @param   nMaximum        the new maximum (0 == no maximum)
     */
    void set_max_bits_per_second(double nMaximum);

    
    /**
     * sets the maximum keys per second
     * 
     * @param   nMaximum        the new maximum (0 == no maximum)
     */
    void set_max_keys_per_second(double nMaximum);

    
protected:
    
    
    /**
     * accept a key for processing
     * 
     * qkd-dekey accepts all keys (even disclosed ones).
     * 
     * @param   cKey            the key to check
     * @return  true, if the key should be processed by this module
     */
    bool accept(UNUSED qkd::key::key const & cKey) const { return true; }
    
    
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
     * @param   cKey                    the key to forward
     * @param   cIncomingContext        incoming crypto context
     * @param   cOutgoingContext        outgoing crypto context
     * @return  always true
     */
    bool process(qkd::key::key & cKey, qkd::crypto::crypto_context & cIncomingContext, qkd::crypto::crypto_context & cOutgoingContext);

    
    // pimpl
    class qkd_throttle_data;
    std::shared_ptr<qkd_throttle_data> d;
    
};


#endif

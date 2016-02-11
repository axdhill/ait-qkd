/*
 * qkd-confirmation.h
 * 
 * This is the QKD CONFIRMATION module definition
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


#ifndef __QKD_MODULE_QKD_CONFIRMATION_H_
#define __QKD_MODULE_QKD_CONFIRMATION_H_


// ------------------------------------------------------------
// incs

// ait
#include <qkd/module/module.h>


// ------------------------------------------------------------
// decl


/**
 * The qkd-confirmation module ensures that the keys
 * are indeed equal on both sides
 * 
 * This is achieved by applying a binary AND on the whole key
 * with a random number and publishing the parity of the result. This
 * is done ROUNDS time.
 * 
 * The qkd-confirmation QKD module supports the "at.ac.ait.qkd.confirmation" Interface.
 * 
 * Properties of at.ac.ait.qkd.confirmation
 * 
 *      -name-                  -read/write-    -description-
 * 
 *      bad_keys                     R          number of bad keys (keys for which confirmation failed)
 * 
 *      confirmed_keys               R          number of good keys (keys for which confirmation succeeded)
 * 
 *      rounds                      R/W         number of confirmation rounds
 * 
 */
class qkd_confirmation : public qkd::module::module {
    
    
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "at.ac.ait.qkd.confirmation")

    Q_PROPERTY(qulonglong bad_keys READ bad_keys)                       /**< absolute number of bad, unconfirmed keys so far */
    Q_PROPERTY(qulonglong confirmed_keys READ confirmed_keys)           /**< absolute number of confirmed keys so far */
    Q_PROPERTY(qulonglong rounds READ rounds WRITE set_rounds)          /**< get/set number of confirmation rounds */


public:


    /**
     * ctor
     */
    qkd_confirmation();
    
    
    /**
     * get the number of bad keys so far
     * 
     * bad keys are keys not been successfully corrected
     * 
     * @return  the number of bad keys
     */
    qulonglong bad_keys() const;
    
    
    /**
     * get the number of confirmed keys so far
     * 
     * @return  the number of confirmation keys done
     */
    qulonglong confirmed_keys() const;
    
    
    /**
     * get the number of confirmation rounds
     * 
     * @return  the number of confirmation rounds 
     */
    qulonglong rounds() const;
    
    
    /**
     * set the new number of confirmation rounds
     * 
     * @param   nRounds     the new number of confirmation rounds
     */
    void set_rounds(qulonglong nRounds);
    
    
protected:
    
    
    /**
     * accept a key for processing
     * 
     * we accept all keys (also the disclosed ones)
     *
     * However, disclosed keys are not confirmed.
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
     * @param   cKey                    the key to confirm
     * @param   cIncomingContext        incoming crypto context
     * @param   cOutgoingContext        outgoing crypto context
     * @return  always true
     */
    virtual bool process(qkd::key::key & cKey, qkd::crypto::crypto_context & cIncomingContext, qkd::crypto::crypto_context & cOutgoingContext);

    
    /**
     * module work as alice
     * 
     * @param   cKey                    the key to confirm
     * @param   cIncomingContext        incoming crypto context
     * @param   cOutgoingContext        outgoing crypto context
     * @return  always true
     */
    bool process_alice(qkd::key::key & cKey, qkd::crypto::crypto_context & cIncomingContext, qkd::crypto::crypto_context & cOutgoingContext);

    
    /**
     * module work as bob
     * 
     * @param   cKey                    the key to confirm
     * @param   cIncomingContext        incoming crypto context
     * @param   cOutgoingContext        outgoing crypto context
     * @return  always true
     */
    bool process_bob(qkd::key::key & cKey, qkd::crypto::crypto_context & cIncomingContext, qkd::crypto::crypto_context & cOutgoingContext);

    
    // pimpl
    class qkd_confirmation_data;
    std::shared_ptr<qkd_confirmation_data> d;
    
};


#endif

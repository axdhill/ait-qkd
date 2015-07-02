/*
 * qkd-sifting-bb84.h
 * 
 * The qkd-sifting-bb84 runs the well know BB84 protocol
 * 
 * Autor: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
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


#ifndef __QKD_MODULE_QKD_SIFTING_BB84_H_
#define __QKD_MODULE_QKD_SIFTING_BB84_H_


// ------------------------------------------------------------
// incs

// ait
#include <qkd/module/module.h>


// ------------------------------------------------------------
// decl


/**
 * The qkd-sifting-bb84 runs the well known BB84 protocol
 * 
 * The "keys" read from the BB84 module are not really keys.
 * The "keys" really contain the quantum table in the data() area
 * of the key.
 * 
 * The qkd-sifting-bb84 QKD module supports the "at.ac.ait.qkd.bb84" Interface.
 * 
 * About the base_ratio value: this value is a moving average of the
 * detected base comparisons. Any equal basis from this instance and the
 * peer during exchange is considered as "good".
 * 
 * Example:
 * 
 *          base_ratio = 0.18  ==>  18 % of the last base comparisons 
 *                                  share equal, valid basis
 * 
 * A base drop may occure:
 * 
 *  - naturally caused by the BB84 protocol (a base_ratio about 0.5 is very good)
 *  - detector quirks (no clicks, or double, triple clicks)
 *  - an evasdropper
 * 
 * 
 * BB84 creates new keys. Key Ids are assigned according to a pattern of shift- and add- values
 * 
 * The algorithm for new key_ids is this: 
 *      - use an internal counter and increment this by 1.
 *      - shift the result by the shift-value bits to the left
 *      - add the add-value
 * (see qkd::key::key_id_counter)
 * 
 * 
 * Properties of at.ac.ait.qkd.bb84
 * 
 *      -name-              -read/write-    -description-
 * 
 *      base_ratio               R          the moving average of the last good base ratio
 *      current_id               R          the current key id we are sifting
 *      current_length           R          the current key length in bits we have sifted so far
 *      key_id_pattern          R/W         the key id pattern used (see qkd::key::key_id_counter)
 *      rawkey_length           R/W         the minimum length of the raw key generated in bytes
 * 
 */
class qkd_sifting_bb84 : public qkd::module::module {
    
    
    Q_OBJECT
    
    Q_CLASSINFO("D-Bus Interface", "at.ac.ait.qkd.bb84")

    Q_PROPERTY(double base_ratio READ base_ratio)                                           /**< get the moving average of good bases */
    Q_PROPERTY(qulonglong current_id READ current_id)                                       /**< get the current key id we are sifting */
    Q_PROPERTY(qulonglong current_length READ current_length)                               /**< get the current key length in bits we have sifted so far */
    Q_PROPERTY(QString key_id_pattern READ key_id_pattern WRITE set_key_id_pattern)         /**< get/set key id pattern */
    Q_PROPERTY(qulonglong rawkey_length READ rawkey_length WRITE set_rawkey_length)         /**< get/set minimum raw key length in bytes */
    

public:


    /**
     * ctor
     */
    qkd_sifting_bb84();
    
    
    /**
     * get the moving average of good shared bases
     * 
     * @return  the moving average of good shared bases
     */
    double base_ratio() const;
    
    
    /**
     * get the current key id we are sifting
     * 
     * @return  the current key id we are sifting
     */
    qulonglong current_id() const;
    
    
    /**
     * get the current key length in bits we have sifted so far
     * 
     * @return  the current key length in bits we have sifted so far
     */
    qulonglong current_length() const;
    
    
    /**
     * return the key id pattern as string
     * 
     * the key id pattern is a string consisting of
     *      SHIFT "/" ADD
     * values for key-id generation
     * 
     * @return  the key-id generation pattern string
     */
    QString key_id_pattern() const;
    
    
    /**
     * get the minimum length of the raw key generated in bytes
     * 
     * @return  the minimum length of the generated raw key length in bytes
     */
    qulonglong rawkey_length() const;
    
    
    /**
     * sets a new key id pattern as string
     * 
     * the key id pattern is a string consisting of
     *      SHIFT "/" ADD
     * values for key-id generation
     * 
     * @param   sPattern    the new key generation pattern
     */
    void set_key_id_pattern(QString sPattern);
    
    
    /**
     * set a new minimum length of the generated raw key in bytes
     * 
     * @param   nLength         the new minimum length of the generated raw key length in bytes
     */
    void set_rawkey_length(qulonglong nLength);
    
    
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
     * @param   cKey                    the raw key with quantum events encoded
     * @param   cIncomingContext        incoming crypto context
     * @param   cOutgoingContext        outgoing crypto context
     * @return  always true
     */
    virtual bool process(qkd::key::key & cKey, 
            qkd::crypto::crypto_context & cIncomingContext, 
            qkd::crypto::crypto_context & cOutgoingContext);

    
    /**
     * module work as alice
     * 
     * @param   cKey                    the raw key with quantum events encoded
     * @param   cIncomingContext        incoming crypto context
     * @param   cOutgoingContext        outgoing crypto context
     * @return  always true
     */
    bool process_alice(qkd::key::key & cKey, 
            qkd::crypto::crypto_context & cIncomingContext, 
            qkd::crypto::crypto_context & cOutgoingContext);

    
    /**
     * module work as bob
     * 
    * @param   cKey                    the raw key with quantum events encoded
     * @param   cIncomingContext        incoming crypto context
     * @param   cOutgoingContext        outgoing crypto context
     * @return  always true
     */
    bool process_bob(qkd::key::key & cKey, 
            qkd::crypto::crypto_context & cIncomingContext, 
            qkd::crypto::crypto_context & cOutgoingContext);

    
    // pimpl
    class qkd_sifting_bb84_data;
    boost::shared_ptr<qkd_sifting_bb84_data> d;
    
};


#endif

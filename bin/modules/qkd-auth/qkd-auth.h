/*
 * qkd-auth.h
 * 
 * The qkd-auth either starts an authentication or ensures
 * the authenticity of QKD postprocessing
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


#ifndef __QKD_MODULE_QKD_AUTH_H_
#define __QKD_MODULE_QKD_AUTH_H_


// ------------------------------------------------------------
// incs

// ait
#include <qkd/module/module.h>


// ------------------------------------------------------------
// decl


/**
 * The qkd-auth starts a authentication by provding keys
 * with a crypo context. If keys do have a crypto context
 * it ensures authencity by running an authentication of the
 * based on the bypassing crypto contexts
 * 
 * This module saves up to twice the threshold of key material
 * for consecutive authentication tasks.
 * 
 * Properties of at.ac.ait.qkd.auth
 * 
 *      -name-              -read/write-    -description-
 * 
 *      available_keys_incoming  R          current available key material for incoming authentication
 * 
 *      available_keys_outgoing  R          current available key material for outgoing authentication
 * 
 *      current_scheme_in        R          the current incoming authentication scheme
 * 
 *      current_scheme_out       R          the current outgoing authentication scheme
 * 
 *      next_scheme_in          R/W         the next incoming authentication scheme to use
 * 
 *      next_scheme_out         R/W         the next outgoing authentication scheme to use
 * 
 *      threshold               R/W         threshold of key material to reserve in bytes (IN and OUT)
 * 
 * 
 * TODO: enable/disable threshold picking
 * TODO: store key file URLs
 *
 * 
 * Methods of at.ac.ait.qkd.auth
 * 
 *      -name-                  -description-
 * 
 *      store_keys_incoming()   add a memory block as incoming key material
 * 
 *      store_keys_outgoing()   add a memory block as outgoing key material
 * 
 */
class qkd_auth : public qkd::module::module {
    
    
    Q_OBJECT

    Q_CLASSINFO("D-Bus Interface", "at.ac.ait.qkd.auth")

    Q_PROPERTY(qulonglong available_keys_incoming READ available_keys_incoming)         /**< available key material for incoming authentication in bytes */
    Q_PROPERTY(qulonglong available_keys_outgoing READ available_keys_outgoing)         /**< available key material for outgoing authentication in bytes */
    Q_PROPERTY(QString current_scheme_in READ current_scheme_in)                        /**< get the current authentication scheme incoming */
    Q_PROPERTY(QString current_scheme_out READ current_scheme_out)                      /**< get the current authentication scheme outgoing */
    Q_PROPERTY(QString next_scheme_in READ next_scheme_in WRITE set_next_scheme_in)     /**< get/set the next authentication incoming scheme */
    Q_PROPERTY(QString next_scheme_out READ next_scheme_out WRITE set_next_scheme_out)  /**< get/set the next authentication outgoing scheme */
    Q_PROPERTY(qulonglong threshold READ threshold WRITE set_threshold)                 /**< get/set the current authentication key material threshold */
    

public:


    /**
     * ctor
     */
    qkd_auth();
    
    
    /**
     * get the amount of available key material for incoming authentication
     * 
     * @return  bytes of available key material for incoming authentication
     */
    qulonglong available_keys_incoming() const;
    
    
    /**
     * get the amount of available key material for outgoing authentication
     * 
     * @return  bytes of available key material for outgoing authentication
     */
    qulonglong available_keys_outgoing() const;
    
    
    /**
     * get the current authentication scheme incoming
     * 
     * @return  the current authentication scheme incoming
     */
    QString current_scheme_in() const;
    
    
    /**
     * get the current authentication scheme outgoing
     * 
     * @return  the current authentication scheme outgoing
     */
    QString current_scheme_out() const;
    
    
   /**
    * get the next incoming authentication scheme
    * 
    * @return  the next incoming authentication scheme
    */
    QString next_scheme_in() const;
    
    
   /**
    * get the next outgoing authentication scheme
    * 
    * @return  the next outgoing authentication scheme
    */
    QString next_scheme_out() const;
    
    
    /**
     * set the next incoming authentication scheme
     * 
     * @param   sScheme     the new next incoming authentication scheme
     */
    void set_next_scheme_in(QString const & sScheme);
    
    
    /**
     * set the next outgoing authentication scheme
     * 
     * @param   sScheme     the new next outgoing authentication scheme
     */
    void set_next_scheme_out(QString const & sScheme);
    
    
    /**
     * set the current authentication key material threshold in bytes
     *
     * @param   nThreshold  the new threshold to buffer key material for authentication
     */
    void set_threshold(qulonglong nThreshold);                                     
    
    
    /**
     * get the current authentication key material threshold in bytes
     *
     * @return  the threshold to buffer key material for authentication
     */
    qulonglong threshold() const;                                     
    
    
public slots:
    
    
    /**
     * store authentication keys to use incoming
     * 
     * @param   cAuthenticationKey      the authentication key to use incoming
     */
    void store_keys_incoming(QByteArray cAuthenticationKey);
    
    
    /**
     * store authentication keys to use outgoing
     * 
     * @param   cAuthenticationKey      the authentication key to use outgoing
     */
    void store_keys_outgoing(QByteArray cAuthenticationKey);
    
    
protected:
    
    
    /**
     * apply the loaded key value map to the module
     * 
     * @param   sURL            URL of config file loaded
     * @param   cConfig         map of key --> value
     */
    void apply_config(std::string const & sURL, qkd::utility::properties const & cConfig);
    

signals:
    
    
    /**
     * the authentication for a key failed
     * 
     * @param   nKeyId          the key id for which the authentication failed
     */
    void authentication_failed(unsigned int nKeyId);
    
    
    /**
     * we are in need of key material
     * 
     * this signal is emitted whenever our internal key
     * storage runs short of aurthentication keys
     * 
     * that is: the amount of the internal keys DB for
     * incoming and/or outgoing is below the threshold
     * limit
     */
    void starving();
    
    
private:
    
    
    /**
     * run authentication
     * 
     * @param   cKey                    the key to authenticate
     * @param   cIncomingContext        incoming crypto context
     * @param   cOutgoingContext        outgoing crypto context
     * @return  true, if authentication run successfully
     */
    bool authenticate(qkd::key::key & cKey, 
            qkd::crypto::crypto_context & cIncomingContext, 
            qkd::crypto::crypto_context & cOutgoingContext);
    
    
    /**
     * create new authenticate context 
     * 
     * @param   cIncomingContext        incoming crypto context
     * @param   cOutgoingContext        outgoing crypto context
     */
    void create_context(qkd::crypto::crypto_context & cIncomingContext, 
            qkd::crypto::crypto_context & cOutgoingContext);
    

    /**
     * module work
     * 
     * @param   cKey                    the key just read from the input pipe
     * @param   cIncomingContext        incoming crypto context
     * @param   cOutgoingContext        outgoing crypto context
     * @return  always true
     */
    virtual bool process(qkd::key::key & cKey, 
            qkd::crypto::crypto_context & cIncomingContext, 
            qkd::crypto::crypto_context & cOutgoingContext);

    
    /**
     * ensure the local key stores for authentication have enough keys
     *
     * @param   cKey        the key incoming
     */
    void refill_local_keystores(qkd::key::key & cKey);


    // pimpl
    class qkd_auth_data;
    boost::shared_ptr<qkd_auth_data> d;
    
};


#endif

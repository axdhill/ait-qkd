/*
 * qkd-drop.h
 * 
 * This is the QKD DROP module definition
 * 
 * Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *
 * Copyright (C) 2013-2015 AIT Austrian Institute of Technology
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


#ifndef __QKD_MODULE_QKD_DROP_H_
#define __QKD_MODULE_QKD_DROP_H_


// ------------------------------------------------------------
// incs

// ait
#include <qkd/module/module.h>


// ------------------------------------------------------------
// decl


/**
 * The qkd-drop module drops randomly incoming keys.
 * 
 * This module is used to test pipeline stability when some
 * modules inside the pipeline go awry.
 * 
 * The qkd-drop QKD module supports the "at.ac.ait.qkd.drop" Interface.
 * 
 * Properties of at.ac.ait.qkd.drop
 * 
 *      -name-                  -read/write-    -description-
 * 
 *      drop_ratio                  R/W         drop ratio between 0.0 (no key) and 1.0 (all keys)
 * 
 */
class qkd_drop : public qkd::module::module {
    
    
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "at.ac.ait.qkd.drop")

    Q_PROPERTY(double drop_ratio READ drop_ratio WRITE set_drop_ratio)      /**< get/set drop ration for incoming keys */


public:


    /**
     * ctor
     */
    qkd_drop();
    
    
    /**
     * get the drop ration for incoming keys
     * 
     * @return  the drop ration for incoming keys
     */
    double drop_ratio() const;
    
    
    /**
     * set the new drop ration for incoming keys
     * 
     * @param   nRatio          the new drop ration for incoming keys
     */
    void set_drop_ratio(double nRatio);
    
    
    /**
     * sets a new LISTEN URL
     *
     * This module does not have a peer
     *
     * @param   sURL        the new LISTEN URL
     */
    virtual void set_url_listen(UNUSED QString sURL) {};
    
    
    /**
     * sets a new PEER URL
     *
     * This module does not have a peer
     *
     * @param   sURL        the new PEER URL
     */
    virtual void set_url_peer(UNUSED QString sURL) {};
    
     
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
     * @param   cKey                    the key to buffer
     * @param   cIncomingContext        incoming crypto context
     * @param   cOutgoingContext        outgoing crypto context
     * @return  always true, when forward key
     */
    bool process(qkd::key::key & cKey, qkd::crypto::crypto_context & cIncomingContext, qkd::crypto::crypto_context & cOutgoingContext);

    
    // pimpl
    class qkd_drop_data;
    boost::shared_ptr<qkd_drop_data> d;
    
};


#endif

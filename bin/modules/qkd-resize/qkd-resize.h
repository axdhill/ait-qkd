/*
 * qkd-resize.h
 * 
 * This is the QKD RESIZE module definition
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


#ifndef __QKD_MODULE_QKD_RESIZE_H_
#define __QKD_MODULE_QKD_RESIZE_H_


// ------------------------------------------------------------
// incs

// ait
#include <qkd/module/module.h>


// ------------------------------------------------------------
// decl


/**
 * The qkd-resize module resizes up incoming keys until a minimum
 * key size is reached.
 * 
 * The qkd-resize QKD module supports the "at.ac.ait.qkd.resize" Interface.
 * 
 * Properties of at.ac.ait.qkd.resize
 * 
 *      -name-                  -read/write-    -description-
 * 
 *      cur_key_size                 R          current key size (in bytes) for forward
 * 
 *      min_key_size                R/W         minimum key size (in bytes) for forward
 * 
 */
class qkd_resize : public qkd::module::module {
    
    
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "at.ac.ait.qkd.resize")

    Q_PROPERTY(qulonglong cur_key_size READ cur_key_size)                                   /**< get current key size (in bytes) for forwarding */
    Q_PROPERTY(qulonglong min_key_size READ min_key_size WRITE set_min_key_size)            /**< get/set minimum key size (in bytes) for forwarding */


public:


    /**
     * ctor
     */
    qkd_resize();
    
    
    /**
     * get the current key size (in bytes) for forwarding
     * 
     * @return  the current key size (in bytes) for forwarding
     */
    qulonglong cur_key_size() const;
    
    
    /**
     * get the minimum key size (in bytes) for forwarding
     * 
     * @return  the minimum key size (in bytes) for forwarding
     */
    qulonglong min_key_size() const;
    
    
    /**
     * set the new minimum key size (in bytes) for forwarding
     * 
     * @param   nSize       the new minimum key size (in bytes) for forwarding
     */
    void set_min_key_size(qulonglong nSize);
    
    
protected:
    
    
    /**
     * accept a key for processing
     * 
     * we accept all keys (also the disclosed ones)
     *
     * However, disclosed keys are not forwarded.
     * 
     * @param   cKey            the key to check
     * @return  true, if the key should be processed by this module
     */
    bool accept(UNUSED qkd::key::key const & cKey) const { return true; };
    
    
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
     * @param   cKey                    the key to resize
     * @param   cIncomingContext        incoming crypto context
     * @param   cOutgoingContext        outgoing crypto context
     * @return  always true
     */
    virtual bool process(qkd::key::key & cKey, qkd::crypto::crypto_context & cIncomingContext, qkd::crypto::crypto_context & cOutgoingContext);

    
    // pimpl
    class qkd_resize_data;
    boost::shared_ptr<qkd_resize_data> d;
    
};


#endif

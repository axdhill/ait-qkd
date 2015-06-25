/*
 * qkd-reorder.h
 * 
 * This is the QKD REORDER module definition
 * 
 * Autor: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
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


#ifndef __QKD_MODULE_QKD_REORDER_H_
#define __QKD_MODULE_QKD_REORDER_H_


// ------------------------------------------------------------
// incs

// ait
#include <qkd/module/module.h>


// ------------------------------------------------------------
// decl


/**
 * The qkd-reorder module reorders randomly reorders keys when forwarding.
 * 
 * This is achieved by having a reorder buffer of a specific size. Whenever
 * a key is pulled from the previous module the module places the key in this
 * buffer. Next the module choses randomly a buffered key to be forwarded.
 * 
 * Yes, this might be an empty key, which is then discarded.
 * 
 * Therefore chances for a key to be forwarded is directly related
 * to the size of the buffer: 1 / (size_of_buffer)
 * 
 * This module is used to test pipeline stability when some
 * modules inside the pipeline go awry.
 * 
 * The qkd-reorder QKD module supports the "at.ac.ait.qkd.reorder" Interface.
 * 
 * Properties of at.ac.ait.qkd.reorder
 * 
 *      -name-                  -read/write-    -description-
 * 
 *      buffer_size                 R/W         size of the reorder buffer.
 * 
 */
class qkd_reorder : public qkd::module::module {
    
    
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "at.ac.ait.qkd.reorder")

    Q_PROPERTY(qulonglong buffer_size READ buffer_size WRITE set_buffer_size)       /**< get/set the size of the reorder buffer */


public:


    /**
     * ctor
     */
    qkd_reorder();
    
    
    /**
     * get the size of the reorder buffer
     * 
     * @return  the size of the reorder buffer
     */
    qulonglong buffer_size() const;
    
    
    /**
     * set the new size of the reorder buffer
     * 
     * @param   nSize           the size of the reorder buffer
     */
    void set_buffer_size(qulonglong nSize);
    
    
    /**
     * sets a new LISTEN URL
     *
     * This module does not have a peer
     *
     * @param   sURL        the new LISTEN URL
     */
    virtual void set_url_listen(UNUSED QString sURL) {
        module::set_url_listen("");
    };
    
    
    /**
     * sets a new PEER URL
     *
     * This module does not have a peer
     *
     * @param   sURL        the new PEER URL
     */
    virtual void set_url_peer(UNUSED QString sURL) {
        set_url_peer("");
    };
    
      
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
    class qkd_reorder_data;
    boost::shared_ptr<qkd_reorder_data> d;
    
};


#endif

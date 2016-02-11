/*
 * qkd-dekey.h
 * 
 * qkd-dekey QKD Module definition
 * 
 * The qkd-dekey QKD writes bypassing raw key data to a file.
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


#ifndef __QKD_MODULE_QKD_DEKEY_H_
#define __QKD_MODULE_QKD_DEKEY_H_


// ------------------------------------------------------------
// incs

// ait
#include <qkd/module/module.h>


// ------------------------------------------------------------
// decl


/**
 * The qkd-dekey writes bypassing raw key data to a file.
 * 
 * This acts much like qkd-tee, but with raw data instead.
 * 
 * The qkd-dekey QKD module supports the ""at.ac.ait.qkd.dekey" Interface.
 * 
 * Properties of at.ac.ait.qkd.dekey
 * 
 *      -name-              -read/write-    -description-
 * 
 *      file_url            R/W             file URL to write ot
 * 
 */
class qkd_dekey : public qkd::module::module {
    
    
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "at.ac.ait.qkd.dekey")

    Q_PROPERTY(QString file_url READ file_url WRITE set_file_url)           /**< get/set the file URL to write keys from */
    
    
public:


    /**
     * ctor
     */
    qkd_dekey();
    
    
    /**
     * return the file URL to write
     * 
     * @return  the file URL to write to
     */
    QString file_url() const;

    
    /**
     * sets the new file URL to write
     * 
     * @param   sFileURL        the new file URL to write to
     */
    void set_file_url(QString sFileURL);

    
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
     * @param   cKey                    will be set to the loaded key from the file
     * @param   cIncomingContext        incoming crypto context
     * @param   cOutgoingContext        outgoing crypto context
     * @return  always true
     */
    bool process(qkd::key::key & cKey, qkd::crypto::crypto_context & cIncomingContext, qkd::crypto::crypto_context & cOutgoingContext);
    
    
    // pimpl
    class qkd_dekey_data;
    std::shared_ptr<qkd_dekey_data> d;
    
};


#endif

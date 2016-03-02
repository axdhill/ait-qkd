/*
 * qkd-enkey.h
 * 
 * qkd-enkey QKD Module definition
 * 
 * The qkd-enkey QKD Module picks up a blob and pushes
 * the content as key-stream to pipeout.
 * 
 * This acts much like qkd-cat but with BLOBs of key data.
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


#ifndef __QKD_MODULE_QKD_ENKEY_H_
#define __QKD_MODULE_QKD_ENKEY_H_


// ------------------------------------------------------------
// incs

// ait
#include <qkd/module/module.h>


// ------------------------------------------------------------
// decl


/**
 * The qkd-enkey picks up a key file and pushes the content to pipeout.
 * 
 * The qkd-enkey QKD module supports the ""at.ac.ait.qkd.enkey" Interface.
 * 
 * Properties of at.ac.ait.qkd.enkey
 * 
 *      -name-         -read/write-         -description-
 * 
 *      current_id           R              current key id
 * 
 *      file_url            R/W             file URL to read from
 * 
 *      key_size            R/W             key_size in bytes for a single key
 * 
 *      loop                R/W             reset to start if EOF
 * 
 */
class qkd_enkey : public qkd::module::module {
    
    
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "at.ac.ait.qkd.enkey")
    
    Q_PROPERTY(qulonglong current_id READ current_id)                       /**< get the current key id we are blob'in */
    Q_PROPERTY(QString file_url READ file_url WRITE set_file_url)           /**< get/set the file URL to read keys from */
    Q_PROPERTY(bool key_size READ key_size WRITE set_key_size)              /**< get/set module key_size flag */
    Q_PROPERTY(bool loop READ loop WRITE set_loop)                          /**< get/set module loop flag */
    
    
public:


    /**
     * ctor
     */
    qkd_enkey();
    
    
    /**
     * get the current key id we are blob'in
     * 
     * @return  the current key id
     */
    qulonglong current_id() const;
    
    
    /**
     * return the file URL to read
     * 
     * @return  the file URL to read from
     */
    QString file_url() const;

    
    /**
     * return the size of a single key in bytes
     * 
     * @return  the size of single key in bytes
     */
    uint64_t key_size() const;

    
    /**
     * return the loop flag
     * 
     * @return  the loop flag
     */
    bool loop() const;

    
    /**
     * sets the new file URL to read
     * 
     * @param   sFileURL        the new file URL to read from
     */
    void set_file_url(QString sFileURL);

    
    /**
     * sets the key_size in bytes
     * 
     * @param   nKeySize        the new size of a single key in bytes
     */
    void set_key_size(uint64_t nKeySize);

    
    /**
     * sets the loop flag
     * 
     * @param   bLoop           the new loop flag
     */
    void set_loop(bool bLoop);

    
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
        module::set_url_peer("");
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
    class qkd_enkey_data;
    std::shared_ptr<qkd_enkey_data> d;
    
};


#endif

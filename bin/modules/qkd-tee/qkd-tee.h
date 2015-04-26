/*
 * qkd-tee.h
 * 
 * qkd-tee QKD Module definition
 * 
 * The qkd-tee QKD Module dumps a copy of the bypassing
 * key stream to a file.
 * 
 * If you need the raw key data without key meta data
 * look at the qkd-dekey module.
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


#ifndef __QKD_MODULE_QKD_TEE_H_
#define __QKD_MODULE_QKD_TEE_H_


// ------------------------------------------------------------
// incs

// ait
#include <qkd/module/module.h>


// ------------------------------------------------------------
// decl


/**
 * The qkd-tee dumpy a copy of the bypassing key-stream to a file.
 * 
 * The qkd-tee QKD module supports the ""at.ac.ait.qkd.tee" Interface.
 * 
 * Properties of at.ac.ait.qkd.tee
 * 
 *      -name-              -read/write-    -description-
 * 
 *      file_url            R/W             file URL to write to
 * 
 */
class qkd_tee : public qkd::module::module {
    
    
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "at.ac.ait.qkd.tee")

    Q_PROPERTY(QString file_url READ file_url WRITE set_file_url)           /**< get/set the file URL to write keys to */
    
    
public:


    /**
     * ctor
     */
    qkd_tee();
    
    
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
    class qkd_tee_data;
    boost::shared_ptr<qkd_tee_data> d;
    
};


#endif
/*
 * qkd-cat.h
 * 
 * qkd-cat QKD Module definition
 * 
 * The qkd-cat QKD Module picks up a key-file and pushes
 * the content to pipeout.
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


#ifndef __QKD_MODULE_QKD_CAT_H_
#define __QKD_MODULE_QKD_CAT_H_


// ------------------------------------------------------------
// incs

// ait
#include <qkd/module/module.h>


// ------------------------------------------------------------
// decl


/**
 * The qkd-cat picks up a key file and pushes the content to pipeout.
 * 
 * The qkd-cat QKD module supports the ""at.ac.ait.qkd.cat" Interface.
 * 
 * Properties of at.ac.ait.qkd.cat
 * 
 *      -name-              -read/write-    -description-
 * 
 *      file_url            R/W             file URL to read from
 * 
 *      loop                R/W             reset to start if EOF
 * 
 */
class qkd_cat : public qkd::module::module {
    
    
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "at.ac.ait.qkd.cat")

    Q_PROPERTY(QString file_url READ file_url WRITE set_file_url)           /**< get/set the file URL to read keys from */
    Q_PROPERTY(bool loop READ loop WRITE set_loop)                          /**< get/set module loop flag */
    
    
public:


    /**
     * ctor
     */
    qkd_cat();
    
    
    /**
     * return the file URL to read
     * 
     * @return  the file URL to read from
     */
    QString file_url() const;

    
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
     * sets the loop flag
     * 
     * @param   bLoop           the new loop flag
     */
    void set_loop(bool bLoop);

    
    /**
     * sets a new LISTEN URL
     *
     * @param   sURL        the new LISTEN URL
     */
    virtual void set_url_listen(QString sURL);
    
    
    /**
     * sets a new PEER URL
     *
     * @param   sURL        the new PEER URL
     */
    virtual void set_url_peer(QString sURL);
    
    
    /**
     * sets a new pipeline INCOMING URL
     *
     * @param   sURL        the new pipe in URL
     */
    virtual void set_url_pipe_in(QString sURL);
    
    
    /**
     * sets a new pipeline OUTGOING URL
     *
     * @param   sURL        the new pipe out URL
     */
    virtual void set_url_pipe_out(QString sURL);


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
     * checks (and opens) the file for valid input
     *
     * @return  true, if we can read from the file
     */
    bool is_data_accessible();


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
    class qkd_cat_data;
    boost::shared_ptr<qkd_cat_data> d;
    
};


#endif

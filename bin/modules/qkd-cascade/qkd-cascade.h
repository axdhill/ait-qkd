/*
 * qkd-cascade.h
 * 
 * The qkd-cascade is the standard implementation 
 * of the cascade error correction
 * 
 * Author: Philipp Grabenweger
 *         Christoph Pacher, <christoph.pacher@ait.ac.at>
 *         Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *
 * Copyright (C) 2014-2016 AIT Austrian Institute of Technology
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


#ifndef __QKD_MODULE_QKD_CASCADE_H_
#define __QKD_MODULE_QKD_CASCADE_H_


// ------------------------------------------------------------
// incs

// ait
#include <qkd/module/module.h>


// ------------------------------------------------------------
// decl


/**
 * The qkd-cascade is the standard cascade error correction
 * 
 * The qkd-confirmation QKD module supports the "at.ac.ait.qkd.cascade" Interface.
 * 
 * Properties of at.ac.ait.qkd.confirmation
 * 
 *      -name-                  -read/write-    -description-
 * 
 *      passes                      R/W         number of confirmation passes
 * 
 */
class qkd_cascade : public qkd::module::module {
    
    
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "at.ac.ait.qkd.cascade")

    Q_PROPERTY(qulonglong passes READ passes WRITE set_passes)          /**< get/set number of confirmation passes */

       
public:


    /**
     * ctor
     */
    qkd_cascade();
    
    
    /**
     * get the number of passes
     * 
     * @return  the number of passes to do
     */
    qulonglong passes() const;
    
    
    /**
     * set the new number of passes
     * 
     * @param   nPasses     the new number of passes
     */
    void set_passes(qulonglong nPasses);
   

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
     * @return  true, if the key is to be pushed to the output pipe
     */
    virtual bool process(qkd::key::key & cKey, qkd::crypto::crypto_context & cIncomingContext, qkd::crypto::crypto_context & cOutgoingContext);
    
  
    // pimpl
    class qkd_cascade_data;
    std::shared_ptr<qkd_cascade_data> d;
    
};


#endif


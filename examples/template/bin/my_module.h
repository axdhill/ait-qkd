/*
 * my_module.h
 * 
 * This is a header file for an arbitrary QKD module
 * 
 * Author: Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *
 *
 *       -----------------------------------------------------
 *       Please substitute the MY_MODULE_* placements as needed
 *       -----------------------------------------------------
 *
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


#ifndef MY_MODULE_H
#define MY_MODULE_H


// ------------------------------------------------------------
// incs


// get QKD
#include <qkd/qkd.h>


// ------------------------------------------------------------
// the module


class my_module : public qkd::module::module {
    
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.any.my_module")

    /* Uncomment the line below to create a read/write DBus property 
     * If you do, you have to create proper methods like:
     * 
     * public:
     *      int my_int_get();
     *      void my_int_set(int n);
     */
    //Q_PROPERTY(int my_int_get READ my_int_get WRITE my_int_set)
    //Q_PROPERTY(double my_double_get READ my_double_get WRITE my_double_set)
    //Q_PROPERTY(QString my_string_get READ my_string_get WRITE my_string_set)
    
    /* Uncomment the line below to create a read only DBus property
     * If you do, you have to create proper methods like:
     * 
     * public:
     *      int my_int_get();
     */
    //Q_PROPERTY(int my_int_get READ my_int_get)
    //Q_PROPERTY(double my_double_get READ my_double_get)
    //Q_PROPERTY(QString my_string_get READ my_string_get)
    
public:
    
    
    /**
     * constructor
     */
    my_module();
    

/* 
 * Uncomment the lines below to create a DBus method "foo_method" taking a variable "bar"
 
public slots:
    
    void foo_method(int bar);
*/


/* 
 * Uncomment the lines below to create a DBus signal "foo_signal" emitted together with a variable "bar"
 
signals:
    
    void foo_signal(int bar);
*/

    
private:
    
    
    /**
     * module work
     * 
     * @param   cKey                the current new key to work on
     * @param   cIncomingContext    incoming authentication context used for receiving data from peer module
     * @param   cOutgoingContext    outgoing authentication context used when sending data to peer module
     * @return  true, if key should be forwarded to next module
     */
    bool process(qkd::key::key & cKey, qkd::crypto::crypto_context & cIncomingContext, qkd::crypto::crypto_context & cOutgoingContext);
    
    
};


#endif

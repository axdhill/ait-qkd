/*
 * context.cpp
 * 
 * base crypto context implementation
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

 
// ------------------------------------------------------------
// incs

// ait
#include <qkd/crypto/context.h>
#include <qkd/crypto/engine.h>


using namespace qkd::crypto;


// ------------------------------------------------------------
// code


/**
 * default clone implementation
 * 
 * get a scheme string and call engine::create
 * with it
 * 
 * Overwrite this is you need a more sophisticated
 * cloning algorithm
 * 
 * @return  a new cloned context
 */
crypto_context context::clone_internal() const {
    return qkd::crypto::engine::create(scheme());
}


/**
 * returns the a null context
 * 
 * @return  a NULL crypto context
 */
crypto_context context::null_context() {
    static qkd::crypto::crypto_context cNullContxt = qkd::crypto::engine::create("null");
    return cNullContxt;
}



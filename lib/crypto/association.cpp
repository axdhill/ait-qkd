/*
 * association.cpp
 * 
 * implement crypto association details
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

 
// ------------------------------------------------------------
// incs

// ait
#include <qkd/crypto/association.h>


using namespace qkd::crypto;


// ------------------------------------------------------------
// code


/**
 * ctor
 * 
 * @param   cDefinition     definition for creation
 * @throws  algorithm_unknown
 * @throws  scheme_invalid
 * @throws  context_init
 * @throws  context_wrong_key
 */
association::association(association::association_definition const & cDefinition) {
    
    qkd::crypto::scheme cAuthenticationIncomingScheme(cDefinition.sAuthenticationIncoming);
    qkd::crypto::scheme cAuthenticationOutgoingScheme(cDefinition.sAuthenticationOutgoing);
    qkd::crypto::scheme cEncryptionIncomingScheme(cDefinition.sEncryptionIncoming);
    qkd::crypto::scheme cEncryptionOutgoingScheme(cDefinition.sEncryptionOutgoing);
    
    m_cAuthentication.cIncoming = qkd::crypto::engine::create(cAuthenticationIncomingScheme);
    m_cAuthentication.cOutgoing = qkd::crypto::engine::create(cAuthenticationOutgoingScheme);
    m_cEncryption.cIncoming = qkd::crypto::engine::create(cEncryptionIncomingScheme);
    m_cEncryption.cOutgoing = qkd::crypto::engine::create(cEncryptionOutgoingScheme);
}


/**
 * calculate how many key material (in bytes) is consumed with one
 * round if given an association definition.
 * 
 * A "round" is: 1 x message send back and forth both authenticated and
 * encrypted.
 * 
 * @param   cDefinition      the current association definition
 * @return  number of bytes needed for 1 round
 */
uint64_t association::key_consumption(association_definition const & cDefinition) {
    
    uint64_t res = 0;
    
    // try to create an association
    association cAssociation;
    try {
        cAssociation = association(cDefinition);
    }
    catch (...) {
        return 0;
    }
    
    // collect data
    res += cAssociation.authentication().cIncoming->final_key_size() + cAssociation.authentication().cIncoming->init_key_size();
    res += cAssociation.authentication().cOutgoing->final_key_size() + cAssociation.authentication().cOutgoing->init_key_size();
    res += cAssociation.encryption().cIncoming->final_key_size() + cAssociation.encryption().cIncoming->init_key_size();
    res += cAssociation.encryption().cOutgoing->final_key_size() + cAssociation.encryption().cOutgoing->init_key_size();

    return res;
}




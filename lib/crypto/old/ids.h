/*
 * ids.h
 *
 * Identifiers of supplied cryptographic algorithms
 * for the q3p crypto engine
 * 
 * Author: Thomas Themel - thomas.themel@ait.ac.at
 *
 * Copyright (C) 2010-2015 Austrian Institute of Technology
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


#ifndef Q3P_CRYPTO_IDS_H_INCLUDED
#define Q3P_CRYPTO_IDS_H_INCLUDED

/** XOR encryption. 
    Key length must be equal to data length. */
#define CRYPT_ALG_XOR 0x5a5a5a5a

/** Evaluation hash producing 32bit tags.
    Note that the output tags need to 
    be encrypted to achieve epsilon-AXUity! */
#define HASH_ALG_EVHASH_32 0x20

/** Evaluation hash producing 64bit tags.
    Note that the output tags need to 
    be encrypted to achieve epsilon-AXUity! */
#define HASH_ALG_EVHASH_64 0x40

/** Evaluation hash producing 96bit tags.
    Note that the output tags need to 
    be encrypted to achieve epsilon-AXUity! */
#define HASH_ALG_EVHASH_96 0x60

/** Evaluation hash producing 128bit tags.
    Note that the output tags need to 
    be encrypted to achieve epsilon-AXUity! */
#define HASH_ALG_EVHASH_128 0x80

/** Evaluation hash producing 256bit tags.
    Note that the output tags need to 
    be encrypted to achieve epsilon-AXUity! */
#define HASH_ALG_EVHASH_256 0x100

#endif


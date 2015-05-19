/*
 * qkd-bb84.cpp
 * 
 * This is the implementation of the famous BB84 protocol
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

#include <boost/algorithm/string.hpp>

// ait
#include <qkd/utility/bigint.h>
#include <qkd/utility/memory.h>
#include <qkd/utility/syslog.h>

#include "qkd-bb84.h"
#include "qkd_bb84_dbus.h"


// ------------------------------------------------------------
// defs

#define MODULE_DESCRIPTION      "This is the qkd-bb84 QKD Module."
#define MODULE_ORGANISATION     "(C)opyright 2012-2015 AIT Austrian Institute of Technology, http://www.ait.ac.at"


// ------------------------------------------------------------
// decl


/**
 * an event measurement
 */
enum class bb84_base : uint8_t {
    
    BB84_BASE_INVALID = 0,          /**< irregular base measument */
    BB84_BASE_DIAGONAL,             /**< diagonal measument */
    BB84_BASE_RECTILINEAR           /**< rectilinear measument */
};



/**
 * the qkd-bb84 pimpl
 */
class qkd_bb84::qkd_bb84_data {
    
public:

    
    /**
     * ctor
     */
    qkd_bb84_data() : nRawKeyLength(1024), nKeyId(1), nCurrentPosition(0) {
        cAvgBaseRatio = qkd::utility::average_technique::create("value", 10);        
    };
    
    std::recursive_mutex cPropertyMutex;    /**< property mutex */
    
    qkd::utility::average cAvgBaseRatio;    /**< the average base ratio */
    uint64_t nRawKeyLength;                 /**< minimum length of raw key generated in bytes */
    
    qkd::key::key_id nKeyId;                /**< current key id we work on */
    qkd::utility::bigint cBits;             /**< the generated key bits so far */
    uint64_t nCurrentPosition;              /**< current bit position to write */
    
};


// fwd 
static bool base_to_bit(bool & bBit, bb84_base eBase, unsigned char nQuantumEvent);
static void bases_to_bits(qkd::utility::bigint & cBits, uint64_t & nPosition, double & nBaseRatio, bool bAlice, qkd::utility::memory const & cBases, qkd::utility::memory const & cQuantumTable);
static bb84_base get_measurement(unsigned char nEvent);
static qkd::utility::memory quantum_table_to_base_table(qkd::utility::memory const & cQuantumTable);


// ------------------------------------------------------------
// vars


/** 
 * lookup table for the parity in a byte 
 */
static uint8_t const g_nParity[] = {
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,     //   0 -  15
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,     //  16 -  31
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,     //  32 -  47
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,     //  48 -  63
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,     //  64 -  79 
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,     //  80 -  95
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,     //  96 - 111  
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,     // 112 - 127
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,     // 128 - 143
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,     // 144 - 159
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,     // 160 - 175
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,     // 176 - 191
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,     // 192 - 207
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,     // 208 - 223
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,     // 224 - 239
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0      // 240 - 255
};


// ------------------------------------------------------------
// code


/**
 * ctor
 */
qkd_bb84::qkd_bb84() : qkd::module::module("bb84", qkd::module::module_type::TYPE_SIFTING, MODULE_DESCRIPTION, MODULE_ORGANISATION) {

    d = boost::shared_ptr<qkd_bb84::qkd_bb84_data>(new qkd_bb84::qkd_bb84_data());
    
    // setup the key id pattern
    set_key_id_pattern("0/0");
    set_rawkey_length(1024);
    set_key_id_pattern("0/0");
    
    // enforce DBus registration
    new Bb84Adaptor(this);
}


/**
 * apply the loaded key value map to the module
 * 
 * @param   sURL            URL of config file loaded
 * @param   cConfig         map of key --> value
 */
void qkd_bb84::apply_config(UNUSED std::string const & sURL, qkd::utility::properties const & cConfig) {
    
    // delve into the given config
    for (auto const & cEntry : cConfig) {
        
        // grab any key which is intended for us
        if (!is_config_key(cEntry.first)) continue;
        
        // ignore standard config keys: they should have been applied already
        if (is_standard_config_key(cEntry.first)) continue;
        
        std::string sKey = cEntry.first.substr(config_prefix().size());
        
        // module specific config here
        if (sKey == "key_id_pattern") {
            set_key_id_pattern(QString::fromStdString(cEntry.second));
        }
        else
        if (sKey == "rawkey_length") {
            std::stringstream ss(cEntry.second);
            uint64_t nLength = 0;
            ss >> nLength;
            set_rawkey_length(nLength);
        }
        else {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "found unknown key: \"" << cEntry.first << "\" - don't know how to handle this.";
        }
    }
}


/**
 * get the moving average of good shared bases
 * 
 * @return  the moving average of good shared bases
 */
double qkd_bb84::base_ratio() const {
    // get exclusive access to properties
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    return d->cAvgBaseRatio->sum();
}


/**
 * get the current key id we are sifting
 * 
 * @return  the current key id we are sifting
 */
qulonglong qkd_bb84::current_id() const {
    // get exclusive access to properties
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    return d->nKeyId;
}


/**
 * get the current key length in bits we have sifted so far
 * 
 * @return  the current key length in bits we have sifted so far
 */
qulonglong qkd_bb84::current_length() const {
    // get exclusive access to properties
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    return d->nCurrentPosition;
}


/**
 * return the key id pattern as string
 * 
 * the key id pattern is a string consisting of
 *      SHIFT "/" ADD
 * values for key-id generation
 * 
 * @return  the key-id generation pattern string
 */
QString qkd_bb84::key_id_pattern() const {
    
    std::stringstream ss;
    ss << qkd::key::key::counter().shift_value() << "/" << qkd::key::key::counter().add_value();
    return QString::fromStdString(ss.str());
}


/**
 * module work
 * 
 * @param   cKey                    the raw key with quantum events encoded
 * @param   cIncomingContext        incoming crypto context
 * @param   cOutgoingContext        outgoing crypto context
 * @return  always true
 */
bool qkd_bb84::process(qkd::key::key & cKey, qkd::crypto::crypto_context & cIncomingContext, qkd::crypto::crypto_context & cOutgoingContext) {

    if (is_alice()) return process_alice(cKey, cIncomingContext, cOutgoingContext);
    if (is_bob()) return process_bob(cKey, cIncomingContext, cOutgoingContext);
    
    // should not happen to reach this line, but 
    // we return true: pass on the key to the next module
    return true;
}


/**
 * module work as alice
 * 
 * @param   cKey                    the raw key with quantum events encoded
 * @param   cIncomingContext        incoming crypto context
 * @param   cOutgoingContext        outgoing crypto context
 * @return  always true
 */
bool qkd_bb84::process_alice(qkd::key::key & cKey, qkd::crypto::crypto_context & cIncomingContext, qkd::crypto::crypto_context & cOutgoingContext) {
    
    bool bForwardKey = false;
    
    qkd::module::message cMessage;
    
    // first some header to ensure we are talking about the same key
    cMessage.data() << cKey.id();
    cMessage.data() << cKey.size();
    cMessage.data() << (uint64_t)rawkey_length();

    // send metadata to bob
    try {
        send(cMessage, cOutgoingContext);
    }
    catch (std::runtime_error const & cRuntimeError) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to send message: " << cRuntimeError.what();
        return false;
    }
    
    // extract the bases
    qkd::utility::memory cBases = quantum_table_to_base_table(cKey.data());

    // get bases from bob
    cMessage = qkd::module::message();
    try {
        if (!recv(cMessage, cIncomingContext)) return false;
    }
    catch (std::runtime_error const & cRuntimeError) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to receive message: " << cRuntimeError.what();
        return false;
    }
    qkd::utility::memory cBasesPeer;
    cMessage.data() >> cBasesPeer;
    
    // sanity check
    if (cBases.size() != cBasesPeer.size()) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "base tables differ - this must not happen";
        terminate();
        return false;
    }

    // merge our basis
    for (unsigned int i = 0; i < cBases.size(); i++) {
        
        // different bases? --> Alice sets here resp. basis to invalid
        if ((cBasesPeer.get()[i] & 0xC0) != (cBases.get()[i] & 0xC0)) cBases.get()[i] &= 0x3F;
        if ((cBasesPeer.get()[i] & 0x30) != (cBases.get()[i] & 0x30)) cBases.get()[i] &= 0xCF;
        if ((cBasesPeer.get()[i] & 0x0C) != (cBases.get()[i] & 0x0C)) cBases.get()[i] &= 0xF3;
        if ((cBasesPeer.get()[i] & 0x03) != (cBases.get()[i] & 0x03)) cBases.get()[i] &= 0xFC;
    }
    
    // send bases back to Bob
    cMessage = qkd::module::message();
    cMessage.data() << cBases;
    try {
        send(cMessage, cOutgoingContext);
    }
    catch (std::runtime_error const & cRuntimeError) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to send message: " << cRuntimeError.what();
        return false;
    }
    
    // convert the bases to bits
    double nBaseRatio = 1.0;
    bases_to_bits(d->cBits, d->nCurrentPosition, nBaseRatio, true, cBases, cKey.data());

    // set base ratio
    {
        std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
        d->cAvgBaseRatio << nBaseRatio;
    }
    
    // debug
    if (qkd::utility::debug::enabled()) {
        qkd::utility::debug() << "sifted bases: " << cBases.size() * 4 << " used ratio: " << nBaseRatio << " total sifted bits for next key: " << d->nCurrentPosition << " (min. " << rawkey_length() * 8 << ")";
    }

    // check if we have a key to forward
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    if (d->nCurrentPosition >= rawkey_length() * 8) {
        
        // create a new key: we cut the keybits at byte boundaries so max. 7 bits a lost
        qkd::utility::memory cKeyBits = d->cBits.memory();
        cKeyBits.resize(d->nCurrentPosition / 8);
        cKey = qkd::key::key(d->nKeyId, cKeyBits);
        
        // housekeeping key data
        cKey.meta().eKeyState = qkd::key::key_state::KEY_STATE_SIFTED;
        d->nKeyId = qkd::key::key::counter().inc();
        d->cBits = qkd::utility::bigint(d->nRawKeyLength);
        d->nCurrentPosition = 0;
        bForwardKey = true;
    }
    
    return bForwardKey;
}


/**
 * module work as bob
 * 
 * @param   cKey                    the raw key with quantum events encoded
 * @param   cIncomingContext        incoming crypto context
 * @param   cOutgoingContext        outgoing crypto context
 * @return  always true
 */
bool qkd_bb84::process_bob(qkd::key::key & cKey, qkd::crypto::crypto_context & cIncomingContext, qkd::crypto::crypto_context & cOutgoingContext) {
    
    bool bForwardKey = false;
    
    qkd::module::message cMessage;

    // receive meta data from alice
    try {
        if (!recv(cMessage, cIncomingContext)) return false;
    }
    catch (std::runtime_error const & cRuntimeError) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to receive message: " << cRuntimeError.what();
        return false;
    }

    qkd::key::key_id nPeerKeyId = 0;
    uint64_t nPeerSize = 0;
    uint64_t nLength = 0;
    cMessage.data() >> nPeerKeyId;
    cMessage.data() >> nPeerSize;
    cMessage.data() >> nLength;
    
    // check if we both have the same input
    if ((nPeerKeyId != cKey.id()) || (nPeerSize != cKey.size())) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "alice has different input data than me - this must not happen";
        terminate();
        return false;
    }

    // fix raw key length
    set_rawkey_length(nLength);
    
    // send bases to alice
    qkd::utility::memory cBases = quantum_table_to_base_table(cKey.data());
    cMessage = qkd::module::message();
    cMessage.data() << cBases;
    try {
        send(cMessage, cOutgoingContext);
    }
    catch (std::runtime_error const & cRuntimeError) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to send message: " << cRuntimeError.what();
        return false;
    }
    
    // get bases from alice
    try {
        if (!recv(cMessage, cIncomingContext)) return false;
    }
    catch (std::runtime_error const & cRuntimeError) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to receive message: " << cRuntimeError.what();
        return false;
    }
    cMessage.data() >> cBases;

    // convert the bases to bits
    double nBaseRatio = 1.0;
    bases_to_bits(d->cBits, d->nCurrentPosition, nBaseRatio, false, cBases, cKey.data());

    // set base ratio
    {
        std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
        d->cAvgBaseRatio << nBaseRatio;
    }
    
    // debug
    if (qkd::utility::debug::enabled()) {
        qkd::utility::debug() << "sifted bases: " << cBases.size() * 4 << " used ratio: " << nBaseRatio << " total sifted bits for next key: " << d->nCurrentPosition << " (min. " << rawkey_length() * 8 << ")";
    }

    // check if we have a key to forward
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    if (d->nCurrentPosition >= rawkey_length() * 8) {
        
        // create a new key: we cut the keybits at byte boundaries so max. 7 bits a lost
        qkd::utility::memory cKeyBits = d->cBits.memory();
        cKeyBits.resize(d->nCurrentPosition / 8);
        cKey = qkd::key::key(d->nKeyId, cKeyBits);
        
        // housekeeping key data
        cKey.meta().eKeyState = qkd::key::key_state::KEY_STATE_SIFTED;
        d->nKeyId = qkd::key::key::counter().inc();
        d->cBits = qkd::utility::bigint(d->nRawKeyLength);
        d->nCurrentPosition = 0;
        bForwardKey = true;
    }
        
    return bForwardKey;
}


/**
 * get the minimum length of the raw key generated ni bytes
 * 
 * @return  the minimum length of the generated raw key length in bytes
 */
qulonglong qkd_bb84::rawkey_length() const {
    
    // get exclusive access to properties
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    return d->nRawKeyLength;
}


/**
 * sets a new key id pattern as string
 * 
 * the key id pattern is a string consisting of
 *      SHIFT "/" ADD
 * values for key-id generation
 * 
 * @param   sPattern    the new key generation pattern
 */
void qkd_bb84::set_key_id_pattern(QString sPattern) {
    
    // get the tokens
    std::string sPatternStdString = sPattern.toStdString();
    std::vector<std::string> sTokens;
    boost::split(sTokens, sPatternStdString, boost::is_any_of("/"));
    if (sTokens.size() != 2) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to parse '" << sPatternStdString << "' for new key-id pattern";
        return;
    }
    
    // extract the values
    uint32_t nShift = atol(sTokens[0].c_str());
    uint32_t nAdd = atol(sTokens[1].c_str());
    
    // debug to the user
    if (qkd::utility::debug::enabled()) {
        qkd::utility::debug() << "parsed key-id pattern '" << sPatternStdString << "' as shift=" << nShift << " and add=" << nAdd << "; setting new key-id pattern";
    }
    
    // setup a new key-id counter
    qkd::key::key::counter() = qkd::key::key::key_id_counter(nShift, nAdd);
    
    // pick the first id
    d->nKeyId = qkd::key::key::counter().inc();
}


/**
 * set a new minimum length of the generated raw key in bytes
 * 
 * @param   nLength         the new minimum length of the generated raw key length in bytes
 */
void qkd_bb84::set_rawkey_length(qulonglong nLength) {
    
    // get exclusive access to properties
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    
    // don't set a length when it ain't new
    if (d->nRawKeyLength == nLength) return;
    
    d->nRawKeyLength = nLength;
    d->cBits.resize(d->nRawKeyLength * 8);
}


/**
 * convert a single base to a bit
 * 
 * @param   bBit            the bit to calculate
 * @param   eBase           the base
 * @param   nQuantumEvent   the quantum event
 * @return  true, if successfully deteced
 */
bool base_to_bit(bool & bBit, bb84_base eBase, unsigned char nQuantumEvent) {
    
    if ((eBase != bb84_base::BB84_BASE_DIAGONAL) && (eBase != bb84_base::BB84_BASE_RECTILINEAR)) return false;

    // check if we have more than 1 click (that is: if we have an odd number of clicks though)
    if (g_nParity[nQuantumEvent]) {
        bBit = ((nQuantumEvent & 0x55) != 0);
    }
    else {
        // role a dice
        double nRandom = 0.0;
        qkd::utility::random_source::source() >> nRandom;
        bBit = (nRandom >= 0.5);
    }
    
    return true;
}

    
/**
 * convert the bases to key bits
 * 
 * the given basetable will be appended to the
 * bits given of the first param. nPosition holds the
 * position to write the next bit despite the size of the 
 * bits-memory block.
 * 
 * @param   cBits           the key bits so far
 * @param   nPosition       the position within cBits to write next
 * @param   nBaseRatio      the ratio of good bases vs. all bases
 * @param   bAlice          act as alice
 * @param   cBases          the bases
 * @param   cQuantumTable   the quantum event table
 */
void bases_to_bits(qkd::utility::bigint & cBits, uint64_t & nPosition, double & nBaseRatio, bool bAlice, qkd::utility::memory const & cBases, qkd::utility::memory const & cQuantumTable) {
    
    // we have 4 bases in each byte encoded
    uint64_t nBases = cBases.size() * 4;

    // ensure we have enought space to write to
    if ((nPosition + nBases) > cBits.bits()) cBits.resize(nPosition + nBases);
    
    // walk over the bases
    uint64_t nErrors = 0;
    for (uint64_t i = 0; i < cBases.size(); i++) {
        
        // pick the detector events
        bb84_base b0 = static_cast<bb84_base>((cBases[i] & 0xC0) >> 6);
        bb84_base b1 = static_cast<bb84_base>((cBases[i] & 0x30) >> 4);
        bb84_base b2 = static_cast<bb84_base>((cBases[i] & 0x0C) >> 2);
        bb84_base b3 = static_cast<bb84_base>(cBases[i] & 0x03);
        
        bool bBit = false;
        
        // base 0
        if (base_to_bit(bBit, b0, (cQuantumTable[i * 2 + 0] & 0xF0) >> 4)) {
            if (!bAlice) bBit = !bBit;
            cBits.set(nPosition++, bBit);
        }
        else nErrors++;
        
        // base 1
        if (base_to_bit(bBit, b1, cQuantumTable[i * 2 + 0] & 0x0F)) {
            if (!bAlice) bBit = !bBit;
            cBits.set(nPosition++, bBit);
        }
        else nErrors++;
        
        // base 2
        if (base_to_bit(bBit, b2, (cQuantumTable[i * 2 + 1] & 0xF0) >> 4)) {
            if (!bAlice) bBit = !bBit;
            cBits.set(nPosition++, bBit);
        }
        else nErrors++;

        // base 3
        if (base_to_bit(bBit, b3, cQuantumTable[i * 2 + 1] & 0x0F)) {
            if (!bAlice) bBit = !bBit;
            cBits.set(nPosition++, bBit);
        }
        else nErrors++;
    }
    
    // calculate the overall base_ratio
    if (!nBases) nBaseRatio = 0.0;
    else nBaseRatio = (nBases - nErrors) / (double)nBases;
}


/**
 * tests a single event of the Quantum table
 * implents "squashing", Ref. arXiv:0804.3082 and following work by Luetgenhaus
 *
 * @param   nEvent          the event
 * @return  a bb84 measurement
 */
bb84_base get_measurement(unsigned char nEvent) {

    if (nEvent == 0x00) return bb84_base::BB84_BASE_INVALID;

    bool bBaseDiag = (nEvent & 0x03);    // either e==0x01, 0x02, or 0x03
    bool bBaseRect = (nEvent & 0x0C);    // either e==0x04, 0x08, or 0x0C

    // clicks in both bases --> eliminate event [N. Luetkenhaus, priv.communic.]
    if (bBaseRect & bBaseDiag) return bb84_base::BB84_BASE_INVALID;    

    if (bBaseRect) return bb84_base::BB84_BASE_RECTILINEAR;
    return bb84_base::BB84_BASE_DIAGONAL;
}


/**
 * turn the quantum table (detector clicks) into a table of bases
 * 
 * The basis table tells which measurement has been done at which position
 * in the quantum table 
 * 
 * a base table is memory block holding 4 bb84_base values in each byte
 *
 * @param   cQuantumTable       as received
 * @return  bases table
 */
qkd::utility::memory quantum_table_to_base_table(qkd::utility::memory const & cQuantumTable) {
    
    // we have 4 detector bits for a base
    // a base is 00, 01, 10 or 11
    qkd::utility::memory cBases((cQuantumTable.size() + 1) / 2);
    
    // now go through the table and collect the bases
    unsigned char const * cQuantumEvent = cQuantumTable.get();
    for (uint64_t i = 0; i < cBases.size(); i++) {

        // get the bases for 4 events
        unsigned char b0 = (unsigned char)get_measurement((cQuantumEvent[i * 2 + 0] & 0xF0) >> 4);
        unsigned char b1 = (unsigned char)get_measurement(cQuantumEvent[i * 2 + 0] & 0x0F);
        unsigned char b2 = (unsigned char)get_measurement((cQuantumEvent[i * 2 + 1] & 0xF0) >> 4);
        unsigned char b3 = (unsigned char)get_measurement(cQuantumEvent[i * 2 + 1] & 0x0F);
        
        cBases.get()[i] = (b0 << 6) | (b1 << 4) | (b2 << 2) | b3;
    }

    return cBases;
}


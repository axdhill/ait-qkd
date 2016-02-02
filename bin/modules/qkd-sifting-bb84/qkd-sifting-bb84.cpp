/*
 * qkd-sifting-bb84.cpp
 * 
 * This is the implementation of the famous BB84 protocol
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

#include <boost/algorithm/string.hpp>

// ait
#include <qkd/utility/bigint.h>
#include <qkd/utility/memory.h>
#include <qkd/utility/syslog.h>

#include "bb84_base.h"
#include "qkd-sifting-bb84.h"
#include "qkd_sifting_bb84_dbus.h"


// ------------------------------------------------------------
// defs

#define MODULE_DESCRIPTION      "This is the qkd-sifting-bb84 QKD Module."
#define MODULE_ORGANISATION     "(C)opyright 2012-2016 AIT Austrian Institute of Technology, http://www.ait.ac.at"


// ------------------------------------------------------------
// decl


/**
 * the qkd-sifting-bb84 pimpl
 */
class qkd_sifting_bb84::qkd_sifting_bb84_data {
    
public:

    
    /**
     * ctor
     */
    qkd_sifting_bb84_data() : nRawKeyLength(1024), nKeyId(1), nCurrentPosition(0) {
        cAvgBaseRatio = qkd::utility::average_technique::create("value", 10);        
    };
    
    std::recursive_mutex cPropertyMutex;        /**< property mutex */
    
    qkd::utility::average cAvgBaseRatio;        /**< the average base ratio */
    uint64_t nRawKeyLength;                     /**< minimum length of raw key generated in bytes */
    
    qkd::key::key_id nKeyId;                    /**< current key id we work on */
    qkd::utility::bigint cBits;                 /**< the generated key bits so far */
    uint64_t nCurrentPosition;                  /**< current bit position to write */
    
    bool bQAuthEnabled;                         /**< true if qauth algorithm is enabled */
    
    qauth_values cQAuthValuesLocal;             /**< local qauth data values */
    qauth_values cQAuthValuesPeer;              /**< remote qauth data values */
};


// fwd 
//static bool base_to_bit(bool & bBit, bb84_base eBase, unsigned char nQuantumEvent);
//static void bases_to_bits(qkd::utility::bigint & cBits, uint64_t & nPosition, double & nBaseRatio, bool bAlice, qkd::utility::memory const & cBases, qkd::utility::memory const & cQuantumTable);
// static bb84_base get_measurement(unsigned char nEvent);
// static qkd::utility::memory quantum_table_to_base_table(qkd::utility::memory const & cQuantumTable, qauth_ptr cQAuth = qauth_ptr(nullptr));
static bool parse_bool(std::string const & s);


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
qkd_sifting_bb84::qkd_sifting_bb84() : qkd::module::module("bb84", 
        qkd::module::module_type::TYPE_SIFTING, 
        MODULE_DESCRIPTION, 
        MODULE_ORGANISATION) {

    d = boost::shared_ptr<qkd_sifting_bb84::qkd_sifting_bb84_data>(new qkd_sifting_bb84::qkd_sifting_bb84_data());
    
    set_key_id_pattern("0/0");
    set_rawkey_length(1024);
    set_key_id_pattern("0/0");
    
    new Bb84Adaptor(this);
}


/**
 * apply the loaded key value map to the module
 * 
 * @param   sURL            URL of config file loaded
 * @param   cConfig         map of key --> value
 */
void qkd_sifting_bb84::apply_config(UNUSED std::string const & sURL, qkd::utility::properties const & cConfig) {
    
    for (auto const & cEntry : cConfig) {
        
        if (!is_config_key(cEntry.first)) continue;
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
        else
        if (sKey == "qauth") {
            bool bEnable = false;
            try {
                bEnable = parse_bool(cEntry.second);
            }
            catch (std::exception & e) {
                qkd::utility::syslog::warning() << "failed to parse 'qauth' value. " + std::string(e.what());
            }
            set_qauth(bEnable);
        }
        else {
            qkd::utility::syslog::warning() << __FILENAME__ 
                    << '@' 
                    << __LINE__ 
                    << ": " 
                    << "found unknown key: \"" 
                    << cEntry.first 
                    << "\" - don't know how to handle this.";
        }
    }
}


/**
 * get the moving average of good shared bases
 * 
 * @return  the moving average of good shared bases
 */
double qkd_sifting_bb84::base_ratio() const {
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    return d->cAvgBaseRatio->sum();
}


/**
 * create the base table
 * 
 * @param   cKey            the key
 * @param   cQAuthInit      the qauth init (if needed)
 * @return  the base tabel memory
 */
qkd::utility::memory qkd_sifting_bb84::create_base_table(qkd::key::key const & cKey, qauth_init const & cQAuthInit) const {
    
    qkd::utility::memory cBases = quantum_table_to_base_table(extract_quantum_table(cKey.data()));
    d->cQAuthValuesLocal.clear();
    
    if (qauth()) {
        d->cQAuthValuesLocal = ::qauth(cQAuthInit).create_min(cBases.size());
        cBases = merge_qauth_values(cBases, d->cQAuthValuesLocal);
    }
    
    return cBases;
}


/**
 * creates a new QAuth init structure
 * 
 * @return  a new QAuth init struct
 */
qauth_init qkd_sifting_bb84::create_qauth_init() {
    
    if (!qauth()) return qauth_init();

    uint32_t nKv;
    uint32_t nKp;
    uint32_t nModulus = QAUTH_DEFAULT_MODULUS;
    uint32_t nPosition0;
    uint32_t nValue0;
    
    random() >> nKv;
    random() >> nKp;
    random() >> nPosition0;
    random() >> nValue0;
    nPosition0 = nPosition0 % nModulus;
    
    return qauth_init{ nKv, nKp, nModulus, nPosition0, nValue0 };
}


/**
 * get the current key id we are sifting
 * 
 * @return  the current key id we are sifting
 */
qulonglong qkd_sifting_bb84::current_id() const {
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    return d->nKeyId;
}


/**
 * get the current key length in bits we have sifted so far
 * 
 * @return  the current key length in bits we have sifted so far
 */
qulonglong qkd_sifting_bb84::current_length() const {
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    return d->nCurrentPosition;
}


/**
 * exchange the bases with the peer
 * 
 * @param   cBasesPeer          will receive peer bases
 * @param   cBasesLocal         out local bases
 * @param   cIncomingContext    incoming crypto context
 * @param   cOutgoingContext    outgoing crypto context
 * @return  true, if exchange has been successful
 */
bool qkd_sifting_bb84::exchange_bases(qkd::utility::memory & cBasesPeer, 
        qkd::utility::memory const & cBasesLocal, 
        qkd::crypto::crypto_context & cIncomingContext, 
        qkd::crypto::crypto_context & cOutgoingContext) {
    
    if (is_alice()) {
        if (!send_bases(cBasesLocal, cOutgoingContext)) return false;
        if (!recv_bases(cBasesPeer, cIncomingContext)) return false;
    }
    else {
        if (!recv_bases(cBasesPeer, cIncomingContext)) return false;
        if (!send_bases(cBasesLocal, cOutgoingContext)) return false;
    }
    
    return true;
}


/**
 * exchange the qauth base
 * 
 * NOTE: this should be done out-of-band elsewhere
 * 
 * @param   cBasesPeer          will receive peer bases
 * @param   cBasesLocal         out local bases
 * @param   cIncomingContext    incoming crypto context
 * @param   cOutgoingContext    outgoing crypto context
 * @return  true, if exchange has been successful
 */
bool qkd_sifting_bb84::exchange_qauth_init(qauth_init & cQAuthInitPeer, 
            qauth_init const & cQAuthInitLocal, 
        qkd::crypto::crypto_context & cIncomingContext, 
        qkd::crypto::crypto_context & cOutgoingContext) {
    
    if (!qauth()) return true;
    
    if (is_alice()) {
        if (!send_qauth_init(cQAuthInitLocal, cOutgoingContext)) return false;
        if (!recv_qauth_init(cQAuthInitPeer, cIncomingContext)) return false;
    }
    else {
        if (!recv_qauth_init(cQAuthInitPeer, cIncomingContext)) return false;
        if (!send_qauth_init(cQAuthInitLocal, cOutgoingContext)) return false;
    }
    
    return true;
}


/**
 * convert a dense quantum table into a sparse one with each event direct accessible
 * 
 * a dense quantum table has stored a single event into 4 bits. each bit
 * corresponds to a single detector. therefore there are 2 events per byte
 * in the dense table.
 * 
 * the sparse quantum table holds now just 1 single event per byte and is thus
 * easier to access.
 * 
 * @param   cQuantumTable       a dense quantum table
 * @return  a sparse quantum table
 */
qkd::utility::memory qkd_sifting_bb84::extract_quantum_table(qkd::utility::memory const & cQuantumTable) const {
    
    qkd::utility::memory res(cQuantumTable.size() * 2);
    unsigned char const * s = cQuantumTable.get();
    unsigned char * d = res.get();
    
    for (uint64_t i = 0; i < cQuantumTable.size(); ++i) {
        
        (*d) = (((*s) & 0xF0) >> 4);
        ++d;
        (*d) = (*s) & 0x0F;
        ++d;
        ++s;
    }
    
    return res;    
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
QString qkd_sifting_bb84::key_id_pattern() const {
    std::stringstream ss;
    ss << qkd::key::key::counter().shift_value() << "/" << qkd::key::key::counter().add_value();
    return QString::fromStdString(ss.str());
}


/**
 * compare the bases and check qauth (if enabled)
 * 
 * @param   cBases              will receive the final base values
 * @param   cBasesLocal         local bases we have
 * @param   cBasesPeer          bases of the peer
 * @param   cQAuthValuesLocal   local QAuth init values
 * @param   cQAuthValuesPeer    peer's QAuth init values
 * @return  true for success
 */
bool qkd_sifting_bb84::match_bases(UNUSED qkd::utility::memory & cBases,
    qkd::utility::memory const & cBasesLocal, 
    qkd::utility::memory const & cBasesPeer, 
    qauth_values const & cQAuthValuesLocal,
    qauth_values const & cQAuthValuesPeer) {


    qkd::utility::memory cBasesLocalPure;
    if (!split_bases(cBasesLocalPure, cBasesLocal, cQAuthValuesLocal)) {
        qkd::utility::syslog::crit() << "failed to check authentcity of local bases (this is a bug!)";
        return false;
    }
    
    qkd::utility::memory cBasesPeerPure;
    if (!split_bases(cBasesPeerPure, cBasesPeer, cQAuthValuesPeer)) {
        qkd::utility::syslog::crit() << "failed to check authentcity of peer bases";
        return false;
    }
    
    if (qauth()) {
        qkd::utility::debug() << "base exchange is authentic according to QAuth";
    }

    return false;
    
/*    
    qauth_values cQAuthValuesLocal;
    qauth_values cQAuthValuesPeer;
    if (qauth()) {
        cQAuthValuesLocal = qauth_ptr(new ::qauth(cQAuthInitLocal)).create
        
    }

    
    // set up the qauth data streams
    qauth_ptr cQAuthLocal = qauth_ptr(nullptr);
    qauth_value cQAuthValuesLocal;
    qauth_ptr cQAuthPeer = qauth_ptr(nullptr);
    qauth_value cQAuthValuesPeer;
    if (qauth()) {
        cQAuthLocal = qauth_ptr(new ::qauth(cQAuthInitLocal));
        cQAuthValuesLocal = cQAuthLocal->next();
        cQAuthPeer = qauth_ptr(new ::qauth(cQAuthInitPeer));
        cQAuthValuesPeer = cQAuthPeer->next();
    }
    
    uint64_t nBaseIndexLocal = 0;
    uint64_t nBaseIndexPeer = 0;
    uint64_t nPositionLocal = 0;
    uint64_t nPositionPeer = 0;
    uint64_t nPosition = 0;
    
    while ((nBaseIndexLocal < cBasesLocal.size()) && (nBaseIndexPeer < cBasesPeer.size())) {
        
        unsigned char bLocal = cBasesLocal.get()[nBaseIndexLocal];
        
        for (unsigned int i = 0; i < 4; ++i) {
            
            unsigned char bLocal = cBasesLocal.get()
            
            
            
        }
        
        if (nBaseIndexLocal < cBasesLocal.size()) ++nBaseIndexLocal;
        if (nBaseIndexPeer < cBasesPeer.size()) ++nBaseIndexPeer;
    }
*/    
}
    
    
/**
 * merge bases and qauth values
 * 
 * @param   cBases          the genuine bases
 * @param   cQAuthValues    the qauth values
 * @return  megred bases values
 */
qkd::utility::memory qkd_sifting_bb84::merge_qauth_values(qkd::utility::memory const & cBases, qauth_values const & cQAuthValues) const {
    
    qkd::utility::buffer res;
    
    uint64_t nPosition = 0;
    uint64_t nBaseIndex = 0;
    auto iter = cQAuthValues.cbegin();
    unsigned char const * nBasesPtr = cBases.get();
    
    while (nBaseIndex < cBases.size()) {
        
        if ((iter != cQAuthValues.end()) && (nPosition == (*iter).nPosition)) {
            if ((*iter).nValue % 2) {
                res << (unsigned char)bb84_base::BB84_BASE_DIAGONAL;
            }
            else {
                res << (unsigned char)bb84_base::BB84_BASE_RECTILINEAR;
            }
            ++iter;
        }
        else {
            res << *nBasesPtr;
            ++nBasesPtr;
            ++nBaseIndex;
        }
        
        ++nPosition;
    }
    
    // we might missed the last
    if ((iter != cQAuthValues.end()) && (nPosition == (*iter).nPosition)) {
        if ((*iter).nValue % 2) {
            res << (unsigned char)bb84_base::BB84_BASE_DIAGONAL;
        }
        else {
            res << (unsigned char)bb84_base::BB84_BASE_RECTILINEAR;
        }
        ++iter;
    }
    
    qkd::utility::debug() << "qauth data merged: " << cQAuthValues.str();
    
    return res;
}
    
    
/**
 * module work
 * 
 * @param   cKey                    the raw key with quantum events encoded
 * @param   cIncomingContext        incoming crypto context
 * @param   cOutgoingContext        outgoing crypto context
 * @return  always true
 */
bool qkd_sifting_bb84::process(qkd::key::key & cKey, 
        qkd::crypto::crypto_context & cIncomingContext, 
        qkd::crypto::crypto_context & cOutgoingContext) {

    if (!sync_key_data(cKey, cIncomingContext, cOutgoingContext)) return false;
    
    qauth_init cQAuthInitLocal = create_qauth_init();
    qkd::utility::memory cBasesLocal = create_base_table(cKey, cQAuthInitLocal);
    
    qkd::utility::memory cBasesPeer;
    if (!exchange_bases(cBasesPeer, cBasesLocal, cIncomingContext, cOutgoingContext)) return false;
    
    qauth_init cQAuthInitPeer;
    if (!exchange_qauth_init(cQAuthInitPeer, cQAuthInitLocal, cIncomingContext, cOutgoingContext)) return false;
    if (qauth()) {
        d->cQAuthValuesPeer = ::qauth(cQAuthInitPeer).create_max(cBasesPeer.size());
    }
    
    qkd::utility::memory cBasesFinal;
    if (!match_bases(cBasesFinal, cBasesLocal, cBasesPeer, d->cQAuthValuesLocal, d->cQAuthValuesPeer)) return false;
    
    
    // TODO

    return true;


/*    
    bool bForwardKey = false;
    
    
    
    qkd::utility::memory cBases = quantum_table_to_base_table(cKey.data());

    cMessage = qkd::module::message();
    try {
        if (!recv(cMessage, cIncomingContext)) return false;
    }
    catch (std::runtime_error const & cRuntimeError) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " 
                << "failed to receive message: " << cRuntimeError.what();
        return false;
    }
    qkd::utility::memory cBasesPeer;
    cMessage.data() >> cBasesPeer;
    
    qauth_ptr cQAuthPeer = qauth_ptr(nullptr);
    if (qauth()) {
        
        cMessage = qkd::module::message();
        try {
            if (!recv(cMessage, cIncomingContext)) return false;
        }
        catch (std::runtime_error const & cRuntimeError) {
            qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " 
                    << "failed to receive message: " << cRuntimeError.what();
            return false;
        }

        qauth_init cQAuthInit;
        cMessage.data() >> cQAuthInit;
        
        cQAuthPeer = qauth_ptr(new ::qauth(cQAuthInit));
    }
    
    
    if (cBases.size() != cBasesPeer.size()) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " 
                << "base tables differ - this must not happen";
        terminate();
        return false;
    }

    for (unsigned int i = 0; i < cBases.size(); i++) {
        
        // different bases? --> Alice sets here resp. basis to invalid
        if ((cBasesPeer.get()[i] & 0xC0) != (cBases.get()[i] & 0xC0)) cBases.get()[i] &= 0x3F;
        if ((cBasesPeer.get()[i] & 0x30) != (cBases.get()[i] & 0x30)) cBases.get()[i] &= 0xCF;
        if ((cBasesPeer.get()[i] & 0x0C) != (cBases.get()[i] & 0x0C)) cBases.get()[i] &= 0xF3;
        if ((cBasesPeer.get()[i] & 0x03) != (cBases.get()[i] & 0x03)) cBases.get()[i] &= 0xFC;
    }
    
    cMessage = qkd::module::message();
    cMessage.data() << cBases;
    try {
        send(cMessage, cOutgoingContext);
    }
    catch (std::runtime_error const & cRuntimeError) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " 
                << "failed to send message: " << cRuntimeError.what();
        return false;
    }
    
    // convert the bases to bits
    double nBaseRatio = 1.0;
    bases_to_bits(d->cBits, d->nCurrentPosition, nBaseRatio, true, cBases, cKey.data());
    {
        std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
        d->cAvgBaseRatio << nBaseRatio;
    }
    
    if (qkd::utility::debug::enabled()) {
        qkd::utility::debug() << "sifted bases: " << cBases.size() * 4 
                << " used ratio: " << nBaseRatio 
                << " total sifted bits for next key: " << d->nCurrentPosition 
                << " (min. " << rawkey_length() * 8 << ")";
    }

    // check if we have a key to forward
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    if (d->nCurrentPosition >= rawkey_length() * 8) {
        
        // create a new key: we cut the keybits at byte boundaries so max. 7 bits a lost
        qkd::utility::memory cKeyBits = d->cBits.memory();
        cKeyBits.resize(d->nCurrentPosition / 8);
        cKey = qkd::key::key(d->nKeyId, cKeyBits);
        
        cKey.meta().eKeyState = qkd::key::key_state::KEY_STATE_SIFTED;
        d->nKeyId = qkd::key::key::counter().inc();
        d->cBits = qkd::utility::bigint(d->nRawKeyLength);
        d->nCurrentPosition = 0;
        bForwardKey = true;
    }
    
    return bForwardKey;
*/    

    return true;
}


/**
 * module work as bob
 * 
 * @param   cKey                    the raw key with quantum events encoded
 * @param   cIncomingContext        incoming crypto context
 * @param   cOutgoingContext        outgoing crypto context
 * @return  always true
 */
/*
bool qkd_sifting_bb84::process_bob(qkd::key::key & cKey, 
        qkd::crypto::crypto_context & cIncomingContext, 
        qkd::crypto::crypto_context & cOutgoingContext) {

    if (!sync_key_data(cKey, cIncomingContext, cOutgoingContext)) return false;
    
    qauth_init cQAuthInitLocal = create_qauth_init();
    qkd::utility::memory cBasesLocal = create_base_table(cKey, cQAuthInit);
    
    qkd::utility::memory cBasesPeer;
    if (!exchange_bases(cBasesPeer, cBasesLocal, cIncomingContext, cOutgoingContext)) return false;
    
    qauth_init cQAuthInitPeer;
    if (!exchange_qauth_init(cQAuthInitPeer, cQAuthInitLocal, cIncomingContext, cOutgoingContext)) return false;

    qkd::utility::memory cBasesFinal;
    if (!match_bases(cBasesFinal, cBasesLocal, cBasesPeer, cQAuthInitLocal, cQAuthInitPeer)) return false;
    
    
    bool bForwardKey = false;
    
    qauth_init cQAuthInit = create_qauth_init();
    qkd::utility::memory cBasesLocal = create_base_table(cKey, cQAuthInit);
    
    
//    qkd::utility::memory cBases = quantum_table_to_base_table(cKey.data(), cQAuth);
    cMessage = qkd::module::message();
    cMessage.data() << cBases;
    try {
        send(cMessage, cOutgoingContext);
    }
    catch (std::runtime_error const & cRuntimeError) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " 
                << "failed to send message: " << cRuntimeError.what();
        return false;
    }
    
    try {
        if (!recv(cMessage, cIncomingContext)) return false;
    }
    catch (std::runtime_error const & cRuntimeError) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " 
                << "failed to receive message: " << cRuntimeError.what();
        return false;
    }
    cMessage.data() >> cBases;

    // convert the bases to bits
    double nBaseRatio = 1.0;
    bases_to_bits(d->cBits, d->nCurrentPosition, nBaseRatio, false, cBases, cKey.data());
    {
        std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
        d->cAvgBaseRatio << nBaseRatio;
    }
    
    if (qkd::utility::debug::enabled()) {
        qkd::utility::debug() << "sifted bases: " << cBases.size() * 4 
                << " used ratio: " << nBaseRatio 
                << " total sifted bits for next key: " << d->nCurrentPosition 
                << " (min. " << rawkey_length() * 8 << ")";
    }

    // check if we have a key to forward
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    if (d->nCurrentPosition >= rawkey_length() * 8) {
        
        // create a new key: we cut the keybits at byte boundaries so max. 7 bits a lost
        qkd::utility::memory cKeyBits = d->cBits.memory();
        cKeyBits.resize(d->nCurrentPosition / 8);
        cKey = qkd::key::key(d->nKeyId, cKeyBits);
        
        cKey.meta().eKeyState = qkd::key::key_state::KEY_STATE_SIFTED;
        d->nKeyId = qkd::key::key::counter().inc();
        d->cBits = qkd::utility::bigint(d->nRawKeyLength);
        d->nCurrentPosition = 0;
        bForwardKey = true;
    }
        
    return bForwardKey;

    return true;
}
*/    


/**
 * convert a sparse quantum table to base table
 * 
 * a sparse quantum table is created via extract_quantum_table
 * 
 * @param   cQuantumTable           the sparse quantum table
 * @return  the base table
 */
qkd::utility::memory qkd_sifting_bb84::quantum_table_to_base_table(qkd::utility::memory const & cQuantumTable) const {
    
    qkd::utility::memory res(cQuantumTable.size());
    unsigned char * d = res.get();
    unsigned char const * s = cQuantumTable.get();
    for (uint64_t i = 0; i < cQuantumTable.size(); ++i) {
        
        (*d) = (unsigned char)bb84_base::BB84_BASE_INVALID;

        bool bBaseDiag = (*s) & 0x03;    // either event == 0x01, 0x02, or 0x03
        bool bBaseRect = (*s) & 0x0C;    // either event == 0x04, 0x08, or 0x0C

        // clicks in both bases --> eliminate event [N. Luetkenhaus, priv.communic.]
        if ((bBaseRect || bBaseDiag) && !(bBaseRect && bBaseDiag)) {
            if (bBaseRect) {
                (*d) = (unsigned char)bb84_base::BB84_BASE_RECTILINEAR;
            }
            else {
                (*d) = (unsigned char)bb84_base::BB84_BASE_DIAGONAL;
            }
        }
        
        ++s;
        ++d;
    }
    
    return res;
}


/**
 * get the minimum length of the raw key generated ni bytes
 * 
 * @return  the minimum length of the generated raw key length in bytes
 */
qulonglong qkd_sifting_bb84::rawkey_length() const {
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    return d->nRawKeyLength;
}


/**
 * return the QAuth enabled state
 * 
 * see: http://www.iaria.org/conferences2015/awardsICQNM15/icqnm2015_a3.pdf
 * 
 * @return  true, if qauth is enabled
 */
bool qkd_sifting_bb84::qauth() const {
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    return d->bQAuthEnabled;
}


/**
 * receive bases from the peer
 * 
 * @param   cBases                  the bases to receive
 * @param   cIncomingContext        incoming crypto context
 * @return  true, for success
 */
bool qkd_sifting_bb84::recv_bases(qkd::utility::memory & cBases, qkd::crypto::crypto_context & cIncomingContext) {
    
    qkd::module::message cMessage;
    try {
        if (!recv(cMessage, cIncomingContext)) return false;
    }
    catch (std::runtime_error const & cRuntimeError) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " 
                << "failed to receive message: " << cRuntimeError.what();
        return false;
    }
    cMessage.data() >> cBases;
    
    return true;
}


/**
 * receive qauth_init from the peer
 * 
 * NOTE: this should be done out-of-band elsewhere
 * 
 * @param   cQAuthInit              qauth init to receive 
 * @param   cIncomingContext        incoming crypto context
 * @return  true, for success
 */
bool qkd_sifting_bb84::recv_qauth_init(qauth_init & cQAuthInit, qkd::crypto::crypto_context & cIncomingContext) {
    
    qkd::module::message cMessage;
    try {
        if (!recv(cMessage, cIncomingContext)) return false;
    }
    catch (std::runtime_error const & cRuntimeError) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " 
                << "failed to receive message: " << cRuntimeError.what();
        return false;
    }
    cMessage.data() >> cQAuthInit;
    
    return true;
}


/**
 * send bases to the peer
 * 
 * @param   cBases                  the bases to send
 * @param   cOutgoingContext        outgoing crypto context
 * @return  true, for success
 */
bool qkd_sifting_bb84::send_bases(qkd::utility::memory const & cBases, qkd::crypto::crypto_context & cOutgoingContext) {
    
    qkd::module::message cMessage;
    cMessage.data() << cBases;
    try {
        send(cMessage, cOutgoingContext);
    }
    catch (std::runtime_error const & cRuntimeError) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " 
                << "failed to send message: " << cRuntimeError.what();
        return false;
    }
    
    return true;
}


/**
 * send qauth_init to the peer
 * 
 * @param   cQAuthInit              the qauth_init to send
 * @param   cOutgoingContext        outgoing crypto context
 * @return  true, for success
 */
bool qkd_sifting_bb84::send_qauth_init(qauth_init const & cQAuthInit, qkd::crypto::crypto_context & cOutgoingContext) {
    
    qkd::module::message cMessage;
    cMessage.data() << cQAuthInit;
    try {
        send(cMessage, cOutgoingContext);
    }
    catch (std::runtime_error const & cRuntimeError) {
        qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " 
                << "failed to send message: " << cRuntimeError.what();
        return false;
    }
    
    return true;
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
void qkd_sifting_bb84::set_key_id_pattern(QString sPattern) {
    
    std::string sPatternStdString = sPattern.toStdString();
    std::vector<std::string> sTokens;
    boost::split(sTokens, sPatternStdString, boost::is_any_of("/"));
    if (sTokens.size() != 2) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " 
                << "failed to parse '" << sPatternStdString << "' for new key-id pattern";
        return;
    }
    
    uint32_t nShift = atol(sTokens[0].c_str());
    uint32_t nAdd = atol(sTokens[1].c_str());
    
    if (qkd::utility::debug::enabled()) {
        qkd::utility::debug() << "parsed key-id pattern '" << sPatternStdString 
                << "' as shift=" << nShift 
                << " and add=" << nAdd 
                << "; setting new key-id pattern";
    }
    
    qkd::key::key::counter() = qkd::key::key::key_id_counter(nShift, nAdd);
    d->nKeyId = qkd::key::key::counter().inc();
}


/**
 * sets the QAuth enabled state
 * 
 * see: http://www.iaria.org/conferences2015/awardsICQNM15/icqnm2015_a3.pdf
 * 
 * @param   bEnable     the new qauth enabled state
 */
void qkd_sifting_bb84::set_qauth(bool bEnable) {
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    d->bQAuthEnabled = bEnable;
}


/**
 * set a new minimum length of the generated raw key in bytes
 * 
 * @param   nLength         the new minimum length of the generated raw key length in bytes
 */
void qkd_sifting_bb84::set_rawkey_length(qulonglong nLength) {
    
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);

    if (d->nRawKeyLength == nLength) return;

    d->nRawKeyLength = nLength;
    d->cBits.resize(d->nRawKeyLength * 8);
}


/**
 * split a mixed base table into pure and qauth values and check authentcity
 * 
 * "authentic" means the extracted qauth values from cBasesMixed to match
 * exactly the given cQAuthValues.
 * 
 * @param   cBasesPure          the pure base values without qauth values
 * @param   cBasesMixed         the base values intermixed with qauth values
 * @param   cQAuthValues        the "should be" qauth values
 * @return  true, for success (and authentic)
 */
bool qkd_sifting_bb84::split_bases(qkd::utility::memory & cBasesPure, qkd::utility::memory const & cBasesMixed, qauth_values const & cQAuthValues) const {
    
    if (!qauth()) {
        cBasesPure = cBasesMixed;
        return true;
    }
    
    if (cQAuthValues.size() && (cBasesMixed.size() < cQAuthValues.back().nPosition)) {
        // the highest position of the qauth values already exceeds the mixed bases --> this is futile!
        qkd::utility::syslog::crit() << "number of bases is less than amount of necessary QAuth values";
        return false;
    }
    
    qkd::utility::buffer b;
    uint64_t nPosition = 0;
    unsigned char const * c = cBasesMixed.get();
    qauth_values cQAuthExtracted;
    auto iter = cQAuthValues.cbegin();

    while (nPosition < cBasesMixed.size()) {
        
        if ((iter != cQAuthValues.end()) && (nPosition == (*iter).nPosition)) {
            cQAuthExtracted.push_back({nPosition, *c});
            ++iter;
        }
        else {
            b << (*c);
        }
        
        ++c;
        ++nPosition;
    }
    
    if (qkd::utility::debug::enabled()) {
        qkd::utility::debug() << cQAuthValues.str(" expected qauth values: ");
        qkd::utility::debug() << cQAuthExtracted.str("extracted qauth values: ");
        qkd::utility::debug() << "           bases mixed: " << dump_bb84_str(cBasesMixed);
        qkd::utility::debug() << "           bases clean: " << dump_bb84_str(b);
    }
    
    // this is the QAuth authentcity check
    if (cQAuthValues != cQAuthExtracted) return false;
    
    cBasesPure = b;
    
    return true;
}


/**
 * synchronize on our key data with the peer
 * 
 * @param   cKey                    the raw key with quantum events encoded
 * @param   cIncomingContext        incoming crypto context
 * @param   cOutgoingContext        outgoing crypto context
 * @return  true, if we sync'ed key meta data
 */
bool qkd_sifting_bb84::sync_key_data(qkd::key::key & cKey,
        qkd::crypto::crypto_context & cIncomingContext, 
        qkd::crypto::crypto_context & cOutgoingContext) {
    
    qkd::module::message cMessage;
    
    if (is_alice()) {
        
        // alice sends her key meta dat
    
        cMessage.data() << cKey.id();
        cMessage.data() << cKey.size();
        cMessage.data() << (uint64_t)rawkey_length();

        try {
            send(cMessage, cOutgoingContext);
        }
        catch (std::runtime_error const & cRuntimeError) {
            qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " 
                    << "failed to send message: " << cRuntimeError.what();
            return false;
        }
    }
    else {
        
        // bob accept's alice key meta data
        
        try {
            if (!recv(cMessage, cIncomingContext)) return false;
        }
        catch (std::runtime_error const & cRuntimeError) {
            qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " 
                    << "failed to receive message: " << cRuntimeError.what();
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
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " 
                    << "alice has different input data than me - this must not happen";
            terminate();
            return false;
        }
        
        set_rawkey_length(nLength);
    }
    
    return true;
}


/**
 * convert a single base to a bit
 * 
 * @param   bBit            the bit to calculate
 * @param   eBase           the base
 * @param   nQuantumEvent   the quantum event
 * @return  true, if successfully deteced
 */
/*
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
*/

    
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
/*
void bases_to_bits(qkd::utility::bigint & cBits, 
        uint64_t & nPosition, 
        double & nBaseRatio, 
        bool bAlice, qkd::utility::memory const & cBases, 
        qkd::utility::memory const & cQuantumTable) {
    
    // we have 4 bases in each byte encoded
    uint64_t nBases = cBases.size() * 4;

    if ((nPosition + nBases) > cBits.bits()) cBits.resize(nPosition + nBases);
    
    uint64_t nErrors = 0;
    for (uint64_t i = 0; i < cBases.size(); i++) {
        
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
    
    if (!nBases) nBaseRatio = 0.0;
    else nBaseRatio = (nBases - nErrors) / (double)nBases;
}
*/


/**
 * tests a single event of the Quantum table
 * implents "squashing", Ref. arXiv:0804.3082 and following work by Luetgenhaus
 *
 * @param   nEvent          the event
 * @return  a bb84 measurement
 */
// bb84_base get_measurement(unsigned char nEvent) {
// 
//     if (nEvent == 0x00) return bb84_base::BB84_BASE_INVALID;
// 
//     bool bBaseDiag = (nEvent & 0x03);    // either e==0x01, 0x02, or 0x03
//     bool bBaseRect = (nEvent & 0x0C);    // either e==0x04, 0x08, or 0x0C
// 
//     // clicks in both bases --> eliminate event [N. Luetkenhaus, priv.communic.]
//     if (bBaseRect & bBaseDiag) return bb84_base::BB84_BASE_INVALID;    
// 
//     if (bBaseRect) return bb84_base::BB84_BASE_RECTILINEAR;
//     return bb84_base::BB84_BASE_DIAGONAL;
// }


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
// qkd::utility::memory quantum_table_to_base_table(qkd::utility::memory const & cQuantumTable, qauth_ptr cQAuth) {
//     
//     qauth_value cQAuthValue = { 0, 0 };
//     if (cQAuth) cQAuthValue = cQAuth->next();
//     
//     unsigned char const * cQuantumEvent = cQuantumTable.get();
//     qkd::utility::buffer cBases;
//     
//     std::stringstream ssQAuthValues;
//     
//     // we have 4 detector bits for a base
//     // a base is 00, 01, 10 or 11
//     uint64_t nBaseIndex = 0;
//     uint64_t nPosition = 0;
//     while (nBaseIndex < (cQuantumTable.size() + 1) / 2) {
//         
//         unsigned char nBase[4];
//         for (unsigned int i = 0; i < 4; ++i) {
//             
//             if (cQAuth && (nPosition == cQAuthValue.nPosition)) {
//                 
//                 // insert QAuth value instead of real basis
//                 nBase[i] = (unsigned char)((cQAuthValue.nValue % 0x00000001) ? bb84_base::BB84_BASE_RECTILINEAR : bb84_base::BB84_BASE_DIAGONAL);
//                 ssQAuthValues << nPosition << " -> " << ((nBase[i] == (unsigned char)bb84_base::BB84_BASE_RECTILINEAR) ? "+" : "x") << "\n";
//                 cQAuthValue = cQAuth->next();
//             }
//             else {
//                 
//                 // insert quantum base
//                 if (nBaseIndex & 0x01) {
//                     nBase[i] = (unsigned char)get_measurement(*cQuantumEvent & 0x0F);
//                 }
//                 else {
//                     nBase[i] = (unsigned char)get_measurement((*cQuantumEvent & 0xF0) >> 4);
//                 }
//                 
//                 ++nBaseIndex;
//                 if (!(nBaseIndex & 0x01)) ++cQuantumEvent;
//             }
//             
//             ++nPosition;
//         }
//         
//         unsigned char nBasesValue = (nBase[0] << 6) | (nBase[1] << 4) | (nBase[2] << 2) | nBase[3];
//         cBases << nBasesValue;
//     }
//     
// qkd::utility::debug() << __DEBUG_LOCATION__ << "QAuth values: " << ssQAuthValues.str();
//     
//     return cBases;
// }


/**
 * parse a string holding a bool value
 * 
 * @param   s       the string holding a bool value
 * @return  true if the string holds "true", "on", "yes" or "1"
 */
bool parse_bool(std::string const & s) {
    
    std::string sLower(s);
    boost::algorithm::to_lower(sLower);
    
    if (sLower == "true") return true;
    if (sLower == "on") return true;
    if (sLower == "yes") return true;
    if (sLower == "1") return true;
    
    if (sLower == "false") return false;
    if (sLower == "off") return false;
    if (sLower == "no") return false;
    if (sLower == "0") return false;
    
    throw std::runtime_error("not a bool value: '" + s + "'");
}

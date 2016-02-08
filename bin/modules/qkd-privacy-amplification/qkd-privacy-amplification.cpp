/*
 * qkd-privacy-amplification.cpp
 * 
 * This is the implementation of the QKD postprocessing
 * privacy amplification
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
#include <qkd/utility/syslog.h>

#include "ntt.h"
#include "qkd-privacy-amplification.h"
#include "qkd_privacy_amplification_dbus.h"


// ------------------------------------------------------------
// defs

#define MODULE_DESCRIPTION      "This is the qkd-privacy-amplification QKD Module."
#define MODULE_ORGANISATION     "(C)opyright 2012-2016 AIT Austrian Institute of Technology, http://www.ait.ac.at"

#define DEFAULT_SECURITY_BITS   100
// ------------------------------------------------------------
// decl


/**
 * the qkd-privacy-amplification pimpl
 */
class qkd_privacy_amplification::qkd_privacy_amplification_data {
    
public:

    
    /**
     * ctor
     */
    qkd_privacy_amplification_data() : 
        nReductionRate(1.0),
        nSecurityBits(DEFAULT_SECURITY_BITS) {};
    
    std::recursive_mutex cPropertyMutex;    /**< property mutex */
    
    double nReductionRate;                  /**< reduction rate of the key */
    uint64_t nSecurityBits;                 /**< security bits introduced into PA */
    
};


// fwd
bool perform(qkd::key::key & cKey, qkd::key::key const & cInput, qkd::utility::bigint const & cSeed, qkd::utility::bigint const & cShift);
double tau(double nErrorRate);


// ------------------------------------------------------------
// code


/**
 * ctor
 */
qkd_privacy_amplification::qkd_privacy_amplification() : qkd::module::module("privacy-amplification", qkd::module::module_type::TYPE_PRIVACY_AMPLIFICATION, MODULE_DESCRIPTION, MODULE_ORGANISATION) {

    d = boost::shared_ptr<qkd_privacy_amplification::qkd_privacy_amplification_data>(new qkd_privacy_amplification::qkd_privacy_amplification_data());
    
    // apply default values
    set_reduction_rate(1.0);
    set_security_bits(100);
    
    // enforce DBus registration
    new PrivacyamplificationAdaptor(this);
}


/**
 * apply the loaded key value map to the module
 * 
 * @param   sURL            URL of config file loaded
 * @param   cConfig         map of key --> value
 */
void qkd_privacy_amplification::apply_config(UNUSED std::string const & sURL, qkd::utility::properties const & cConfig) {
    
    // delve into the given config
    for (auto const & cEntry : cConfig) {
        
        // grab any key which is intended for us
        if (!is_config_key(cEntry.first)) continue;
        
        // ignore standard config keys: they should have been applied already
        if (is_standard_config_key(cEntry.first)) continue;
        
        std::string sKey = cEntry.first.substr(config_prefix().size());
        
        // module specific config here
        if (sKey == "reduction_rate") {
            set_reduction_rate(std::stod(cEntry.second.c_str()));
        }
        else
        if (sKey == "security_bits") {
            set_security_bits(std::stoull(cEntry.second.c_str()));
        }
        else {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "found unknown key: \"" << cEntry.first << "\" - don't know how to handle this.";
        }
    }
}


/**
 * module work
 * 
 * @param   cKey                    will be set to the loaded key from the file
 * @param   cIncomingContext        incoming crypto context
 * @param   cOutgoingContext        outgoing crypto context
 * @return  always true
 */
bool qkd_privacy_amplification::process(qkd::key::key & cKey, UNUSED qkd::crypto::crypto_context & cIncomingContext, UNUSED qkd::crypto::crypto_context & cOutgoingContext) {

    // get size of seed and shift key
    uint64_t nKeyBits = cKey.data().size() * 8;
    uint64_t const nDisclosedBits = cKey.meta().nDisclosedBits;
    uint64_t nSecurityBits = security_bits();
    uint64_t nSizeOfSeedKey = nKeyBits;
    double nReductionRate = reduction_rate();
    
    // check if we should apply either reduction rate or security bits
    if ((nSecurityBits != 0) && (nReductionRate != 1.0)) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "security bits AND reduction rate set - which to apply? please choose one! confused ...";
    }
    
    // this is the size of the final key
    uint64_t nSizeOfShiftKey = nKeyBits;
    
    // apply security bits?
    if (nSecurityBits > 0) nSizeOfShiftKey = std::floor((double)nSizeOfShiftKey * tau(cKey.meta().nErrorRate) - nDisclosedBits - nSecurityBits);
    
    // apply reduction rate?
    if (nReductionRate != 1.0) nSizeOfShiftKey = std::floor((double)nSizeOfShiftKey * reduction_rate());

    // align to pack size in bytes
    nSizeOfSeedKey = (nSizeOfSeedKey / 8) * 8;
    if ((int64_t)nSizeOfShiftKey <= 0) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "privacy amplification will reduce key size to a value <= 0 - key discarded.";
        return false;
    }

    // some user output to let 'em know what we are up to
    qkd::utility::debug() << "running privacy amplification on key " << cKey.id() << " size (bits) = " << nKeyBits << " error rate = " << cKey.meta().nErrorRate << " disclosed bits = " << nDisclosedBits << " size of reduced key = " << nSizeOfShiftKey;
    
    qkd::utility::memory cSeed(nSizeOfSeedKey / 8);
    qkd::utility::memory cShift(nSizeOfShiftKey / 8);
    
    // alice generates random values and sends them to bob
    if (is_alice()) {

        random() >> cSeed;
        random() >> cShift;
        qkd::module::message cMessage;
        cMessage.data() << cSeed;
        cMessage.data() << cShift;
        try {
            send(cMessage, cOutgoingContext);
        }
        catch (std::runtime_error const & cRuntimeError) {
            qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to send message: " << cRuntimeError.what();
            return false;
        }
        
    }
    else {
        
        qkd::module::message cMessage;
        try {
            if (!recv(cMessage, cIncomingContext, qkd::module::message_type::MESSAGE_TYPE_DATA)) return false;
        }
        catch (std::runtime_error const & cRuntimeError) {
            qkd::utility::syslog::crit() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to receive message: " << cRuntimeError.what();
            return false;
        }
        cMessage.data() >> cSeed;
        cMessage.data() >> cShift;
        
        // verify what we have
        if ((cSeed.size() != nSizeOfSeedKey / 8) || (cShift.size() != nSizeOfShiftKey / 8)) {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "alice sent us seed and/or shift values with unexpected sizes";
            return false;
        }
    }
    
    // run the privacy amplification
    bool bPrivacyAmplification = perform(cKey, cKey, cSeed, cShift);
    if (!bPrivacyAmplification) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "privacy amplification failed";
    }
    
    // fix meta data on result key
    cKey.meta().eKeyState = qkd::key::key_state::KEY_STATE_AMPLIFIED;
    
    return bPrivacyAmplification;
}


/**
 * get the reduction rate of the key
 * 
 * the size of the key is shrunk by this rate value
 * 
 * rate: 0.0 ==> no final key
 *       1.0 ==> no reduction
 *
 * @return  the reduction rate
 */
double qkd_privacy_amplification::reduction_rate() const {
    
    // get exclusive access to properties
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    return d->nReductionRate;
}


/**
 * get the number of security bits
 * 
 * @return  number of security bits introduced into privacy amplification
 */
qulonglong qkd_privacy_amplification::security_bits() const {
    
    // get exclusive access to properties
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    return d->nSecurityBits;
}


/**
 * set the reduction rate of the key
 * 
 * the size of the key is shrunk by this rate value
 * 
 * rate: 0.0 ==> no final key
 *       1.0 ==> no reduction
 *
 * @param   nRate       the new reduction rate of the key
 */
void qkd_privacy_amplification::set_reduction_rate(double nRate) {
    
    // sanity check
    if ((nRate < 0.0) || (nRate > 1.0)) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "refusing to set reduction rate to an invalid value: " << nRate;
        return;
    }
    if (nRate == 0.0) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "reduction rate is 0.0: no final key will be produced - is this intended?";
    }
    
    // get exclusive access to properties
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    d->nReductionRate = nRate;
}


/**
 * set the new number of security bits introduced into privacy amplification
 * 
 * @param   nBits       the new number of security bits 
 */
void qkd_privacy_amplification::set_security_bits(qulonglong nBits) {
    
    // get exclusive access to properties
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    d->nSecurityBits = nBits;
}


/** 
 * performs the privacy amplification hash
 *
 * this is a hash function which merges a seed key, a shift key into a final result
 *
 * @param   cKey            the result key
 * @param   cInput          the original key
 * @param   cSeed           the seed data
 * @param   cShift          the shift key
 * @return  true for ok
 */
bool perform(qkd::key::key & cKey, qkd::key::key const & cInput, qkd::utility::bigint const & cSeed, qkd::utility::bigint const & cShift) {
    
    // TODO: chris: please check
    
    const uint64_t nSumBitCount = cSeed.bits() + cShift.bits();
    const uint64_t nNextLog2 = ld_ceil(nSumBitCount);
    const uint64_t nNTTLength = 1 << nNextLog2;
    const uint64_t nNumberZeroPaddingToeplitz = nNTTLength - nSumBitCount;
    const uint64_t nNumberZeroPaddingInput = nNTTLength - cInput.data().size() * 8;

    // needed resources
    mod * nToeplitz = new mod[nNTTLength];
    mod * nInput    = new mod[nNTTLength];

    assert(nInput);
    assert(nToeplitz);

    /*
     * convert all keys into elements of the finite field
     * 
     * we have to build |first column|first row| of the Toeplitz matrix. 
     * This binary string has to be padded with zeros at one end to reach
     * a length which is a power of 2. we choose to pad this 
     * (not yet reversed) string on the left side. Then the initial key which 
     * has to be privacy-amplified must also be padded with zeros on the left
     * side.
     * 
     * Then we have to do the cross correlation of 
     * |00...00|first column|first row| with |00...00|cKey|, which is in fact the multiplication
     * of the Toeplitz matrix with cKey. 
     *
     * |00...00|first column|first row| is in our case 
     *                      |00...00|cShiftKey|cSeedKey|
     * 
     * The (circular) cross-correlation is defined to be
     *
     * (a cross-cor b)_j = \sum_{i=0}^{N-1} a_i b_{j+i (mod N)}
     *
     * To calculate the cross-correlation we make use of the following property of NTT:
     *
     * InverseNTT(NTT(a)*NTT(b))= (a conv b), where conv
     * denotes (circular) convolution, which is defined as
     *
     * (a conv b)_j = \sum_{i=0}^{N-1} a_i b_{j-i (mod N)},
     *
     * note the minus in front of the i!
     * To get the cross-correlation we thus have to rearrange 
     * the b array, so that
     *
     * index i (mod N) goes into -i (mod N) = (N-i) (mod N).
     *
     * Note that the index i=0 goes into 0, 
     * for i \ne 0 the index i goes into N-i.
     *
     * Also, note that conv is commutative.
     *
     * In our case, b is the "Toeplitz matrix"-input array. 
     *
     * The final result has to be taken modulo 2, of course.
     */

    // index i = 0 remains
    if (nNumberZeroPaddingToeplitz) nToeplitz[0] = 0;
    else nToeplitz[0] =  cShift.get(cShift.bits() - 1);

    // index i goes to N-i which is cSeed in reverse order with offset 1
    mod_from_bigint(nToeplitz + 1, cSeed, true);

    // same as above with cShift
    mod_from_bigint(nToeplitz + cSeed.bits() + 1, cShift, true);

    // now pad with zeros to reach a length which is a power of two, needed for NTT
    for (uint64_t i = nSumBitCount + 1; i < nNTTLength; i++) nToeplitz[i] = 0;

    // pad with zeros in front of the input key 
    for (uint64_t i= 0; i < nNumberZeroPaddingInput; i++) nInput[i] = 0;
    
    // now insert the input key in plain order
    mod_from_bigint(nInput + nNumberZeroPaddingInput, qkd::utility::bigint(cKey.data()), false);

    // now do the actual calculation !!!
    ntt_convolution(nToeplitz, nInput, nNextLog2);
    
    // collect the final bits
    qkd::utility::bigint cBI(cShift.bits());
    for (uint64_t i = 0; i < cShift.bits(); i++) cBI.set(i, (nToeplitz[i] & 0x1) != 0);
    
    // the final key
    cKey.meta() = cInput.meta();
    cKey.data() = cBI.memory();
    
    // clean up the mod arrays
    delete [] nToeplitz;
    delete [] nInput;
    
    return true;
}


/**
 * security coefficient tau
 *
 * @param   nErrorRate       error rate of the key
 * @return  tau
 */
double tau(double nErrorRate) {
    
    // 1 - h(nErrorRate)
    // where h is the binary entropy function
    if (nErrorRate == 0.0) return 1.0;

    // we want positive correlation!
    if (nErrorRate > 0.5) return 0.0;   
    
    return 1 - (-nErrorRate * std::log2(nErrorRate) - (1 - nErrorRate) * log2(1 - nErrorRate));
}



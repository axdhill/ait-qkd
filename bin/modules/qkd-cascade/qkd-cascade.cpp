/*
 * qkd-cascade.cpp
 * 
 * This is the implementation of the cascade 
 * error correction protocol
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


// ------------------------------------------------------------
// incs

#include <random>

// ait
#include <qkd/utility/shannon.h>
#include <qkd/utility/syslog.h>
#include <qkd/utility/random.h>

#include "category.h"
#include "qkd-cascade.h"
#include "parity-checker.h"


// ------------------------------------------------------------
// defs

#define MODULE_DESCRIPTION      "This is the qkd-cascade QKD Module. The AIT standard implementation of the cascade error correction."
#define MODULE_ORGANISATION     "(C)opyright 2014-2016 AIT Austrian Institute of Technology, http://www.ait.ac.at"


// ------------------------------------------------------------
// decl


/**
 * the qkd-cascade pimpl
 */
class qkd_cascade::qkd_cascade_data {
    
public:

    
    /**
     * ctor
     */
    qkd_cascade_data() : nPasses(14) {
        cAvgError = qkd::utility::average_technique::create("value", 10);
        cRandom = qkd::utility::random_source::create("");
    };


    /**
     * generate the identity permutation. 
     *
     * @param   perm        permutation
     * @param   inv_perm    inverse permutation
     * @param   n           size of permutations
     */
    void generate_identity_permutation(std::vector<uint64_t> & perm, std::vector<uint64_t> & inv_perm, uint64_t n);


    /**
     * generate a random permutation. 
     *
     * @param   perm        permutation
     * @param   inv_perm    inverse permutation
     * @param   n           size of permutations
     */
    void generate_random_permutation(std::vector<uint64_t> & perm, std::vector<uint64_t> & inv_perm, uint64_t n);


    /**
     * return a random bit
     *
     * @param   p       bit set probability
     * @return  random bit
     */
    inline bool rand_bit(double p) {
        std::bernoulli_distribution cDistribution(p); 
        return cDistribution(*cRandom);
    }


    /**
     * return a random value in an interval
     *
     * @param   a       left side of interval
     * @param   b       right side of interval
     * @return  a random value between a and b
     */
    int64_t rand_int(int64_t a, int64_t b) {                                                                                                                                                                                                                               
        std::uniform_int_distribution<int64_t> cDistribution(a,b);
        return cDistribution(*cRandom);
    }


    /**
     * set random number generator seed
     *
     * @param   nSeed           the new random number generator seed
     */
    void set_random_seed(qkd::utility::random_source::result_type nSeed) { cRandom->seed(nSeed); };


    std::recursive_mutex cPropertyMutex;                      /**< property mutex */

    qkd::utility::average cAvgError;                          /**< the error rate averaged over the last samples */
    uint64_t nPasses;                                         /**< number of passes */
    std::shared_ptr<qkd::utility::random_source> cRandom;     /**< random engine used */
};


// ------------------------------------------------------------
// code


/**
 * generate the identity permutation. 
 *
 * @param   perm        permutation
 * @param   inv_perm    inverse permutation
 * @param   n           size of permutations
 */
void qkd_cascade::qkd_cascade_data::generate_identity_permutation(std::vector<uint64_t> & perm, std::vector<uint64_t> & inv_perm, uint64_t n) {

    perm.resize(n);
    inv_perm.resize(n);

    for (uint64_t i = 0; i < n; ++i) {
        perm[i] = i;
        inv_perm[i] = i;
    }
}


/**
 * generate a random permutation. 
 *
 * @param   perm        permutation
 * @param   inv_perm    inverse permutation
 * @param   n           size of permutations
 */
void qkd_cascade::qkd_cascade_data::generate_random_permutation(std::vector<uint64_t> & perm, std::vector<uint64_t> & inv_perm, uint64_t n) {

    perm.resize(n);
    inv_perm.resize(n);

    // identity
    for (uint64_t i = 0; i < n; ++i) {
        perm[i] = i;
    }
    
    // swap random positions
    for (unsigned int i = 0; i < n; ++i) {
        uint64_t r = rand_int(i, n - 1);
        std::swap(perm[i], perm[r]);
    }
    
    // collect inverse
    for (unsigned int i = 0; i < n; ++i) {
        inv_perm[perm[i]] = i;
    }
}


/**
 * ctor
 */
qkd_cascade::qkd_cascade() : qkd::module::module("cascade", qkd::module::module_type::TYPE_ERROR_CORRECTION, MODULE_DESCRIPTION, MODULE_ORGANISATION) {
    d = std::shared_ptr<qkd_cascade::qkd_cascade_data>(new qkd_cascade::qkd_cascade_data());
}


/**
 * apply the loaded key value map to the module
 * 
 * @param   sURL            URL of config file loaded
 * @param   cConfig         map of key --> value
 */
void qkd_cascade::apply_config(UNUSED std::string const & sURL, qkd::utility::properties const & cConfig) {
    
    // delve into the given config
    for (auto const & cEntry : cConfig) {
        
        // grab any key which is intended for us
        if (!is_config_key(cEntry.first)) continue;
        
        // ignore standard config keys: they should have been applied already
        if (is_standard_config_key(cEntry.first)) continue;
        
        std::string sKey = cEntry.first.substr(config_prefix().size());

        // module specific config here
        if (sKey == "passes") {
            set_passes(atoll(cEntry.second.c_str()));
        }
        else {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "found unknown key: \"" << cEntry.first << "\" - don't know how to handle this.";
        }
    }
}


/**
 * get the number of passes
 * 
 * @return  the number of passes 
 */
qulonglong qkd_cascade::passes() const {
    
    // get exclusive access to properties
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    return d->nPasses;
}


/**
 * module work
 * 
 * @param   cKey                    the key to correct
 * @param   cIncomingContext        incoming crypto context
 * @param   cOutgoingContext        outgoing crypto context
 * @return  always true
 */
bool qkd_cascade::process(qkd::key::key & cKey, qkd::crypto::crypto_context & cIncomingContext, qkd::crypto::crypto_context & cOutgoingContext) {

    // calculate k values according to opt. (8) in J. Martinez-Mateo, C. Pacher, M. Peev, A. Ciurana, and V. Martin,
    // "Demystifying the Information Reconciliation Protocol Cascade", arXiv:1407.3257v1 

    uint64_t k1;
    uint64_t k2;
    
    // rho is the expected error in the key
    double rho = d->cAvgError->avg();
    uint64_t nKeySizeInBits = cKey.size() * 8;

    // define the 4 block sizes
    if (rho != 0.0) {

        int alpha = lround(ceil(log2(1.0 / rho) - 0.5));
        if (rho <= 0.25) {
            // use original k1 size
            k1 = std::min<uint64_t>(1 << alpha, (nKeySizeInBits + 1) / 2);                
        }
        else {
            // use half k1 size, because experiments have shown is most cases 
            // better efficiency of this choice for rho > 0.25
            k1 = std::min<uint64_t>(1 << std::max<uint64_t>(0, alpha - 1), (nKeySizeInBits + 1) / 2);  
        }
        k2 = std::min<uint64_t>(1 << lround(ceil((alpha + 12.0) / 2.0)), (nKeySizeInBits + 1) / 2);
    }
    else {

        // first run or recent error rate is 0.0
        k1 = (nKeySizeInBits + 1) / 2;
        k2 = (nKeySizeInBits + 1) / 2;
    }

    const uint64_t k3 = std::min<uint64_t>(4096, (nKeySizeInBits + 1) / 2);
    const uint64_t k4 = (nKeySizeInBits + 1) / 2;

    // the cascade key frame
    frame cFrame(cKey);

    // send/recv seed for permutations
    // TODO: remove hardcoded seed value for something dynamic
    // TODO: oliver: ask christoph/phillip if this message exchange should be added
    //               to the frame's exchanged message counter, unsure
    try {

        std::default_random_engine::result_type nSeed = 1;

        if (is_alice()) {

            qkd::utility::buffer cSendBuffer;
            cSendBuffer << nSeed;
            comm(cIncomingContext, cOutgoingContext) << cSendBuffer;
        }
        else {

            qkd::utility::buffer cRecvBuffer;
            comm(cIncomingContext, cOutgoingContext) >> cRecvBuffer;
            cRecvBuffer.reset();
            cRecvBuffer >> nSeed;
        }

        // set seed
        d->set_random_seed(nSeed);

    }
    catch (std::exception & e) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "failed to transmit seed value - " << e.what();
        return false;
    }

    // create the list of different pass categories
    // hence, currently this is not used but defaults
    // to the trivial to use one category for the
    // whole key in each pass
    std::vector<category> cCategories;

    // this is the main cascade pass loop
    for (unsigned int step = 1; step <= passes(); ++step) {              

        std::vector<uint64_t> perm(nKeySizeInBits);
        std::vector<uint64_t> inv_perm(nKeySizeInBits);
        category cCategory;
        cCategory.size = nKeySizeInBits;
        switch (step) {

        case 1:
            cCategory.k = k1;
            break;

        case 2:
            cCategory.k = k2;
            break;

        case 3:
            cCategory.k = k3;
            break;

        default:
            cCategory.k = k4;
        }

        // create permutations
        if (step == 1) {

            // in the first step there is no need to shuffle --> identity permutation
            d->generate_identity_permutation(perm, inv_perm, nKeySizeInBits); 
            cCategory.diffparity_must_be_even = false;                          
        }
        else {

            // for step >=2 the total frame parity must be even
            d->generate_random_permutation(perm, inv_perm, nKeySizeInBits);
            cCategory.diffparity_must_be_even = true; 
        }  

        cCategories.clear();
        cCategories.push_back(cCategory);

    	// for all steps: add "parity-checks", i.e. parity infos to list of checks
        try {
            cFrame.add_checker(new parity_checker(cFrame, perm, inv_perm, cCategories, comm(cIncomingContext, cOutgoingContext)));
        }
        catch (std::exception & e) {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " 
                    << "exception caught while exchanging parities - " << e.what();
            return false;
        }

	    // correct all blocks with different parity in first step
        if (step == 1) {

            // enforce the correction of all odd (parity differs) block in the first step
            try {
                cFrame.checkers()[0]->correct_blocks(cFrame.checkers()[0]->get_odd_parity_blocks()); 
            }
            catch (std::exception & e) {
                qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " 
                        << "exception caught while exchanging parities - " << e.what();
                return false;
            }
        }
        else {

            // invoke parity checker correction on
            // known diff (odd) parity blocks
            // in ascending order until there is no
            // more odd parity block left to check
            while (true) {

                // pick first step with known odd parities
                int corr_step = -1;
                for (unsigned int i = 0; i < step; ++i) {
                    if (cFrame.checkers()[i]->get_odd_parity_blocks().size() > 0) {
                        corr_step = i;
                        break;
                    }
                }

                // found?
                if (corr_step < 0) {
                    break;
                }
	            
                // correct all odd parity blocks (in parallel)
                try {
                    cFrame.checkers()[corr_step]->correct_blocks(cFrame.checkers()[corr_step]->get_odd_parity_blocks());
                }
                catch (std::exception & e) {
                    qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " 
                            << "exception caught while exchanging parities - " << e.what();
                    return false;
                }
            }
        } 
    } 

    // fix key meta data
    cKey.meta().nDisclosedBits = cFrame.transmitted_parities();
    cKey.meta().nErrorRate = (double)cFrame.corrected_bits().size() / ((double)cKey.size() * 8);
    cKey.meta().eKeyState = qkd::key::key_state::KEY_STATE_CORRECTED;

    // output efficiency values
    if (qkd::utility::debug::enabled()) {
        double nDisclosedRate = (double)cKey.meta().nDisclosedBits / ((double)cKey.size() * 8);
        qkd::utility::debug() 
            << "cascade done: " 
            << "errors = " << cFrame.corrected_bits().size() << "/" << cKey.size() * 8
            << ", error rate = " << cKey.meta().nErrorRate 
            << ", disclosed = " << cKey.meta().nDisclosedBits << "/" << cKey.size() * 8 
            << ", efficiency = " << qkd::utility::shannon_efficiency(cKey.meta().nErrorRate, nDisclosedRate);
    }

    return true;
}


/**
 * set the new number of passes
 * 
 * @param   nPasses     the new number of passes
 */
void qkd_cascade::set_passes(qulonglong nPasses) {

    // get exclusive access to properties
    std::lock_guard<std::recursive_mutex> cLock(d->cPropertyMutex);
    d->nPasses = nPasses;
}


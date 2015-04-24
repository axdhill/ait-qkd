/*
 * parity-checker.cpp
 *  
 * Autor: Philipp Grabenweger
 *        Christoph Pacher, <christoph.pacher@ait.ac.at>
 *        Oliver Maurhart, <oliver.maurhart@ait.ac.at>
 *
 * Copyright (C) 2014-2015 AIT Austrian Institute of Technology
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

#include <exception>
#include <list>

// ait
#include <qkd/common_macros.h>
#include <qkd/utility/syslog.h>

#include "category.h"
#include "parity-checker.h"


// ------------------------------------------------------------
// code


/**
 * constructor
 *
 * the constructor already calls the parity block exchange for the
 * very first comparision of the whole frame. from this comparision
 * the odd parity blocks (== parity block peer mismatch) are collected
 * the first time. the number of blocks checked in this first round
 * depend on the block size stored within the given category value.
 *
 * @param   cFrame          the cascade key frame to operate on (key + cascade utility methods)
 * @param   perm            bit position permutation
 * @param   inv_perm        inverse bit position permutation
 * @param   cCategories     categories for parity blocks
 * @param   is_bob          states whether this is bob
 */
parity_checker::parity_checker(
        frame & cFrame, 
        std::vector<uint64_t> const & perm, 
        std::vector<uint64_t> const & inv_perm, 
        std::vector<category> const & categories,     
        qkd::module::communicator cComm)
    : m_cComm(cComm), m_cFrame(cFrame), perm(perm), inv_perm(inv_perm) {

    is_bob = cComm.mod()->is_bob();
    uint64_t nKeySizeInBits = m_cFrame.key().size() * 8;

    // based on the given permutation calculate
    // the partial parity sums
    partial_parity_sums.resize(nKeySizeInBits + 1);
    partial_parity_sums[0] = false;
    for (uint64_t i = 0; i < nKeySizeInBits; ++i) {
        partial_parity_sums[i + 1] = partial_parity_sums[i] ^ m_cFrame.key().get_bit(inv_perm[i]);
    }

    // insert indices of already known correct frame bits into this->m_cCorrectBits
    std::set<uint64_t> const & cCorrectFrameBits = m_cFrame.correct_bits();
    for (auto it = cCorrectFrameBits.begin(); it != cCorrectFrameBits.end(); ++it) {
        m_cCorrectBits.insert(perm[*it]);
    }

    // create the set of parity blocks to check
    // this is done according to the categories.
    // categories devide the whole range of bits
    // into different segements of parity blocks
    // to check

    uint64_t nCategoryOffset = 0;
    for (auto & cCategory : categories) {

        uint64_t nCategorySize = cCategory.size;
        uint64_t k = cCategory.k;
        uint64_t nParityBlocks = (nCategorySize + k - 1) / k;

        std::vector<parity_block> cCalcBlocks;

    	// divide category into new blocks
        for (uint64_t i = 0; i < nParityBlocks; ++i) {

            parity_block cParityBlock;
            cParityBlock.offset = nCategoryOffset + i * k;

            // adjustment for last block size in case remainder is smaller than k
            cParityBlock.size = std::min<uint64_t>(k, nCategorySize - i * k); 
            cCalcBlocks.push_back(cParityBlock);
        }

        // calculate parities and compare with peer
        // this triggers the very first parity comparision on the
        // round this parity checker is repsonible for
        calculate_block_diffparities(cCalcBlocks, cCategory.diffparity_must_be_even);

    	// add to parity_blocks and odd_parity_blocks
        for (auto & cParityBlock : cCalcBlocks) {
            m_cParityBlocks.insert(m_cParityBlocks.end(), cParityBlock);
            if (cParityBlock.diffparity) {

                // comparision in the calculation method found a parity mismatch
                // this block is subject to further investigation
                m_cOddParityBlocks.insert(m_cOddParityBlocks.end(), cParityBlock);
            }
        }

        // move to next segment in the bitstream
        nCategoryOffset += nCategorySize;
    }
}


/**
 * calculate the differential parity between Alice and Bob of a multiple blocks
 * 
 * here parity messages are exchanged with the peer
 *
 * the given cCalcBlocks vector serves as in-out parameter:
 *
 *  1. this is the set of blocks to verify and calculate the parity with the peer
 *  2. if the size of a block inside this list is 1 (==> 1 bit) then we are done
 *  3. else we verify for each remaining block in cCalcBlocks the parity with the peer
 *  4. this sets the parity blocks diffparity value to either true/false
 *  5. if after this step the parity block diffparity is false, the block is treated as correct
 *  6. any parity block with diffparity value set to true is subject to further comparisions
 *
 * @param   cCalcBlocks                     set containing the parity blocks for which their parity shall be calculated
 * @param   bTotalDiffParityMustBeEven      states whether the total differential parity sum of all blocks must be even 
 * @return  true if successful, false otherwise
 */
bool parity_checker::calculate_block_diffparities(std::vector<parity_block> & cCalcBlocks, bool bTotalDiffParityMustBeEven) {

    // the number of parity bits we need to exchange
    unsigned int nExchangeParities = 0;
   
    // calculate number of parities that have to be exchanged and mark
    // each block to validate
    //
    // we iterate over the parity blocks given and set the diffparity to true
    // whenever we detect the need of parity exchange
    //
    for (auto it = cCalcBlocks.begin(); it != cCalcBlocks.end(); ++it) {

        // sanity check: parity block offset + size may not exceed key length
        if (it->offset + it->size > m_cFrame.key().size() * 8) {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "parity_checker::calculate_block_diffparities: block position out of range";
            return false;
        }
        
        // if this block contains only correct bits, we know its diffparity sum is 0
        if (count_correct_bits_in_block(it->offset, it->size) == it->size) {

            // used as signal that we need not exchange this parity
            it->diffparity = false; 
        }
        else {
            // used as signal that we need to exchange this parity
            it->diffparity = true; 
            nExchangeParities++;
        }
    }

    // anything to do at all? --> do not proceed if there ain't not parity block to check
    if (nExchangeParities == 0) {
        return true;
    }
    
    // if the total parity must be even, we can exchange one parity less
    // TODO: oliver: ask christoph/phillip why?
    if (bTotalDiffParityMustBeEven) {
        nExchangeParities--;
    }
    
    // exchange parities with peer
    std::vector<uint8_t> cExchangeParities;
    if (nExchangeParities > 0) { 

        // walk over all given blocks
        for (auto it = cCalcBlocks.begin(); it != cCalcBlocks.end() && (cExchangeParities.size() < nExchangeParities); ++it) {

            // omit those blocks we do not need to exchange parities for
            if (!it->diffparity) continue;

		    // here starts the calculation of the parity of the block
            // calculate block parity without including
            // changes of frame bits after initialisation
            bool bParity = partial_parity_sums[it->offset + it->size] ^ partial_parity_sums[it->offset]; 
                
            auto it1 = m_cChangedBits.lower_bound(it->offset);
            auto it2 = m_cChangedBits.lower_bound(it->offset + it->size);
            unsigned int nChangedBits  = std::distance(it1, it2);
            
            if ((nChangedBits % 2) != 0) {
                // if nChangedBits is odd, the parity must be inverted
                bParity ^= 1; 
            }

            // here ends the calculation of the parity of the block
            cExchangeParities.push_back((uint8_t) bParity);
            m_cFrame.add_transmitted_parities(1);
        }
        
        // for two-party-mode, exchange parities in vector cExchangeParities and XOR them
	    // TODO: we exchange a uint8 for each bit --> change to bit vector
        std::vector<uint8_t> cRemoteParities;
        
        try {

            // send our parities 
            qkd::utility::buffer cSendBuffer;
            cSendBuffer << cExchangeParities;
            m_cComm << cSendBuffer;

            // recv remote parities
            qkd::utility::buffer cRecvBuffer;
            m_cComm >> cRecvBuffer;
            cRecvBuffer.reset();
            cRecvBuffer >> cRemoteParities;
        }
        catch (std::exception & e) {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "exception caught while exchanging parities - " << e.what();
            return false;
        }
        catch (...) {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "unkown exception caught while exchanging parity bits with bob.";
            return false;
        }

        // peer must have sent the same amount of bits
        if (cRemoteParities.size() != cExchangeParities.size()) {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "cascade parity exchange size mismatch with peer - protocol error";
            return false;
        }

        // walk over all parities and compare remote with local ones
        for (uint64_t i = 0; i < cExchangeParities.size(); ++i) {
            cExchangeParities[i] ^= cRemoteParities[i];
        }
        
        m_cFrame.add_transmitted_messages(1); 
    }

    // write back our findings
    bool bParitySum = false;
    uint64_t j = 0;
    for (auto it = cCalcBlocks.begin(); it != cCalcBlocks.end(); ++it) {

        // not all bits of the block had been known --> we had to exchange the parity of this block
        if (it->diffparity) {

            // for all blocks (but the last block in case we have total even parity, i.e. 0)
            if (j < nExchangeParities) {
                
                // now set really the parity difference of the block
                it->diffparity = (bool) cExchangeParities[j]; 

                // running parity sum, needed only in case we have total even parity
                bParitySum ^= (bool) cExchangeParities[j];    

                j++;
            }
            else {

                // in the case the total sum of exchanged parities must be 0, 
                // the last parity is equal to the running sum (0+0=0, 1+1=0)
                it->diffparity = bParitySum;
            }
            
            // add to list of correct bits if parity block size == 1
            if (!it->diffparity && (it->size == 1) && (m_cCorrectBits.find(it->offset) == m_cCorrectBits.end())) {
                m_cFrame.notify_correct_bit(inv_perm[it->offset]);
            }
        }
    }
    
    return true;
}


/**
 * correct multiple blocks of odd parity
 *
 * this is the main work method of the parity-checker object.
 *
 * @param   cCorrBlocks                 the parity blocks to be corrected        
 */
void parity_checker::correct_blocks(std::set<parity_block, compare_odd_parity_block> const & cCorrBlocks) {

    // the idea: 
    //
    // 1. according to the set of blocks to be corrected 
    //    stored in cCorrBlocks we first make a sanity check
    //    on them that checks
    //      a) they are indeed within our set of parity blocks
    //      b) they are indeed to be checked (parity diff flag set)
    //
    // 2. we walk over the corrected blocks now again and
    //    if the parity block has only size 1 (single bit)
    //    we have a bit correction done, otherwise we
    //    split the block in 2 halves and proceed with either
    //    the first or the second half

    std::list<std::set<parity_block, compare_parity_block>::iterator> cCorrBlocksIterators;
    
    // create a list of iterators to all cCorrBlocks
    // do sanity checks whether all blocks to be corrected exist and have different parity
    for (auto iter = cCorrBlocks.begin(); iter != cCorrBlocks.end(); ++iter) {

        auto cParityBlockIterator = m_cParityBlocks.find(*iter);
        if (cParityBlockIterator == m_cParityBlocks.end()) {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "parity_checker::correct_blocks: block not found!";
            return;
        }
        if (!cParityBlockIterator->diffparity) {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "parity_checker::correct_blocks: block has even parity!";
            return;
        }
        
        // the block is valid
        cCorrBlocksIterators.push_back(cParityBlockIterator);
    }

    // container to hold parity subblocks to calculate
    std::vector<parity_block> cCalcBlocks;

    // as long as we have blocks to correct...
    while (cCorrBlocksIterators.size() > 0) {       

        cCalcBlocks.clear();

        // first and second half subblocks
        parity_block cParityBlock1;
        parity_block cParityBlock2;

	    // optimiziation: detect sub-blocks where we know already the parity and single bit blocks
        for (auto iti = cCorrBlocksIterators.begin(); iti != cCorrBlocksIterators.end(); ) {

            // subblock is larger than 1 bit...
            if ((*iti)->size > 1) {

                // first half block
                cParityBlock1.offset = (*iti)->offset;         
                cParityBlock1.size = ((*iti)->size + 1) / 2;

                // second half block
                cParityBlock2.offset = cParityBlock1.offset + cParityBlock1.size;  
                cParityBlock2.size = (*iti)->size - cParityBlock1.size;
                
                uint64_t nCorrectBits = count_correct_bits_in_block(cParityBlock2.offset, cParityBlock2.size);

                // in case the parity of the 2nd half is already known 
                // let us work with the 2nd half (and later do nothing)
                if (nCorrectBits == cParityBlock2.size) {
                    cCalcBlocks.push_back(cParityBlock2);
                }
                else {
                    // let us work on the first half
                    cCalcBlocks.push_back(cParityBlock1);
                }
                iti++;
            }
            else {               

                // single bit block ==> bit error
                uint64_t nCorrectBitOffset = (*iti)->offset;
                if (!is_bob) {

                    // alice notes the bit as "changed remotely"
                    m_cFrame.notify_bit_change_remote(inv_perm[nCorrectBitOffset]);
                }
                else {
                    // bob actually flips the bit realy
                    m_cFrame.flip_bit(inv_perm[nCorrectBitOffset]);
                }
               
                // mark the bit as been corrected
                m_cFrame.notify_correct_bit(inv_perm[nCorrectBitOffset]);

                // remove single bit blocks from the set of block to check
                iti = cCorrBlocksIterators.erase(iti); 
            }
        } 

    	// now we have decided for which half we will calculate the parity
        // ... do the exchange of parity bits with the peer!
    	calculate_block_diffparities(cCalcBlocks, false);

        // sanity check, whether we have not forgotten a block
	    // TODO: how to proceed if the condition below holds true?
        //       the method calculate_block_diffparities does not modify
        //       the size of the cCalcBlocks vector, so why make this
        //       check here, after network transmission?
	    if (cCalcBlocks.size() != cCorrBlocksIterators.size()) {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "parity_checker::correct_blocks: unequal container sizes!";
        }

        // 
        auto it = cCalcBlocks.begin();
        for (auto iti = cCorrBlocksIterators.begin(); iti != cCorrBlocksIterators.end(); ++iti) {

            // first half block
            cParityBlock1.offset = (*iti)->offset;        
            cParityBlock1.size = ((*iti)->size + 1) / 2;

            // second half block
            cParityBlock2.offset = cParityBlock1.offset + cParityBlock1.size; 
            cParityBlock2.size = (*iti)->size - cParityBlock1.size;

            // explanation of following logical expression:
	        // first term on the rhs means that we calculate parity of 2nd half,  
            // 2nd term means that the parity of this half is different
	        // parity of 1st half = EITHER first OR second term is true --> 
	        // we calculated parity of 2nd half and the parity of 2nd half is NOT different --> 
            // parity of 1st half differs
	        // we did NOT calculate parity of the 2nd half (i.e. 1st half) 
            // and the parity of this IS different --> parity of 1st differs
	        cParityBlock1.diffparity = ((it->offset == cParityBlock2.offset) ^ it->diffparity); 
            cParityBlock2.diffparity = !cParityBlock1.diffparity;

	        // add single correct bit to set of correct bits (if not already contained)
            if (!cParityBlock1.diffparity && (cParityBlock1.size == 1) && (m_cCorrectBits.find(cParityBlock1.offset) == m_cCorrectBits.end())) {

                // single bit corrected in first half
                m_cFrame.notify_correct_bit(inv_perm[cParityBlock1.offset]);
            }
            if (!cParityBlock2.diffparity && (cParityBlock2.size == 1) && (m_cCorrectBits.find(cParityBlock2.offset) == m_cCorrectBits.end())) {

                // single bit corrected in second half
                m_cFrame.notify_correct_bit(inv_perm[cParityBlock2.offset]);
            }

            // remove current parity block from the list of odd parity blocks
            if (m_cOddParityBlocks.erase(**iti) != 1) {
                qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "parity_checker::correct_blocks: could not erase **iti! (remove parity block from odd parity blocks)";
            }

            // insert new odd parity block (either first or second)
            // (insert returns a pair, the second part of which indicates success)
            if (!m_cOddParityBlocks.insert(cParityBlock1.diffparity ? cParityBlock1 : cParityBlock2).second)             {
                qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "parity_checker::correct_blocks: could not insert cParityBlock1/cParityBlock2!";
            }

            // update list of parity blocks of this parity checker
            *iti = m_cParityBlocks.erase(*iti);          
            *iti = m_cParityBlocks.insert(*iti, cParityBlock2);
            *iti = m_cParityBlocks.insert(*iti, cParityBlock1);

            // set *iti to block with wrong parity
            if (cParityBlock2.diffparity) {
                (*iti)++;
            }

            // move to next block to calculate
            it++; 
        } 
    }
}


/**
 * get number of surely correct bits inside a block
 * 
 * @param   offset          block offset
 * @param   size            block size in bits
 * @return  number of surely correct bits inside the block
 */
uint64_t parity_checker::count_correct_bits_in_block(uint64_t offset, uint64_t size) const {

    // sanity check: must be inside of key
    if (offset + size > m_cFrame.key().size() * 8)  {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "parity_checker::count_correct_bits_in_block: block position out of range";
        return 0;
    }
    
    auto it1 = m_cCorrectBits.lower_bound(offset);
    auto it2 = m_cCorrectBits.lower_bound(offset + size);
    return std::distance(it1, it2);
}


/**
 * notification function to be called by the frame in case of a bit change 
 *
 * @param   pos         position of the bit change
 */
void parity_checker::notify_bit_change_local(uint64_t pos) {
    notify_bit_change_remote(pos);
    m_cChangedBits.insert(perm[pos]);
}


/**
 * notification function to be called by the frame in case of a bit correction at the other side 
 *
 * this method does either add or remove the parity block to which
 * the given bit position belongs from the set of parity blocks
 * known to be odd (--> to be checked).
 *
 * @param   pos         position of the bit change
 */
void parity_checker::notify_bit_change_remote(uint64_t pos) {   

    // define a parity block element as search pattern 
    // for our set of parity blocks defined
    parity_block cParityBlockSearch;

    cParityBlockSearch.offset = perm[pos];
    cParityBlockSearch.size = 1;

    // find first parity block whose offset is greater than cParityBlockSearch.offset
    auto it = m_cParityBlocks.upper_bound(cParityBlockSearch); 

    // if we decrement the iterator it, then it is now pointing 
    // to the parity block that contains the bit with index cParityBlockSearch.offset
    it--; 

    // check if we have to right block at hand
    if ((perm[pos] < it->offset) || (perm[pos] >= (it->offset + it->size))) {
        qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "unable to locate right parity block in parity checker for bit position";
        return;
    }

    // copy, invert diffparity, erase original and insert updated block
    // this has O(1) complexity as we use the return value of erase as hint for insert
    parity_block cParityBlockNew;
    cParityBlockNew.offset = it->offset;
    cParityBlockNew.size = it->size;
    cParityBlockNew.diffparity = (it->diffparity ^ 1);
    it = m_cParityBlocks.erase(it);
    m_cParityBlocks.insert(it, cParityBlockNew);
   
    // update odd_parity_blocks 
    if (cParityBlockNew.diffparity) {

        // block has changed from even to odd parity: insert it into set to check
        if (!m_cOddParityBlocks.insert(cParityBlockNew).second) {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "unable to insert new parity block after remote change";
        }
    }
    else {

        // block has changed from odd to even parity: remove from set to check
        if (m_cOddParityBlocks.erase(cParityBlockNew) != 1) {
            qkd::utility::syslog::warning() << __FILENAME__ << '@' << __LINE__ << ": " << "unable to erase parity block after remote change";
        }
    }
}


/**
 * notification function to be called by the frame to notify of a correct bit
 *
 * @param   pos         position of the bit change
 */
void parity_checker::notify_correct_bit(uint64_t pos) {
    m_cCorrectBits.insert(perm[pos]);
}


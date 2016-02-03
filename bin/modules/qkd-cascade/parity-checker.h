/*
 * parity-checker.h
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


#ifndef __QKD_MODULE_QKD_CASCADE_PARIRY_CHECKER_H_
#define __QKD_MODULE_QKD_CASCADE_PARIRY_CHECKER_H_


// ------------------------------------------------------------
// incs

// ait
#include <qkd/module/communicator.h>
#include <qkd/module/module.h>

#include "category.h"
#include "frame.h"


// ------------------------------------------------------------
// decl


/**
 * a parity block inside a binary message
 */
struct parity_block {
    unsigned int offset;        /**< start bit index */
    unsigned int size;          /**< block size in bits */
    bool diffparity;            /**< differential parity between Alice and Bob of this block */
};


/**
 * A comparison class for parity blocks
 */
class compare_parity_block {

public:
    
    /**
     * comparison function defining the strict weak ordering criterion. 
     * Parity blocks are compared solely based on their offset
     */
    inline bool operator()(const parity_block & a, const parity_block & b) const { 
        return (a.offset < b.offset); 
    }
};


/**
 * Another comparison class for parity blocks, used for sorting odd parity blocks by size
 */
class compare_odd_parity_block {

public:
    
    /**
     * comparison function defining the strict weak ordering criterion.
     * Parity blocks are compared primarily based on their size; 
     * for equal sizes, their offsets are also compared
     */
    inline bool operator()(const parity_block & a, const parity_block & b) const { 
        return (a.size < b.size || (a.size == b.size && a.offset < b.offset)); 
    }
};


/**
 * parity checker for one cascade step
 *
 * a parity checker is responsible to check the parities with its
 * peer instance in a single cascade step.
 */
class parity_checker {


public:


    /**
     * constructor
     *
     * the constructor already calls the parity block exchange for the
     * very first comparison of the whole frame. from this comparison
     * the odd parity blocks (== parity block peer mismatch) are collected
     * the first time. the number of blocks checked in this first round
     * depend on the block size stored within the given category value.
     *
     * @param   cFrame          the cascade key frame to operate on (key + cascade utility methods)
     * @param   perm            bit position permutation
     * @param   inv_perm        inverse bit position permutation
     * @param   cCategories     categories for parity blocks
     * @param   cComm           module communicator with peer instance
     */
    parity_checker(
            frame & cFrame,
            std::vector<uint64_t> const & perm, 
            std::vector<uint64_t> const & inv_perm, 
            std::vector<category> const & categories,
            qkd::module::communicator cComm); 

   
    /**
     * correct multiple blocks of odd parity
     *
     * this is the main work method of the parity-checker object.
     *
     * @param   cCorrBlocks                 the parity blocks to be corrected        
     */
    void correct_blocks(std::set<parity_block, compare_odd_parity_block> const & cCorrBlocks);


    /**
     * get odd parity blocks
     *
     * all odd parity blocks do have at least 1 error bit
     *
     * @return  const reference to odd_parity_blocks
     */
    std::set<parity_block, compare_odd_parity_block> const & get_odd_parity_blocks() const { return m_cOddParityBlocks; }


    /**
     * notification function to be called by the frame in case of a bit change 
     *
     * @param   pos         position of the bit change
     */
    void notify_bit_change_local(uint64_t pos);


    /**
     * notification function to be called by the frame in case of a bit correction at the other side 
     *
     * @param   pos         position of the bit change
     */
    void notify_bit_change_remote(uint64_t pos);
 

    /**
     * notification function to be called by the frame to notify of a correct bit
     *
     * @param   pos         position of the bit change
     */
    void notify_correct_bit(uint64_t pos);


private:


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
     *  6. any parity block with diffparity value set to true is subject to further comparisons
      *
     * @param   cCalcBlocks                     set containing the parity blocks for which their parity shall be calculated
     * @param   bTotalDiffParityMustBeEven      states whether the total differential parity sum of all blocks must be even 
     * @return  true if successful, false otherwise
     */
    bool calculate_block_diffparities(std::vector<parity_block> & cCalcBlocks, bool bTotalDiffParityMustBeEven);
    
   
     /**
     * get number of surely correct bits inside a block
     * 
     * @param   offset          block offset
     * @param   size            block size in bits
     * @return  number of surely correct bits inside the block
     */
    uint64_t count_correct_bits_in_block(uint64_t offset, uint64_t size) const;
 

private:


    /**
     * communicator with peer module instance
     */    
    qkd::module::communicator m_cComm;


    /**
     * the frame we are working on
     */
    frame & m_cFrame;


    /**
     * permutation of the frame bits 
     */    
    std::vector<uint64_t> perm;                                   


    /**
     * inverse permutation of perm 
     */    
    std::vector<uint64_t> inv_perm;                               


    /**
     * states whether this is bob 
     */    
    bool is_bob;                                                          


    /**
     * partial parity sums for the frame passed at object initialisation 
     * NOT updated for later frame corrections!) 
     */    
    std::vector<bool> partial_parity_sums;                                


    /**
     * a set containing the positions of all those frame bits that were changed since initialisation 
     */    
    std::set<uint64_t> m_cChangedBits;                                


    /**
     * a set containing the positions of all those frame bits that are known to be correct 
     */    
    std::set<uint64_t> m_cCorrectBits;                                


    /**
     * a set of disjoint parity blocks covering the whole frame 
     */    
    std::set<parity_block, compare_parity_block> m_cParityBlocks;           


    /**
     * a set containing all odd parity blocks that are inside parity_blocks 
     * this is a subset of m_cParityBlocks
     */    
    std::set<parity_block, compare_odd_parity_block> m_cOddParityBlocks;   

};

#endif


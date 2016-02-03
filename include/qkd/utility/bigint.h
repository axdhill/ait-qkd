/*
 * bigint.h
 *
 * declare the bigint interface
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


/*
 * TODO: find a way to incorporate a neat index operator like:
 *              bool & bigint::operator[](unsigned long index)
 *       currently only get and set yield something reasonable
 */



#ifndef __QKD_UTILITY_BIGINT_H_
#define __QKD_UTILITY_BIGINT_H_


// ------------------------------------------------------------
// incs

#include <exception>
#include <string>

#include <inttypes.h>

#include <boost/shared_ptr.hpp>

// ait
#include <qkd/utility/memory.h>


// ------------------------------------------------------------
// decls


namespace qkd {
    
namespace utility {    
    

/**
 * the bigint is a large memory block with the ability to easily manipulate single bits
 * 
 * Beware, assignment and copy constructor do SHALLOW copies! That is, the memory block
 * is shared among each copy of the bigint. This is most resource efficient, but may lead
 * to fancy behavior.
 * 
 * If you want to work on a concrete copy of the original object you have to invoke
 * 
 *      clone();
 * 
 * which makes a deep copy.
 * 
 * E.g.
 * 
 *          // create a bigint (holding 1 bit)
 *          bigint A(1);
 * 
 *          // set first bit to TRUE
 *          A.set(0, true);
 * 
 *          // this is a shallow copy
 *          bigint B = A;
 * 
 *          // this is a deep copy
 *          bigint C = A.clone();
 * 
 *          // modify a bit
 *          B.set(0, false);
 * 
 *          bool x = A.get(0);        ===> this will yield now FALSE (B holds the same mem as A)
 *          bool y = C.get(0);        ===> this will yield now TRUE (C has its own memory)
 * 
 * 
 * The bigint supports common binary operations:
 * 
 *      & .... binary AND
 *      | .... binary OR
 *      ^ .... binary XOR
 *      ~ .... binary NOT
 * 
 *      >> ... shift left n bits
 *      << ... shift right n bits
 * 
 * However, one can improve performance by using assignment operators. So instead of
 * 
 *      bigint A = ...
 *      bigint B = ...
 * 
 *      A = A & B;
 * 
 * one is better of with
 * 
 *      A &= B;
 * 
 * since this applies the operation in-place whereas in the first example 
 * on the left hand side a deep copy of A is required first.
 * 
 * Supporting functions are
 * 
 *      as_hex(), as_dec() and as_dual() which return stringified version for human
 *      readable output.
 * 
 *      A bigint can be created by a memory block object (see qkd::utility::memory)
 *      and the later can be created by a hex string (see: qkd::utility::memory::from_hex).
 * 
 * Finally sub() extracts a subset of the bigint. 
 *      But beware: this is a deep copy thus sub() is expensive.
 * 
 */
class bigint {

    
public:


    /**
     * exception type thrown for out of range when accessing bits within the bigint
     */
    struct bigint_bit_out_of_range : virtual std::exception, virtual boost::exception { };
    
    
    /**
     * ctor
     *
     * @param   nBitCount       number of bits this bigint manages
     */
    bigint(uint64_t nBitCount = 0);


    /**
     * ctor
     * 
     * Construct from memory. This is a deep copy. Expensive.
     *
     * @param   cMemory         import a memory blob as bigint
     */
    bigint(qkd::utility::memory const & cMemory);


    /**
     * copy ctor
     * 
     * Beware: this is a shallow copy! 
     * If you want a full copy use the clone() method.
     * 
     * @param   rhs             right hand side
     */
    bigint(bigint const & rhs);
    
    
    /**
     * return a decimal description of the bigint
     * 
     * @return  a string showing the value in decimal
     */
    std::string as_dec() const;
    
    
    /**
     * return a dual description of the bigint
     * 
     * Leading zeros will be padded into the string 
     * to fill bits() length
     * 
     * @return  a string showing the value in dual
     */
    std::string as_dual() const;
    
    
    /**
     * return a hex description of the bigint
     * 
     * Leading zeros will be padded into the string 
     * to fill bits() / 4 length
     * 
     * @return  a string showing the value in hex
     */
    std::string as_hex() const;
    
    
    /**
     * returns the number of bits managed by this bigint
     * 
     * @return  the bigint's bits number
     */
    uint64_t bits() const;

    
    /**
     * return the number of bits set to true
     * 
     * @return  number of bits set
     */
    uint64_t bits_set() const;
    
    
    /**
     * clears the bigint
     * 
     * this sets all bits to 0
     */
    void clear();
    
    
    /**
     * clone myself
     * 
     * this is a deep copy. 
     * If not need be, try to avoid this function. It's expensive.
     * 
     * @return  a full clone of this
     */
    bigint clone() const;
    
    
    /**
     * compares this bigint with another one
     * 
     * @param   rhs     right hand side
     * @return  0 is this == rhs, <0 if this < rhs, >0 if this > rhs
     */
    int64_t compare(bigint const & rhs) const;
    
    
    /**
     * fills the bigint
     * 
     * this sets all bits to 1
     */
    void fill();
    
    
    /**
     * return a bit
     * 
     * @param   nPosition   bit position
     * @return  the bit
     * @throws  bigint_bit_out_of_range
     */
    inline bool get(uint64_t nPosition) const { 
        if (!is_within_range(nPosition)) throw bigint_bit_out_of_range(); 
        return get_bit(nPosition); 
    }

    
    /**
     * checks if the given position is valid within a certain range
     * 
     * @param   nPosition       a bit position
     * @return  true, of this position points to a valid bit of this bigint
     */
    inline bool is_within_range(uint64_t nPosition) const { return (nPosition < bits()); }
    
    
    /**
     * create a bitmask
     * 
     * The size of the bitmask is nSize with all bits set to 0, except for
     * nWidth bits starting at position nStartPos.
     * 
     * @param   nSize           size of the bigint returned
     * @param   nWidth          width of the bitmask
     * @param   nStartPos       starting position of the bitpattern within the mask
     * @return  a bitmask
     */
    static qkd::utility::bigint mask(uint64_t nSize, uint64_t nWidth, uint64_t nStartPos);
    
    
    /**
     * returns a memory blob holding the bigint
     * 
     * This is a deep copy. Expensive.
     * 
     * @return  a memory BLOB describing the bigint
     */
    qkd::utility::memory memory() const;
    
    
    /**
     * binary and
     * 
     * @param   rhs     right hand side
     * @return  this
     */
    bigint & op_and(bigint const & rhs);


    /**
     * binary not
     * 
     * @return  this
     */
    bigint & op_not();


    /**
     * binary or
     * 
     * @param   rhs     right hand side
     * @return  this
     */
    bigint & op_or(bigint const & rhs);
    
    
    /**
     * binary shift left of n bits
     * 
     * empty places are filled with 0
     * 
     * @param   n       number of bits to shift
     * @return  this
     */
    bigint & op_shift_left(uint64_t n);
    

    /**
     * binary shift right of n bits
     * 
     * empty places are filled with 0
     * 
     * @param   n       number of bits to shift
     * @return  this
     */
    bigint & op_shift_right(uint64_t n);
    

    /**
     * binary xor
     * 
     * @param   rhs     right hand side
     * @return  this
     */
    bigint & op_xor(bigint const & rhs);
    
    
    /**
     * return the parity of the bigint
     * 
     * @return  true, if the parity is odd
     */
    inline bool parity() const { return ((bits_set() & 0x01) != 0); }


    /**
     * resizes the bigint
     * 
     * any new bits created here are random
     * 
     * @param   nBits           the new number of bits
     */
    void resize(uint64_t nBits);

    
    /**
     * set a bit
     * 
     * @param   nPosition   bit position
     * @param   bValue      bit value
     * @throws  bigint_bit_out_of_range
     */
    inline void set(uint64_t nPosition, bool bValue) { 
        if (!is_within_range(nPosition)) throw bigint_bit_out_of_range(); 
        set_bit(nPosition, bValue); 
    }
    
    
    /**
     * return a sub-bigint
     * 
     * the bigint returned is a sub-bigint, which starts
     * at the given position and has nLength bits.
     * 
     * if nLength exceeds the bit count it is trimmed.
     * 
     * this is a deep copy.
     * 
     * @param   nPosition       first bit (0-based) of the new bigint
     * @param   nLength         number of bits of the new bigint
     * @return  a deep copy of a bitsting in between this bigint
     */
    bigint sub(uint64_t nPosition, uint64_t nLength) const;

    
private:
    
    
    /**
     * return a bit
     * 
     * no range checking here. if the position is out-of-bounds: bumm!
     * 
     * @param   nPosition   bit position
     * @return  the bit
     */
    bool get_bit(uint64_t nPosition) const;

    
    /**
     * set a bit
     * 
     * no range checking here. if the position is out-of-bounds: bumm!
     * 
     * @param   nPosition   bit position
     * @param   bValue      bit value
     */
    void set_bit(uint64_t nPosition, bool bValue);

    
    // pimpl
    class bigint_data;
    boost::shared_ptr<bigint_data> d;

};


}
    
}


/**
 * == - comparator
 * 
 * @param   lhs     left hand side
 * @param   rhs     right hand side
 * @return  this
 */
inline bool operator==(qkd::utility::bigint const & lhs, qkd::utility::bigint const & rhs) { return lhs.compare(rhs) == 0; }


/**
 * != - comparator
 * 
 * @param   lhs     left hand side
 * @param   rhs     right hand side
 * @return  this
 */
inline bool operator!=(qkd::utility::bigint const & lhs, qkd::utility::bigint const & rhs) { return !(lhs == rhs); }


/**
 * < - comparator
 * 
 * @param   lhs     left hand side
 * @param   rhs     right hand side
 * @return  this
 */
inline bool operator<(qkd::utility::bigint const & lhs, qkd::utility::bigint const & rhs) { return lhs.compare(rhs) < 0; }


/**
 * <= - comparator
 * 
 * @param   lhs     left hand side
 * @param   rhs     right hand side
 * @return  this
 */
inline bool operator<=(qkd::utility::bigint const & lhs, qkd::utility::bigint const & rhs) { return ((lhs < rhs) || (lhs == rhs)); }


/**
 * > - comparator
 * 
 * @param   lhs     left hand side
 * @param   rhs     right hand side
 * @return  this
 */
inline bool operator>(qkd::utility::bigint const & lhs, qkd::utility::bigint const & rhs) { return lhs.compare(rhs) > 0; }


/**
 * >= - comparator
 * 
 * @param   lhs     left hand side
 * @param   rhs     right hand side
 * @return  this
 */
inline bool operator>=(qkd::utility::bigint const & lhs, qkd::utility::bigint const & rhs) { return ((lhs > rhs) || (lhs == rhs)); }


/**
 * & - operator (binary and)
 * 
 * @param   lhs     left hand side
 * @param   rhs     right hand side
 * @return  this
 */
inline qkd::utility::bigint operator&(qkd::utility::bigint const & lhs, qkd::utility::bigint const & rhs) { return lhs.clone().op_and(rhs); }


/**
 * &= - operator (binary and) and assignment
 * 
 * @param   lhs     left hand side
 * @param   rhs     right hand side
 * @return  this
 */
inline qkd::utility::bigint operator&=(qkd::utility::bigint & lhs, qkd::utility::bigint const & rhs) { return lhs.op_and(rhs); }


/**
 * ~ - operator (binary not)
 * 
 * @param   rhs     right hand side
 * @return  this
 */
inline qkd::utility::bigint operator~(qkd::utility::bigint const & rhs) { return rhs.clone().op_not(); }


/**
 * | - operator (binary or)
 * 
 * @param   lhs     left hand side
 * @param   rhs     right hand side
 * @return  this
 */
inline qkd::utility::bigint operator|(qkd::utility::bigint const & lhs, qkd::utility::bigint const & rhs) { return lhs.clone().op_or(rhs); }


/**
 * |= - operator (binary or) and assignment
 * 
 * @param   lhs     left hand side
 * @param   rhs     right hand side
 * @return  this
 */
inline qkd::utility::bigint operator|=(qkd::utility::bigint & lhs, qkd::utility::bigint const & rhs) { return lhs.op_or(rhs); }


/**
 * ^ - operator (binary xor)
 * 
 * @param   lhs     left hand side
 * @param   rhs     right hand side
 * @return  this
 */
inline qkd::utility::bigint operator^(qkd::utility::bigint const & lhs, qkd::utility::bigint const & rhs) { return lhs.clone().op_xor(rhs); }


/**
 * ^= - operator (binary xor) and assignment
 * 
 * @param   lhs     left hand side
 * @param   rhs     right hand side
 * @return  this
 */
inline qkd::utility::bigint operator^=(qkd::utility::bigint & lhs, qkd::utility::bigint const & rhs) { return lhs.op_xor(rhs); }


/**
 * << - shift left
 * 
 * @param   lhs     left hand side
 * @param   rhs     right hand side
 * @return  this
 */
inline qkd::utility::bigint operator<<(qkd::utility::bigint const & lhs, uint64_t rhs) { return lhs.clone().op_shift_left(rhs); }


/**
 * <<= - shift left assignment
 * 
 * @param   lhs     left hand side
 * @param   rhs     right hand side
 * @return  this
 */
inline qkd::utility::bigint operator<<=(qkd::utility::bigint & lhs, uint64_t rhs) { return lhs.op_shift_left(rhs); }


/**
 * >> - shift right
 * 
 * @param   lhs     left hand side
 * @param   rhs     right hand side
 * @return  this
 */
inline qkd::utility::bigint operator>>(qkd::utility::bigint const & lhs, uint64_t rhs) { return lhs.clone().op_shift_right(rhs); }


/**
 * >>= - shift right assignment
 * 
 * @param   lhs     left hand side
 * @param   rhs     right hand side
 * @return  this
 */
inline qkd::utility::bigint operator>>=(qkd::utility::bigint & lhs, uint64_t rhs) { return lhs.op_shift_right(rhs); }


#endif


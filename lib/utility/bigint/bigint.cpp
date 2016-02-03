/*
 * bigint.cpp
 *
 * implement the bigint
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

#include <gmp.h>

// ait
#include <qkd/utility/bigint.h>


using namespace qkd::utility;


// ------------------------------------------------------------
// decl


/**
 * the bigint pimpl
 */
class qkd::utility::bigint::bigint_data {
    
    
public:
    
    
    /**
     * ctor
     */
    bigint_data(uint64_t nBits) : m_nBits(nBits) { mpz_init2(m_cMPZ, nBits); };
    

    /**
     * dtor
     */
    ~bigint_data() { mpz_clear(m_cMPZ); };
    
    
    /**
     * return the number of bits managed
     */
    uint64_t bits() const { return m_nBits; };
    
    
    /**
     * return the value
     */
    mpz_t & get_mpz() { return m_cMPZ; };
    
    
    /**
     * return the value
     */
    mpz_t const & get_mpz() const { return m_cMPZ; };
    
    
    /**
     * resize the data
     * 
     * @param   nSize       the new size in bits
     */
    void resize(uint64_t nBits) { mpz_realloc2(m_cMPZ, nBits); m_nBits = nBits; };
    
    
private:
    
    
    /**
     * number of bits managed
     */
    uint64_t m_nBits;
    
    
    /**
     * the bigint data member
     */
    mpz_t m_cMPZ;
    
};


// ------------------------------------------------------------
// code


/**
 * ctor
 *
 * @param   nBitCount       number of bits this bigint manages
 */
bigint::bigint(uint64_t nBitCount) {
    d = boost::shared_ptr<qkd::utility::bigint::bigint_data>(new qkd::utility::bigint::bigint_data(nBitCount));
}


/**
 * ctor
 * 
 * Construct from memory. This is a deep copy. Expensive.
 *
 * @param   cMemory         import a memory blob as bigint
 */
bigint::bigint(qkd::utility::memory const & cMemory) {
    d = boost::shared_ptr<qkd::utility::bigint::bigint_data>(new qkd::utility::bigint::bigint_data(cMemory.size() * 8));
    mpz_import(d->get_mpz(), cMemory.size(), -1, 1, -1, 0, cMemory.get());
}


/**
 * copy ctor
 * 
 * Beware: this is a shallow copy! 
 * If you want a full copy use the clone() method.
 */
bigint::bigint(bigint const & rhs) {
    d = rhs.d;
}


/**
 * return a decimal description of the bigint
 * 
 * @return  a string showing the value in decimal
 */
std::string bigint::as_dec() const {
    char * sDec = mpz_get_str(nullptr, 10, d->get_mpz());
    std::string sResult = sDec;
    free(sDec);
    return sResult;
}


/**
 * return a dual description of the bigint
 * 
 * Leading zeros will be padded into the string 
 * to fill bits() length
 * 
 * @return  a string showing the value in dual
 */
std::string bigint::as_dual() const {
    
    // get the dual
    std::string sResult(bits(), '0');
    char * sDual = mpz_get_str(nullptr, 2, d->get_mpz());
    
    // overlap?
    if (strlen(sDual) > bits()) {
        sResult = sDual;
    }
    else {
        sResult.replace(bits() - strlen(sDual), strlen(sDual), sDual);
    }

    free(sDual);
    
    return sResult;
}


/**
 * return a hex description of the bigint
 * 
 * Leading zeros will be padded into the string 
 * to fill bits() / 4 length
 * 
 * @return  a string showing the value in hex
 */
std::string bigint::as_hex() const {
    
    // get ceil(bits() /4) as number of digits needed
    uint64_t nDigits = bits() / 4 + ((bits() % 4) ? 1 : 0);

    // get the hex
    std::string sResult(nDigits, '0');
    char * sHex = mpz_get_str(nullptr, 16, d->get_mpz());

    // overlap?
    if (strlen(sHex) > nDigits) {
        sResult = sHex;
    }
    else {
        sResult.replace(nDigits - strlen(sHex), strlen(sHex), sHex);
    }
    
    free(sHex);

    return sResult;
}


/**
 * returns the number of bits managed by this bigint
 * 
 * @return  the bigint's bits number
 */
uint64_t bigint::bits() const {
    return d->bits();
}


/**
 * return the number of bits set to true
 * 
 * @return  number of bits set
 */
uint64_t bigint::bits_set() const {
    return mpz_popcount(d->get_mpz());
}


/**
 * clears the bigint
 * 
 * this sets all bits to 0
 */
void bigint::clear() {
    mpz_xor(d->get_mpz(), d->get_mpz(), d->get_mpz());
}


/**
 * clone myself
 * 
 * this is a deep copy.
 * 
 * @return  a full clone of this
 */
bigint bigint::clone() const {
    
    // create a new bigint
    bigint cBI(d->bits());
    mpz_set(cBI.d->get_mpz(), d->get_mpz());
    
    return cBI;
}


/**
 * compares this bigint with another one
 * 
 * @param   rhs     right hand side
 * @return  0 is this == rhs, <0 if this < rhs, >0 if this > rhs
 */
int64_t bigint::compare(bigint const & rhs) const {
    return mpz_cmp(d->get_mpz(), rhs.d->get_mpz());
}


/**
 * fills the bigint
 * 
 * this sets all bits to 1
 */
void bigint::fill() {
    // TODO: find a better performing way
    for (uint64_t i = 0; i < bits(); i++) set_bit(i, true);
}


/**
 * return a bit
 * 
 * no range checking here. if the position is out-of-bounds: bumm!
 * 
 * @param   nPosition   bit position
 * @return  the bit
 */
bool bigint::get_bit(uint64_t nPosition) const {
    return mpz_tstbit(d->get_mpz(), nPosition);
}


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
qkd::utility::bigint bigint::mask(uint64_t nSize, uint64_t nWidth, uint64_t nStartPos) {
    
    if (!nSize) return bigint(0);
    if (nStartPos >= nSize) throw bigint_bit_out_of_range();
    
    bigint cMask(nSize); 
    cMask.clear();
    
    bigint cBits(nWidth); 
    cBits.fill();
    
    cMask.op_or(cBits);
    cMask.op_shift_left(nStartPos);
    
    return cMask;
}


/**
 * returns a memory blob holding the bigint
 * 
 * This is a deep copy. Expensive.
 * 
 * @return  a memory BLOB describing the bigint
 */
qkd::utility::memory bigint::memory() const {
    
    // the needed size is the number of bits 
    // in bytes rounded up.
    uint64_t nBytesNeeded = (bits() + 7) / 8;
    qkd::utility::memory cMemory(nBytesNeeded);
    
    // write to memory BLOB
    size_t nWritten = 0;
    mpz_export(cMemory.get(), &nWritten, -1, 1, -1, 0, d->get_mpz());
    
    // fix tail (MPIR cuts off the tail if it is 0)
    for (uint64_t i = nWritten; i < nBytesNeeded; i++) cMemory.get()[i] = 0;
    
    return cMemory;
}


/**
 * binary and
 * 
 * @param   rhs     right hand side
 * @return  this
 */
bigint & bigint::op_and(bigint const & rhs) {
    mpz_and(d->get_mpz(), d->get_mpz(), rhs.d->get_mpz());
    return *this;
}


/**
 * binary not
 * 
 * @return  this
 */
bigint & bigint::op_not() {
    
    bigint cBI(bits());
    cBI.fill();
    mpz_xor(d->get_mpz(), d->get_mpz(), cBI.d->get_mpz());
    
    return *this;
}


/**
 * binary or
 * 
 * @param   rhs     right hand side
 * @return  this
 */
bigint & bigint::op_or(bigint const & rhs) {
    mpz_ior(d->get_mpz(), d->get_mpz(), rhs.d->get_mpz());
    return *this;
}


/**
 * binary shift left of n bits
 * 
 * empty places are filled with 0
 * 
 * @param   n       number of bits to shift
 * @return  this
 */
bigint & bigint::op_shift_left(uint64_t n) {

    // don't proceed if we shift all bits
    if (n >= bits()) {
        clear();
        return *this;
    }
    
    // clear the most left bits
    for (uint64_t i = bits(); i > bits() - n; i--) set_bit(i - 1, false);
    
    // a shift left of n bits is a multiplication by 2^n
    mpz_mul_2exp(d->get_mpz(), d->get_mpz(), n);

    return *this;
}


/**
 * binary shift right of n bits
 * 
 * empty places are filled with 0
 * 
 * @param   n       number of bits to shift
 * @return  this
 */
bigint & bigint::op_shift_right(uint64_t n) {
    
    // don't proceed if we shift all bits
    if (n >= bits()) {
        clear();
        return *this;
    }
    
    // a shift right of n bits is a division by 2^n
    mpz_tdiv_q_2exp(d->get_mpz(), d->get_mpz(), n);
    
    // clear insert
    for (uint64_t j = bits() - n; j < bits(); j++) set_bit(j, false);

    return *this;
}


/**
 * binary xor
 * 
 * @param   rhs     right hand side
 * @return  this
 */
bigint & bigint::op_xor(bigint const & rhs) {
    mpz_xor(d->get_mpz(), d->get_mpz(), rhs.d->get_mpz());
    return *this;
}


/**
 * resizes the bigint
 * 
 * any new bits created here are random
 * 
 * @param   nBits           the new number of bits
 */
void bigint::resize(uint64_t nBits) {
    
    if (nBits < bits()) {
        
        // mask out overlapping bits
        // in order not to confuse mpir
        bigint cBI(nBits);
        cBI.fill();
        op_and(cBI);
    }
    
    d->resize(nBits);
}


/**
 * set a bit
 * 
 * no range checking here. if the position is out-of-bounds: bumm!
 * 
 * @param   nPosition   bit position
 * @param   bValue      bit value
 */
void bigint::set_bit(uint64_t nPosition, bool bValue) {
    if (bValue) mpz_setbit(d->get_mpz(), nPosition);
    else mpz_clrbit(d->get_mpz(), nPosition);
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
bigint bigint::sub(uint64_t nPosition, uint64_t nLength) const {
    
    // sanity check
    if (nPosition >= bits()) {
        return bigint(0);
    }
    
    uint64_t nBits = (nPosition + nLength > bits() ? bits() - nPosition : nLength);
    
    // deep copy and shift right
    bigint cBI = clone();
    cBI.op_shift_right(nPosition);
    
    // cut off the most left bits
    bigint cBIMask(nBits);
    cBIMask.fill();
    cBI &= cBIMask;
    
    // fix size
    cBI.resize(nBits);
    
    return cBI;
}

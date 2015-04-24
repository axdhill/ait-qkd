/*
 * CipherValidator.cpp
 *
 *  Created on: 14.11.2013
 *      Author: mrs
 */

#include <iostream>
#include <string>

#include "CipherValidator.h"

using namespace qkd::IPsec;

CipherValidator::CipherValidator(std::string algorithm) {
	if((algorithm=="cipher_null")||(algorithm=="ecb(cipher_null)")) key_length=0;
	else if((algorithm=="des" 	  ) || (algorithm=="cbc(des)"     ))   	key_length=64;
	else if((algorithm=="des3_ede") || (algorithm=="cbc(des3_ede)"))    key_length=192;
	else if((algorithm=="cast5"   ) || (algorithm=="cbc(cast5)"   ))    key_length=128;
	else if((algorithm=="blowfish") || (algorithm=="cbc(blowfish)"))    key_length=448;
	else if((algorithm=="aes"     ) || (algorithm=="cbc(aes)"     ))    key_length=256;
	else if((algorithm=="serpent" ) || (algorithm=="cbc(serpent)" ))    key_length=256;
	else if((algorithm=="camellia") || (algorithm=="cbc(camellia)"))    key_length=256;
	else if((algorithm=="twofish" ) || (algorithm=="cbc(twofish)" ))    key_length=256;
	else if(algorithm=="rfc3686(ctr(aes))") 							key_length=256;
	else key_length=-1;
}


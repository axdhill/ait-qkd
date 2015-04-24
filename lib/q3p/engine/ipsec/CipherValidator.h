/*
 * CipherValidator.h
 *
 *  Created on: 14.11.2013
 *      Author: mrs
 */

#ifndef CIPHERVALIDATOR_H_
#define CIPHERVALIDATOR_H_

#include <string>

namespace qkd{

namespace IPsec{

class CipherValidator {
public:
	CipherValidator(std::string algorithm);
	//virtual ~CipherValidator();
	int getKeyLength() {return key_length;};
private:
	int key_length;
};

} /* namepsace QuaKE */

} /* namepace qkd */

#endif /* CIPHERVALIDATOR_H_ */

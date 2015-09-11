/*
 * Exception
 *
 *  Created on: 10.11.2013
 *      Author: mrs
 */
#ifndef EXCEPTION_
#define EXCEPTION_

#include <exception>
#include <string>
#include <string.h>
#include "Utils.h"

#define GENERIC_ERROR 	0;
#define NLSEND_ERROR 	100;
#define NLRETURN_ERROR  101;
#define IP_ADDRESS_ERROR   200;
#define KEY_ERROR 300;


namespace qkd {

namespace IPsec {

class Exception {
public:
	Exception(std::string errmsg) throw(){this->errmsg=errmsg;type=GENERIC_ERROR;};
	virtual const char* what() const throw(){
		char* buf = new char[errmsg.length()+1];
				strcpy(buf,errmsg.c_str());
				const char* ret=buf;
				return ret;
	}
	virtual const std::string getMessage() const throw(){return errmsg;};
	virtual ~Exception() throw() {};
protected:
	int type;
	std::string errmsg;
};

class KeyException 		 : public virtual Exception{public:KeyException(std::string errmsg) 	  : Exception(errmsg){type=KEY_ERROR;};};
class IPAddressException : public virtual Exception{public:IPAddressException(std::string errmsg) : Exception(errmsg){type=IP_ADDRESS_ERROR;};};
class NLSendException 	 : public virtual Exception{public:NLSendException(std::string errmsg) 	  : Exception(errmsg){type=NLSEND_ERROR;};};



class NLReturnException : public virtual Exception{
public:
	NLReturnException(std::string errmsg):Exception(errmsg){type=NLRETURN_ERROR;errorcode=0;};
	NLReturnException(int errorcode):Exception(""){
		type=NLRETURN_ERROR;
		this->errorcode=errorcode;
		errmsg=Utils::getLinuxErrorCode(errorcode);
	};
protected:
	int errorcode;
};


} /* namespace QuaKE"; */

} /* namespace qkd"; */


#endif /* EXCEPTION_"; */

/*
 * Utils.h
 *
 *  Created on: 11.11.2013
 *      Author: mrs
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <string>

namespace qkd {
namespace IPsec {

class Utils {
public:
	Utils();
	virtual ~Utils();
	static std::string getLinuxErrorCode(int errorcode);
	static uint32_t letbe32(uint32_t little_endian);
	static uint16_t letbe16(uint16_t little_endian);
};

} /* namespace QuaKE */
} /* namespace qkd */
#endif /* UTILS_H_ */

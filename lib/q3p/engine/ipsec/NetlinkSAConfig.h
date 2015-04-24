/*
 * NetlinkSAConfig.h
 *
 *  Created on: 01.12.2013
 *      Author: mrs
 */

#ifndef NETLINKSACONFIG_H_
#define NETLINKSACONFIG_H_

#include "NLconstants.h"
#include <string>


namespace qkd {

namespace IPsec {

class NetlinkSAConfig {
public:
	NetlinkSAConfig(char* key= NULL, std::string cipher = "aes", unsigned int spi = 0,
			unsigned long long hard_byte_limit = XFRM_INF,
			unsigned long long soft_byte_limit=XFRM_INF,
			unsigned long long hard_packet_limit=XFRM_INF,
			unsigned long long soft_packet_limit= XFRM_INF);
	char* getKey();
	std::string getCipher();
	uint32_t getSpi();
	uint64_t getSoft_byte_limit();
	uint64_t getHard_byte_limit();
	uint64_t getSoft_packet_limit();
	uint64_t getHard_packet_limit();
private:
	char* key;
	std::string cipher;
	int spi;
	long long hard_byte_limit, soft_byte_limit, hard_packet_limit, soft_packet_limit;
};

} /* namespace IPsec */

} /* namespace qkd */

#endif /* NETLINKSACONFIG_H_ */

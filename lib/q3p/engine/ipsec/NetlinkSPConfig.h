/*
 * SPconfig.h
 *
 *  Created on: 17.11.2013
 *      Author: mrs
 */

#ifndef SPCONFIG_H_
#define SPCONFIG_H_

#include "NLconstants.h"

namespace qkd {

namespace IPsec {


class NetlinkSPConfig {
public:
	NetlinkSPConfig(bool outbound=false, unsigned spi=0, unsigned source_port=0, unsigned destination_port=0, unsigned source_portmask=0, unsigned destination_portmask=0);
	bool getOutbound();
	uint16_t  getSource_port();
	uint16_t  getDestination_port();
	uint16_t  getSource_portmask();
	uint16_t  getDestination_portmask();
	uint32_t  getSpi();
private:
	bool outbound;
	unsigned  spi, source_port, destination_port, source_portmask, destination_portmask;
};

} /* namespace QuaKE */

} /* namespace qkd */

#endif /* SPCONFIG_H_ */

/*
 * IPsecConnectionManager.h
 *
 *  Created on: 18.11.2013
 *      Author: mrs
 */

#include "KernelIPsecManager.h"
#include "NetlinkIPsecManager.h"

#ifndef IPSECCONNECTIONMANAGER_H_
#define IPSECCONNECTIONMANAGER_H_


namespace qkd {

namespace IPsec {


class IPsecConnectionManager {
public:
	IPsecConnectionManager(KernelIPsecManager& manager_param);//& manager_param);

	KernelIPsecManager const & manager() const { return kernel_manager; }
	
private:
	KernelIPsecManager &kernel_manager;
};

#endif /* IPSECCONNECTIONMANAGER_H_ */

} /* namespace QuaKE */

} /* namespace qkd */

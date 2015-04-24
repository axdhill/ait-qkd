/*
 * KernelManager.h
 *
 *  Created on: 18.11.2013
 *      Author: mrs
 */

#ifndef KERNELIPSECMANAGER_H_
#define KERNELIPSECMANAGER_H_

#include "IPAddress.h"
#include "NetlinkSPConfig.h"
#include "NetlinkSAConfig.h"

namespace qkd {

namespace IPsec {

class KernelIPsecManager {
public:
	//KernelIPsecManager(IPAddress source_add, IPAddress destination_add);
	KernelIPsecManager(){};
	virtual ~KernelIPsecManager(){};
	virtual int addSP(NetlinkSPConfig config=NetlinkSPConfig())=0;
	virtual int addSP(IPAddress tunnel_source_add, IPAddress tunnel_destination_add, NetlinkSPConfig config=NetlinkSPConfig())=0;
	virtual int deleteSP()=0;
	virtual int addSA(NetlinkSAConfig config=NetlinkSAConfig())=0;
	virtual int deleteSA()=0;
	virtual void UpdateKey(char* key)=0;
};

} /* namespace QuaKE */

} /* namespace qkd */

#endif /* KERNELIPSECMANAGER_H_ */

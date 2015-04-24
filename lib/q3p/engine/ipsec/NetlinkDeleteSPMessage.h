/*
 * NetlinkDeleteSPMessage.h
 *
 *  Created on: 15.11.2013
 *      Author: mrs
 */

#ifndef NETLINKDELETESPMESSAGE_H_
#define NETLINKDELETESPMESSAGE_H_

#include "NLconstants.h"
#include "NetlinkMessage.h"
#include "NetlinkSPConfig.h"

namespace qkd {
namespace IPsec {

class NetlinkDeleteSPMessage: public NetlinkMessage {
public:
	NetlinkDeleteSPMessage(IPAddress source_add, IPAddress destination_add,NetlinkSPConfig config=NetlinkSPConfig());
private:
	xfrm_userpolicy_id	xpid;
};

} /* namespace QuaKE */
} /* namespace qkd */
#endif /* NETLINKDELETESPMESSAGE_H_ */

/*
 * IPsecConnectionManager.cpp
 *
 *  Created on: 18.11.2013
 *      Author: mrs
 */

#include "IPsecConnectionManager.h"
#include "KernelIPsecManager.h"
#include "NetlinkIPsecManager.h"

using namespace qkd::IPsec;

IPsecConnectionManager::IPsecConnectionManager(KernelIPsecManager& manager_param):kernel_manager(manager_param) {

}


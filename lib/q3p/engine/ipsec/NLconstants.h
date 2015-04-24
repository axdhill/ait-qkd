/*
 * Utils.h
 *
 *  Created on: 10.11.2013
 *      Author: mrs
 */


#include <arpa/inet.h> //for IPPROTO_ESP and IPPROTO_AH
#include <linux/xfrm.h>
#include <linux/netlink.h>
#include <sys/socket.h>
#define  NETLINK_TEST 		17
#define  MAX_PAYLOAD 		2048
#define  MAX_TEMPLATE_SIZE 	1024
#define  MAX_KEY_SIZE		512
#define  SPICONST			101
#define  NL_SAD_NEW			1
#define  NL_SPD_NEW			2
#define  NL_SAD_DELETE		3
#define  DIR_IN 			0
#define  DIR_OUT			1



#ifndef NLCONSTANTS_H_
#define NLCONSTANTS_H_


#endif /* NLCONSTANTS_H_ */

//============================================================================
// Name        : QuaKE.cpp
// Author      : Stefan Marksteiner
// Version     :
// Copyright   : Your copyright notice
// Description : Test program for the AIT QKD software's IPsec module
//============================================================================

#include <linux/netlink.h>
#include <linux/xfrm.h>
#include <iostream>
#include "IPAddress.h"
#include "NetlinkSPConfig.h"
#include "NetlinkSAConfig.h"
#include "KernelIPsecManager.h"
#include "NetlinkIPsecManager.h"
#include "Exception.h"
#include "TunTapManager.h"
#include <string>


#include <linux/netlink.h> //for NLMSG_DATA(), types sockaddr_nl, nlmsghdr  and constants like NETLINK_XFRM
#include <linux/xfrm.h> //for constant XFRM_MSG_NEWPOLICY and type xfrm_userpolicy_info
#include <iostream> //for cin, cout, pthread_self()
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>




int main() {


	//*****TUN/TAP TESTING******************************
	//TunTapManager manager;
	//manager.test();
	 //**************************************************
	char keyfield[447];
		for(int i=0;i<55;i++){
			keyfield[8*i+0]=(uint8_t)170;
			keyfield[8*i+1]=(uint8_t)187;
			keyfield[8*i+2]=(uint8_t)204;
			keyfield[8*i+3]=(uint8_t)221;
			keyfield[8*i+4]=(uint8_t)238;
			keyfield[8*i+5]=(uint8_t)255;
			keyfield[8*i+6]=(uint8_t)0;
			keyfield[8*i+7]=(uint8_t)17;
		}

	//*****IPsecManager TESTING**************************
	qkd::IPsec::KernelIPsecManager* st = new qkd::IPsec::NetlinkIPsecManager(qkd::IPsec::IPAddress("10.163.247.67"),qkd::IPsec::IPAddress("143.224.185.41"));
	for (int x=0; x!=7;){
	  std::cout << "\n1: add SA; 2: delete SA; 3: add SP; 4: add SP Tunnel; 5 add SP Param; 6 delete SP; 7: exit\n";
	  std::cin >> x;
	  qkd::IPsec::IPAddress tun_src("2001:db8::1");
	  qkd::IPsec::IPAddress tun_dst("2001:db8::2");
	  switch(x){
		  case 1: st->addSA(qkd::IPsec::NetlinkSAConfig(keyfield,"blowfish",1234,200));	break;//st6.addSA();							break;
		  case 2: st->deleteSA(); 				break;//st6.deleteSA(); 						break;
		  case 3: st->addSP(); 					break;//st6.addSP(); 							break;
		  case 4: st->addSP(qkd::IPsec::IPAddress("10.0.0.3"),qkd::IPsec::IPAddress("10.0.0.4"));    break;//st6.addSP("::3","1:3::3:1");			break;
		  case 5: st->addSP(qkd::IPsec::NetlinkSPConfig(true));break;//case 5: st.addSP(false,8080,9090,0,0);	break;
		  case 6: st->deleteSP();				break;//st6.delSP();							break;
	  }
	}
	//**************************************************

	//*****ALGORITHM TESTING*****************************
	/*
	qkd::IPsec::NetlinkManager st1("143.224.0.0","143.224.1.1");
	qkd::IPsec::NetlinkManager st0("2001::1","2001::2");
	qkd::IPsec::NetlinkManager st2("143.224.0.0","143.224.1.2");
	qkd::IPsec::NetlinkManager st3("143.224.0.0","143.224.1.3");
	qkd::IPsec::NetlinkManager st4("143.224.0.0","143.224.1.4");
	qkd::IPsec::NetlinkManager st5("143.224.0.0","143.224.1.5");
	qkd::IPsec::NetlinkManager st6("143.224.0.0","143.224.1.6");
	qkd::IPsec::NetlinkManager st7("143.224.0.0","143.224.1.7");
	qkd::IPsec::NetlinkManager st8("143.224.0.0","143.224.1.8");
	qkd::IPsec::NetlinkManager st9("143.224.0.0","143.224.1.9");
	st1.addSA(keyfield,"des" 	  );
	st2.addSA(keyfield,"des3_ede");
	st3.addSA(keyfield,"cast5"   );
	st4.addSA(keyfield,"blowfish");
	st5.addSA(keyfield,"aes"     );
	st6.addSA(keyfield,"serpent" );
	st7.addSA(keyfield,"camellia");
	st8.addSA(keyfield,"twofish" );
	st9.addSA(keyfield,"rfc3686(ctr(aes))");*/
	//***************************************************


	//*****PERFORMANCE TESTING***************************
	/*qkd::IPsec::NetlinkIPsecManager st6(qkd::IPsec::IPAddress("fe:501:4819::2"),qkd::IPsec::IPAddress("3ffe:501:481d::2"));
	qkd::IPsec::NetlinkIPsecManager st4(qkd::IPsec::IPAddress("10.9.170.22"),qkd::IPsec::IPAddress("143.224.185.41"));
	st6.addSA(keyfield,"des");
	st4.addSA(keyfield,"blowfish");
	for(int i=0; i<10000; i++){
		st4.fastUpdateKey(keyfield);
	}*/
	//***************************************************

	/*char keyfield[8];
	keyfield[0]=(uint8_t)170;
	keyfield[1]=(uint8_t)187;
	keyfield[2]=(uint8_t)204;
	keyfield[3]=(uint8_t)221;
	keyfield[4]=(uint8_t)238;
	keyfield[5]=(uint8_t)255;
	keyfield[6]=(uint8_t)0;
	keyfield[7]=(uint8_t)17;

	qkd::IPsec::NetlinkIPsecManager st4(qkd::IPsec::IPAddress("10.9.170.22"),qkd::IPsec::IPAddress("143.224.185.41"));
	qkd::IPsec::NetlinkSPConfig SPconfig(true,1234);
	qkd::IPsec::NetlinkSAConfig SAconfig(keyfield,"blowfish",1234,200);
	st4.addSP(SPconfig);
	st4.addSA(SAconfig);*/


}



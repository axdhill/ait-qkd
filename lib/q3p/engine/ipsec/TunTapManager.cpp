/*
 * TunTapManager.cpp
 *
 *  Created on: 19.11.2013
 *      Author: mrs
 */

#include "TunTapManager.h"
#include <net/if.h>
#include <linux/if_tun.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h> //for close()



TunTapManager::TunTapManager() {

}

int TunTapManager::test(void){
	/*from: http://backreference.org/2010/03/26/tuntap-interface-tutorial/ */
		  int flags=0;
		  char dev[20]= {"ppp0"};
		  struct ifreq ifr;
		  int fd, err;
		  char clonedev[20]= {"/dev/net/tun"};
		  if( (fd = open(clonedev, O_RDWR)) < 0 ) {
			return fd;
		  }

		  ifr.ifr_flags = flags;   /* IFF_TUN or IFF_TAP, plus maybe IFF_NO_PI */

		     if (*dev) {
		       /* if a device name was specified, put it in the structure; otherwise,
		        * the kernel will try to allocate the "next" device of the
		        * specified type */
		       strncpy(ifr.ifr_name, dev, IFNAMSIZ);
		     }

		     /* try to create the device */
		     if( (err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0 ) {
		       close(fd);
		       return err;
		     }

		    /* if the operation was successful, write back the name of the
		     * interface to the variable "dev", so the caller can know
		     * it. Note that the caller MUST reserve space in *dev (see calling
		     * code below) */
		    strcpy(dev, ifr.ifr_name);

		    /* this is the special file descriptor that the caller will use to talk
		     * with the virtual interface */
		    return fd;
}

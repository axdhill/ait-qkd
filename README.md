AIT QKD R10 Software
====================


This is the AIT QKD Software Suite containing Q3P, the Q3P KeyStore, QKD Modules, Cascade, and others.

The source code is arranged as a CMake (see: [http://www.cmake.org](http://www.cmake.org/)) project.

1. Preparation
--------------

1.1 For compilation
-------------------

In order to compile the QKD sources we need at least the developer versions of:

* gcc and g++ at least version 4.6.3
* boost at least version 1.49
* OpenSSL
* UUID
* CMake
* GMP
* 0MQ (Zero Message Queue) version 2.2 (**NOT** version 3)
* Qt4 at least version 4.4
* Doxygen

Here are the steps which help you to setup a build system capable of compiling the sources on a pure Debian Wheezy system.

    # apt-get install build-essential g++ gcc libboost-all-dev libssl-dev uuid-dev cmake libssl-dev uuid-dev libgmp3-dev libzmq-dev libdbus-1-dev libqt4-dev doxygen 


To clone the sources from the AIT servers:
    
    # apt-get install git
    
(switch back to normal user)
    
    $ git clone http://git-service.ait.ac.at/quantum-cryptography/qkd.git

    
    
1.2 For package install only
----------------------------

Despite a normal Debian Wheezy implementation you'll need:  

**To Be Written**
    
    

2. Compilation
--------------

Step into the "build" folder, invoke cmake and then make.

    $ mkdir build 2> /dev/null
    $ cd build
    $ cmake ..
    $ make
    

    
3. Test
-------

Once you've run sucessful the "make" command you are enabled to run tests at your disposal. To do so switch into the build folder again.

    $ cd build
    $ make test

This will run a series of tests to check the sanity of the qkd sytstem built. If one or more test failed then this is a good indicator that something has gone awry. Please consult the AIT with the output of the command for support.
   

4. Packaging
------------

After a sucessfull build, you might as well create DEB packages for install on various Debian or Debain-based systems.

    $ cd build
    $ make package
    
(Note: the DEB package does currently not contain correct dependency information to install the QKD software on non compile-prepared Debian machines.)


5. Installation from DEB
------------------------

On a Debian machine prepared for source compilation a

    $ sudo dpkg --install qkd*.deb
   
is sufficient.

This will install the qkd binaries into your system. However further prepartions are needed. Depending on your system being sysv or systemd started. Check the INSTALL.sys* scripts in the root folder for further explainations.

        
6. License
----------

The AIT QKD Software Suite is licensed under the GPL 3.

Some GUI parts use images and graphics created by [http://www.creativefreedom.co.uk](http://www.creativefreedom.co.uk) under the "CC Attribution-No Derivative 3.0" License. See [http://creativecommons.org/licenses/by-nd/3.0](http://creativecommons.org/licenses/by-nd/3.0) for details.


(C)opyright 2012-2015 AIT Austrian Instiute of Technology


Oliver Maurhart  
[oliver.maurhart@ait.ac.at](mailto:oliver.maurhart@ait.ac.at)  
[http://ait.ac.at](http://ait.ac.at)  
[https://sqt.ait.ac.at/software](https://sqt.ait.ac.at/software)  
[https://git-service.ait.ac.at/software](https://git-service.ait.ac.at/software)  


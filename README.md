AIT QKD R10 Software
====================


This is the AIT QKD Software Suite containing Q3P, the Q3P KeyStore, QKD Modules, Cascade, and others. This is the **public**, **free** repository. We do have additonal stuff like LDPC error correction, QKD presfiting and QKD GUI. If you have interest in these please contact us.

The source code is arranged as a CMake (see: [http://www.cmake.org](http://www.cmake.org/)) project.


1. Preamble
-----------

The AIT QKD R10 Software as hosted here is **work in progress**. This said, some features are not working currently or have been deliberately turned off due to bugs.

Among these are:

* The Q3P encryption and authentication is currently turned off.
* The Q3P protocol is not fully implemented yet.
* The IPSec implemenation - though present - is not yet fully included.
* The QKD Module Manager (in the tools section) is not fully implemented.
* Package creation does yet not consider correct dependencies.
* There is currently no developer package including all header files present.
* The examples and the demo is not up-to-date.
* The documentation is slightly out of date.


2. Content
----------

The whole project compiles in one single step, meaning no subprojects. This results in a single package for deployment for ease of intsallation and upgrade. The structure is:

    qkd
    +-- bin                                             all executable programs
    |   +-- modules                                         all QKD post processing modules
    |   |   +-- qkd-auth                                        authentication module
    |   |   +-- qkd-bb84                                        BB84 module
    |   |   +-- qkd-buffer                                      bufferinge module, keypool
    |   |   +-- qkd-cascade                                     cascade error correction
    |   |   +-- qkd-cat                                         load a keystream from a file and feed the pipeline
    |   |   +-- qkd-confirmation                                confirmation module
    |   |   +-- qkd-debug                                       print keystream meta data to stderr
    |   |   +-- qkd-dekey                                       remove metadate from a keytream (turns keystream to BLOB)
    |   |   +-- qkd-drop                                        randomly dropping keys on one side (development module)
    |   |   +-- qkd-enkey                                       add metadate to a BLOB (turns BLOB to keystream)
    |   |   +-- qkd-error-estimation                            error rstimation module
    |   |   +-- qkd-ping                                        touch remote peer module (administration module)
    |   |   +-- qkd-privacy-amplification                       privacy amplification module
    |   |   +-- qkd-reorder                                     randomly reorder keys in a keystream (development module)
    |   |   +-- qkd-tee                                         fork keystream to stderr and the next module
    |   |   +-- qkd-throttle                                    throttle key stream on bits/keys per second (development module)
    |   +-- q3pd                                            The Q3P node
    |   +-- tools                                           tools
    |       +-- q3p-inject                                      Feed a Q3P node with keys, BASH-Script
    |       +-- q3p-keystore-dump                               dump content of Q3P database human readable on stdout
    |       +-- q3p-mq-reader                                   read qkd Q3P message queue
    |       +-- qkd-blob-keystream                              wrapper around qkd-enkey, BASH-Script
    |       +-- qkd-key-dump                                    dump a keystream file human readable to stdout
    |       +-- qkd-key-gen                                     create pairs of pseudo random input keys
    |       +-- qkd-module-manager                              GUI for qkd post processing
    |       +-- qkd-pipeline                                    high level qkd post processing admin (start/stop pipeline)
    |       +-- qkd-simulate                                    GUI with simulating quantum events continuously 
    |       +-- qkd-view                                        dump current status of all qkd objects of the system to stdout
    +-- cmake                                           cmake relevant build details
    +-- doc                                             documentation
    |   +-- handbook                                        AIT QKD Handbook (OUTDATED)
    |   +-- simulator                                       QKD Simulator
    +-- etc                                             system configuration for installment
    +-- examples                                        examples
    |   +-- demo-setup                                      sample setup scenarion (OUTDATED)
    |   +-- module-1                                        "Hello World!" qkd module (OUTDATED)
    |   +-- module-2                                        continuative example (OUTDATED)
    |   +-- module-3                                        continuative example (OUTDATED)
    |   +-- module-4                                        continuative example (OUTDATED)
    |   +-- module-5                                        complex example incl. DBus
    +-- include                                         header files
    +-- lib                                             libqkd CORE (QKD Module Framework / Q3P Links / etc.)  <--- !! THIS IS THE CORE !! --->
    +-- share                                           shared files (like graphics)
    |   +-- qkd-module-manager                              graphic files for qkd module manager
    |   +-- qkd-simulate                                    graphic files for qkd simulator
    +-- test                                            test-cases to check build integrity (these are also samples on how to work with the source)



3. Preparation
--------------

3.1 For compilation
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

    # apt-get install build-essential g++ gcc libboost-all-dev libssl-dev uuid-dev cmake libssl-dev uuid-dev libgmp3-dev libzmq-dev libdbus-1-dev libqt4-dev libqwt-dev doxygen 


To clone the sources from the AIT servers:
    
    # apt-get install git
    
(switch back to normal user)
    
    $ git clone http://git-service.ait.ac.at/quantum-cryptography/qkd.git

    
    
3.2 For package install only
----------------------------

Despite a normal Debian Wheezy implementation you'll need:  

**To Be Written**
    
    

4. Compilation
--------------

Step into the "build" folder, invoke cmake and then make.

    $ mkdir build 2> /dev/null
    $ cd build
    $ cmake ..
    $ make
    

    
5. Test
-------

Once you've run sucessful the "make" command you are enabled to run tests at your disposal. To do so switch into the build folder again.

    $ cd build
    $ make test

This will run a series of tests to check the sanity of the qkd sytstem built. If one or more test failed then this is a good indicator that something has gone awry. Please consult the AIT with the output of the command for support.
   

6. Packaging
------------

After a sucessfull build, you might as well create DEB packages for install on various Debian or Debain-based systems.

    $ cd build
    $ make package
    
(Note: the DEB package does currently not contain correct dependency information to install the QKD software on non compile-prepared Debian machines.)


7. Installation from DEB
------------------------

On a Debian machine prepared for source compilation a

    $ sudo dpkg --install qkd*.deb
   
is sufficient.

This will install the qkd binaries into your system. However further prepartions are needed. Depending on your system being sysv or systemd started. Check the INSTALL.sys* scripts in the root folder for further explainations.

        
8. License
----------

The AIT QKD Software Suite is licensed under the GPL 3.

Some GUI parts use images and graphics created by [http://www.creativefreedom.co.uk](http://www.creativefreedom.co.uk) under the "CC Attribution-No Derivative 3.0" License. See [http://creativecommons.org/licenses/by-nd/3.0](http://creativecommons.org/licenses/by-nd/3.0) for details.


(C)opyright 2012-2015 AIT Austrian Instiute of Technology


Oliver Maurhart  
[oliver.maurhart@ait.ac.at](mailto:oliver.maurhart@ait.ac.at)  
[http://ait.ac.at](http://ait.ac.at)  
[https://sqt.ait.ac.at/software](https://sqt.ait.ac.at/software)  
[https://git-service.ait.ac.at/software](https://git-service.ait.ac.at/software)  


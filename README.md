AIT QKD R10 Software
====================

This is the AIT QKD Software Suite containing Q3P, the Q3P KeyStore, QKD Modules, Cascade, and others.

For scientists, universities, and security centered companies who want to concentrate on certain topics of QKD like sifting or error correction or simply use the off-the-shelf toolchain for their products the AIT QKD R10 is a full featured quantum key distribution implementation. Besides been a free open source solution the AIT comes with well defined interfaces and well documented sources which already have been used by several teams around the globe. 

This is the **public**, **free** repository. We do have additional stuff like LDPC error correction, QKD presfiting and QKD GUI but we currently do not ship them for free of charge. If you have interest in these please contact us.

The source code is arranged as a CMake (see: [http://www.cmake.org](http://www.cmake.org/)) project in a git (see: [http://www.git-scm.com](http://www.git-scm.com)) repository. The reference machines are Debian 7.8 ("Wheezy") and Debian 8 ("Jessie"). Any other Linux distribution might work as well, e.g. Ubuntu or Mint, but this is not tested yet.


1. Preamble
-----------

The AIT QKD R10 Software as hosted here is **work in progress**. This said, some features are not working currently or have been deliberately turned off due to bugs.

Among these are:

* The Q3P encryption and authentication is currently turned off.
* The Q3P protocol is not fully implemented yet.
* The IPSec implementation - though present - is not yet fully included.
* The QKD Module Manager (in the tools section) is not fully implemented.
* Package creation does yet not consider correct dependencies.
* There is currently no developer package including all header files present.
* The examples and the demo is not up-to-date.
* The documentation is slightly out of date.


2. Content
----------

The whole project compiles in one single step, meaning no subprojects. This results in a single package for deployment for ease of installation and upgrade. The structure is:

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
* Qwt (Qt Widgets for Technical Applications)
* Doxygen

Here are the steps which help you to setup a build system capable of compiling the sources on a pure Debian Wheezy/Jessie system.

    # apt-get install build-essential g++ gcc libboost-all-dev libssl-dev uuid-dev cmake libssl-dev uuid-dev libgmp3-dev libzmq-dev libdbus-1-dev libqt4-dev libqwt-dev doxygen texlive-latex-base texlive-latex-extra texlive-font-utils dbus-x11 libcap2-bin


To clone the sources from the AIT servers:
    
    # apt-get install git
    
(switch back to normal user)
    
    $ git clone http://git-service.ait.ac.at/quantum-cryptography/qkd.git

    
    
3.2 For package install only
----------------------------

Despite a normal Debian Wheezy/Jessie implementation you'll need:  

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

Once you've run successful the "make" command you are enabled to run tests at your disposal. To do so switch into the build folder again.

    $ cd build
    $ make test

This will run a series of tests to check the sanity of the qkd system built. If one or more test failed then this is a good indicator that something has gone awry. Please consult the AIT with the output of the command for support.
   

6. Packaging
------------

After a successful build, you might as well create DEB packages for install on various Debian or Debian-based systems.

    $ cd build
    $ make package
    
(Note: the DEB package does currently not contain correct dependency information to install the QKD software on non compile-prepared Debian machines.)


7. Installation from DEB
------------------------

On a Debian machine prepared for source compilation a

    $ sudo dpkg --install qkd*.deb
   
is sufficient.

This will install the qkd binaries into your system. However further preparations are needed. Depending on your system being sysv or systemd started. Check the INSTALL.sys\* scripts in the root folder for further explanations.


8. Known Issues
---------------

**8.1 locale**

If you come along such messages:

    $ make
    [  0%] Doxygen running ...
    perl: warning: Setting locale failed.
    perl: warning: Please check that your locale settings:
            LANGUAGE = "en_US:en",
            LC_ALL = "",
            LC_TIME = "de_AT.UTF-8",
            LC_MONETARY = "de_AT.UTF-8",
            LC_ADDRESS = "de_AT.UTF-8",
            LC_TELEPHONE = "de_AT.UTF-8",
            LC_NAME = "de_AT.UTF-8",
            LC_MEASUREMENT = "de_AT.UTF-8",
            LC_IDENTIFICATION = "de_AT.UTF-8",
            LC_NUMERIC = "de_AT.UTF-8",
            LC_PAPER = "de_AT.UTF-8",
            LANG = ""
        are supported and installed on your system.
    perl: warning: Falling back to the standard locale ("C").

or 

    $ locale
    locale: Cannot set LC_ALL to default locale: No such file or directory
    LANG=en_US.UTF-8
    LANGUAGE=en_US:en
    LC_CTYPE="en_US.UTF-8"
    LC_NUMERIC=de_AT.UTF-8
    LC_TIME=de_AT.UTF-8
    LC_COLLATE="en_US.UTF-8"
    LC_MONETARY=de_AT.UTF-8
    LC_MESSAGES="en_US.UTF-8"
    LC_PAPER=de_AT.UTF-8
    LC_NAME=de_AT.UTF-8
    LC_ADDRESS=de_AT.UTF-8
    LC_TELEPHONE=de_AT.UTF-8
    LC_MEASUREMENT=de_AT.UTF-8
    LC_IDENTIFICATION=de_AT.UTF-8
    LC_ALL=


Then you are lacking a locale installation. Some parts, especially boost-filesystem, are known to suffer badly from missing locale installation files. Reinstall them by

    # dpkg-reconfigure locales

and verify that all locales as defined by

    $ locale

are installed which is seen by

    $ locale -a

    

**8.2 Remote SSH login and DBus server**

The AIT QKD depends on a running DBus server. When you ssh into a remote machine as usually via

    $ ssh user@machine

then you will lack DBus capabilities. This results in warnings and messages like this:

    Could not connect to D-Bus server: org.freedesktop.DBus.Error.NotSupported: Unable to autolaunch a dbus-daemon without a $DISPLAY for X11

Please enable X11 forwarding support via 

    $ ssh -X user@machine

Note: the AIT QKD contains the configuration files to start up a dedicated DBus only for QKD purpose. The intended way is to install these configuration files which differ if you use sysv or systemd boot. However the address of the local running DBus is stored in the QKD\_DBUS\_SESSION\_ADDRESS environment variable. Check the INSTALL.sys\* files for details. 


    
9. License
----------

The AIT QKD Software Suite is licensed under the GPL 3.

Some GUI parts use images and graphics created by [http://www.creativefreedom.co.uk](http://www.creativefreedom.co.uk) under the "CC Attribution-No Derivative 3.0" License. See [http://creativecommons.org/licenses/by-nd/3.0](http://creativecommons.org/licenses/by-nd/3.0) for details.


(C)opyright 2012-2015 AIT Austrian Institute of Technology


Oliver Maurhart  
[oliver.maurhart@ait.ac.at](mailto:oliver.maurhart@ait.ac.at)  
[http://ait.ac.at](http://ait.ac.at)  
[https://sqt.ait.ac.at/software](https://sqt.ait.ac.at/software)  
[https://git-service.ait.ac.at/quantum-cryptography/qkd](http://git-service.ait.ac.at/quantum-cryptography/qkd)


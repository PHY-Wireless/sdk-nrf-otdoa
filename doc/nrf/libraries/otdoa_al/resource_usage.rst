OTDOA Library Resource Usage
============================

RAM Usage
---------

The OTDOA library currently uses about 25KB of RAM

ROM Usage
---------

The OTDOA library currently uses about 50KB of FLASH for program and constant data storage

FLASH File System
-----------------

The OTDOA library uses a FLASH file system for storage of uBSA information.  Typically this makes use of the Zephyr Little FS file system, mounted on either an external serial FLASH or using the NRF SoC internal FLASH.

Approximately 25KB of storage are used in this file system.

Dependencies
------------

This OTDOA library uses the following nRF Connect SDK libraries:

* LTE Link Control
* AT Command Library
* Modem Key Management
* Modem Information
* RS Capture (a new library supporting OTDOA)

It also uses the following secure firmware component:

* Trusted Firmware-M

It uses the following Zephyr libraries and services:

* Threads
* Queues
* Timers
* Memory Slabs
* File System (typically Little FS)

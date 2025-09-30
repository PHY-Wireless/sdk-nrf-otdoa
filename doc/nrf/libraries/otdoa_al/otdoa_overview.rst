OTDOA Overview
==============

Observed Time-Difference of Arrival (OTDOA) is a technique for estimating a UE's position.  It involves estimating time differences between signals broadcast using by the terrestrial LTE cellular network.  These time difference estimates can then be used to triangulate the UE's position based upon known cellular base station positions, in much the same way as GNSS systems triangulate a position based upon know satellite positions.

For OTDOA, received signal timing is estimated using Positioning Reference Signals (PRS) broadcast by cells in the LTE network.  The PRS signals are periodically broadcast by the LTE network (typically for 1 msec. every 160 msec.), and they are multiplexed in a way that allows the UE to detect PRS broadcast by multiple base stations at the same time.

Information about the cellular base stations, including their positions, is contained in micro-Base Station Almanac (uBSA) file that is downloaded from a network server.  This uBSA is stored in a file system on the UE, and typically covers a large geographic area.  The UE can use this uBSA for many position estimates; if it moves outside the geographic coverage area a new uBSA is automatically downloaded.  An important function of the OTDOA Adaption Layer is to download the uBSA from a network server, and store it in the UE file system for access by the OTDOA binary library.

Once the uBSA has been downloaded, all position estimate calculations are done on the UE application processor.  This includes collection of the PRS signals, estimation of time differences, and triangulation calculations necessary to estimate the UE position.

Use of OTDOA for UE position estimate has the following advantages

* Very low power consumption
* Excellent privacy, since position estimates are calculated by the UE without network involvement.
* Good accuracy comparted to alternative cellular positioning technologies.

Implementation
--------------

See :doc:`otdoa_firmware` and :doc:`otdoa_data_flow`

Supported Features
------------------
The OTDOA adaption layer supports the following features:

* OTDOA position estimate
* Enhanced Cell ID (ECID) position estimate
* Position estimate accuracy estimation
* uBSA download via an HTTP REST interface
* Configuration file download via an HTTP REST interface
* Optional position estimate results upload to a server via an HTTP REST interface


Supported backends
------------------
Not applicable

Requirements
------------
The OTDOA adaption layer requires the OTDOA binary library to be separately downloaded and integrated
into the nRF Connect SDK by the user.

Configuration
-------------
To use the OTDOA adaption layer, enable the :kconfig:option:`CONFIG_OTDOA_AL_LIB` configuration option.

You can also configure the following options:

* :kconfig:option:`OTDOA_DEBUG_LOGS` to link with a version of the OTDOA binary library that includes verbose debug logging.
* :kconfig:option:`OTDOA_CFG_DL_INTERVAL` to configure the interval between config downloads.
* :kconfig:option:`OTDOA_PRS_SAMPLES_BUFFER_COUNT` to configure the number of buffer slabs for PRS samples.
* :kconfig:option:`OTDOA_HTTP_BLACKLIST_TIMEOUT` to configure the number of uBSA requests to make before removing an SC ECGI from the DL blacklist.
* :kconfig:option:`OTDOA_ENABLE_RESULTS_UPLOAD` to enable uploading of OTDOA results to a server.

These options control the HTTP thread and the RS thread operations:

* :kconfig:option:`OTDOA_HTTP_QUEUE_STACK_SIZE` to configure the HTTP work queue stack size.
* :kconfig:option:`OTDOA_HTTP_QUEUE_PRIORITY` to configure the HTTP work queue priority.
* :kconfig:option:`OTDOA_RS_THREAD_STACK_SIZE` to configure the RS work queue stack size in bytes.
* :kconfig:option:`OTDOA_RS_THREAD_PRIORITY` to configure the RS thread priority.

These options control the BSA download server and the results upload server:

* :kconfig:option:`OTDOA_BSA_DL_SERVER_URL` to configure the URL to download BSA files from.
* :kconfig:option:`OTDOA_HTTPS_PORT` to configure the port to use for OTDOA HTTPS operations.
* :kconfig:option:`OTDOA_HTTP_PORT` to configure the port to use for OTDOA HTTP operations.
* :kconfig:option:`OTDOA_RESULTS_UPLOAD_PW` to configure the password to use for Phywi API authentication when uploading OTDOA results.
* :kconfig:option:`OTDOA_UPLOAD_SERVER_URL` to configure the URL to upload OTDOA results to.

Miscellaneous options:

* :kconfig:option:`OTDOA_TLS_SEC_TAG` to configure the TLS security tag slot to install PhyWi TLS certificate to.

Shell commands list
-------------------

Usage
-----

Samples using the library
-------------------------
The following |NCS| samples use the OTDOA library and adaption layer:

* :ref:`otdoa_sample`


Application integration
-----------------------

Additional information
----------------------

Limitations
-----------

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

FLASH File System
~~~~~~~~~~~~~~~~~

The OTDOA library uses a FLASH file system for storage of uBSA information.  Typically this makes use of the Zephyr Little FS file system, mounted on either an external serial FLASH or using the NRF SoC internal FLASH.

Approximately 25KB of storage are used in this file system.

API documentation
-----------------

| Header file: :file:`include/otdoa_al/phywi_otdoa_api.h`
| Source files: :file:`lib/otdoa_al/*.c`

.. doxygengroup:: phywi_otdoa_api

.. _otdoa:

Cellular: OTDOA Positioning
###########################

.. contents::
   :local:
   :depth: 2

This sample application illustrates use of the OTDOA libraries to perform position estimation on an embedded device

Requirements
************

This sample supports the following development kits:

.. table-from-sample-yaml::

Overview
********

The OTDOA sample initializes the OTDOA AL library and provides shell commands to run tests and display metadata to demonstrate the basic use of the OTDOA API.

Configuration
*************

|config|

Configuration options
=====================

Set this option to provide the OTDOA binary
.. _OTDOA_DEBUG_LOGS:

OTDOA_DEBUG_LOGS
	Link with debug logging version of OTDOA library for verbose logging.

.. _OTDOA_CFG_DL_INTERVAL:

OTDOA_CFG_DL_INTERVAL
	Interval between uBSA downloads to trigger a config download.

.. _OTDOA_PRS_SAMPLES_BUFFER_COUNT:

OTDOA_PRS_SAMPLES_BUFFER_COUNT
	Number of buffer slabs for PRS samples.

.. _OTDOA_HTTP_BLACKLIST_TIMEOUT:

OTDOA_HTTP_BLACKLIST_TIMEOUT
	Number of uBSA requests to make before removing an SC ECGI from the DL blacklist.

.. _OTDOA_HTTP_QUEUE_STACK_SIZE:

OTDOA_HTTP_QUEUE_STACK_SIZE
	HTTP work queue stack size in bytes.

.. _OTDOA_HTTP_QUEUE_PRIORITY:

OTDOA_HTTP_QUEUE_PRIORITY
	HTTP work queue priority.

.. _OTDOA_RS_THREAD_STACK_SIZE:

OTDOA_RS_THREAD_STACK_SIZE
	RS work queue stack size in bytes.

.. _OTDOA_RS_THREAD_PRIORITY:

OTDOA_RS_THREAD_PRIORITY
	RS work queue priority.

.. _OTDOA_API_TLS_CERT_INSTALL:

OTDOA_API_TLS_CERT_INSTALL
	Enable otdoa_api_install_tls_cert() to install a TLS certificate through the OTDOA library.

.. _OTDOA_TLS_SEC_TAG:

OTDOA_TLS_SEC_TAG
	Modem security tag slot to install PhyWi TLS certificate to.

.. _OTDOA_BSA_DL_SERVER_URL:

OTDOA_BSA_DL_SERVER_URL
	URL to download BSA files from.

.. _OTDOA_HTTPS_PORT:

OTDOA_HTTPS_PORT
	Port to use for OTDOA HTTPS operations.

.. _OTDOA_HTTP_PORT:

OTDOA_HTTP_PORT
	Port to use for OTDOA HTTP operations.

.. _OTDOA_ENABLE_RESULTS_UPLOAD:

OTDOA_ENABLE_RESULTS_UPLOAD
	Enable uploading of OTDOA results to a server.

.. _OTDOA_RESULTS_UPLOAD_PW:

OTDOA_RESULTS_UPLOAD_PW
    Password to use for Phywi API authentication when uploading OTDOA results

.. _OTDOA_UPLOAD_SERVER_URL:

OTDOA_UPLOAD_SERVER_URL
    URL to upload OTDOA results to

Configuration files
===================

The sample provides predefined configuration files for typical use cases.

The following files are available:

* :file:`prj.conf` - Standard default configuration file.

Building and running
********************

.. |sample path| replace:: :file:`samples/cellular/otdoa`

.. include:: /includes/build_and_run_ns.txt

Running the sample application
=======

|test_sample|

#. |connect_kit|
#. |connect_terminal|
#. Power on or reset your device
#. Observe that the sample starts and connects to the LTE network

    .. code-block:: console

        [00:00:00.404,449] <inf> main: OTDOA Sample Application started
        [00:00:00.404,479] <dbg> main: main: Mounting littleFS
        [00:00:00.404,510] <inf> main: Area 0 at 0x0 on gd25wb256e3ir@1 for 4194304 bytes
        [00:00:00.835,723] <inf> main: /lfs mount: 0
        [00:00:01.429,321] <inf> otdoa_sample: OTDOA sample started
        [00:00:01.708,068] <inf> otdoa_sample: Modem version = 0.1.0-rscapture-709b4abeebf7
        [00:00:02.070,129] <inf> otdoa_sample: Connecting to LTE...
        [00:00:02.208,770] <inf> otdoa: OTDOA Thread Started
        Connected to LTE
        [00:00:05.264,556] <inf> otdoa_sample: Connected!
        otdoa:~$

#. The ``phywi info`` command can be used to get version information and other information about the device:

    .. code-block:: console

        otdoa:~$ phywi info
                Version: otdoa_lib_v0.0.1rcX
                  IMEI: 358299840004820
               %XICCID: 89148000006474646138

Provisioning the device
=======

The device must be provisioned to enable it to communicate with the PHY Wireless server in order to download a uBSA. The provisioning process is as follows:

#. Install the ``openssl`` library on a Linux PC
#. Compile the key generation utility ``gkey`` as described in ``lib/otdoa/utils/generate_privkey/README.md``
#. Generate a keypair on a Linux PC

    .. code-block:: console

        ./gkey <file name>.pem

#. Provision the device with the generated key. There is a python script ``/lib/otdoa/utils/provision_key/pkey`` to do this.

    .. code-block:: console

        # Assumes the device is connected with CLI console on /dev/ttyACM0
        ./pkey <file name>.pem /dev/ttyACM0

#. Prepare a public key to deliver to PHY Wireless, along with the device IMEI

    .. code-block:: console

        openssl ec -in <file name>.pem -pubout > <file name>.pub

Running a positioning estimate test
=======

Once the device is properly provisioned and the public key / IMEI are provided to PHY Wireless, the sample application can be used to perform position estimates using the OTDOA algorithm. The `phywi test` CLI command can be used to trigger a position estimate.2

Sample CLI output for a typical positon esimation:

.. code-block:: console

    [00:00:00.311,767] <inf> main: OTDOA Sample Application started
    [00:00:00.311,798] <dbg> main: main: Mounting littleFS
    [00:00:00.311,828] <inf> main: Area 0 at 0x0 on gd25wb256e3ir@1 for 4194304 bytes
    [00:00:00.745,330] <inf> main: /lfs mount: 0
    [00:00:01.342,193] <inf> otdoa_sample: OTDOA sample started
    [00:00:01.621,215] <inf> otdoa_sample: Modem version = 0.1.0-rscapture-709b4abeebf7
    [00:00:02.121,917] <inf> otdoa: OTDOA Thread Started
    [00:00:02.563,598] <inf> otdoa_sample: Connecting to LTE...
    Connected to LTE[00:00:06.323,822] <inf> otdoa_sample: Connected!
    otdoa:~$ phywi test
    [00:00:17.854,370] <err> otdoa: Invalid magic (0xDEE22D5B) in uBSA file
    [00:00:17.854,705] <err> otdoa: Failed to open BSA file /lfs/ubsa.csv
    [00:00:17.854,797] <err> otdoa: Serving cell 13922819 not found in uBSA!
    [00:00:17.864,379] <inf> otdoa_sample: OTDOA_EVENT_UBSA_DL_REQ:
    [00:00:17.864,410] <inf> otdoa_sample:   ecgi: 13922819  dlearfcn: 5255
    [00:00:17.864,410] <inf> otdoa_sample:   max cells: 1000  radius: 100000
    [00:00:17.864,532] <err> otdoa: otdoa_rs_trigger failed: Serving cell not in uBSA
    ofs_fopen("/lfs/config","w") unlink existing file
    ofs_fopen("/lfs/ubsa.csv","wb") unlink existing file
    [00:00:47.529,968] <inf> otdoa_sample: OTDOA_EVENT_UBSA_DL_COMPL:
    [00:00:47.529,998] <inf> otdoa_sample:   status: 0
    [00:00:47.657,897] <wrn> otdoa: Serv. Cell 13922819 DLEARFCN/Nrb_prs mismatch.  Forced to  5255/25
    [00:00:58.016,082] <inf> otdoa_sample: OTDOA_EVENT_RESULTS:
    [00:00:58.016,143] <inf> otdoa_sample:   latitude: 33.129574
    [00:00:58.016,143] <inf> otdoa_sample:   longitude: -117.329620
    [00:00:58.016,174] <inf> otdoa_sample:   accuracy: 320.0 m
    [00:00:58.016,204] <inf> otdoa_sample:   serv. ecgi: 13922819
    [00:00:58.016,204] <inf> otdoa_sample:   rssi: -83
    [00:00:58.016,235] <inf> otdoa_sample:   dlearfcn: 5255
    [00:00:58.016,265] <inf> otdoa_sample:   length: 32
    [00:00:58.016,296] <inf> otdoa_sample:   est. algorithm: OTDOA (FH Enabled - 8 RBs)
    [00:00:58.016,326] <inf> otdoa_sample:   measured cells: 75
    [00:00:58.016,326] <inf> otdoa_sample:   Detected Cells:
    [00:00:58.016,387] <inf> otdoa_sample:     ecgi: 13922819  detects: 8
    [00:00:58.016,387] <inf> otdoa_sample:     ecgi: 13922817  detects: 6
    [00:00:58.016,418] <inf> otdoa_sample:     ecgi: 13922818  detects: 2
    [00:00:58.016,448] <inf> otdoa_sample:     ecgi: 90631945  detects: 1
    [00:00:58.016,448] <inf> otdoa_sample:     ecgi: 13855489  detects: 8
    [00:00:58.016,479] <inf> otdoa_sample:     ecgi: 13855490  detects: 5
    [00:00:58.016,510] <inf> otdoa_sample:     ecgi: 13953025  detects: 8
    [00:00:58.016,540] <inf> otdoa_sample: OTDOA position estimate SUCCESS
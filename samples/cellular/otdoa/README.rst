.. otdoa sample:

Cellular: OTDOA Positioning

.. contents::
   :local:
   :depth: 2

This sample demonstrats how to use the OTDOA library on your device

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

.. OTDOA_LIB_PATH:

OTDOA_LIB_PATH
   Path to the OTDOA library binary to link

Configuration files
===================

The sample provides predefined configuration files for typical use cases.

The following files are available:

* :file:`prj.conf` - Standard default configuration file.

Building and running
********************

.. |sample path| replace :file:`samples/cellular/otdoa_sample`

.. include:: /includes/build_and_run_ns.txt

Testing
=======

|test_sample|

#. |connect_kit|
#. |connect_terminal|
#. Power on or reset your device
#. Observe that the sample starts and connects to the LTE network
#. Run `phywi test` on the device

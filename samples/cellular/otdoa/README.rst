.. _otdoa_sample:

Cellular: OTDOA Positioning
###########################

.. contents::
   :local:
   :depth: 2

This sample demonstrates how to use the OTDOA library and adaptation layer
to estimate the position of your device.

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


Configuration files
===================

The sample provides predefined configuration files for typical use cases.

The following files are available:

* :file:`prj.conf` - Standard default configuration file.

Building and running
********************

.. |sample path| replace:: :file:`samples/cellular/otdoa`

.. include:: /includes/build_and_run_ns.txt

Testing
=======

|test_sample|

#. |connect_kit|
#. |connect_terminal|
#. Power on or reset your device
#. Observe that the sample starts and connects to the LTE network
#. Run `phywi test` on the device

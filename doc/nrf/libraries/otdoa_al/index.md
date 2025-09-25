# OTDOA Adaptation Layer
The OTDOA Adaption Layer library enables applications to perform position estimates on cellular LTE signals using the OTDOA binary library.  It adapts the OTDOA binary library for use in the nRF cellular environment.

## Overview
Observed Time-Difference of Arrival (OTDOA) is a technique for estimating a UE's position.  It involves estimating time differences between signals broadcast using by the terrestrial LTE cellular network.  These time difference estimates can then be used to triangulate the UE's position based upon known cellular base station positions, in much the same way as GNSS systems triangulate a position based upon know satellite positions.

For OTDOA, received signal timing is estimated using Positioning Reference Signals (PRS) broadcast by cells in the LTE network.  The PRS signals are periodically broadcast by the LTE network (typically for 1 msec. every 160 msec.), and they are multiplexed in a way that allows the UE to detect PRS broadcast by multiple base stations at the same time.

Information about the cellular base stations, including their positions, is contained in micro-Base Station Almanac (uBSA) file that is downloaded from a network server.  This uBSA is stored in a file system on the UE, and typically covers a large geographic area.  The UE can use this uBSA for many position estimates; if it moves outside the geographic coverage area a new uBSA is automatically downloaded.  An important function of the OTDOA Adaption Layer is to download the uBSA from a network server, and store it in the UE file system for access by the OTDOA binary library.

Once the uBSA has been downloaded, all position estimate calculations are done on the UE application processor.  This includes collection of the PRS signals, estimation of time differences, and triangulation calculations necessary to estimate the UE position.

Use of OTDOA for UE position estimate has the following advantages
- Very low power consumption
- Excellent privacy, since position estimates are calculated by the UE without network involvement.
- Good accuracy comparted to alternative cellular positioning technologies.


## Implementation
See [OTDOA Firmware Overview](otdoa_firmware.md) and [OTDOA Data Flow](otdoa_data_flow.md)

## Supported Features



## Supported backends
Not applicable

## Requirements
The OTDOA adaption layer requires the OTDOA binary library to be separately downloaded and integrated
into the nRF Connect SDK by the user.

## Configuration

## Shell commands list

## Usage

## Samples using the library

## Application integration

## Additional information

## Limitations

## Dependencies
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


### FLASH File System
The OTDOA library uses a FLASH file system for storage of uBSA information.  Typically this makes use of the Zephyr Little FS file system, mounted on either an external serial FLASH or using the NRF SoC internal FLASH.

Approximately 25KB of storage are used in this file system.

## API documentation


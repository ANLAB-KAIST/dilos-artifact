UEFI Secure Boot Support


===============================================================================
1. Overview
===============================================================================
All kernel modules included in MLNX_OFED for RHEL7 and SLES12 are signed with
Mellanox's x.509 key to support loading the modules when Secure Boot is enabled.

In order to support loading MLNX_OFED drivers when an OS supporting Secure Boot
boots on a UEFI-based system with Secure Boot enabled, the Mellanox x.509 public
key should be added to the UEFI Secure Boot key database and loaded into the
system key ring by the kernel.


===============================================================================
2. Enrolling Mellanox's x.509 Public Key On your Systems:
===============================================================================
This section explains the procedure for enrolling the Mellanox's x.509 Public
Key On your Systems.


-------------------------------------------------------------------------------
2.1 Hardware and Software Requirements
-------------------------------------------------------------------------------
Please make sure:
1. The 'mokutil' package is installed on your system
2. The system is booted in UEFI mode


-------------------------------------------------------------------------------
2.2 Enrolling the key
-------------------------------------------------------------------------------
Step 1: Download the x.509 public key
# wget http://www.mellanox.com/downloads/ofed/mlnx_signing_key_pub.der
NOTE: you can also use the "mlnx_signing_key_pub.der" key located in this directory.

Step 2: Add the public key to the MOK list using the mokutil utility.
You will be asked to enter and confirm a password for this MOK enrollment request.
# mokutil --import mlnx_signing_key_pub.der

Step 3. Reboot the system
The pending MOK key enrollment request will be noticed by shim.efi and will launch
MokManager.efi to allow you to complete the enrollment from the UEFI console.
You will need to enter the password you previously associated with this request and
confirm the enrollment. Once done, the public key will be added to the MOK list,
which is persistent.
Once a key is in the MOK list, it will automatically be propagated to the system key
ring, and will subsequently be booted when the UEFI Secure Boot is enabled.
-------------------------------------------------------------------------------


NOTE:
To see what keys have been added to the system key ring on the current boot, install
the 'keyutils' package and run:
# keyctl list %:.system_keyring

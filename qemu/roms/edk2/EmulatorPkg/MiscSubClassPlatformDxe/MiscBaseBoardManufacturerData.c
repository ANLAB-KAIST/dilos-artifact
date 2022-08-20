/*++

Copyright (c) 2006 - 2009, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

Module Name:

  MiscBaseBoardManufacturerData.c

Abstract:

  This driver parses the mMiscSubclassDataTable structure and reports
  any generated data to the DataHub.

**/

#include "MiscSubClassDriver.h"

//
// Static (possibly build generated) Bios Vendor data.
//
MISC_SMBIOS_TABLE_DATA(EFI_MISC_BASE_BOARD_MANUFACTURER_DATA, MiscBaseBoardManufacturer) = {
  STRING_TOKEN(STR_MISC_BASE_BOARD_MANUFACTURER),
  STRING_TOKEN(STR_MISC_BASE_BOARD_PRODUCT_NAME),
  STRING_TOKEN(STR_MISC_BASE_BOARD_VERSION),
  STRING_TOKEN(STR_MISC_BASE_BOARD_SERIAL_NUMBER),
  STRING_TOKEN(STR_MISC_BASE_BOARD_ASSET_TAG),
  STRING_TOKEN(STR_MISC_BASE_BOARD_CHASSIS_LOCATION),
  {                         // BaseBoardFeatureFlags
    1,                      // Motherboard
    0,                      // RequiresDaughterCard
    0,                      // Removable
    1,                      // Replaceable,
    0,                      // HotSwappable
    0,                      // Reserved
  },
  EfiBaseBoardTypeUnknown,  // BaseBoardType
  {                         // BaseBoardChassisLink
    EFI_MISC_SUBCLASS_GUID, // ProducerName
    1,                      // Instance
    1,                      // SubInstance
  },
  0,                        // BaseBoardNumberLinks
  {                         // LinkN
    EFI_MISC_SUBCLASS_GUID, // ProducerName
    1,                      // Instance
    1,                      // SubInstance
  },
};

/* eof - MiscBaseBoardManufacturerData.c */

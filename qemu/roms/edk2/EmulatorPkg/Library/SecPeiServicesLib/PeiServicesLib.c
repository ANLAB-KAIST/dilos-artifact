/** @file
  Implementation for PEI Services Library.

  Copyright (c) 2006 - 2010, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/


#include <PiPei.h>
#include <Library/EmuMagicPageLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>



EFI_STATUS
SecFfsFindNextFile (
  IN EFI_FV_FILETYPE            SearchType,
  IN EFI_PEI_FV_HANDLE          VolumeHandle,
  IN OUT EFI_PEI_FILE_HANDLE    *FileHandle
  );

EFI_STATUS
SecFfsFindSectionData (
  IN EFI_SECTION_TYPE           SectionType,
  IN EFI_PEI_FILE_HANDLE        FileHandle,
  OUT VOID                      **SectionData
  );


/**
  This service enables a given PEIM to register an interface into the PEI Foundation.

  @param  PpiList               A pointer to the list of interfaces that the caller shall install.

  @retval EFI_SUCCESS           The interface was successfully installed.
  @retval EFI_INVALID_PARAMETER The PpiList pointer is NULL.
  @retval EFI_INVALID_PARAMETER Any of the PEI PPI descriptors in the list do not have the
                                EFI_PEI_PPI_DESCRIPTOR_PPI bit set in the Flags field.
  @retval EFI_OUT_OF_RESOURCES  There is no additional space in the PPI database.

**/
EFI_STATUS
EFIAPI
PeiServicesInstallPpi (
  IN CONST EFI_PEI_PPI_DESCRIPTOR     *PpiList
  )
{
  ASSERT (FALSE);
  return EFI_OUT_OF_RESOURCES;
}

/**
  This service enables PEIMs to replace an entry in the PPI database with an alternate entry.

  @param  OldPpi                The pointer to the old PEI PPI Descriptors.
  @param  NewPpi                The pointer to the new PEI PPI Descriptors.

  @retval EFI_SUCCESS           The interface was successfully installed.
  @retval EFI_INVALID_PARAMETER The OldPpi or NewPpi is NULL.
  @retval EFI_INVALID_PARAMETER Any of the PEI PPI descriptors in the list do not have the
                                EFI_PEI_PPI_DESCRIPTOR_PPI bit set in the Flags field.
  @retval EFI_OUT_OF_RESOURCES  There is no additional space in the PPI database.
  @retval EFI_NOT_FOUND         The PPI for which the reinstallation was requested has not been
                                installed.

**/
EFI_STATUS
EFIAPI
PeiServicesReInstallPpi (
  IN CONST EFI_PEI_PPI_DESCRIPTOR     *OldPpi,
  IN CONST EFI_PEI_PPI_DESCRIPTOR     *NewPpi
  )
{
  ASSERT (FALSE);
  return EFI_OUT_OF_RESOURCES;
}

/**
  This service enables PEIMs to discover a given instance of an interface.

  So this is, well a hack, so we can reuse the same libraries as the PEI Core
  for XIP modules....

  @param  Guid                  A pointer to the GUID whose corresponding interface needs to be
                                found.
  @param  Instance              The N-th instance of the interface that is required.
  @param  PpiDescriptor         A pointer to instance of the EFI_PEI_PPI_DESCRIPTOR.
  @param  Ppi                   A pointer to the instance of the interface.

  @retval EFI_SUCCESS           The interface was successfully returned.
  @retval EFI_NOT_FOUND         The PPI descriptor is not found in the database.

**/
EFI_STATUS
EFIAPI
PeiServicesLocatePpi (
  IN CONST EFI_GUID             *Guid,
  IN UINTN                      Instance,
  IN OUT EFI_PEI_PPI_DESCRIPTOR **PpiDescriptor,
  IN OUT VOID                   **Ppi
  )
{
  EFI_PEI_PPI_DESCRIPTOR *PpiList;

  if (Instance != 0) {
    return EFI_NOT_FOUND;
  }

  for (PpiList = EMU_MAGIC_PAGE()->PpiList; ; PpiList++) {
    if (CompareGuid (PpiList->Guid, Guid)) {
      if (PpiDescriptor != NULL) {
        *PpiDescriptor = PpiList;
      }
      if (Ppi != NULL) {
        *Ppi = PpiList->Ppi;
      }
      return EFI_SUCCESS;
    }

    if ((PpiList->Flags & EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST) == EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST) {
      break;
    }
  }


  return EFI_NOT_FOUND;
}

/**
  This service enables PEIMs to register a given service to be invoked when another service is
  installed or reinstalled.

  @param  NotifyList            A pointer to the list of notification interfaces
                                that the caller shall install.

  @retval EFI_SUCCESS           The interface was successfully installed.
  @retval EFI_INVALID_PARAMETER The NotifyList pointer is NULL.
  @retval EFI_INVALID_PARAMETER Any of the PEI notify descriptors in the list do
                                 not have the EFI_PEI_PPI_DESCRIPTOR_NOTIFY_TYPES
                                 bit set in the Flags field.
  @retval EFI_OUT_OF_RESOURCES  There is no additional space in the PPI database.

**/
EFI_STATUS
EFIAPI
PeiServicesNotifyPpi (
  IN CONST EFI_PEI_NOTIFY_DESCRIPTOR  *NotifyList
  )
{
  ASSERT (FALSE);
  return EFI_OUT_OF_RESOURCES;
}

/**
  This service enables PEIMs to ascertain the present value of the boot mode.

  @param  BootMode              A pointer to contain the value of the boot mode.

  @retval EFI_SUCCESS           The boot mode was returned successfully.
  @retval EFI_INVALID_PARAMETER BootMode is NULL.

**/
EFI_STATUS
EFIAPI
PeiServicesGetBootMode (
  OUT EFI_BOOT_MODE          *BootMode
  )
{
  ASSERT (FALSE);
  return EFI_OUT_OF_RESOURCES;
}

/**
  This service enables PEIMs to update the boot mode variable.

  @param  BootMode              The value of the boot mode to set.

  @retval EFI_SUCCESS           The value was successfully updated

**/
EFI_STATUS
EFIAPI
PeiServicesSetBootMode (
  IN EFI_BOOT_MODE              BootMode
  )
{
  ASSERT (FALSE);
  return EFI_OUT_OF_RESOURCES;
}

/**
  This service enables a PEIM to ascertain the address of the list of HOBs in memory.

  @param  HobList               A pointer to the list of HOBs that the PEI Foundation
                                will initialize.

  @retval EFI_SUCCESS           The list was successfully returned.
  @retval EFI_NOT_AVAILABLE_YET The HOB list is not yet published.

**/
EFI_STATUS
EFIAPI
PeiServicesGetHobList (
  OUT VOID                      **HobList
  )
{
  ASSERT (FALSE);
  return EFI_OUT_OF_RESOURCES;
}

/**
  This service enables PEIMs to create various types of HOBs.

  @param  Type                  The type of HOB to be installed.
  @param  Length                The length of the HOB to be added.
  @param  Hob                   The address of a pointer that will contain the
                                HOB header.

  @retval EFI_SUCCESS           The HOB was successfully created.
  @retval EFI_OUT_OF_RESOURCES  There is no additional space for HOB creation.

**/
EFI_STATUS
EFIAPI
PeiServicesCreateHob (
  IN UINT16                     Type,
  IN UINT16                     Length,
  OUT VOID                      **Hob
  )
{
  ASSERT (FALSE);
  return EFI_OUT_OF_RESOURCES;
}

/**
  This service enables PEIMs to discover additional firmware volumes.

  @param  Instance              This instance of the firmware volume to find.  The
                                value 0 is the Boot Firmware Volume (BFV).
  @param  VolumeHandle          Handle of the firmware volume header of the volume
                                to return.

  @retval EFI_SUCCESS           The volume was found.
  @retval EFI_NOT_FOUND         The volume was not found.
  @retval EFI_INVALID_PARAMETER FwVolHeader is NULL.

**/
EFI_STATUS
EFIAPI
PeiServicesFfsFindNextVolume (
  IN UINTN                          Instance,
  IN OUT EFI_PEI_FV_HANDLE          *VolumeHandle
  )
{
  ASSERT (FALSE);
  return EFI_OUT_OF_RESOURCES;
}

/**
  This service enables PEIMs to discover additional firmware files.

  @param  SearchType            A filter to find files only of this type.
  @param  VolumeHandle          The pointer to the firmware volume header of the
                                volume to search. This parameter must point to a
                                valid FFS volume.
  @param  FileHandle            Handle of the current file from which to begin searching.

  @retval EFI_SUCCESS           The file was found.
  @retval EFI_NOT_FOUND         The file was not found.
  @retval EFI_NOT_FOUND         The header checksum was not zero.

**/
EFI_STATUS
EFIAPI
PeiServicesFfsFindNextFile (
  IN EFI_FV_FILETYPE            SearchType,
  IN EFI_PEI_FV_HANDLE          VolumeHandle,
  IN OUT EFI_PEI_FILE_HANDLE    *FileHandle
  )
{
  return SecFfsFindNextFile (SearchType, VolumeHandle, FileHandle);
}

/**
  This service enables PEIMs to discover sections of a given type within a valid FFS file.

  @param  SectionType           The value of the section type to find.
  @param  FileHandle            A pointer to the file header that contains the set
                                of sections to be searched.
  @param  SectionData           A pointer to the discovered section, if successful.

  @retval EFI_SUCCESS           The section was found.
  @retval EFI_NOT_FOUND         The section was not found.

**/
EFI_STATUS
EFIAPI
PeiServicesFfsFindSectionData (
  IN EFI_SECTION_TYPE           SectionType,
  IN EFI_PEI_FILE_HANDLE        FileHandle,
  OUT VOID                      **SectionData
  )
{
  return SecFfsFindSectionData (SectionType, FileHandle, SectionData);
}

/**
  This service enables PEIMs to register the permanent memory configuration
  that has been initialized with the PEI Foundation.

  @param  MemoryBegin           The value of a region of installed memory.
  @param  MemoryLength          The corresponding length of a region of installed memory.

  @retval EFI_SUCCESS           The region was successfully installed in a HOB.
  @retval EFI_INVALID_PARAMETER MemoryBegin and MemoryLength are illegal for this system.
  @retval EFI_OUT_OF_RESOURCES  There is no additional space for HOB creation.

**/
EFI_STATUS
EFIAPI
PeiServicesInstallPeiMemory (
  IN EFI_PHYSICAL_ADDRESS       MemoryBegin,
  IN UINT64                     MemoryLength
  )
{
  ASSERT (FALSE);
  return EFI_OUT_OF_RESOURCES;
}

/**
  This service enables PEIMs to allocate memory after the permanent memory has been
   installed by a PEIM.

  @param  MemoryType            Type of memory to allocate.
  @param  Pages                 The number of pages to allocate.
  @param  Memory                Pointer of memory allocated.

  @retval EFI_SUCCESS           The memory range was successfully allocated.
  @retval EFI_INVALID_PARAMETER Type is not equal to AllocateAnyPages.
  @retval EFI_NOT_AVAILABLE_YET Called with permanent memory not available.
  @retval EFI_OUT_OF_RESOURCES  The pages could not be allocated.

**/
EFI_STATUS
EFIAPI
PeiServicesAllocatePages (
  IN EFI_MEMORY_TYPE            MemoryType,
  IN UINTN                      Pages,
  OUT EFI_PHYSICAL_ADDRESS      *Memory
  )
{
  ASSERT (FALSE);
  return EFI_OUT_OF_RESOURCES;
}

/**
  This service allocates memory from the Hand-Off Block (HOB) heap.

  @param  Size                  The number of bytes to allocate from the pool.
  @param  Buffer                If the call succeeds, a pointer to a pointer to
                                the allocate buffer; otherwise, undefined.

  @retval EFI_SUCCESS           The allocation was successful
  @retval EFI_OUT_OF_RESOURCES  There is not enough heap to allocate the requested size.

**/
EFI_STATUS
EFIAPI
PeiServicesAllocatePool (
  IN UINTN                      Size,
  OUT VOID                      **Buffer
  )
{
  ASSERT (FALSE);
  return EFI_OUT_OF_RESOURCES;
}

/**
  Resets the entire platform.

  @retval EFI_SUCCESS           The function completed successfully.
  @retval EFI_NOT_AVAILABLE_YET The service has not been installed yet.

**/
EFI_STATUS
EFIAPI
PeiServicesResetSystem (
  VOID
  )
{
  ASSERT (FALSE);
  return EFI_OUT_OF_RESOURCES;
}

/**
  This service is a wrapper for the PEI Service RegisterForShadow(), except the
  pointer to the PEI Services Table has been removed.  See the Platform
  Initialization Pre-EFI Initialization Core Interface Specification for details.

  @param FileHandle             PEIM's file handle. Must be the currently
                                executing PEIM.

  @retval EFI_SUCCESS           The PEIM was successfully registered for
                                shadowing.

  @retval EFI_ALREADY_STARTED   The PEIM was previously
                                registered for shadowing.

  @retval EFI_NOT_FOUND         The FileHandle does not refer to a
                                valid file handle.
**/
EFI_STATUS
EFIAPI
PeiServicesRegisterForShadow (
  IN  EFI_PEI_FILE_HANDLE FileHandle
  )
{
  ASSERT (FALSE);
  return EFI_OUT_OF_RESOURCES;
}

/**
  This service is a wrapper for the PEI Service FfsGetFileInfo(), except the pointer to the PEI Services
  Table has been removed.  See the Platform Initialization Pre-EFI Initialization Core Interface
  Specification for details.

  @param FileHandle              The handle of the file.

  @param FileInfo                 Upon exit, points to the file's
                                  information.

  @retval EFI_SUCCESS             File information returned.

  @retval EFI_INVALID_PARAMETER   If FileHandle does not
                                  represent a valid file.

  @retval EFI_INVALID_PARAMETER   FileInfo is NULL.

**/
EFI_STATUS
EFIAPI
PeiServicesFfsGetFileInfo (
  IN CONST  EFI_PEI_FILE_HANDLE   FileHandle,
  OUT EFI_FV_FILE_INFO            *FileInfo
  )
{
  ASSERT (FALSE);
  return EFI_OUT_OF_RESOURCES;
}


/**
  This service is a wrapper for the PEI Service FfsFindByName(), except the pointer to the PEI Services
  Table has been removed.  See the Platform Initialization Pre-EFI Initialization Core Interface
  Specification for details.

  @param FileName                 A pointer to the name of the file to
                                  find within the firmware volume.

  @param VolumeHandle             The firmware volume to search FileHandle
                                  Upon exit, points to the found file's
                                  handle or NULL if it could not be found.
  @param FileHandle               The pointer to found file handle

  @retval EFI_SUCCESS             File was found.

  @retval EFI_NOT_FOUND           File was not found.

  @retval EFI_INVALID_PARAMETER   VolumeHandle or FileHandle or
                                  FileName was NULL.

**/
EFI_STATUS
EFIAPI
PeiServicesFfsFindFileByName (
  IN CONST  EFI_GUID            *FileName,
  IN CONST  EFI_PEI_FV_HANDLE   VolumeHandle,
  OUT       EFI_PEI_FILE_HANDLE *FileHandle
  )
{
  ASSERT (FALSE);
  return EFI_OUT_OF_RESOURCES;
}


/**
  This service is a wrapper for the PEI Service FfsGetVolumeInfo(), except the pointer to the PEI Services
  Table has been removed.  See the Platform Initialization Pre-EFI Initialization Core Interface
  Specification for details.

  @param VolumeHandle             Handle of the volume.

  @param VolumeInfo               Upon exit, points to the volume's
                                  information.

  @retval EFI_SUCCESS             File information returned.

  @retval EFI_INVALID_PARAMETER   If FileHandle does not
                                  represent a valid file.

  @retval EFI_INVALID_PARAMETER   If FileInfo is NULL.

**/
EFI_STATUS
EFIAPI
PeiServicesFfsGetVolumeInfo (
  IN  EFI_PEI_FV_HANDLE       VolumeHandle,
  OUT EFI_FV_INFO             *VolumeInfo
  )
{
  ASSERT (FALSE);
  return EFI_OUT_OF_RESOURCES;
}

/**
  Install a EFI_PEI_FIRMWARE_VOLUME_INFO_PPI instance so the PEI Core will be notified about a new firmware volume.

  This function allocates, initializes, and installs a new EFI_PEI_FIRMWARE_VOLUME_INFO_PPI using
  the parameters passed in to initialize the fields of the EFI_PEI_FIRMWARE_VOLUME_INFO_PPI instance.
  If the resources can not be allocated for EFI_PEI_FIRMWARE_VOLUME_INFO_PPI, then ASSERT().
  If the EFI_PEI_FIRMWARE_VOLUME_INFO_PPI can not be installed, then ASSERT().


  @param  FvFormat             Unique identifier of the format of the memory-mapped
                               firmware volume.  This parameter is optional and
                               may be NULL.  If NULL is specified, the
                               EFI_FIRMWARE_FILE_SYSTEM2_GUID format is assumed.
  @param  FvInfo               Points to a buffer which allows the
                               EFI_PEI_FIRMWARE_VOLUME_PPI to process the volume.
                               The format of this buffer is specific to the FvFormat.
                               For memory-mapped firmware volumes, this typically
                               points to the first byte of the firmware volume.
  @param  FvInfoSize           The size, in bytes, of FvInfo. For memory-mapped
                               firmware volumes, this is typically the size of
                               the firmware volume.
  @param  ParentFvName         If the new firmware volume originated from a file
                               in a different firmware volume, then this parameter
                               specifies the GUID name of the originating firmware
                               volume. Otherwise, this parameter must be NULL.
  @param  ParentFileName       If the new firmware volume originated from a file
                               in a different firmware volume, then this parameter
                               specifies the GUID file name of the originating
                               firmware file. Otherwise, this parameter must be NULL.
**/
VOID
EFIAPI
PeiServicesInstallFvInfoPpi (
  IN CONST EFI_GUID                *FvFormat, OPTIONAL
  IN CONST VOID                    *FvInfo,
  IN       UINT32                  FvInfoSize,
  IN CONST EFI_GUID                *ParentFvName, OPTIONAL
  IN CONST EFI_GUID                *ParentFileName OPTIONAL
  )
{
  ASSERT (FALSE);
  return;
}


#ifndef __REPARSETAGS_H_VER__
#define __REPARSETAGS_H_VER__ 2017091421
#if (defined(_MSC_VER) && (_MSC_VER >= 1020)) || defined(__MCPP)
#pragma once
#endif /* Check for #pragma support */

/* Reparse tags */

#ifndef IO_REPARSE_TAG_MOUNT_POINT
#define IO_REPARSE_TAG_MOUNT_POINT              (0xA0000003L)
#endif // IO_REPARSE_TAG_MOUNT_POINT

#ifndef IO_REPARSE_TAG_HSM
#define IO_REPARSE_TAG_HSM                      (0xC0000004L)
#endif // IO_REPARSE_TAG_HSM

#ifndef IO_REPARSE_TAG_HSM2
#define IO_REPARSE_TAG_HSM2                     (0x80000006L)
#endif // IO_REPARSE_TAG_HSM2

#ifndef IO_REPARSE_TAG_SIS
#define IO_REPARSE_TAG_SIS                      (0x80000007L)
#endif // IO_REPARSE_TAG_SIS

#ifndef IO_REPARSE_TAG_WIM
#define IO_REPARSE_TAG_WIM                      (0x80000008L)
#endif // IO_REPARSE_TAG_WIM

#ifndef IO_REPARSE_TAG_CSV
#define IO_REPARSE_TAG_CSV                      (0x80000009L)
#endif // IO_REPARSE_TAG_CSV

#ifndef IO_REPARSE_TAG_DFS
#define IO_REPARSE_TAG_DFS                      (0x8000000AL)
#endif // IO_REPARSE_TAG_DFS

#ifndef IO_REPARSE_TAG_SYMLINK
#define IO_REPARSE_TAG_SYMLINK                  (0xA000000CL)
#endif // IO_REPARSE_TAG_SYMLINK

#ifndef IO_REPARSE_TAG_DFSR
#define IO_REPARSE_TAG_DFSR                     (0x80000012L)
#endif // IO_REPARSE_TAG_DFSR

#ifndef IO_REPARSE_TAG_DEDUP
#define IO_REPARSE_TAG_DEDUP                    (0x80000013L)
#endif // IO_REPARSE_TAG_DEDUP

#ifndef IO_REPARSE_TAG_NFS
#define IO_REPARSE_TAG_NFS                      (0x80000014L)
#endif // IO_REPARSE_TAG_NFS

#ifndef IO_REPARSE_TAG_FILE_PLACEHOLDER
#define IO_REPARSE_TAG_FILE_PLACEHOLDER         (0x80000015L)
#endif // IO_REPARSE_TAG_FILE_PLACEHOLDER

#ifndef IO_REPARSE_TAG_WOF
#define IO_REPARSE_TAG_WOF                      (0x80000017L)
#endif // IO_REPARSE_TAG_WOF

#ifndef IO_REPARSE_TAG_WCI
#define IO_REPARSE_TAG_WCI                      (0x80000018L)
#endif // IO_REPARSE_TAG_WCI

#ifndef IO_REPARSE_TAG_GLOBAL_REPARSE
#define IO_REPARSE_TAG_GLOBAL_REPARSE           (0xA0000019L)
#endif // IO_REPARSE_TAG_GLOBAL_REPARSE

#ifndef IO_REPARSE_TAG_CLOUD
#define IO_REPARSE_TAG_CLOUD                    (0x9000001AL)
#endif // IO_REPARSE_TAG_CLOUD

#ifndef IO_REPARSE_TAG_CLOUD_ROOT
#define IO_REPARSE_TAG_CLOUD_ROOT               (0x9000101AL)
#endif // IO_REPARSE_TAG_CLOUD_ROOT

#ifndef IO_REPARSE_TAG_CLOUD_ON_DEMAND
#define IO_REPARSE_TAG_CLOUD_ON_DEMAND          (0x9000201AL)
#endif // IO_REPARSE_TAG_CLOUD_ON_DEMAND

#ifndef IO_REPARSE_TAG_CLOUD_ROOT_ON_DEMAND
#define IO_REPARSE_TAG_CLOUD_ROOT_ON_DEMAND     (0x9000301AL)
#endif // IO_REPARSE_TAG_CLOUD_ROOT_ON_DEMAND

#ifndef IO_REPARSE_TAG_APPEXECLINK
#define IO_REPARSE_TAG_APPEXECLINK              (0x8000001BL)
#endif // IO_REPARSE_TAG_APPEXECLINK

#ifndef IO_REPARSE_TAG_GVFS
#define IO_REPARSE_TAG_GVFS                     (0x9000001CL)
#endif // IO_REPARSE_TAG_GVFS

#ifndef IO_REPARSE_TAG_WCI_TOMBSTONE
#define IO_REPARSE_TAG_WCI_TOMBSTONE            (0xA000001FL)
#endif // IO_REPARSE_TAG_WCI_TOMBSTONE

#ifndef IO_REPARSE_TAG_UNHANDLED
#define IO_REPARSE_TAG_UNHANDLED                (0x80000020L)
#endif // IO_REPARSE_TAG_UNHANDLED

#ifndef IO_REPARSE_TAG_ONEDRIVE
#define IO_REPARSE_TAG_ONEDRIVE                 (0x80000021L)
#endif // IO_REPARSE_TAG_ONEDRIVE

#ifndef IO_REPARSE_TAG_GVFS_TOMBSTONE
#define IO_REPARSE_TAG_GVFS_TOMBSTONE           (0xA0000022L)
#endif // IO_REPARSE_TAG_GVFS_TOMBSTONE

/* Reparse tags from MS-FSCC */

#ifndef IO_REPARSE_TAG_RESERVED_ZERO
/* Reserved reparse tag value */
#define IO_REPARSE_TAG_RESERVED_ZERO            (0x00000000L)
#endif // IO_REPARSE_TAG_RESERVED_ZERO

#ifndef IO_REPARSE_TAG_RESERVED_ONE
/* Reserved reparse tag value */
#define IO_REPARSE_TAG_RESERVED_ONE             (0x00000001L)
#endif // IO_REPARSE_TAG_RESERVED_ONE

#ifndef IO_REPARSE_TAG_DRIVER_EXTENDER
/* Home server drive extender */
#define IO_REPARSE_TAG_DRIVER_EXTENDER          (0x80000005L)
#endif // IO_REPARSE_TAG_DRIVER_EXTENDER

#ifndef IO_REPARSE_TAG_FILTER_MANAGER
/* Used by filter manager test harness */
#define IO_REPARSE_TAG_FILTER_MANAGER           (0x8000000BL)
#endif // IO_REPARSE_TAG_FILTER_MANAGER

#endif // __REPARSETAGS_H_VER__

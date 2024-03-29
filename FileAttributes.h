#ifndef __FILEATTRIBUTES_H_VER__
#define __FILEATTRIBUTES_H_VER__ 2017091421
#if (defined(_MSC_VER) && (_MSC_VER >= 1020)) || defined(__MCPP)
#    pragma once
#endif /* Check for #pragma support */

/* File attributes */

#ifndef FILE_ATTRIBUTE_READONLY
#    define FILE_ATTRIBUTE_READONLY 0x00000001
#endif // FILE_ATTRIBUTE_READONLY

#ifndef FILE_ATTRIBUTE_HIDDEN
#    define FILE_ATTRIBUTE_HIDDEN 0x00000002
#endif // FILE_ATTRIBUTE_HIDDEN

#ifndef FILE_ATTRIBUTE_SYSTEM
#    define FILE_ATTRIBUTE_SYSTEM 0x00000004
#endif // FILE_ATTRIBUTE_SYSTEM

#ifndef FILE_ATTRIBUTE_DIRECTORY
#    define FILE_ATTRIBUTE_DIRECTORY 0x00000010
#endif // FILE_ATTRIBUTE_DIRECTORY

#ifndef FILE_ATTRIBUTE_ARCHIVE
#    define FILE_ATTRIBUTE_ARCHIVE 0x00000020
#endif // FILE_ATTRIBUTE_ARCHIVE

#ifndef FILE_ATTRIBUTE_DEVICE
#    define FILE_ATTRIBUTE_DEVICE 0x00000040
#endif // FILE_ATTRIBUTE_DEVICE

#ifndef FILE_ATTRIBUTE_NORMAL
#    define FILE_ATTRIBUTE_NORMAL 0x00000080
#endif // FILE_ATTRIBUTE_NORMAL

#ifndef FILE_ATTRIBUTE_TEMPORARY
#    define FILE_ATTRIBUTE_TEMPORARY 0x00000100
#endif // FILE_ATTRIBUTE_TEMPORARY

#ifndef FILE_ATTRIBUTE_SPARSE_FILE
#    define FILE_ATTRIBUTE_SPARSE_FILE 0x00000200
#endif // FILE_ATTRIBUTE_SPARSE_FILE

#ifndef FILE_ATTRIBUTE_REPARSE_POINT
#    define FILE_ATTRIBUTE_REPARSE_POINT 0x00000400
#endif // FILE_ATTRIBUTE_REPARSE_POINT

#ifndef FILE_ATTRIBUTE_COMPRESSED
#    define FILE_ATTRIBUTE_COMPRESSED 0x00000800
#endif // FILE_ATTRIBUTE_COMPRESSED

#ifndef FILE_ATTRIBUTE_OFFLINE
#    define FILE_ATTRIBUTE_OFFLINE 0x00001000
#endif // FILE_ATTRIBUTE_OFFLINE

#ifndef FILE_ATTRIBUTE_NOT_CONTENT_INDEXED
#    define FILE_ATTRIBUTE_NOT_CONTENT_INDEXED 0x00002000
#endif // FILE_ATTRIBUTE_NOT_CONTENT_INDEXED

#ifndef FILE_ATTRIBUTE_ENCRYPTED
#    define FILE_ATTRIBUTE_ENCRYPTED 0x00004000
#endif // FILE_ATTRIBUTE_ENCRYPTED

#ifndef FILE_ATTRIBUTE_INTEGRITY_STREAM
#    define FILE_ATTRIBUTE_INTEGRITY_STREAM 0x00008000
#endif // FILE_ATTRIBUTE_INTEGRITY_STREAM

#ifndef FILE_ATTRIBUTE_VIRTUAL
#    define FILE_ATTRIBUTE_VIRTUAL 0x00010000
#endif // FILE_ATTRIBUTE_VIRTUAL

#ifndef FILE_ATTRIBUTE_NO_SCRUB_DATA
#    define FILE_ATTRIBUTE_NO_SCRUB_DATA 0x00020000
#endif // FILE_ATTRIBUTE_NO_SCRUB_DATA

#ifndef FILE_ATTRIBUTE_EA
#    define FILE_ATTRIBUTE_EA 0x00040000
#endif // FILE_ATTRIBUTE_EA

#ifndef FILE_ATTRIBUTE_PINNED
#    define FILE_ATTRIBUTE_PINNED 0x00080000
#endif // FILE_ATTRIBUTE_PINNED

#ifndef FILE_ATTRIBUTE_UNPINNED
#    define FILE_ATTRIBUTE_UNPINNED 0x00100000
#endif // FILE_ATTRIBUTE_UNPINNED

#ifndef FILE_ATTRIBUTE_RECALL_ON_OPEN
#    define FILE_ATTRIBUTE_RECALL_ON_OPEN 0x00040000
#endif // FILE_ATTRIBUTE_RECALL_ON_OPEN

#ifndef FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS
#    define FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS 0x00400000
#endif // FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS

#endif // __FILEATTRIBUTES_H_VER__

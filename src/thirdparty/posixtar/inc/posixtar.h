#ifndef     POSIXTAR_H
#define     POSIXTAR_H

/*****************************************************************************
**
**  Functions for reading posix 2001.1 tar files.
**
**  Creation date:  2015/06/12
**  Created by:     Morgun Dmytro
**
**
**  Copyright (c) 2015 Piql AS. All rights reserved.
**
*****************************************************************************/


//  PROJECT INCLUDES
//

//  SYSTEM INCLUDES
//

#include <stdio.h>
#if defined( __STDC__ )
#include <inttypes.h>
#include <stdbool.h>
#elif !defined( __cplusplus )
typedef int bool;
#define false 0
#define true 1
#endif
#include <stddef.h>
#include <time.h>

   
#define POSIXTAR_CHUNK_SIZE 512

typedef enum
{
    POSIXTAR_TYPE_NOT_TAR,
    POSIXTAR_TYPE_POSIXTAR,
    POSIXTAR_TYPE_USTAR,
    POSIXTAR_TYPE_GNU,
    POSIXTAR_TYPE_V7
} POSIXTAR_TARTYPE;

typedef enum
{
    POSIXTAR_SUCCESS = 0,
    POSIXTAR_EOF,
    POSIXTAR_UNEXPECTED_EOF,
    POSIXTAR_EMPTY_HEADER,
    POSIXTAR_WRONG_SIZE,
    POSIXTAR_EXT_ERROR,
    POSIXTAR_GNULONG_FAIL,
    POSIXTAR_UNKNOWN

} POSIXTAR_NEXT_HEADER_TYPE;

/*
 * This is the ustar (Posix 1003.1) header.
 */
typedef struct
{
    char name[100];        /*   0 Filename               */
    char mode[8];          /* 100 Permissions            */
    char uid[8];           /* 108 Numerical User ID      */
    char gid[8];           /* 116 Numerical Group ID     */
    char size[12];         /* 124 Filesize               */
    char mtime[12];        /* 136 st_mtime               */
    char chksum[8];        /* 148 Checksum               */
    char typeflag;         /* 156 Typ of File            */
    char linkname[100];    /* 157 Target of Links        */
    char magic[6];         /* 257 "ustar"                */
    char version[2];       /* 263 Version fixed to 00    */
    char uname[32];        /* 265 User Name              */
    char gname[32];        /* 297 Group Name             */
    char devmajor[8];      /* 329 Major for devices      */
    char devminor[8];      /* 337 Minor for devices      */
    char prefix[155];      /* 345 Prefix for t_name      */
    /* 500 End                    */
    char padding[12];      /* 500 Filler up to 512       */
} posixtar_ustar_header;


enum posixtar_file_type
{
    posixtar_entry_normal = '0', // It also can be '\0', but that '\0' will be changed to '0' when parsing.
    posixtar_entry_hardlink = '1',
    posixtar_entry_symlink = '2',
    posixtar_entry_charspecial = '3',
    posixtar_entry_blockspecial = '4',
    posixtar_entry_directory = '5',
    posixtar_entry_fifo = '6',
    posixtar_entry_eof = '\\'
};

typedef struct
{
    unsigned long long  header_offset; // Offset from start of tar of this header in bytes.
    unsigned long long  data_offset; // Offset from start of tar of the file's data in bytes (may be sparse in the future).
    unsigned long long  data_size; // Bytes
    unsigned long long  next_header_offset; // From start of tar to next header
    time_t              time;
    char *              filename; // Care about the memory here. This name is not limited to any size (unlike that headers). Use free() to free it
    char                type; // For all that links etc.
} posixtar_header;


typedef struct _posixtar_posixtar
{
    FILE *  file;
    unsigned long long      next_entry_pos;

    bool                    have_global_file_size;
    bool                    have_global_mtime;
    bool                    have_global_filename;
    unsigned long long      global_file_size;
    unsigned long long      global_mtime;
    char *                  global_filename;
} posixtar_posixtar, *posixtar_handle;


#ifdef __cplusplus
extern "C"
{
#endif

posixtar_handle     posixtar_open(const char* filename);
posixtar_handle     posixtar_open_offset(const char* filename, unsigned long long offset);
posixtar_handle     posixtar_open_handle(FILE* handle);
void                posixtar_close(posixtar_handle handle);
bool                posixtar_is_posixtar(const char* filename); // if the library supports this tar type
bool                posixtar_is_posixtar_offset(const char* filename, unsigned long long offset); // if the library supports this tar type
bool                posixtar_is_posixtar_handle(FILE* handle); // if the library supports this tar type
POSIXTAR_TARTYPE    posixtar_get_type(const char* filename);
POSIXTAR_TARTYPE    posixtar_get_type_offset(const char* filename, unsigned long long offset);
POSIXTAR_TARTYPE    posixtar_get_type_handle(FILE* handle);
posixtar_header*    posixtar_header_next(posixtar_handle); // This function creates a header, so
int                 posixtar_header_is_empty(posixtar_header * header);
void                posixtar_header_free(posixtar_header* header); // you must free the header with this function.

// Functions to handle gzip compressed archives
bool                posixtar_is_gzip_compressed(const char* filename);
int                 posixtar_gunzip_file(const char* source, const char* dest);
posixtar_handle     posixtar_open_gzip(const char* filename);
posixtar_handle     posixtar_open_offset_gzip(const char* filename, unsigned long long offset);


#ifdef __cplusplus
}
#endif

#endif /* POSIXTAR_H */

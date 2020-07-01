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

#include "dconfig.h"

// For the Large Files Support, should be included prior to <stdio.h>
#define _FILE_OFFSET_BITS  64

#if defined ( D_OS_UNIX )
#define _LARGEFILE64_SOURCE
#endif

#include "posixtar.h"


//  SYSTEM INCLUDES
//
#if defined ( D_OS_UNIX )
#include <sys/types.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <assert.h>

#if defined ( D_OS_WIN32 )
#define ZLIB_WINAPI
#endif
#include <zlib.h>

#if !defined( __STDC__ )
#define ULONGLONG __int64
#define atoll     _atoi64
#else
#define ULONGLONG unsigned long long
#endif

//  FORWARD DECLARATIONS
//
static size_t              read_ustar_header(posixtar_ustar_header *header, posixtar_handle handle);
static size_t              read_extended_header_data(bool is_global, posixtar_header * info, size_t all_headers_size, posixtar_handle handle);
static size_t              read_GNU_long_name(posixtar_header * info, posixtar_handle handle);
static char *              get_filename_from_header(posixtar_ustar_header * header);
static void                apply_globals(posixtar_header * info, posixtar_handle handle);
static bool                process_extended_header(const char * key, const char * value, posixtar_header * info, bool isGlobal, posixtar_handle handle);
static posixtar_header*    posixtar_header_create();
static char *              string_create_copy(const char *str);
static void                string_append(char **to, const char *from);
static unsigned char       octal_to_numeric(char octal);
static size_t              octal8_to_numeric(const char octal[8]);
static long long           byte12_to_numeric(const char octal[12]);
static ULONGLONG           get_header_checksum(const posixtar_ustar_header * header);
static char                decode_base64char(unsigned char byte);

#define POSIXTAR_SAFE_FREE(pointer) {free(pointer); pointer = NULL;}

/****************************************************************************/
/*! posixtar  posixtar.h
 *  \brief  Holds basic POSIX 2001 operations.
 *
 *  Cross platform POSIX tar format reader. Supports extended POSIX pax headers 
 *  (thus supporting arbitrary length files and filenames) and old GNU long 
 *  filenames.
 */

//============================================================================
//  P U B L I C   I N T E R F A C E
//============================================================================

#if defined ( D_OS_WIN32 )
#define DFSEEK64 _fseeki64
#elif defined ( D_OS_X )
#define DFSEEK64 fseeko
#else
#define DFSEEK64 fseeko64
#endif


//----------------------------------------------------------------------------
/*!
 *  Returns the type of the (tar) file given.
 *  \return Returns the type of the (tar) file given. The enum values are:
 *  POSIXTAR_TYPE_NOT_TAR,
 *  POSIXTAR_TYPE_POSIXTAR,
 *  POSIXTAR_TYPE_USTAR,
 *  POSIXTAR_TYPE_GNU,
 *  POSIXTAR_TYPE_V7
 *  \param filename The file's name to check.
 */
POSIXTAR_TARTYPE posixtar_get_type(const char* filename)
{
#if defined ( D_OS_WIN32 )
    FILE *file = fopen(filename, "rb");
#else
    FILE *file = fopen(filename, "r");
#endif
    if (file == NULL)
    {
        return false;
    }

    POSIXTAR_TARTYPE result = posixtar_get_type_handle( file );

    fclose(file);

    return result;
}

POSIXTAR_TARTYPE posixtar_get_type_offset(const char* filename, unsigned long long offset)
{
#if defined ( D_OS_WIN32 )
    FILE *file = fopen(filename, "rb");
#else
    FILE *file = fopen(filename, "r");
#endif
    if (file == NULL)
    {
        return false;
    }

    DFSEEK64(file, offset, SEEK_CUR );

    POSIXTAR_TARTYPE result = posixtar_get_type_handle( file );

    fclose(file);

    return result;
}

POSIXTAR_TARTYPE posixtar_get_type_handle(FILE* file)
{
    /* // some info on types tested (that can be used to improve the support and detection):
    posix (and pax):
    name: ./PaxHeaders.
    typeflag: 'x'
    magic: 'u' 's' 't' 'a' 'r' '\0'
    version: '0' '0'

    ustar:
    name: basefile.dat
    typeflag: 0
    magic: 'u' 's' 't' 'a' 'r' '\0'
    version: '0' '0'
    
    gnu/oldgnu:
    name: basefile.dat
    typeflag: uhm, different, 0 generally, it was 5 on osx.
    magic: 'u' 's' 't' 'a' 'r' ' '
    version: '0' '0' (' ' '\0' on LNX)

    v7:
    name: basefile.dat
    typeflag: 0
    magic: ustar (space terminated) OR all \0
    version:  \0 (space and \0)
    */
    char magick_ustar[] = "ustar";

    posixtar_ustar_header header;
    size_t read_bytes = fread(&header, 1, 512, file);
    fseek(file, -(long)read_bytes, SEEK_CUR);

    if (512 != read_bytes)
    {
        return POSIXTAR_TYPE_NOT_TAR;
    }

    if (get_header_checksum(&header) != octal8_to_numeric(header.chksum))
    {
        return POSIXTAR_TYPE_NOT_TAR;
    }
    
    if (memcmp(header.magic, magick_ustar, 5) == 0)
    {
        if (header.typeflag == 'x')
        {
            return POSIXTAR_TYPE_POSIXTAR;
        }
        else
        {
            if (header.magic[5] == '\0')
            {
                return POSIXTAR_TYPE_USTAR;
            }
            else if (header.magic[5] == ' ')
            {
                return POSIXTAR_TYPE_GNU;
            }
            else
            {
                return POSIXTAR_TYPE_NOT_TAR;
            }
        }
    }
    else
    {
        memset(magick_ustar, 0, 6);
        if (memcmp(magick_ustar, header.magic, 6) == 0)
        {
            return POSIXTAR_TYPE_V7;
        }
        else
        {
            return POSIXTAR_TYPE_NOT_TAR;
        }
    }
}

//----------------------------------------------------------------------------
/*!
 *  Checks if the given file is supported by the library. Note, NOT is it posixtar, the library supports a bit more.
 *  \return Returns if the given file is supported by the library.
 *  \param filename The file's name to check.
 */
bool posixtar_is_posixtar(const char* filename)
{
#if defined ( D_OS_WIN32 )
    FILE *file = fopen(filename, "rb");
#else
    FILE *file = fopen(filename, "r");
#endif

    if (file == NULL)
    {
        return false;
    }

    return posixtar_is_posixtar_handle(file);
}

bool posixtar_is_posixtar_offset(const char* filename, unsigned long long offset)
{
#if defined ( D_OS_WIN32 )
    FILE *file = fopen(filename, "rb");
#else
    FILE *file = fopen(filename, "r");
#endif

    if (file == NULL)
    {
        return false;
    }

    DFSEEK64(file, offset, SEEK_CUR );

    return posixtar_is_posixtar_handle(file);
}

bool posixtar_is_posixtar_handle(FILE* file)
{
    char magick_ustar[] = "ustar";
    char magick_posix[] = "posix";
    posixtar_ustar_header header;
    size_t read_bytes = fread(&header, 1, 512, file);
    fclose(file);

    if (512 != read_bytes)
    {
        return false;
    }

    if (get_header_checksum(&header) != octal8_to_numeric(header.chksum))
    {
        return false;
    }

    if (memcmp(header.magic, magick_ustar, 5) != 0 && memcmp(header.magic, magick_posix, 6) != 0)
    {
        memset(magick_ustar, 0, 6);
        if (memcmp(header.magic, magick_ustar, 5) != 0)
        {
            return false;
        }
    }

    return true;
}

//----------------------------------------------------------------------------
/*!
 *  Checks if the given header entry is empty.
 *  \return Returns 0 (POSIXTAR_SUCCESS) if the header is NOT empty.
 *     Otherwise returns:
 *          POSIXTAR_EOF            - if there is a graceful end of file found.
 *          POSIXTAR_UNEXPECTED_EOF - if unexpected end of file found.
 *          POSIXTAR_EMPTY_HEADER   - if the header is null or the file is not open.
 *          POSIXTAR_EXT_ERROR      - error reading extended header.
 *          POSIXTAR_WRONG_SIZE     - wrong file size, less than 0.
 *          POSIXTAR_GNULONG_FAIL   - failed reading header after GNU long filename.
 *          POSIXTAR_UNKNOWN        - if the header is unknown. Probably file format is corrupted.
 *  \param header The header to check.
 */

int posixtar_header_is_empty(posixtar_header * header)
{
    if (header != NULL)
    {
        if (header->filename != NULL)
        {
            return POSIXTAR_SUCCESS;
        }
        else
        {
            switch(header->type)
            {
                case posixtar_entry_eof:
                    return POSIXTAR_EOF;

                case POSIXTAR_UNEXPECTED_EOF:
                    return POSIXTAR_UNEXPECTED_EOF;

                case POSIXTAR_EMPTY_HEADER:
                    return POSIXTAR_EMPTY_HEADER;

                case POSIXTAR_EXT_ERROR:
                    return POSIXTAR_EXT_ERROR;

                case POSIXTAR_WRONG_SIZE:
                    return POSIXTAR_WRONG_SIZE;

                case POSIXTAR_GNULONG_FAIL:
                    return POSIXTAR_GNULONG_FAIL;

                default:
                    return POSIXTAR_UNKNOWN;
            }
        }
    }
    else
    {
        return POSIXTAR_EMPTY_HEADER;
    }
}


//----------------------------------------------------------------------------
/*!
 *  Opens the posixtar handler to a given filename tar archive.
 *  \return Not NULL handle if successfull, NULL otherwise.
 *  \param filename Name of the tar file to be opened.
 *
 *  Only opens for reading now.
 */

posixtar_handle posixtar_open(const char* filename)
{
#if defined ( D_OS_WIN32 )
    FILE* file = fopen(filename, "rb");
#else
    FILE* file = fopen(filename, "r");
#endif

    if (file)
    {
        POSIXTAR_TARTYPE tarType = posixtar_get_type_handle(file);
        switch (tarType)
        {
        case POSIXTAR_TYPE_GNU:
        case POSIXTAR_TYPE_POSIXTAR:
        case POSIXTAR_TYPE_USTAR:
        case POSIXTAR_TYPE_V7:
        {
            return posixtar_open_handle(file);
        }
        break;
        default:
            return NULL;
        }
    }
    return NULL;
}

posixtar_handle posixtar_open_offset(const char* filename, unsigned long long offset)
{
    posixtar_handle handle = posixtar_open(filename);
    if (handle)
    {
        handle->next_entry_pos = offset;
    }
    return handle;
}


posixtar_handle posixtar_open_handle(FILE* file)
{
    posixtar_handle handle = (posixtar_handle)malloc(sizeof(posixtar_posixtar));
    handle->file = file;
    if (handle->file != NULL)
    {
        handle->have_global_filename = false;
        handle->have_global_file_size = false;
        handle->have_global_mtime = false;
        handle->global_filename = NULL;
        handle->next_entry_pos = 0;
        return handle;
    }
    else
    {
        free(handle);
        return NULL;
    }
}

//----------------------------------------------------------------------------
/*!
 *  Closes the handler. Handler is no longer valid after this call.
 *  \param handle The handle to close.
 */

void posixtar_close(posixtar_handle handle)
{
    if (handle != NULL)
    {
        fclose(handle->file);
        POSIXTAR_SAFE_FREE(handle->global_filename);
        free(handle);
    }
}


//----------------------------------------------------------------------------
/*! Releases the header created by posixtar_next_header
 *  \param header The header to free.
 *  Safely accepts NULL as a parameter.
 */

void posixtar_header_free(posixtar_header* header)
{
    if (header != NULL)
    {
        POSIXTAR_SAFE_FREE(header->filename);
        free(header);
    }
}


//----------------------------------------------------------------------------
/*! Returns the next entry in the opened archive.
 *  \return Returns the next entry in the opened archive. Check if it is posixtar_header_is_empty() to know if it was the last.
 *  \param handle The handle to open archive.
 *  May return NULL on error. You must free the returned header by calling posixtar_header_free, it is safe to pass NULL there.
 */

posixtar_header* posixtar_header_next(posixtar_handle handle)
{
    posixtar_header *return_value;
    posixtar_ustar_header header;
    size_t read_header_size;

    if (handle == NULL || handle->file == NULL)
    {
        return NULL;
    }
    return_value = posixtar_header_create();
    return_value->header_offset = handle->next_entry_pos;

    read_header_size = read_ustar_header(&header, handle);
    if (read_header_size == 0)
    {
        return_value->type = posixtar_entry_eof;
        return return_value;
    }
    else if (read_header_size != sizeof(posixtar_ustar_header))
    {
        return_value->type = POSIXTAR_UNEXPECTED_EOF;
        return return_value;
    }

    return_value->time = 0;
    return_value->data_size = 0;
    
    return_value->data_offset = return_value->header_offset + POSIXTAR_CHUNK_SIZE;
    
    while (header.typeflag == 'x' || header.typeflag == 'g') // read all the extended headers
    {
        long long size_read = byte12_to_numeric(header.size);
        if (size_read < 0)
        {
            return_value->type = POSIXTAR_WRONG_SIZE;
            return return_value;
        }
        unsigned long long headers_size = (unsigned long long)size_read;
        size_t extended_data_size = read_extended_header_data(header.typeflag == 'g', return_value, (size_t)headers_size, handle);
        size_t extended_headers_number;

        if ((size_t)-1 == extended_data_size) // Error reading extended header
        {
            return_value->data_offset = return_value->header_offset + POSIXTAR_CHUNK_SIZE;
            return_value->data_size = 0;
            POSIXTAR_SAFE_FREE(return_value->filename); // To indicate end.
            return_value->next_header_offset = return_value->data_offset;
            handle->next_entry_pos = return_value->next_header_offset;
            return_value->type = POSIXTAR_EXT_ERROR;
            return return_value;
        }
        
        extended_headers_number = (extended_data_size + POSIXTAR_CHUNK_SIZE - 1) / POSIXTAR_CHUNK_SIZE;
        return_value->data_offset += POSIXTAR_CHUNK_SIZE * extended_headers_number;
    
        handle->next_entry_pos = return_value->data_offset;
    
        // Read the next header
        read_header_size = read_ustar_header(&header, handle);
    
        return_value->data_offset += POSIXTAR_CHUNK_SIZE;
        handle->next_entry_pos = return_value->data_offset;
    
        if (read_header_size == 0)
        {
            return_value->type = posixtar_entry_eof;
            return return_value;
        }
        else if (read_header_size != sizeof(header))
        {
            return return_value;
        }
    }
    
    if (return_value->time == 0)
    {
        return_value->time = byte12_to_numeric(header.mtime);
    }
    
    if ('L' == header.typeflag) // GNU long header
    {
        size_t extended_data_size;
        posixtar_header * realInfo;

        POSIXTAR_SAFE_FREE(return_value->filename);
        extended_data_size = read_GNU_long_name(return_value, handle);
        return_value->data_offset = return_value->header_offset + extended_data_size;
        long long size_read = byte12_to_numeric(header.size);
        if (size_read < 0)
        {
            return_value->type = POSIXTAR_WRONG_SIZE;
            return NULL;
        }
        return_value->data_size = (unsigned long long)size_read;
    
        return_value->next_header_offset = return_value->data_offset + POSIXTAR_CHUNK_SIZE * ((return_value->data_size + POSIXTAR_CHUNK_SIZE - 1) / POSIXTAR_CHUNK_SIZE);
        handle->next_entry_pos = return_value->next_header_offset;
    
        realInfo = posixtar_header_next(handle);
        if (realInfo != NULL)
        {
            return_value->data_offset = realInfo->data_offset;
            return_value->data_size = realInfo->data_size;
            return_value->next_header_offset = realInfo->next_header_offset;
            return_value->time = realInfo->time;
            posixtar_header_free(realInfo);
            return return_value;
        }
        else
        {
            POSIXTAR_SAFE_FREE(return_value->filename);
            return_value->type = POSIXTAR_GNULONG_FAIL;
            return NULL;
        }
    }
    
    // If these values are not set in the previous headers, then set them from the last header.
    if (return_value->filename == NULL)
    {
        return_value->filename = get_filename_from_header(&header);
    }
    if (return_value->data_size == 0)
    {
        long long size_read = byte12_to_numeric(header.size);
        if (size_read < 0)
        {
            POSIXTAR_SAFE_FREE(return_value->filename);
            return_value->type = POSIXTAR_WRONG_SIZE;
            return return_value;
        }
        return_value->data_size = (unsigned long long)size_read;
    }
    
    return_value->type = (char)header.typeflag;
    
    apply_globals(return_value, handle);
    
    return_value->next_header_offset = return_value->data_offset + POSIXTAR_CHUNK_SIZE * ((return_value->data_size + POSIXTAR_CHUNK_SIZE - 1) / POSIXTAR_CHUNK_SIZE);
    
    handle->next_entry_pos = return_value->next_header_offset;
    
    
    return return_value;
}



//============================================================================
//  P R I V A T E   I N T E R F A C E
//============================================================================

size_t read_ustar_header(posixtar_ustar_header *header, posixtar_handle handle)
{
    size_t bytes_read;
    bool all_zero_record;
    int i;

    DFSEEK64(handle->file, handle->next_entry_pos, SEEK_SET);
    bytes_read = fread(header, 1, sizeof(posixtar_ustar_header), handle->file);

    handle->next_entry_pos += bytes_read;

    if (sizeof(posixtar_ustar_header) != bytes_read)
    {
        return -2; // Unsupported or malformed tar.
    }

    // Check if it is not zero record. It indicates the end of the archive.

    all_zero_record = true;
    for (i = 0; i < 512; ++i)
    {
        if (((char *)header)[i] != 0)
        {
            all_zero_record = false;
            break;
        }
    }
    if (all_zero_record)
    {
        return 0; // End of the archive.
    }

    if (get_header_checksum(header) != octal8_to_numeric(header->chksum))
    {
        return (size_t)-1; // Checksum error.
    }

    if (header->typeflag == '\0' || header->typeflag == '7') // 7 is CONTTYPE, we don't care if it is continuous. \0 is old format.
    {
        header->typeflag = '0';
    }
    return bytes_read;
}

posixtar_header* posixtar_header_create()
{
    posixtar_header * return_value = (posixtar_header *)malloc(sizeof(posixtar_header));
    memset(return_value, 0, sizeof(posixtar_header));
    return return_value;
}


void apply_globals(posixtar_header * info, posixtar_handle handle)
{
    if (handle->have_global_filename)
    {
        size_t filename_size;

        POSIXTAR_SAFE_FREE(info->filename);
        filename_size = strlen(handle->global_filename) + 1;
        info->filename = (char *)malloc(filename_size);
        memcpy(info->filename, handle->global_filename, filename_size);
    }
    if (handle->have_global_file_size)
    {
        info->data_size = handle->global_file_size;
    }
    if (handle->have_global_mtime)
    {
        info->time = handle->global_mtime;
    }
}


char *string_create_copy(const char *str)
{
    size_t size = strlen(str) + 1;
    char *return_value = (char *)malloc(size);
    memcpy(return_value, str, size);
    return return_value;
}

void string_append(char **to, const char *from)
{
    if (*to != NULL)
    {
        size_t toSize = strlen(*to);
        size_t fromSize = strlen(from);
        size_t size = toSize + fromSize + 1;
        char *old = *to;
        
        *to = (char *)malloc(size);
        memcpy(*to, old, toSize);
        memcpy(*to + toSize, from, fromSize + 1);
        POSIXTAR_SAFE_FREE(old);
    }
    else
    {
        *to = string_create_copy(from);
    }
}


char * get_filename_from_header(posixtar_ustar_header * header)
{
    size_t base_length;
    size_t final_length;
    char base_name[101];
    base_name[100] = '\0';
    memcpy(base_name, header->name, 100);
    base_length = strlen(base_name);
    final_length = base_length;

    if (header->prefix[0] != '\0')
    {
        char prefix[156];
        char *return_value;
        size_t prefix_length;
        prefix[155] = '\0';
        memcpy(prefix, header->prefix, 155);
        prefix_length = strlen(prefix);
        final_length += prefix_length + 1;

        return_value = (char *)malloc(final_length + 1);
        memcpy(return_value, prefix, prefix_length);
        return_value[prefix_length] = '/';
        memcpy(return_value + prefix_length + 1, prefix, base_length + 1);
        return return_value;
    }
    else
    {
        char *return_value = (char *)malloc(final_length + 1);
        memcpy(return_value, base_name, base_length + 1);
        return return_value;
    }
}


/*
 * EXTENDED TAR (PAX) HEADER FORMAT
 *
 *      The  data  block that follows a tar archive header with typeflag �’g�’ or
 *      �’x�’ contains one or more records in the following format:
 *
 *             "%d %s=%s\n", <length>, <keyword>, <value>
 *
 *      Each record starts with a a decimal length field. The  length  includes
 *      the  total  size  of a record including the length field itself and the
 *      trailing new line.
 *
 *      The keyword may not include an equal sign.  All keywords beginning with
 *      lower  case letters and digits are reserved for future use by the POSIX
 *      standard.
 *
 *      If the value field is of zero length, it deletes any  header  field  of
 *      the same name that is in effect from the same extended header or from a
 *      previous global header.
 *
 *      Null characters do not delimit any value. The value is only limited  by
 *      its implicit length.
 *
 *      (c) ubuntu man 5 star http://manpages.ubuntu.com/manpages/intrepid/man5/star.5.html
 */

size_t read_extended_header_data(bool is_global, posixtar_header * info, size_t all_headers_size, posixtar_handle handle)
{
    char * read_buffer = (char *)malloc(all_headers_size);
    char *current_char = read_buffer;
    char number_buffer[512];
    char * record_end;
    char * record;
    size_t bytes_read = fread(read_buffer, 1, all_headers_size, handle->file);
    if (bytes_read != all_headers_size) // Malformed tar!
    {
        free(read_buffer);
        return (size_t)-1;
    }

    do // We're reading the strings formatted like %d %s=%s\n
    {
        // Read %d till we encounter ' '
        unsigned int number_index = 0;
        long long record_size = 0;

        char *record_start = current_char;

        while (*current_char != ' ')
        {
            number_buffer[number_index] = *current_char;
            ++current_char;
            ++number_index;
            if (number_index == all_headers_size) // We don't supprt numbers longer then 511 here.
            {
                free(read_buffer);
                return (size_t)-1;
            }

            if (current_char == read_buffer + all_headers_size)
            {
                free(read_buffer);
                return (size_t)-1;
            }
        }
        number_buffer[number_index] = '\0';
        record_size = atoll(number_buffer);

        if (record_size < number_index + 4)
        {
            free(read_buffer);
            return (size_t)-1; // Wrong record size.
        }

        ++current_char;
        record = current_char; //points to %s=%s\n...

        while (*current_char != '=' && current_char != read_buffer + all_headers_size)
        {
            ++current_char;
        }
        if ((size_t)(current_char - read_buffer) >= all_headers_size - 1)
        {
            free(read_buffer);
            return (size_t)-1; // Wrong record, it misses '='.
        }
        *current_char = '\0';
        ++current_char;

        record_end = record_start + record_size - 1; // Points to '\n'.
        *record_end = '\0';

        process_extended_header(record, current_char, info, is_global, handle);

        current_char = record_end + 1;
    } while (current_char < read_buffer + all_headers_size);

    free(read_buffer);

    return all_headers_size;
}


bool  process_extended_header(const char * key, const char * value, posixtar_header * info, bool is_global, posixtar_handle handle)
{
    if (strncmp("size", key, 4) == 0)
    {
        if ('\0' == value[0])
        {
            if (is_global)
            {
                handle->have_global_file_size = false;
            }
        }
        else
        {
            info->data_size = (unsigned long long)atoll(value);
            if (is_global)
            {
                handle->have_global_file_size = true;
                handle->global_file_size = info->data_size;
            }
        }
        return true;
    }
    else if (strncmp("path", key, 4) == 0 || strncmp("linkpath", key, 8) == 0)
    {
        if ('\0' == value[0])
        {
            if (is_global)
            {
                handle->have_global_filename = false;
                POSIXTAR_SAFE_FREE(handle->global_filename);
            }
        }
        else
        {
            POSIXTAR_SAFE_FREE(info->filename);
            info->filename = string_create_copy(value);
            if (is_global)
            {
                handle->have_global_filename = true;
                POSIXTAR_SAFE_FREE(handle->global_filename);
                handle->global_filename = string_create_copy(value);
            }
        }
        return true;
    }
    else if (strncmp("mtime", key, 5) == 0)
    {
        if ('\0' == value[0])
        {
            if (is_global)
            {
                handle->have_global_mtime = false;
            }
        }
        else
        {
            info->time = atoll(value);
            if (is_global)
            {
                handle->have_global_mtime = true;
                handle->global_mtime = info->time;
            }
        }
        return true;
    }
    else
    {
        return false;
    }
}


size_t read_GNU_long_name(posixtar_header * info, posixtar_handle handle)
{
    char name_buffer[POSIXTAR_CHUNK_SIZE + 1];

    int counter = 1;

    while (feof(handle->file) == 0)
    {
        size_t read_size = fread(name_buffer, 1, POSIXTAR_CHUNK_SIZE, handle->file);
        if (read_size < POSIXTAR_CHUNK_SIZE)
        {
            name_buffer[read_size] = '\0';
            string_append(&info->filename, name_buffer);
        }
        else
        {
            int i;
            for (i = 0; i < POSIXTAR_CHUNK_SIZE; ++i)
            {
                if (name_buffer[i] == '\0')
                {
                    string_append(&info->filename, name_buffer);
                    return counter * POSIXTAR_CHUNK_SIZE;
                }
            }

            name_buffer[POSIXTAR_CHUNK_SIZE] = '\0';
            string_append(&info->filename, name_buffer);
            ++counter;
        }
    }
    return counter * POSIXTAR_CHUNK_SIZE;
}

// Thanks to Marius Bancila for this conversion functions!

unsigned char octal_to_numeric(char octal)
{
    return (octal >= '0' && octal <= '7') ? octal - '0' : 0;
}


size_t octal8_to_numeric(const char octal[8])
{
    size_t result = 0;

    int i, pos = 0;
    while (octal[pos] == ' ' || octal[pos] == '0') ++pos;

    for (i = pos; i < 8 && octal[i] != 0 && octal[i] != ' '; ++i)
    {
        result = (result << 3) + octal_to_numeric(octal[i]);
    }

    return result;
}


static long long byte12_to_numeric(const char byte12[12])
{
    unsigned long long result = 0;
    char temp;
    char negativeMultyplier = 1;

    int i, pos = 0;
    switch(byte12[0])
    {
        case ' ': // octal
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            
            while (byte12[pos] == ' ' || byte12[pos] == '0') ++pos;

            for (i = pos; i < 12 && byte12[i] != 0 && byte12[i] != ' '; ++i)
            {
                result = (result << 3) + octal_to_numeric(byte12[i]);
            }
            break;
        case (char)0xFF:// base 256 negative
            negativeMultyplier = -1;
        case (char)0x80: // base 256
            for (i = 1; i < 12; ++i)
            {
                result <<= 8; /* result *= 256 */
                result += (unsigned char)byte12[i];
            }
            result *= negativeMultyplier;
            break;
        case '-': // base 64 negative
            negativeMultyplier = -1;
        case '+': // base 64
            for (i = 1; i < 12; ++i)
            {
                if ((temp = decode_base64char(byte12[i])) >= 0)
                {
                    result <<= 6; /* result *= 64 */
                    result += temp;
                }
            }
            result *= negativeMultyplier;
            break;
    }

    return result;
}


ULONGLONG get_header_checksum(const posixtar_ustar_header * header)
{
    ULONGLONG crc = 0;
    int i;

    for (i = 0; i < 148; ++i)
        crc += ((unsigned char *)header)[i];
    for (i = 0; i < 8; ++i)
        crc += 32; // (unsigned char)' ';
    for (i = 156; i < 500; ++i) // You must not calculate the checksum after 508th byte. Starting from 500 to 508 they must be 0 anyway.
        crc += ((unsigned char *)header)[i];
    return crc;
}


/* ======================================================================
 * return the RFC 2045 base 64 value for a byte character 
 * -1 for any out of range
 *
 * base 64 uses A-Z as 0-25, a-z as 26-51, 0-9 as 52-61, + as 62 and / as 63
 */
static char decode_base64char (unsigned char byte)
{
    /* 
     * trade off of space for table versus time to upsize the return
     * and reupsize where it is used
     */
    static const unsigned char table[] = 
    {
        /*
         * 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F
         */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, /* 00 */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, /* 10 */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63, /* 20 */
        52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1, /* 30 */
        -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14, /* 40 */
        15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1, /* 50 */
        -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40, /* 60 */
        41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1, /* 70 */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, /* 80 */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, /* 90 */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, /* A0 */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, /* B0 */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, /* C0 */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, /* D0 */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, /* E0 */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1  /* F0 */
    };
    return table[byte];
}


//==================================================================
// Decompression files using zlib

#define CHUNK 16384

int posixtar_gunzip_file(const char* source, const char* dest)
{
    gzFile file_source = gzopen(source, "rb");
    FILE* file_dest = fopen(dest, "wb");

    if (file_source && file_dest)
    {
        while (true)
        {
            int err;
            int bytes_read;
            unsigned char buffer[CHUNK];
            bytes_read = gzread (file_source, buffer, CHUNK - 1);
            buffer[bytes_read] = '\0';

            fwrite(buffer, bytes_read, 1, file_dest);

            if (bytes_read < CHUNK - 1)
            {
                if (gzeof(file_source))
                {
                    break;
                }
                else
                {
                    gzerror(file_source, &err);
                    fclose(file_dest);
                    return err;
                }
            }
        }
        gzclose(file_source);
        fclose(file_dest);
        return Z_OK;
    }
    return Z_DATA_ERROR;
}

#undef CHUNK

posixtar_handle posixtar_open_gzip(const char* filename)
{
    // Create name of unzipped file
    char file_name_dest[512];
    strcpy(file_name_dest, filename);
    const size_t file_name_length = strlen(file_name_dest);
    for (size_t i = 0; i < file_name_length; ++i)
    {
        if (file_name_dest[i] == '.')
        {
            file_name_dest[i] = '\0';
            break;
        }
    }
    strcat(file_name_dest, ".tar");

    // Decompress the file to the destination tar file.
    const int gunzip_result = posixtar_gunzip_file(filename, file_name_dest);

    if (gunzip_result == 0)
    {
        return posixtar_open(file_name_dest);
    }

    return NULL;
}

posixtar_handle posixtar_open_offset_gzip(const char* filename, unsigned long long offset)
{
    posixtar_handle handle = posixtar_open_gzip(filename);
    if (handle)
    {
        handle->next_entry_pos = offset;
    }
    return handle;
}

bool posixtar_is_gzip_compressed(const char* filename)
{
    // Here is should be magic number at the beginning.
    // For gzip compressed files first two bytes are 0x1f, 0x8b
    // Read it separately due to different endianness
    FILE* file = fopen(filename, "rb");
    if (file)
    {
        unsigned char first_byte = 0;
        unsigned char second_byte = 0;
        const size_t read_bytes_first = fread(&first_byte, 1, 1, file);
        const size_t read_bytes_second = fread(&second_byte, 1, 1, file);
        if (read_bytes_first  == 1    &&
            read_bytes_second == 1    &&
            first_byte        == 0x1f &&
            second_byte       == 0x8b)
        {
            fclose(file);
            return true;
        }
        fclose(file);
    }
    return false;
}

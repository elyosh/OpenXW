#ifndef XW_ASSETS_FILE_H
#define XW_ASSETS_FILE_H
#ifdef XW_MODERN
#include "xw_runtime/storage/file_io.h"
#include "xw_runtime/storage/storage.h"
#endif

#include <stdint.h>
#include <stdio.h>

#ifdef XW_MODERN
typedef AeronFile XwFile;

enum { FILE_MOUNTED_CD_DRIVE = 'D', FILE_DRIVE_PATH_PREFIX_LENGTH = 3 };

#define File_RawOpenText XwStorage_OpenText
#define File_RawOpen XwStorage_Open
#define File_RawReadTextLine XwFile_GetsText
#define File_RawReadLine XwFile_Gets
#define File_RawHasError AeronVfs_HasError
#define File_RawAtEnd AeronVfs_Eof
#define File_RawClose XwFile_Close
#define File_RawRemove XwStorage_Remove
#define File_RawRead XwFile_Read
#define File_RawWrite XwFile_Write
#define File_RawGetChar XwFile_Getc
#define File_RawPutChar XwFile_Putc
#define File_RawTell XwFile_Tell
#define File_RawSeek XwFile_Seek
#define File_RawLength XwFile_Length
#define File_FindMountedCdDrive XwStorage_FindMountedCdDrive
#else
#include <io.h>
typedef FILE XwFile;
#define File_RawOpen fopen
#define File_RawOpenText fopen
#define File_RawReadTextLine fgets
#define File_RawReadLine fgets
#define File_RawHasError ferror
#define File_RawAtEnd feof
#define File_RawClose fclose
#define File_RawRemove remove
#define File_RawRead fread
#define File_RawWrite fwrite
#define File_RawGetChar fgetc
#define File_RawPutChar fputc
#define File_RawTell ftell
#define File_RawSeek fseek
#define File_RawLength(stream) _filelength((_fileno)(stream))
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* 0x41EDC0 */
int File_RemoveFromDataDirectory(const char* filename);

#ifdef __cplusplus
}
#endif
#endif

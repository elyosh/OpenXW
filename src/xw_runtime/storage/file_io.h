#ifndef XW_RUNTIME_FILE_IO_H
#define XW_RUNTIME_FILE_IO_H
#include <aeron/vfs.h>
#ifdef __cplusplus
extern "C" {
#endif

/* fgets semantics with DOS CRLF translation and control-Z end of text. */
char* XwFile_GetsText(char* buffer, int capacity, AeronFile* stream);
/* fgets on a binary stream: retain CR bytes and control-Z. */
char* XwFile_Gets(char* buffer, int capacity, AeronFile* stream);
/* CRT convention: zero on success, EOF on failure. */
int XwFile_Close(AeronFile* stream);

/* Return complete elements transferred, preserving partial-read byte consumption. */
size_t XwFile_Read(void* buffer, size_t elementSize, size_t elementCount, AeronFile* stream);
size_t XwFile_Write(const void* buffer, size_t elementSize, size_t elementCount, AeronFile* stream);

/* Returns an unsigned byte as an int, or EOF when no byte was read. */
int XwFile_Getc(AeronFile* stream);
/* Writes the low byte and returns it as an int, or EOF on failure. */
int XwFile_Putc(int character, AeronFile* stream);

/* Original signed 32-bit file positions; -1 denotes tell/seek failure. */
int32_t XwFile_Tell(AeronFile* stream);
int XwFile_Seek(AeronFile* stream, int32_t offset, int origin);
/* Signed 32-bit file length, or -1 on failure; preserves the current position. */
int32_t XwFile_Length(AeronFile* stream);

#ifdef __cplusplus
}
#endif
#endif

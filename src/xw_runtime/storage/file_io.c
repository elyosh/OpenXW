#include "xw_runtime/storage/file_io.h"
#include "xw_runtime/storage/storage.h"
#include <stdio.h>

char* XwFile_GetsText(char* buffer, int capacity, AeronFile* stream) {
	int index;
	if (capacity <= 0)
		return NULL;
	for (index = 0; index < capacity - 1; ++index) {
		int character = XwFile_Getc(stream);
		if (character == EOF)
			break;
		if (character == '\x1a') {
			XwFile_Seek(stream, -1, SEEK_CUR);
			break;
		}
		if (character == '\r') {
			int nextCharacter = XwFile_Getc(stream);
			if (nextCharacter == '\n')
				character = '\n';
			else if (nextCharacter != EOF)
				XwFile_Seek(stream, -1, SEEK_CUR);
		}
		buffer[index] = (char)character;
		if (character == '\n') {
			++index;
			break;
		}
	}
	if (index == 0 && capacity > 1)
		return NULL;
	buffer[index] = '\0';
	return buffer;
}

char* XwFile_Gets(char* buffer, int capacity, AeronFile* stream) {
	int index;
	if (capacity <= 0)
		return NULL;
	for (index = 0; index < capacity - 1; ++index) {
		int character = XwFile_Getc(stream);
		if (character == EOF)
			break;
		buffer[index] = (char)character;
		if (character == '\n') {
			++index;
			break;
		}
	}
	if (index == 0 && capacity > 1)
		return NULL;
	buffer[index] = '\0';
	return buffer;
}

size_t XwFile_Read(void* buffer, size_t elementSize, size_t elementCount, AeronFile* stream) {
	size_t bytesRead = 0;
	if (elementSize == 0 || elementCount == 0 || elementCount > SIZE_MAX / elementSize)
		return 0;
	AeronVfs_Read(stream, buffer, elementSize * elementCount, &bytesRead);
	return bytesRead / elementSize;
}

size_t XwFile_Write(const void* buffer, size_t elementSize, size_t elementCount, AeronFile* stream) {
	size_t bytesWritten = 0;
	if (elementSize == 0 || elementCount == 0 || elementCount > SIZE_MAX / elementSize)
		return 0;
	AeronVfs_Write(stream, buffer, elementSize * elementCount, &bytesWritten);
	return bytesWritten / elementSize;
}

int XwFile_Getc(AeronFile* stream) {
	unsigned char value;
	size_t count = 0;

	AeronVfs_Read(stream, &value, sizeof(value), &count);
	return count != 0 ? value : EOF;
}

int XwFile_Putc(int character, AeronFile* stream) {
	unsigned char value = (unsigned char)character;
	size_t count = 0;
	AeronVfs_Write(stream, &value, sizeof(value), &count);
	return count != 0 ? value : EOF;
}

int32_t XwFile_Tell(AeronFile* stream) {
	int64_t position = AeronVfs_Tell(stream);
	if (position < 0 || position > INT32_MAX) {
		return -1;
	}
	return (int32_t)position;
}

int XwFile_Seek(AeronFile* stream, int32_t offset, int origin) {
	int64_t base;
	if (origin == SEEK_SET)
		base = 0;
	else if (origin == SEEK_CUR)
		base = AeronVfs_Tell(stream);
	else if (origin == SEEK_END)
		base = AeronVfs_GetSize(stream);
	else
		return -1;
	if (base < 0 || base > INT32_MAX || base + offset < 0 || base + offset > INT32_MAX)
		return -1;
	return AeronVfs_Seek(stream, base + offset, SEEK_SET) ? 0 : -1;
}

int32_t XwFile_Length(AeronFile* stream) {
	int64_t size = AeronVfs_GetSize(stream);
	return size < 0 || size > INT32_MAX ? -1 : (int32_t)size;
}

int XwFile_Close(AeronFile* stream) {
	XwStorage_ForgetFile(stream);
	return stream ? (AeronVfs_Close(stream) ? 0 : EOF) : 0;
}

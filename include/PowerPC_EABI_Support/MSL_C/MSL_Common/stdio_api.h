#ifndef _STDIO_API_H
#define _STDIO_API_H

#include "types.h"
#include "ansi_files.h"

enum __ReadProcActions { __GetAChar, __UngetAChar, __TestForError };

enum __WReadProcActions { __GetAwChar, __UngetAwChar, __TestForwcsError };

// __fread
size_t __fwrite(const void* pPtr, size_t memb_size, size_t num_memb, FILE* pFile);
int __StringRead(void*, int, int);
// wint_t __wStringRead(void*, wint_t, int);

#endif // STDIO_API_H

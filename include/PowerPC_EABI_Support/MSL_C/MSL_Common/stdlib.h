#ifndef _STDLIB_H_
#define _STDLIB_H_

#include <stddef.h>
// #include <wchar.h>

#define RAND_MAX 32767

#ifdef __cplusplus
extern "C" {
#endif

void srand(unsigned int seed);
int rand(void);
void abort(void);
void exit(int status);
// size_t wcstombs(char* dest, const wchar_t* src, size_t max);

typedef int (*_compare_function)(const void*, const void*);
void qsort(void*, size_t, size_t, _compare_function);

#ifdef __cplusplus
}
#endif

#endif

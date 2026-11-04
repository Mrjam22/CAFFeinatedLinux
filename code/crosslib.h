#ifndef _WIN32
#include <string.h>

#include <wchar.h>

#ifndef RT_FONT
#define RT_FONT L"FONT"
#endif


#ifdef __cplusplus
#include <vector>
#include <string>
#include <iostream>
#include <filesystem>
#endif

#ifndef wmemcpy_s
#define wmemcpy_s(dest, dest_size, src, count) wmemcpy(dest, src, count)
#endif

#ifndef memcpy_s
#define memcpy_s(dest, dest_size, src, count) memcpy(dest, src, count)
#endif

#define ASSERT(fmt, ...) (printf("%s %s %d - "#fmt,__FILE__, __func__, __LINE__, ##__VA_ARGS__))
#define PRINT(fmt, ...) (printf(fmt, ##__VA_ARGS__))

#ifndef strncpy_s
#define strncpy_s(dest, dest_size, src, count) strncpy(dest, src, count)
#endif

#define MAX_PATH PATH_MAX

#ifndef strcpy_s
#define strcpy_s(dest, dest_size, src) strcpy(dest, src)
#endif
#endif


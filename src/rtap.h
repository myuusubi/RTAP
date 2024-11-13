#ifndef RTAP_H
#define RTAP_H

#ifdef _WIN32

#define RTAPEXPORT __declspec(dllexport)
#define RTAPCALL __stdcall

#else

#ifndef __has_attribute
#define __has_attribute(x) 0
#endif

#if (defined(__GNUC__) && ((__GNUC__ > 4) || (__GNUC__ == 4) && (__GNUC_MINOR__ > 2))) || __has_attribute(visibility)

#ifdef ARM
#define RTAPEXPORT __attribute__((externally_visible,visibility("default")))
#else
#define RTAPEXPORT __attribute__((visibility("default")))
#endif

#endif

#endif

#ifndef RTAPEXPORT
#define RTAPEXPORT
#endif

#ifndef RTAPCALL
#define RTAPCALL
#endif

#define RTAP_FLAG_16		(1)
#define RTAP_FLAG_STEREO	(1 << 1)
#define RTAP_FLAG_ADPCM		(1 << 2)

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline void* RTAP__memset(void* ptr, int value, size_t num)
{
	for (size_t i = 0; i < num; ++i) {
		((char*)ptr)[i] = value;
	}
	return ptr;
}

static inline void* RTAP__memcpy(void* dest, const void* src, size_t count)
{
	for (size_t i = 0; i < count; ++i) {
		((char*)dest)[i] = ((char const*)src)[i];
	}
	return dest;
}

#ifdef __cplusplus
}
#endif

#endif

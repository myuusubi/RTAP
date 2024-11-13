#ifndef RTAP_SPRING_H
#define RTAP_SPRING_H

#include <stdint.h>

#include "rtap.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _RTAPSpring {
	uint8_t* data;
	int32_t data_size;
	int32_t format;
	int32_t sample_rate;
	int32_t block_align;
} RTAPSpring;

RTAPEXPORT int32_t RTAPCALL RTAP__RTAPSpring__alloc_size();

RTAPEXPORT void RTAPCALL RTAP__RTAPSpring__init(RTAPSpring* _this, uint8_t* data_ptr, int32_t data_size, int32_t format, int32_t sample_rate, int32_t block_align);

RTAPEXPORT int32_t RTAPCALL RTAP__RTAPSpring__get_length(RTAPSpring* _this);

RTAPEXPORT double RTAPCALL RTAP__RTAPSpring__get_duration(RTAPSpring* _this);

#ifdef __cplusplus
}
#endif

#endif
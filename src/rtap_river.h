#ifndef RTAP_RIVER_H
#define RTAP_RIVER_H

#include <stdint.h>

#include "rtap.h"
#include "rtap_spring.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _RTAPRiver {
	RTAPSpring* spring;
	int32_t read_head;
	int32_t cache_size;
	int32_t l_data[5];
	int32_t r_data[5];
	uint8_t cache[8];
} RTAPRiver;

RTAPEXPORT int32_t RTAPCALL RTAP__RTAPRiver__alloc_size();

RTAPEXPORT void RTAPCALL RTAP__RTAPRiver__set_spring(RTAPRiver* _this, RTAPSpring* spring_ptr);

RTAPEXPORT void RTAPCALL RTAP__RTAPRiver__init(RTAPRiver* _this, RTAPSpring* spring_ptr);

RTAPEXPORT void RTAPCALL RTAP__RTAPRiver__reset(RTAPRiver* _this);

RTAPEXPORT int32_t RTAPCALL RTAP__RTAPRiver__read_into(RTAPRiver* _this, uint8_t* buffer_ptr, int32_t start_idx, int32_t length);

#ifdef __cplusplus
}
#endif

#endif
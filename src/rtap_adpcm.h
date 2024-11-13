#ifndef RTAP_ADPCM_H
#define RTAP_ADPCM_H

#include <stdint.h>

#include "rtap.h"
#include "rtap_river.h"

#ifdef __cplusplus
extern "C" {
#endif

void RTAP__RTAPRiver__read_adpcm(RTAPRiver* _this, uint8_t* buffer_ptr, int start_idx, int length);

#ifdef __cplusplus
}
#endif

#endif
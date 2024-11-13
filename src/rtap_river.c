#include "rtap_adpcm.h"
#include "rtap_river.h"

int32_t RTAP__RTAPRiver__alloc_size()
{
	return sizeof(RTAPRiver);
}

void RTAP__RTAPRiver__set_spring(RTAPRiver* _this, RTAPSpring* spring_ptr)
{
	RTAP__memset((char*)_this, 0, sizeof(RTAPRiver));

	_this->spring = spring_ptr;
	_this->read_head = 0;
}

void RTAP__RTAPRiver__init(RTAPRiver* _this, RTAPSpring* spring_ptr)
{
	RTAP__RTAPRiver__set_spring(_this, spring_ptr);
}

void RTAP__RTAPRiver__reset(RTAPRiver* _this)
{
	RTAP__RTAPRiver__set_spring(_this, (RTAPSpring*)0);
}

int32_t RTAP__RTAPRiver__read_into(RTAPRiver* _this, uint8_t* buffer_ptr, int32_t start_idx, int32_t length)
{
	if (_this == (RTAPRiver*)0) {
		return -1;
	}

	if (_this->spring == (RTAPSpring*)0) {
		return -1;
	}

	if (length <= 0) {
		return -1;
	}

	RTAPSpring* spring = _this->spring;

	if (spring->format & RTAP_FLAG_ADPCM) {
		RTAP__RTAPRiver__read_adpcm(_this, buffer_ptr, start_idx, length);
	} else {
		RTAP__memcpy(buffer_ptr, spring->data + start_idx, length);
	}

	return 0;
}

#include "rtap_spring.h"

int32_t RTAP__RTAPSpring__alloc_size()
{
	return sizeof(RTAPSpring);
}

void RTAP__RTAPSpring__init(RTAPSpring* _this, uint8_t* data_ptr, int32_t data_size, int32_t format, int32_t sample_rate, int32_t block_align)
{
	_this->data = data_ptr;
	_this->data_size = data_size;
	_this->format = format;
	_this->sample_rate = sample_rate;
	_this->block_align = block_align;
}

int32_t RTAP__RTAPSpring__get_length(RTAPSpring* _this)
{
	if (!(_this->format & RTAP_FLAG_ADPCM)) {
		return _this->data_size;
	}

	int32_t stereo = 0;
	if (_this->format & RTAP_FLAG_STEREO) {
		stereo = 1;
	}

	int32_t full_block_samples = (((_this->block_align >> stereo) - 7) << 1) + 2;
	int32_t partial_block_bytes = (_this->data_size % _this->block_align);
	int32_t partial_block_samples = 0;

	if (partial_block_bytes > 0) {
		partial_block_samples = (((partial_block_bytes >> stereo) - 7) << 1) + 2;
	}

	if (partial_block_samples < 2) {
		partial_block_samples = 0;
	}

	int32_t total_samples = ((_this->data_size / _this->block_align) * full_block_samples) + partial_block_samples;

	return total_samples * sizeof(int16_t) * (stereo + 1);
}

double RTAP__RTAPSpring__get_duration(RTAPSpring* _this)
{
	double divisor = _this->sample_rate;

	if (_this->format & RTAP_FLAG_16) {
		divisor *= 2.0;
	}

	if (_this->format & RTAP_FLAG_STEREO) {
		divisor *= 2.0;
	}

	if (divisor <= 0.00001) {
		return 0.0;
	}

	return ((double)(_this->data_size)) / divisor;
}
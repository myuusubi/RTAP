#include "rtap_adpcm.h"

static int const ADAPTATION_TABLE[16] = {
	230, 230, 230, 230, 307, 409, 512, 614,
	768, 614, 512, 409, 307, 230, 230, 230
};

static int const ADAPTATION_COEFF1[7] = {
	256, 512, 0, 192, 240, 460, 392
};

static int const ADAPTATION_COEFF2[7] = {
	0, -256, 0, 64, 0, -208, -232
};

#define COEFF1   0
#define COEFF2   1
#define DELTA    2
#define SAMPLE1  3
#define SAMPLE2  4

static void adpcm_write_header_samples(uint8_t* dest, int32_t* channels, int STEREO)
{
	if (dest == (uint8_t*)0) {
		return;
	}

	for (int i = 0; i < 2; ++i) {
		for (int c = 0; c <= STEREO; ++c) {
			int base_idx = (i * 2 * (STEREO + 1)) + (c * 2);
			int32_t s = (channels + (c * 5))[4 - i];
			dest[base_idx] = (uint8_t)(s & 0xFF);
			dest[base_idx + 1] = (uint8_t)((s >> 8) & 0xFF);
		}
	}
}

static int adpcm_expand_nibble(int32_t* channel, int32_t nibble)
{
	int32_t nibbleSign = nibble;
	if (nibble & 0x08) {
		nibbleSign -= 0x10;
	}

	int32_t predictor = ((channel[SAMPLE1] * channel[COEFF1]) + (channel[SAMPLE2] * channel[COEFF2])) / 256 + (nibbleSign * channel[DELTA]);

	if (predictor < -0x8000) {
		predictor = -0x8000;
	} else if (predictor > 0x7FFF) {
		predictor = 0x7FFF;
	}

	channel[SAMPLE2] = channel[SAMPLE1];
	channel[SAMPLE1] = predictor;

	int32_t delta = (ADAPTATION_TABLE[nibble] * channel[DELTA]) / 256;
	if (delta < 16) {
		delta = 16;
	}

	channel[DELTA] = delta;

	return predictor;
}

static void adpcm_write_payload_samples(uint8_t* dest, int32_t dest_offset, int32_t* channels, int32_t nibbles, int32_t STEREO)
{
	int32_t sample1 = adpcm_expand_nibble(channels, (nibbles >> 4) & 0x0F);
	int32_t sample2 = adpcm_expand_nibble(channels + (STEREO * 5), nibbles & 0x0F);

	if (dest == (uint8_t*)0) {
		return;
	}

	dest += dest_offset;
	dest[0] = (uint8_t)(sample1 & 0xFF);
	dest[1] = (uint8_t)((sample1 >> 8) & 0xFF);
	dest[2] = (uint8_t)(sample2 & 0xFF);
	dest[3] = (uint8_t)((sample2 >> 8) & 0xFF);
}

static int rtap_river_read_adpcm_helper(RTAPRiver* _this, uint8_t* dest, int32_t dest_size)
{
	RTAPSpring* spring = _this->spring;

	int STEREO = 0;
	if (spring->format & RTAP_FLAG_STEREO) {
		STEREO = 1;
	}

	int HEADER_SIZE = (STEREO + 1) + (((STEREO + 1) << 1) * 3);
	int HEADER_DBYTES = (STEREO + 1) << 2;

	int32_t channels[10];

	RTAP__memcpy(channels, _this->l_data, sizeof(int32_t) * 5);
	if (STEREO) {
		RTAP__memcpy(channels + 5, _this->r_data, sizeof(int32_t) * 5);
	}

	uint8_t cache[8];
	int32_t cache_size = 0;

	uint8_t* buffer = spring->data;
	int32_t block_align = spring->block_align;

	int32_t read_head = _this->read_head;
	int32_t data_size = spring->data_size;

	while (read_head < data_size && dest_size > 0) {
		int32_t block_size = block_align;

		do
		{
			int32_t read_block = read_head / block_align;
			int32_t remaining = data_size - (read_block * block_align);
			if (remaining < block_size) {
				block_size = remaining;
			}
			if (((block_size >> STEREO) - 7) < 0) {
				goto WRITE_TO_RIVER;
			}
		} while(0);

		int32_t block_offset = read_head % block_align;

		if (block_offset == 0) {
			for (int c = 0; c <= STEREO; ++c) {
				int32_t block_predictor = buffer[read_head + c];
				if (block_predictor > 6) {
					block_predictor = 6;
				}
				int32_t* channel = channels + (c * 5);
				channel[COEFF1] = ADAPTATION_COEFF1[block_predictor];
				channel[COEFF2] = ADAPTATION_COEFF2[block_predictor];
			}

			for (int i = 0; i < 3; ++i) {
				for (int c = 0; c <= STEREO; ++c) {
					int32_t base_idx = read_head + (STEREO + 1) + (((STEREO + 1) << 1) * i) + (c << 1);
					int32_t s = buffer[base_idx];
					s |= buffer[base_idx + 1] << 8;
					if (s & 0x8000) {
						s -= 0x10000;
					}
					(channels + (c * 5))[i + 2] = s;
				}
			}

			read_head += HEADER_SIZE;
			block_offset = HEADER_SIZE;

			if (dest_size >= HEADER_DBYTES) {
				adpcm_write_header_samples(dest, channels, STEREO);
				if (dest != (uint8_t*)0) {
					dest += HEADER_DBYTES;
				}
				dest_size -= HEADER_DBYTES;
			}
			else
			{
				adpcm_write_header_samples(cache, channels, STEREO);
				cache_size = HEADER_DBYTES;
				goto WRITE_TO_RIVER;
			}
		}

		block_size -= block_offset;

		int32_t iterations = dest_size >> 2;
		if (iterations > block_size) {
			iterations = block_size;
		}

		for (int i = 0; i < iterations; ++i) {
			adpcm_write_payload_samples(dest, (i << 2), channels, buffer[read_head + i], STEREO);
		}

		read_head += iterations;
		if (dest != (uint8_t*)0) {
			dest += (iterations << 2);
		}
		dest_size -= (iterations << 2);

		if (iterations < block_size && (dest_size & 0x3) != 0) {
			adpcm_write_payload_samples(cache, 0, channels, buffer[read_head], STEREO);
			cache_size = 4;
			++read_head;
			goto WRITE_TO_RIVER;
		}
	}

WRITE_TO_RIVER:
	_this->read_head = read_head;
	_this->cache_size = cache_size;
	RTAP__memcpy(_this->l_data, channels, sizeof(int32_t) * 5);
	if (STEREO) {
		RTAP__memcpy(_this->r_data, channels + 5, sizeof(int32_t) * 5);
	}
	RTAP__memcpy(_this->cache, cache, sizeof(uint8_t) * 8);

	return dest_size;
}

void RTAP__RTAPRiver__read_adpcm(RTAPRiver* _this, uint8_t* buffer_ptr, int start_idx, int length)
{
	RTAPSpring* spring = _this->spring;
	uint8_t* dest = buffer_ptr;
	uint8_t* river_cache = _this->cache;

	int32_t dest_size = length;

	int32_t stereo = 0;
	if (spring->format & RTAP_FLAG_STEREO) {
		stereo = 1;
	}
	int32_t read_head = _this->read_head;
	int32_t block_align = spring->block_align;
	int32_t data_size = spring->data_size;

	int32_t samples_per_block = (((block_align >> stereo) - 7) << 1) + 2;
	int32_t dbyte_per_block = (samples_per_block << stereo) * sizeof(int16_t);

	int32_t request_block = start_idx / dbyte_per_block;
	int32_t request_offset = start_idx - (request_block * dbyte_per_block);

	int32_t cache_size;

	_this->read_head = request_block * block_align;
	if (request_offset != 0) {
		// TODO: Add optimization when requesting the same block that we're currently in

		_this->read_head = request_block * block_align;
		int remaining = rtap_river_read_adpcm_helper(_this, (uint8_t*)0, request_offset);
		read_head = _this->read_head;

		cache_size = _this->cache_size;
		int32_t current_block = read_head / block_align;
		int32_t current_offset = read_head - (current_block * block_align);
		int32_t current_offset_samples = ((current_offset - (7 << stereo)) << 1) + (2 << stereo);
		if (current_offset_samples < 0) {
			current_offset_samples = 0;
		}
		int32_t current_offset_dbytes = current_offset_samples * sizeof(int16_t);
		int32_t read_head_dbytes = (current_block * dbyte_per_block) + current_offset_dbytes;
		int32_t cache_request = read_head_dbytes - start_idx;
		if (cache_request > 0 && cache_request <= cache_size) {
			uint8_t* cache = river_cache + (cache_size - cache_request);
			RTAP__memcpy(dest, cache, sizeof(uint8_t) * cache_request);
			dest += cache_request;
			dest_size -= cache_request;
		}
	}

	if (dest_size <= 0) {
		return;
	}

	dest_size = rtap_river_read_adpcm_helper(_this, dest, dest_size);
	if (dest_size <= 0) {
		return;
	}

	cache_size = _this->cache_size;
	if (cache_size > 0) {
		int32_t cache_request = dest_size;
		if (cache_request > cache_size) {
			cache_request = cache_size;
		}

		uint8_t* cache = river_cache;
		RTAP__memcpy(dest, cache, sizeof(uint8_t) * cache_request);
		dest += cache_request;
		dest_size -= cache_request;
	}

	RTAP__memset(dest, 0, sizeof(uint8_t) * dest_size);
}

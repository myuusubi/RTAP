#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rtap_river.h"
#include "rtap_spring.h"

#define BUFFER_SIZE (1024 * 16)

int main()
{
	FILE* file = fopen("packed.adpcm", "rb");

	fseek(file, 0, SEEK_END);
	long int file_size = ftell(file);
	rewind(file);

	uint8_t* data = (uint8_t*)malloc(sizeof(uint8_t) * file_size);

	size_t read_size = fread(data, sizeof(char), file_size, file);

	if (read_size != file_size) {
		free(data);
		return -1;
	}

	size_t read_head = 0;
	while (read_head < read_size) {
		int32_t codec;
		int32_t channels;
		int32_t rate;
		int32_t alignment;
		int32_t bits;
		int32_t length;

		memcpy(&codec, data + read_head, sizeof(int32_t));
		memcpy(&channels, data + read_head + 4, sizeof(int32_t));
		memcpy(&rate, data + read_head + 8, sizeof(int32_t));
		memcpy(&alignment, data + read_head + 12, sizeof(int32_t));
		memcpy(&bits, data + read_head + 16, sizeof(int32_t));
		memcpy(&length, data + read_head + 20, sizeof(int32_t));

		alignment = (alignment + 22) * channels;

		int times_played = 0;
		int bindex = 0;

		uint8_t* buffers[4];
		buffers[0] = (uint8_t*)malloc(sizeof(uint8_t) * BUFFER_SIZE);
		buffers[1] = (uint8_t*)malloc(sizeof(uint8_t) * BUFFER_SIZE);
		buffers[2] = (uint8_t*)malloc(sizeof(uint8_t) * BUFFER_SIZE);
		buffers[3] = (uint8_t*)malloc(sizeof(uint8_t) * BUFFER_SIZE);

		uint8_t* spring_buffer = (uint8_t*)malloc(length);
		memcpy(spring_buffer, data + read_head + 24, length);

		int32_t format = RTAP_FLAG_ADPCM;
		if (channels == 2) {
			format |= RTAP_FLAG_STEREO;
		}
		if (bits == 16) {
			format |= RTAP_FLAG_16;
		}

		RTAPSpring* spring = (RTAPSpring*)malloc(RTAP__RTAPSpring__alloc_size());
		RTAP__RTAPSpring__init(spring, spring_buffer, length, format, rate, alignment);

		RTAPRiver* river = (RTAPRiver*)malloc(RTAP__RTAPRiver__alloc_size());
		RTAP__RTAPRiver__reset(river);
		RTAP__RTAPRiver__set_spring(river, spring);

		int32_t springLength = RTAP__RTAPSpring__get_length(spring);

		uint8_t* wrap_buffer = (uint8_t*)malloc(springLength % BUFFER_SIZE);
		size_t buffer_head;

RESTART:
		buffer_head = 0;
		while (buffer_head < springLength) {
			uint8_t* buffer = wrap_buffer;

			int32_t read = springLength - buffer_head;
			if (read >= BUFFER_SIZE) {
				read = BUFFER_SIZE;
				buffer = buffers[bindex];
				bindex = (bindex + 1) % 4;
			}

			RTAP__RTAPRiver__read_into(river, buffer, buffer_head, read);

			buffer_head += read;

			if (buffer_head >= springLength) {
				goto DONE;
			}
		}

DONE:
		if ((times_played++) < 3) {
			goto RESTART;
		}

		free(wrap_buffer);

		free(river);

		free(spring);

		free(spring_buffer);

		free(buffers[0]);
		free(buffers[1]);
		free(buffers[2]);
		free(buffers[3]);

		read_head += length + 24;
	}

	fclose(file);

	free(data);

	printf("%li\n", file_size);
	return 0;
}

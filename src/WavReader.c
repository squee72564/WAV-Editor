#include "WavReader.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

// Union to hold sample bytes and interpret "dynamically"
union SampleUnion {
	int8_t  i8;
	int16_t i16;
	int32_t i24;
	int32_t i32;
};

void WAV_init(
		struct WAV_file *wav,
		const uint16_t num_channels,
		const uint32_t sample_rate,
		const uint16_t bits_per_sample)
{

	*wav = (struct WAV_file) {
		
		.riff = (struct RIFF_chunk) {
			.id   = {'R','I','F','F'},
			.size = 36, // We have no sound data so far
			.format     = {'W','A','V','E'},
		},

		.fmt = (struct FMT_chunk) {
			.id    = {'f', 'm', 't', ' '},
			.size	 = 16,	// assuming PCM
			.audio_format    = 1,	// assuming PCM
			.num_channels    = num_channels,
			.sample_rate	 = sample_rate,
			.byte_rate       = sample_rate * num_channels * (bits_per_sample / 8),
			.block_align     = num_channels * (bits_per_sample / 8),
			.bits_per_sample = bits_per_sample,
		},

		.data = (struct DATA_chunk) {
			.id   = {'d', 'a', 't', 'a'},
			.size = 0,
			.buff = NULL,
		},

		.extra = NULL,

	};
}

void WAV_print(struct WAV_file *wav)
{
	printf("RIFF_CHUNK:\n");
	printf("-- id: RIFF\n");
	printf("-- size: %d\n", wav->riff.size);
	printf("-- format: WAVE\n");

	printf("FMT_CHUNK:\n");
	printf("-- id: fmt \n");
	printf("-- size: %u\n", wav->fmt.size);
	printf("-- audio_format: %hu\n", wav->fmt.audio_format);
	printf("-- num_channels: %hu\n", wav->fmt.num_channels);
	printf("-- sample_rate: %hu\n", wav->fmt.sample_rate);
	printf("-- byte_rate: %hu\n", wav->fmt.byte_rate);
	printf("-- block_align: %hu\n", wav->fmt.block_align);
	printf("-- bits_per_sample: %hu\n", wav->fmt.bits_per_sample);

	printf("DATA_CHUNK\n");
	printf("-- id: data\n");
	printf("-- size: %d\n", wav->data.size);

	struct EXTRA_chunk *extra = wav->extra;

	unsigned char buff[5] = {0};	
	buff[4] = '\0';

	while (extra != NULL) {

		memcpy(buff, extra->id, sizeof(extra->id));

		printf("EXTRA_CHUNK:\n");
		printf("-- id: %s\n", buff);
		printf("-- size: %d\n", extra->size);

		extra = extra->next;
	}
}

void WAV_print_metadata(struct WAV_file *wav)
{
	int found = 0;

	struct EXTRA_chunk *metadata_chunk = wav->extra;

	while (metadata_chunk != NULL) {
		if (memcmp(metadata_chunk->id,   "LIST", 4) == 0 &&
		    memcmp(metadata_chunk->buff, "INFO", 4) == 0) {
			found = 1;
			break;
		}

		metadata_chunk = metadata_chunk->next;
	}

	if (found == 0) {
		perror("No existing metadata chunk found\n");
		return;
	}

	uint32_t byte_pos = 4;

	uint32_t info_id_sz = 0;
	unsigned char buff[5] = {0};
	buff[4] = '\0';

	while (byte_pos < metadata_chunk->size) {
		memcpy(&buff, &metadata_chunk->buff[byte_pos], 4);
		printf("%s\n", buff);
		byte_pos += 4;

		memcpy(&info_id_sz, &metadata_chunk->buff[byte_pos], 4);
		byte_pos += 4;

		if (info_id_sz % 2 == 1) info_id_sz++;	// make even

		printf("-- data (size %d):\n\t", info_id_sz);

		// Right now skip stuff like Traktors proprietary data
		if (memcmp(&buff, "NITR", 4) == 0) {
			byte_pos += info_id_sz;
			continue;
		}

		for (int i = 0; i < info_id_sz; ++i) {
			putchar(metadata_chunk->buff[byte_pos]);
			byte_pos++;
		}

		printf("\n");
	}

	printf("\n\n");
}

uint64_t WAV_get_max_amp(struct WAV_file *wav)
{
	const uint32_t bytes_per_sample = (wav->fmt.bits_per_sample / 8);

	int64_t max_amp = 0;

	union SampleUnion sample;
	sample.i32 = 0;

	for (int i = 0; i < ( wav->data.size / bytes_per_sample); ++i) {
		memcpy(&sample, &wav->data.buff[i*bytes_per_sample], bytes_per_sample);

		int64_t t = 0;
		switch (bytes_per_sample) {
			case 1:
				t = sample.i8;
				break;
			case 2:
				t = sample.i16;
				break;
			case 3:
				t = sample.i24;
				break;
			case 4:
				t = sample.i32;
				break;
		}

		t = abs(t);
		if (t > max_amp) max_amp = t;
	}

	return max_amp;
}

double WAV_get_max_db(struct WAV_file *wav)
{
	if (wav == NULL) {
		perror("Error: Cannot get max Db; wav is NULL.\n");
		return -999.0f;
	} else if (wav->data.buff == NULL) {
		perror("Error: Cannot get max Db; wav music data is NULL.\n");
		return -999.0f;
	}
	
	const int64_t max_amp = WAV_get_max_amp(wav);

	return 20.0f * log10f((double)max_amp / (double)((1 << (wav->fmt.bits_per_sample-1))-1));
}

WAV_State WAV_normalize_max_db(struct WAV_file *wav, double db)
{
	if (wav == NULL) return Error;
	if (wav->data.buff == NULL || wav->data.size == 0) return Error;
	if (db > 0.0f) db = 0.0f;

	const uint64_t max_amp = WAV_get_max_amp(wav);
	const int16_t new_max_amp = pow(10, db / 20.0) * (pow(2, wav->fmt.bits_per_sample - 1) - 1);
	
	const uint32_t bytes_per_sample = (wav->fmt.bits_per_sample / 8);

	union SampleUnion sample;
	sample.i32 = 0;

	for (int i = 0; i < ( wav->data.size / bytes_per_sample); ++i) {
		memcpy(&sample, &wav->data.buff[i*bytes_per_sample], bytes_per_sample);

		int64_t t = 0;
		switch (bytes_per_sample) {
			case 1:
				t = sample.i8;
				break;
			case 2:
				t = sample.i16;
				break;
			case 3:
				t = sample.i24;
				break;
			case 4:
				t = sample.i32;
				break;
		}

		const double p = (double)t / (double)max_amp;
		t = new_max_amp * p;
		
		memcpy(&wav->data.buff[i*bytes_per_sample], &t, bytes_per_sample);
	}
	
	return Success;
}

WAV_State WAV_write_sin_wave(
        struct WAV_file *wav,
        const double freq,
        const uint32_t duration,
	float db)
{
	if (wav == NULL) {
		return Error;
	} else if (wav->data.buff != NULL || wav->data.size != 0) {
		free(wav->data.buff);
		wav->data.buff = NULL;
		wav->data.size = 0;
		wav->riff.size = 36;
	}

	if (db > 0.0f) db = 0.0f;

	const uint32_t size =
		(wav->fmt.bits_per_sample / 8) 
		* wav->fmt.num_channels
		* wav->fmt.sample_rate
		* duration;

	wav->data.size = size;
	wav->data.buff = (unsigned char*)malloc(sizeof(unsigned char) * size);
	
	if (wav->data.buff == NULL) return Error;

	wav->riff.size = 36 + size;

	// -6Db amplitude by default
	const double amp = pow(10, db / 20.0) * (pow(2, wav->fmt.bits_per_sample - 1) - 1);
	const double sample_period = 1.0 / wav->fmt.sample_rate;
	const double ang_freq = 2.0 * M_PI * freq;
	const uint16_t bytes_per_sample = (wav->fmt.bits_per_sample / 8);

    // Loop for the number of samples
    for (uint32_t sample_index = 0;
         sample_index < wav->data.size /  (wav->fmt.num_channels * bytes_per_sample);
         sample_index++) {

        const double t = (double)sample_index * sample_period;
        const int16_t sample = (int16_t)(amp * sin(ang_freq * t));

        // Loop for each channel
        for (uint16_t channel = 0; channel < wav->fmt.num_channels; ++channel) {
            const uint32_t channel_index = sample_index * wav->fmt.num_channels + channel;

            // Calculate the byte offsets based on the bit depth and channel
            const uint32_t byte_offset = channel_index * bytes_per_sample;

            // Write the sample bytes to the buffer based on the bit depth
            for (uint16_t byte = 0; byte < bytes_per_sample; ++byte) {
                wav->data.buff[byte_offset + byte] = (sample >> (byte * 8)) & 0xFF;
            }
        }
    }

    return Success;
}

WAV_State WAV_write_binaural_wave(
        struct WAV_file *wav,
        const double freq1,
        const double freq2,
        const uint32_t duration,
	float db)
{
	if (wav == NULL) {
		return Error;
	} else if (wav->data.buff != NULL || wav->data.size != 0) {
		free(wav->data.buff);
		wav->data.buff = NULL;
		wav->data.size = 0;
		wav->riff.size = 36;
	}

	if (db > 0.0f) db = 0.0f;

	const uint32_t size =
		(wav->fmt.bits_per_sample / 8) 
		* wav->fmt.num_channels
		* wav->fmt.sample_rate
		* duration;

	wav->data.size = size;
	wav->data.buff = (unsigned char*)malloc(sizeof(unsigned char) * size);
	
	if (wav->data.buff == NULL) return Error;

	wav->riff.size = 36 + size;

	// -6Db amplitude by default
	const double amp = pow(10, db / 20.0) * (pow(2, wav->fmt.bits_per_sample - 1) - 1);
	const double sample_period = 1.0 / wav->fmt.sample_rate;
	const double ang_freq1 = 2.0 * M_PI * freq1;
	const double ang_freq2 = 2.0 * M_PI * freq2;
	const uint16_t bytes_per_sample = (wav->fmt.bits_per_sample / 8);

    // Loop for the number of samples
    for (uint32_t sample_index = 0;
         sample_index < wav->data.size /  (wav->fmt.num_channels * bytes_per_sample);
         sample_index++) {

        const double t = (double)sample_index * sample_period;
        const int16_t sample1 = (int16_t)(amp * sin(ang_freq1 * t));
        const int16_t sample2 = (int16_t)(amp * sin(ang_freq2 * t));

        // Loop for each channel
        for (uint16_t channel = 0; channel < wav->fmt.num_channels; ++channel) {
            const uint32_t channel_index = sample_index * wav->fmt.num_channels + channel;

            // Calculate the byte offsets based on the bit depth and channel
            const uint32_t byte_offset = channel_index * bytes_per_sample;

            // Write the sample bytes to the buffer based on the bit depth
            for (uint16_t byte = 0; byte < bytes_per_sample; ++byte) {
                if (channel % 2 == 0) {
                    wav->data.buff[byte_offset + byte] = (sample1 >> (byte * 8)) & 0xFF;
                } else {
                    wav->data.buff[byte_offset + byte] = (sample2 >> (byte * 8)) & 0xFF;
                }
            }
        }
    }

    return Success;
}

static WAV_State read_RIFF_chunk(struct WAV_file *wav, FILE *file, unsigned char* id)
{
	memcpy(wav->riff.id, id, sizeof(wav->riff.id));
	fread(&wav->riff.size, sizeof(wav->riff.size), 1, file);
	fread(&wav->riff.format, sizeof(wav->riff.format), 1, file);

	if (memcmp(&wav->riff.format, "WAVE", sizeof(wav->riff.format) != 0)) {
		fclose(file);
		return Error;
	}

	return Success;
	
}

static WAV_State read_FMT_chunk(struct WAV_file *wav, FILE *file)
{
	memcpy(wav->fmt.id, "fmt ", sizeof(wav->fmt.id));
	fread(&wav->fmt.size, sizeof(wav->fmt.size), 1, file);
	fread(&wav->fmt.audio_format, sizeof(wav->fmt.audio_format), 1, file);
	fread(&wav->fmt.num_channels, sizeof(wav->fmt.num_channels), 1, file);
	fread(&wav->fmt.sample_rate, sizeof(wav->fmt.sample_rate), 1, file);
	fread(&wav->fmt.byte_rate, sizeof(wav->fmt.byte_rate), 1, file);
	fread(&wav->fmt.block_align, sizeof(wav->fmt.block_align), 1, file);
	fread(&wav->fmt.bits_per_sample, sizeof(wav->fmt.bits_per_sample), 1, file);

	return Success;
}

static WAV_State read_DATA_chunk(struct WAV_file *wav, FILE *file)
{
	memcpy(wav->data.id, "data", sizeof(wav->data.id));
	fread(&wav->data.size, sizeof(wav->data.size), 1, file);

	wav->data.buff = (unsigned char*)malloc(sizeof(unsigned char) * wav->data.size);

	if (wav->data.buff == NULL ) {
		perror("Could not alloc wav data buffer.\n");
		fclose(file);
		return Error;
	}

	if (fread(wav->data.buff, wav->data.size, 1, file) != 1) {
		perror("Could not write to wav data buffer.\n");
		fclose(file);
		free(wav->data.buff);	
		return Error;
	}
	
	return Success;
}

static WAV_State read_EXTRA_chunk(struct WAV_file *wav, FILE *file, unsigned char* chunk_id)
{

	// Copy data into new EXTRA_chunk struct to be appended to list of EXTRA chunks
	struct EXTRA_chunk *extra = (struct EXTRA_chunk*)malloc(sizeof(struct EXTRA_chunk));

	if (extra == NULL) {
		return Error;
	}

	memcpy(extra->id, chunk_id, sizeof(extra->id));

	if (fread(&extra->size, sizeof(extra->size), 1, file) != 1) {
		free(extra);
		return Error;
	}

	extra->buff = (unsigned char*)malloc(extra->size);

	if (extra->buff == NULL) {
		free(extra);
		return Error;
	}

	if (fread(extra->buff, extra->size, 1, file) != 1) {
		free(extra->buff);
		free(extra);
		return Error;
	}

	extra->next = NULL;

	// Set new EXTRA_chunk to last node in list
	
	if (wav->extra == NULL) {
		wav->extra = extra;
		return Success;
	}

	struct EXTRA_chunk *curr = wav->extra;
	while (curr->next != NULL) {
		curr = curr->next;
	}

	curr->next = extra;

	return Success;
}

WAV_State WAV_read_file(struct WAV_file *wav, const char *file_name)
{
	if (file_name == NULL) return Error;

	FILE *file = fopen(file_name, "rb");

	if (file == NULL) {
		perror("Failed to open file for read.\n");
		return Error;
	}

	while (!feof(file) && !ferror(file) ) {
		unsigned char id[4] = {0};
		
		// Read the 4 bytes for the id of the next chunk
		if ( fread(&id, sizeof(id), 1, file) != 1 ) break;

		if (memcmp(id, "RIFF", sizeof(id)) == 0 ||
		    memcmp(id, "RIFX", sizeof(id)) == 0) {
			if (!read_RIFF_chunk(wav, file, id)) break;
		}
		else if (memcmp(id, "fmt ", sizeof(id)) == 0) {
			if (!read_FMT_chunk(wav, file)) break;
		}
		else if (memcmp(id, "data", sizeof(id)) == 0) {
			if (!read_DATA_chunk(wav, file)) break;
		}
		else {
			if (!read_EXTRA_chunk(wav, file, id)) break;
		}
	}

	if (ferror(file)) {
		perror("I/O error when parsing WAV file.\n");
		fclose(file);
		return Error;
	} else if (!feof(file)) {
		perror("Did not parse entire WAV file.\n");
		fclose(file);
		return Error;
	}

	fclose(file);
	
	return Success;
}

WAV_State WAV_write_to_file(
        struct WAV_file* wav,
        const char* file_name)
{
	if (wav == NULL) return Error;
	if (file_name == NULL) return Error;

	FILE* file = fopen(file_name, "wb");

	if (file == NULL) {
		perror("File opening failed\n");
		fclose(file);
		return Error;
	}

	size_t riff_ret = fwrite(
			&wav->riff,
			sizeof(wav->riff),
			1,
			file
		);

	if (riff_ret != 1) {
		perror("Failed to write RIFF chunk\n");
		fclose(file);
		return Error;
	}

	size_t fmt_ret = fwrite(
			&wav->fmt,
			sizeof(wav->fmt),
			1,
			file
		);

	if (fmt_ret != 1) {
		perror("Failed to write FMT chunk\n");
		fclose(file);
		return Error;
	}

	size_t data_ret = fwrite(
			&wav->data,
			sizeof(wav->data) - sizeof(wav->data.buff),
			1,
			file
		);
	
	if (data_ret != 1) {
		perror("Failed to write DATA chunk\n");
		fclose(file);
		return Error;
	}

	size_t sound_data_ret = fwrite(
			wav->data.buff,
			sizeof(unsigned char),
			wav->data.size,
			file
		);

	if (sound_data_ret != wav->data.size) {
		perror("Failed to write sound data\n");
		return Error;
	}
	
	struct EXTRA_chunk *sentinel = wav->extra;

	while (sentinel != NULL) {	
		size_t extra_ret = fwrite(
				&sentinel,
				sizeof(sentinel)
					- sizeof(sentinel->buff)
					- sizeof(sentinel->next),
				1,
				file
			);

		if (extra_ret != 1) {
			perror("Failed to write an EXTRA chunk\n");
			fclose(file);
			return Error;
		}

		size_t extra_data_ret = fwrite(
				sentinel->buff,
				sizeof(unsigned char),
				sentinel->size,
				file
			);

		if (extra_data_ret != sentinel->size) {
			perror("Failed to write the data of an EXTRA chunk\n");
			fclose(file);
			return Error;
		}

		sentinel = sentinel->next;
	}


	fclose(file);

	return Success;
}

void WAV_free(struct WAV_file *wav) {
    if (wav == NULL) return;

    if (wav->data.buff != NULL) {
	free(wav->data.buff);
	wav->riff.size -= wav->data.size;
	wav->data.size = 0;
    }

    while (wav->extra != NULL) {
	if (wav->extra->buff != NULL) {
		free(wav->extra->buff);
		wav->extra->buff = NULL;
	}
 
	struct EXTRA_chunk *t = wav->extra;
	wav->extra = wav->extra->next;
	free(t);
    }
}

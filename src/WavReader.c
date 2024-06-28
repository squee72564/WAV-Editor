#include "WavReader.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

struct WAV_file alloc_WAV_file(
		uint16_t num_channels,
		uint32_t sample_rate,
		uint16_t bits_per_sample)
{

	const struct WAV_file wav = (struct WAV_file) {
		
		.riff = (struct RIFF_chunk) {
			.chunk_id   = {'R','I','F','F'},
			.chunk_size = 36, // We have no sound data so far
			.format     = {'W','A','V','E'},
		},

		.fmt = (struct FMT_chunk) {
			.subchunk1_id    = {'f', 'm', 't', ' '},
			.subchunk1_size	 = 16,	// assuming PCM
			.audio_format    = 1,	// assuming PCM
			.num_channels    = num_channels,
			.sample_rate	 = sample_rate,
			.byte_rate       = sample_rate * num_channels * (bits_per_sample / 8),
			.block_align     = num_channels * (bits_per_sample / 8),
			.bits_per_sample = bits_per_sample,
		},

		.data = (struct DATA_chunk) {
			.subchunk2_id   = {'d', 'a', 't', 'a'},
			.subchunk2_size = 0,
			.buff = NULL,
		},

	};

	return wav;
}

struct WAV_file* read_WAV_file(char *file_name)
{
	if (file_name == NULL) return NULL;

	FILE *file = fopen(file_name, "r");

	if (file == NULL) {
		perror("Failed to open file for read.\n");
		fclose(file);
		return NULL;
	}

	struct WAV_file wav;

	printf("RIFF_CHUNK:\n");

	fread(&wav.riff.chunk_id, sizeof(wav.riff.chunk_id), 1, file);

	if (memcmp(&wav.riff.chunk_id, "RIFF", sizeof(wav.riff.chunk_id) != 0) ||
	    memcmp(&wav.riff.chunk_id, "RIFX", sizeof(wav.riff.chunk_id) != 0)) {
		fclose(file);
		return NULL;
	}

	printf("-- chunk_id: RIFF\n");

	fread(&wav.riff.chunk_size, sizeof(wav.riff.chunk_size), 1, file);
	
	printf("-- chunk_size: %d\n", wav.riff.chunk_size);

	fread(&wav.riff.format, sizeof(wav.riff.format), 1, file);

	if (memcmp(&wav.riff.format, "WAVE", sizeof(wav.riff.format) != 0)) {
		fclose(file);
		return NULL;
	}

	printf("-- format: WAVE\n");

	printf("FMT_CHUNK:\n");

	fread(&wav.fmt.subchunk1_id, sizeof(wav.fmt.subchunk1_id), 1, file);

	if (memcmp(&wav.fmt.subchunk1_id, "fmt ", sizeof(wav.fmt.subchunk1_id)) != 0) {
		fclose(file);
		return NULL;
	}

	printf("-- subchunk1_id: fmt \n");

	fread(&wav.fmt.subchunk1_size, sizeof(wav.fmt.subchunk1_size), 1, file);
	printf("-- subchunk1_size: %u\n");

	fread(&wav.fmt.audio_format, sizeof(wav.fmt.audio_format), 1, file);
	printf("-- audio_format: %hu\n");

	fread(&wav.fmt.num_channels, sizeof(wav.fmt.num_channels), 1, file);
	printf("-- num_channels: %hu\n");

	fread(&wav.fmt.sample_rate, sizeof(wav.fmt.sample_rate), 1, file);
	printf("-- sample_rate: %hu\n");

	fread(&wav.fmt.byte_rate, sizeof(wav.fmt.byte_rate), 1, file);
	printf("-- byte_rate: %hu\n");

	fread(&wav.fmt.block_align, sizeof(wav.fmt.block_align), 1, file);
	printf("-- block_align: %hu\n");

	fread(&wav.fmt.bits_per_sample, sizeof(wav.fmt.bits_per_sample), 1, file);
	printf("-- bits_per_sample: %hu\n");

	printf("DATA_CHUNK\n");

	fread(&wav.data.subchunk2_id, sizeof(wav.data.subchunk2_id), 1, file);

	printf("-- subchunk2_id: data\n");

	fread(&wav.data.subchunk2_size, sizeof(wav.data.subchunk2_size), 1, file);

	printf("-- subchunk2_size: %d\n", wav.data.subchunk2_size);

	printf("SOUND_DATA:\n");
	for (int i = 0; i < wav.data.subchunk2_size; ++i) {
		fgetc(file);
	}

	printf("Data after sound data:\n");
	
	unsigned char buff[5] = {0};	
	buff[4] = '\0';
	unsigned char buff2[5] = {0};	
	buff2[4] = '\0';
	uint32_t sz = 0;

	const size_t a = fread(buff, 4, 1, file);
	const size_t b = fread(&sz, 4, 1, file);
	const size_t z = fread(buff2, 4, 1, file);

	printf("ID: %s\n", buff);
	printf("Size: %d\n", sz);
	printf("List type ID: %s\n\n", buff2);

	while (1) {
		uint32_t sz2 = 0;
		unsigned char buff3[5] = {0};
		buff3[4] = '\0';
		fread(buff3, 4, 1, file);
		fread(&sz2, 4, 1, file);
		
		printf("%s\n", buff3);
		printf("%d\n", sz2);

		int flag = 0;
		for (int i = 0; i < sz2; ++i) {
			int c = fgetc(file);
			if (c == EOF) {
				flag = 1;
				break;
			}
		}
		if (flag) break;
	}
	
	if (ferror(file)) {
		perror("\n\nI/O error when reading file.\n");
	} else if (feof(file)) {
		perror("\n\nEnd of file is reached.\n");
	} else {
		perror("\n\nEnd of file not reached.\n");
	}

	fclose(file);
	
	return NULL;
}

void WAV_file_write_sin_wave(
        struct WAV_file *wav,
        double freq,
        uint32_t duration)
{
    if (wav == NULL) {
	perror("Cannot write sin wave to WAV as WAV struct is NULL");   
        return;
    } else if (wav->data.buff != NULL || wav->data.subchunk2_size != 0) {
        free(wav->data.buff);
        wav->data.buff = NULL;
        wav->data.subchunk2_size = 0;
        wav->riff.chunk_size = 36;
    }

	const uint32_t subchunk2_size =
		(wav->fmt.bits_per_sample / 8) 
		* wav->fmt.num_channels
		* wav->fmt.sample_rate
		* duration;

    wav->data.subchunk2_size = subchunk2_size;
    wav->data.buff = (unsigned char*)malloc(sizeof(unsigned char) * subchunk2_size);
	wav->riff.chunk_size = 36 + subchunk2_size;

    // -6Db amplitude by default
    const double amp = pow(10, -6.0 / 20.0) * (pow(2, wav->fmt.bits_per_sample - 1) - 1);
    const double sample_period = 1.0 / wav->fmt.sample_rate;
    const double ang_freq = 2.0 * M_PI * freq;
    const uint16_t bytes_per_sample = (wav->fmt.bits_per_sample / 8);

    // Loop for the number of samples
    for (uint32_t sample_index = 0;
         sample_index < wav->data.subchunk2_size /  (wav->fmt.num_channels * bytes_per_sample);
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
}

void WAV_file_write_binaural_wave(
        struct WAV_file *wav,
        double freq1,
        double freq2,
        uint32_t duration)
{
    if (wav == NULL) {
        perror("Cannot write sin wave to WAV as WAV struct is NULL");   
        return;
    } else if (wav->data.buff != NULL || wav->data.subchunk2_size != 0) {
        free(wav->data.buff);
        wav->data.buff = NULL;
        wav->data.subchunk2_size = 0;
        wav->riff.chunk_size = 36;
    }

	const uint32_t subchunk2_size =
		(wav->fmt.bits_per_sample / 8) 
		* wav->fmt.num_channels
		* wav->fmt.sample_rate
		* duration;

    wav->data.subchunk2_size = subchunk2_size;
    wav->data.buff = (unsigned char*)malloc(sizeof(unsigned char) * subchunk2_size);
	wav->riff.chunk_size = 36 + subchunk2_size;

    // -6Db amplitude by default
    const double amp = pow(10, -6.0 / 20.0) * (pow(2, wav->fmt.bits_per_sample - 1) - 1);
    const double sample_period = 1.0 / wav->fmt.sample_rate;
    const double ang_freq1 = 2.0 * M_PI * freq1;
    const double ang_freq2 = 2.0 * M_PI * freq2;
    const uint16_t bytes_per_sample = (wav->fmt.bits_per_sample / 8);

    // Loop for the number of samples
    for (uint32_t sample_index = 0;
         sample_index < wav->data.subchunk2_size /  (wav->fmt.num_channels * bytes_per_sample);
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
}

void free_WAV_file(struct WAV_file *file) {
    if (file == NULL) return;
    if (file->data.buff == NULL) return;
    file->riff.chunk_size = 36;
    free(file->data.buff);
    file->data.subchunk2_size = 0;
}

void write_WAV_to_file(
        struct WAV_file* wav,
        const char* file_name)
{
	if (wav == NULL) return;
	if (file_name == NULL) return;

	FILE* file = fopen(file_name, "w");

	if (file == NULL) {
		perror("File opening failed\n");
		fclose(file);
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
	}

	size_t sound_data_ret = fwrite(
			wav->data.buff,
			sizeof(unsigned char),
			wav->data.subchunk2_size,
			file
        );

	if (sound_data_ret != wav->data.subchunk2_size) {
		perror("Failed to write sound data\n");
	}

    fclose(file);
}

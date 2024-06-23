#include "WavReader.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

struct WAV_file alloc_WAV_file(
		uint16_t num_channels,
		uint32_t sample_rate,
		uint16_t bits_per_sample,	
		uint32_t duration)
{

	const uint32_t subchunk2_size =
		(bits_per_sample / 8) 	// bytes for a sample
		* num_channels		// now bytes for all channels for one sample
		* sample_rate 		// now bytes for all channels for all samples in one second
		* duration;		// now total bytes overall for total duration in seconds

	const uint32_t chunk_size = 36 + subchunk2_size;

	const struct WAV_file wav = (struct WAV_file) {
		
		.riff = (struct RIFF_chunk) {
			.chunk_id   = {'R','I','F','F'},
			.chunk_size = chunk_size,
			.format     = {'W','A','V','E'},
		},

		.fmt = {
			.subchunk1_id    = {'f', 'm', 't', ' '},
			.subchunk1_size	 = 16,	// assuming PCM
			.audio_format    = 1,	// assuming PCM
			.num_channels    = num_channels,
			.sample_rate	 = sample_rate,
			.byte_rate       = sample_rate * num_channels * (bits_per_sample / 8),
			.block_align     = num_channels * (bits_per_sample / 8),
			.bits_per_sample = bits_per_sample,
		},

		.data = {
			.subchunk2_id   = {'d', 'a', 't', 'a'},
			.subchunk2_size = subchunk2_size,
			.buff = (unsigned char*)malloc(subchunk2_size),
		},

	};

	return wav;
}

void WAV_file_write_sin_wave(
        struct WAV_file *wav,
        double freq)
{
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
        double freq2)
{
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

void free_WAV_file(struct WAV_file file) {
	if (file.data.buff == NULL) return;
	free(file.data.buff);
}

void write_WAV_to_file(
        struct WAV_file* wav,
        const char* file_name)
{
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

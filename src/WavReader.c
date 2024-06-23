#include "WavReader.h"

#include <stdlib.h>
#include <math.h>

struct WAV_file create_WAV_file_sin_wave(
		/* Endian-ness? */
		double	 freq,
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
	
    double amp = pow(10, -6.0 / 20.0) * (pow(2, bits_per_sample - 1) - 1);
    double sample_period = 1.0 / sample_rate;
    double ang_freq = 2.0 * M_PI * freq;

    for (uint32_t i = 0; i < subchunk2_size / (bits_per_sample / 8); i++) {
        double t = (double)i * sample_period;
        int16_t sample = (int16_t)(amp * sin(ang_freq * t));

        // For stereo, write the same sample to both left and right channels
        for (uint16_t channel = 0; channel < num_channels; ++channel) {
            uint32_t sample_index = i * num_channels + channel;
            wav.data.buff[sample_index * 2] = sample & 0xFF;
            wav.data.buff[sample_index * 2 + 1] = (sample >> 8) & 0xFF;
        }
    }

	return wav;
}

void free_WAV_file(struct WAV_file file) {
	if (file.data.buff == NULL) return;
	free(file.data.buff);
}

FILE* write_WAV_to_file(struct WAV_file wav, const char* file_name)
{
	FILE* ret_file = fopen(file_name, "w");

	if (ret_file == NULL) {
		perror("File opening failed\n");
		fclose(ret_file);
		return NULL;
	}

	size_t riff_ret = fwrite(
			&wav.riff,
			sizeof(wav.riff),
			1,
			ret_file
		);

	if (riff_ret != 1) {
		perror("Failed to write RIFF chunk\n");
		fclose(ret_file);
		return NULL;
	}

	size_t fmt_ret = fwrite(
			&wav.fmt,
			sizeof(wav.fmt),
			1,
			ret_file
		);

	if (fmt_ret != 1) {
		perror("Failed to write FMT chunk\n");
		fclose(ret_file);
		return NULL;
	}

	size_t data_ret = fwrite(
			&wav.data,
			sizeof(wav.data) - sizeof(wav.data.buff),
			1,
			ret_file
		);
	
	if (data_ret != 1) {
		perror("Failed to write DATA chunk\n");
		fclose(ret_file);
		return NULL;
	}

	size_t sound_data_ret = fwrite(
			wav.data.buff,
			sizeof(unsigned char),
			wav.data.subchunk2_size,
			ret_file);

	if (sound_data_ret != wav.data.subchunk2_size) {
		perror("Failed to write sound data\n");
		fclose(ret_file);
		return NULL;
	}

	return ret_file;
}

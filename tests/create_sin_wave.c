#include <string.h>
#include <stdio.h>

#include "WavReader.h"

int main(void) {

	printf("\nCreating example sin and binaural wav files:\n\n");

	char file1_name[] = "test-sin.wav\0";
	char file2_name[] = "test-binaural.wav\0";
	WAV_State ret = Error;

	struct WAV_file wav;
	memset(&wav, 0, sizeof(wav));

	// Use this to init the skeleton
	// of a .wav WITHOUT any waveform data
	WAV_init(
		&wav,
		2,	// channels
		44100,	// sample rate
		16	// bits per sample
	);

	ret = WAV_write_sin_wave(
			&wav,
			174.0f, // frequency
			15,	// duration (sec)
			-6.0f	// max decibel level
		);

	if (ret == Error) {
		perror("ERROR: Could not write sin wave to WAV struct!\n");   
		return 1;
	}

	if (WAV_write_to_file(&wav, file1_name) == Error) {
		fprintf(stderr, "ERROR: Could not write WAV struct to %s!\n", file1_name);   
		return 1;
	}

	printf("\nWrote sin wav to file: %s\n\n", file1_name);

	WAV_print(&wav);

	ret = WAV_write_binaural_wave(
			&wav,
			174.0f, // frequency 1
			164.0f,	// frequency 2
			20,	// duration (sec)
			-1.0f	// max decibel level
		);

	if (ret == Error) {
		perror("ERROR: Could not write sin wave to .wav file!\n");   
		return 1;
	}

	if (WAV_write_to_file(&wav, file2_name) == Error) {
		fprintf(stderr, "ERROR: Could not write WAV struct to %s!\n", file2_name);   
		return 1;
	}

	printf("\nWrote binaural wav to file: %s\n\n", file2_name);

	WAV_print(&wav);

	printf("\n");

	// Free data allocated for waveform & EXTRA_chunk(s)
	WAV_free(&wav);

	return 0;
}

#include <string.h>
#include <stdio.h>

#include "WavReader.h"

int main(void) {

	printf("\nCreating example sin and binaural wav files:\n\n");

	struct WAV_file wav;
	memset(&wav, 0, sizeof(wav));

	// Use this to init the skeleton
	// of a .wav WITHOUT any waveform data
	init_WAV_file(
		&wav,
		2,	// channels
		44100,	// sample rate
		16	// bits per sample
	);

	char file1[] = "test-sin.wav\0";
	char file2[] = "test-binaural.wav\0";

	// Write sine wave to the wav struct
	WAV_file_write_sin_wave(
			&wav,
			174.0f, // frequency
			15,	// duration (sec)
			-6.0f	// max decibel level
		);

	write_WAV_to_file(&wav, file1);

	printf("\nWrote sin wav to file: %s\n\n", file1);

	print_WAV_file(&wav);

	// Write binaural wave to the wav struct
	WAV_file_write_binaural_wave(
			&wav,
			174.0f, // frequency 1
			164.0f,	// frequency 2
			20,	// duration (sec)
			-1.0f	// max decibel level
		);

	write_WAV_to_file(&wav, file2);

	printf("\nWrote binaural wav to file: %s\n\n", file2);

	print_WAV_file(&wav);

	printf("\n");

	// Free data allocated for waveform & EXTRA_chunk(s)
	free_WAV_file(&wav);

	return 0;
}

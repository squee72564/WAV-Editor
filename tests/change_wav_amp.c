#include <string.h>
#include <stdio.h>

#include "WavReader.h"

int main(void) {

	printf("\nCreating example sin wav and then changing the amp:\n\n");

	const char file1_name[] = "test-sin-amp1.wav\0";
	const char file2_name[] = "test-sin-amp2.wav\0";
	const double old_db = -6.0f;
	const double new_db = -16.0f;
	WAV_State ret = Error;

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

	ret = WAV_file_write_sin_wave(
			&wav,
			174.0f, // frequency
			15,	// duration (sec)
			old_db	// max decibel level
		);

	if (ret == Error) {
		perror("ERROR: Could not write sin wave to WAV struct!\n");   
		return 1;
	}

	if (write_WAV_to_file(&wav, file1_name) == Error) {
		fprintf(stderr, "ERROR: Could not write WAV struct to %s!\n", file1_name);   
		return 1;
	}

	printf("\nWrote sin wav to file at %.2fdb: %s\n\n", old_db, file1_name);

	print_WAV_file(&wav);

	printf("\nChanging sin wav amplitude to -32.0db\n\n");


	if (normalize_WAV_max_db(&wav, new_db) == Error) {
		fprintf(stderr, "ERROR: Could not change WAV struct to %d db!\n", new_db);   
		return 1;
	}

	if (write_WAV_to_file(&wav, file2_name) == Error) {
		fprintf(stderr, "ERROR: Could not write WAV struct to %s!\n", file2_name);   
		return 1;
	}

	print_WAV_file(&wav);

	printf("\nWrote sin wav to file at %.2fdb: %s\n\n", new_db, file2_name);

	// Free data allocated for waveform & EXTRA_chunk(s)
	free_WAV_file(&wav);

	return 0;
}

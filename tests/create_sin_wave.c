#include <string.h>
#include "WavReader.h"

int main(void) {

	struct WAV_file wav;
	memset(&wav, 0, sizeof(wav));

	init_WAV_file(
		&wav,
		2,	// channels
		44100,	// sample rate
		16	// bits per sample
	);

	WAV_file_write_sin_wave(
			&wav,
			174.0f,
			20,
			-6.0f
		);

	write_WAV_to_file(&wav, "test-sin.wav");

	WAV_file_write_binaural_wave(
			&wav,
			174.0f,
			164.0f,
			20,
			-1.0f
		);

	write_WAV_to_file(&wav, "test-binaural.wav");

	free_WAV_file(&wav);

	return 0;
}

#include "WavReader.h"

int main(void) {

	struct WAV_file wav = alloc_WAV_file(
			2,	// channels
			44100,	// sample rate
			16,	// bits per sample
			20	// duration (seconds)
		);

    WAV_file_write_sin_wave(&wav, 174.0);

	write_WAV_to_file(&wav, "test-sin.wav");

	free_WAV_file(wav);

	return 0;
}

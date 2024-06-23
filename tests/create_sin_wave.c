#include "WavReader.h"
#include <stdio.h>

int main(void) {

	struct WAV_file wav = create_WAV_file_sin_wave(
			100,	// frequency
			2,	// channels
			44100,	// sample rate
			16,	// bits per sample
			10	// duration (seconds)
		);

	FILE* file = write_WAV_to_file(wav, "test-sin.wav");

	free_WAV_file(wav);

	fclose(file);

	return 0;
}

#include <string.h>
#include <stdio.h>
#include "WavReader.h"

int main(int argc, char** argv)
{
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <wav file path>\n", argv[0]);
		return 1;
	}

	struct WAV_file wav;
	memset(&wav, 0, sizeof(wav));

	// Read wav file into struct
	if (read_WAV_file(&wav, argv[1]) == Error) {
		perror("Error: Could not read wav file!\n");
		return 1;
	}

	printf("\nReading wav file: %s\n\n", argv[1]);
	
	print_WAV_file(&wav);

	printf("\nWav file max db: %.2f dB.\n\n", get_WAV_max_db(&wav));

	// Free data allocated for waveform & EXTRA_chunk(s)
	free_WAV_file(&wav);

	return 0;
}

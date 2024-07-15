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

	printf("\nReading wav file: %s\n\n", argv[1]);

	// Read wav file into struct
	if (WAV_read_file(&wav, argv[1]) == Error) {
		perror("Error: Could not read wav file!\n");
		return 1;
	}
	
	WAV_print_metadata(&wav);

	// Free data allocated for waveform & EXTRA_chunk(s)
	WAV_free(&wav);

	return 0;
}

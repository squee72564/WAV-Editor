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

	if (!read_WAV_file(&wav, argv[1])) {
		perror("Error: Could not read wav file!\n");
		return 1;
	}

	print_WAV_file(&wav);

	printf("Wav file db: %.2f dB.\n", get_WAV_max_db(&wav));

	free_WAV_file(&wav);

	return 0;
}

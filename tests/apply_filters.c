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

	const float freq = 30.0f;

	printf("\nApplying lowpass at %.2fhz to wav file: %s\n\n", freq, argv[1]);

	// Read wav file into struct
	if (WAV_read_file(&wav, argv[1]) == Error) {
		perror("Error: Could not read wav file!\n");
		return 1;
	}

	WAV_apply_low_pass_filter(&wav, freq);

	char file_name[] = "test-lowpass.wav";

	if (WAV_write_to_file(&wav, file_name) == Error) {
		fprintf(stderr, "ERROR: Could not write WAV struct to %s!\n", file_name);   
		return 1;
	}

	printf("\nWrote file with lowpass applied to %s\n\n", file_name);

	WAV_free(&wav);

	memset(&wav, 0, sizeof(wav));

	const float freq2 = 2200.0f;

	printf("\nApplying highpass at %.2fhz to wav file: %s\n\n", freq2, argv[1]);

	// Read wav file into struct
	if (WAV_read_file(&wav, argv[1]) == Error) {
		perror("Error: Could not read wav file!\n");
		return 1;
	}

	WAV_apply_high_pass_filter(&wav, freq2);

	char file_name2[] = "test-highpass.wav";

	if (WAV_write_to_file(&wav, file_name2) == Error) {
		fprintf(stderr, "ERROR: Could not write WAV struct to %s!\n", file_name2);   
		return 1;
	}

	printf("\nWrote file with highpass applied to %s\n\n", file_name2);
	
	WAV_free(&wav);

	return 0;
}

#include <string.h>
#include <stdio.h>
#include "WavReader.h"

int main(void) {

	struct WAV_file wav;
	memset(&wav, 0, sizeof(wav));

	int ret = read_WAV_file(
			&wav,
			"/home/alan/Music/My Records/[Kimochi]/[Kimochi 23]  Area - Anhedral/A1 For HN.wav"
		);

	if (ret == 1) {
		print_WAV_file(&wav);
	} else {
		perror("Could not read wav file\n");
	}

	return 0;
}

#ifndef WAV_READER_C_H 
#define WAV_READER_C_H 

#include <stdint.h>

// Source : https://ccrma.stanford.edu/courses/422-winter-2014/projects/WaveFormat/

/*
 * ----------------------------------------
 *
 * 		WAV STRUCTS 
 *
 * ----------------------------------------
 */

#ifdef __cplusplus
extern "C" {
#endif

struct RIFF_chunk {
	unsigned char 	id[4];		// ascii letters "RIFF" for little-endian, "RIFX" for big-endian
	uint32_t 	size;		// 36 + subchunk2_size
	unsigned char 	format[4];	// ascii letters "WAVE"
};

struct FMT_chunk {
	unsigned char 	id[4];		// ascii letters "fmt"
	uint32_t 	size;		// size of rest of subchunk following this field; 16 for PCM
	uint16_t  	audio_format;	// PCM = 1, other number indicates some form of compression
	uint16_t 	num_channels;	// 1 = mono, 2 = stereo, etc.
	uint32_t	sample_rate;	// 8000, 44100, 48000, etc.
	uint32_t	byte_rate;	// sample rate * num channels * (bits per sample / 8)
	uint16_t 	block_align;	// num channels * (bits per sample / 8);
	uint16_t 	bits_per_sample;// 8-bit samples are stored as unsigned bytes \
					// 16-bit samples are stores as 2's complement signed integers
};

struct DATA_chunk {
	unsigned char 	id[4];	// ascii letters "data"
	uint32_t	size;	// Number of bytes in sound data
	unsigned char   *buff;	// actual sound data
};

// Note: There may be additional subchunks in a Wave data stream.
// If so, each will have a char[4] SubChunkID, and unsigned long SubChunkSize, and SubChunkSize amount of data.
// This struct functions as a singly-linked list to hold additional data chunks right now
struct EXTRA_chunk {
	unsigned char id[4];
	uint32_t size;
	unsigned char *buff;
	struct EXTRA_chunk *next;
};

struct WAV_file {
	struct RIFF_chunk riff;
	struct FMT_chunk  fmt;
	struct DATA_chunk data;
	struct EXTRA_chunk *extra;
};

/*
 * ----------------------------------------
 *
 * 		WAV FUNCTIONS
 *
 * ----------------------------------------
 */

void init_WAV_file(
		struct WAV_file *wav,
		uint16_t num_channels,
		uint32_t sample_rate,
		uint16_t bits_per_sample
	);

int read_WAV_file(
		struct WAV_file *wav,
		char *file_name
	);

void print_WAV_file(struct WAV_file *wav);

double get_WAV_max_db(struct WAV_file * wav);

void WAV_file_write_sin_wave(
		struct WAV_file *file,
		double frequency,
		uint32_t duration
	);

void WAV_file_write_binaural_wave(
		struct WAV_file *wav,
		double frequency1,
		double frequency2,
		uint32_t duration
	);

void free_WAV_file(struct WAV_file *wav);

void write_WAV_to_file(
		struct WAV_file *wav,
		const char* file_name
	);

#ifdef __cplusplus
}
#endif

#endif

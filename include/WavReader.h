#ifndef WAV_READER_C_H 
#define WAV_READER_C_H 

#include <stdint.h>

// Source : https://ccrma.stanford.edu/courses/422-winter-2014/projects/WaveFormat/

struct RIFF_chunk {
	unsigned char 	chunk_id[4];		// ascii letters "RIFF" for little-endian, "RIFX" for big-endian
	uint32_t 	chunk_size;		// 36 + subchunk2_size
	unsigned char 	format[4];		// ascii letters "WAVE"
};

struct FMT_chunk {
	unsigned char 	subchunk1_id[4];	// ascii letters "fmt"
	uint32_t 	subchunk1_size;		// size of rest of subchunk following this field; 16 for PCM
	uint16_t  	audio_format;		// PCM = 1, other number indicates some form of compression
	uint16_t 	num_channels;		// 1 = mono, 2 = stereo, etc.
	uint32_t	sample_rate;		// 8000, 44100, 48000, etc.
	uint32_t	byte_rate;		// sample rate * num channels * (bits per sample / 8)
	uint16_t 	block_align;	 	// num channels * (bits per sample / 8);
	uint16_t 	bits_per_sample;	// 8-bit samples are stored as unsigned bytes (0-255), \
						// 16-bit samples are stores as 2's complement signed integers (-32768, 32767)
};

struct DATA_chunk {
	unsigned char 	subchunk2_id[4];	// ascii letters "data"
	uint32_t	subchunk2_size;		// Number of bytes in sound data
	unsigned char* 	buff;			// actual sound data
};

// Note: There may be additional subchunks in a Wave data stream. If so, each will have a char[4] SubChunkID, and unsigned long SubChunkSize, and SubChunkSize amount of data.

struct WAV_file {
	struct RIFF_chunk riff;
	struct FMT_chunk  fmt;
	struct DATA_chunk data;
};

struct WAV_file alloc_WAV_file(
		uint16_t num_channels,
		uint32_t sample_rate,
		uint16_t bits_per_sample
	);

struct WAV_file read_WAV_file(
        char *file_name
    );

void WAV_file_write_sin_wave(
        struct WAV_file *file,
        double frequency,
        uint32_t duration
    );

void WAV_file_write_binaural_wave(
        struct WAV_file *file,
        double frequency1,
        double frequency2,
        uint32_t duration
    );

void free_WAV_file(struct WAV_file *file);

void write_WAV_to_file(
        struct WAV_file *wav,
        const char* file_name
    );

#endif

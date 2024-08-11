
# WAV Editor

This program is a work in progress; it will eventually be a command line tool to allow a user to edit WAV file metadata and sound data.

Implemented so far:

- Read a wav file into struct, write the wav struct to a file
- Create wav files with a sin wave or binaural beat for a specified duration
- Print wav file information & metadata
	- So far this is simply printing the standard chunks defined in the wav spec
	- Basic functionality to print additional optional chunks included in the file, but the ability to parse those optional chunks is to be included
- Get max decibel level of the wav file
- Normalize the wav file to a new maximum specified decible level
- Apply high and low pass filter to entire wav file at specified cutoff frequency

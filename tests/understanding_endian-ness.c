#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int readLittleEndian(const unsigned char *data) {
	return  *(int *)data;
}

int readBigEndian(const unsigned char *data) {
	return 	((int)data[0] << 24) |
	   	((int)data[1] << 16) |
	   	((int)data[2] << 8 ) |
	   	((int)data[3] << 0 );
}

int main(void) {
	
	printf("This is a test reading from a 4-byte buffer to see how the\n");
	printf("memory layout of the system is and how reading memory from\n");
	printf("a buffer into a datastructure like an int works under the hood\n\n");
	
	printf("Data Buffer: {0x24, 0x08, 0x00, 0x00}\n");
	unsigned char data[] = { 0x24, 0x08, 0x00, 0x00 };

	int le = readLittleEndian(data);
	int be = readBigEndian(data);

	// Interpret and print results
	printf("Little-endian interpretation: 0x%08x, %d\n", le, le);
	printf("Big-endian interpretation:    0x%08x, %d\n", be, be);

	printf("Ouput of a integer 2084 by reading each of its 4 bytes sequentially: ");

	int n = 2084;
	unsigned char* c = (unsigned char*)(&n);

	for (int i = 0; i < sizeof(int); ++i) {
		printf("0x%02x ", c[i]);
	}

	printf("\n---------------------------------------------------------------------\n");

	
	printf("Data Buffer: { 'R', 'I', 'F', 'F', '\\0' }\n");

	int buff_sz = 5*sizeof(char);
	unsigned char c_data[] = { 'R', 'I', 'F', 'F', '\0'};

	for (int i = 0; i < 4; ++i) {
		printf("0x%02x ", c_data[i]);
	}

	printf("\nReading data buffer into new char buffer\n");

	char c_data2[5] = {0};
	memcpy(c_data2, c_data, buff_sz);


	for (int i = 0; i < 4; ++i) {
		printf("0x%02x ", c_data[i]);
	}

	printf("\n%s\n", c_data2);

    
	return 0;
}

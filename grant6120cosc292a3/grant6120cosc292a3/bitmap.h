#ifndef BITMAP_H
#define BITMAP_H

#include <stdio.h>

#define PI 3.1415926535897932384626433832

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int DWORD;

/* The default byte alignment for structs is 4 bytes in the x86 environment.  We want
to change the alignement to align on 1-byte boundaries. This way, there will be no padding
between attributes and the size of the struct will be exactly 54 bytes, matching the
signature of the header in the BMP file.
The byte alignment can be changed with a pre-processor directive called the "#pragma"
statement. This applies to all structs that follow the pragma statement until another
pragma is encountered. Now we can do a single read from the file to populate all the
attributes of the BITMAPHDR. */

#pragma pack(push) // Used to store the default byte alignment
#pragma pack(1) // Set the byte alignment to 1

typedef struct
{
	// Bitmap file header - 14 bytes
	WORD wType; // Should be 'B' 'M'
	DWORD dwFileSize;
	WORD wReserved1;
	WORD wReserved2;
	DWORD dwDataOffset; // Should be 54 for our application
	// DIB header (bitmap information header) - 40 bytes
	DWORD dwHeaderSize; // Should be 40 for our application
	int dwWidth;
	int dwHeight;
	WORD wPlanes; // should be 1
	WORD wBitCount; // should be 24 for our application
	DWORD dwCompression;
	DWORD dwImageSize;
	int dwXPelsPerMeter;
	int dwYPelsPerMeter;
	DWORD dwClrUsed;
	DWORD dwClrImportant;
} BITMAPHDR;

typedef struct
{
	// Strangely, blue comes first in the pixel...
	BYTE bBlu, bGrn, bRed;
} PIXEL;

// A struct that represents the entire image
typedef struct
{
	BITMAPHDR* bmHDR;
	PIXEL* bmData;
} IMAGE;

#pragma pack(pop) // Used to reset the default byte alignment


// Function pointers for editing a bitmap image
typedef void (*BM_FUNC_PTR)(PIXEL*);
typedef void (*BM_TWO_PIXELS)(PIXEL*, PIXEL*);


FILE* GetFile(char* cPrompt, char* cMode);

void ReadImage(IMAGE*, FILE*);
void ReadHeader(IMAGE*, FILE*);
void ReadData(IMAGE*, FILE*);


PIXEL* GetPixel(int row, int col, IMAGE* imgPtr);


void WriteImage(IMAGE*, FILE*);

void FreeImage(IMAGE*);

unsigned int GetFileSize(FILE* filePtr);

void ExtractFileFromImage(IMAGE* imgPtr, FILE* filePtr);

void HideInImage(IMAGE* imgPtr, FILE* filePtr);

void testExtractFileFromImage();

void testHideInImage();

#endif // !BITMAP_H
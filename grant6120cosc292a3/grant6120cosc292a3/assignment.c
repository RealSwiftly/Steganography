#define _CRT_SECURE_NO_DEPRECATE // to use older functions like fopen instead of fopen_s
#include "bitmap.h"
#include <malloc.h>
#include <stdlib.h>

#define MAX_FILENAME_SIZE	256

FILE* GetFile(char* cPrompt, char* cMode)
{
	FILE* aFile = NULL;
	char cFileName[MAX_FILENAME_SIZE];

	// Get the name from the user
	printf("%s", cPrompt);
	gets_s(cFileName, MAX_FILENAME_SIZE);

	// Open the file with the given name and the mode passed in
	aFile = fopen(cFileName, cMode);

	return aFile;
}


void ReadImage(IMAGE* imgPtr, FILE* infile)
{
	// Read in the header
	ReadHeader(imgPtr, infile);

	if (imgPtr->bmHDR != NULL)
	{
		// Read in the data
		ReadData(imgPtr, infile);
	}
}

void ReadHeader(IMAGE* imgPtr, FILE* infile)
{
	// Allocate memory for the bitmap header
	imgPtr->bmHDR = (BITMAPHDR*)malloc(sizeof(BITMAPHDR));

	if (imgPtr->bmHDR != NULL)
	{
		if (fread(imgPtr->bmHDR, sizeof(BITMAPHDR), 1, infile) != 1)
		{
			// No success reading, but malloc worked
			// Free the memory
			free(imgPtr->bmHDR);
			imgPtr->bmHDR = NULL;
			printf("Reading header from file did not work\n");
		}
	}
}


void ReadData(IMAGE* imgPtr, FILE* infile)
{
	// The padding, in bytes, for image is:
	unsigned int padding = imgPtr->bmHDR->dwWidth % 4;
	// Calculate the image size in bytes = size of row * number of rows
	unsigned int imageSize = (imgPtr->bmHDR->dwWidth * sizeof(PIXEL) + padding)
		* imgPtr->bmHDR->dwHeight;

	// Allocate memory for pixel data
	imgPtr->bmData = (PIXEL*)malloc(imageSize);
	if (imgPtr->bmData != NULL)
	{
		if (fread(imgPtr->bmData, imageSize, 1, infile) != 1)
		{
			// Failed to read in the image data
			free(imgPtr->bmHDR);
			imgPtr->bmHDR = NULL;
			free(imgPtr->bmData);
			imgPtr->bmData = NULL;
		}
	}
	else
	{
		free(imgPtr->bmHDR);
		imgPtr->bmHDR = NULL;
	}
}

void WriteImage(IMAGE* imgPtr, FILE* outfile)
{
	// The padding, in bytes, for image is:
	unsigned int padding = imgPtr->bmHDR->dwWidth % 4;
	// Calculate the image size in bytes = size of row * number of rows
	unsigned int imageSize = (imgPtr->bmHDR->dwWidth * sizeof(PIXEL) + padding)
		* imgPtr->bmHDR->dwHeight;

	// Write the header
	if (fwrite(imgPtr->bmHDR, sizeof(BITMAPHDR), 1, outfile) != 1)
	{
		printf("Failed to write image header\n");
	}
	else
	{
		if (fwrite(imgPtr->bmData, imageSize, 1, outfile) != 1)
		{
			printf("Failed to write image data\n");
		}
	}
}

void ManipulateImage(IMAGE* imgPtr, BM_FUNC_PTR pixelFunc)
{
	// Goal through each pixel and call the pixelFunc on the pixel
	for (int i = 0; i < imgPtr->bmHDR->dwHeight; i++)
	{
		for (int j = 0; j < imgPtr->bmHDR->dwWidth; j++)
		{
			pixelFunc(GetPixel(i, j, imgPtr));
		}
	}
}

PIXEL* GetPixel(int row, int col, IMAGE* imgPtr)
{
	unsigned int padding = imgPtr->bmHDR->dwWidth % 4;

	// Offset in bytes = size of the row in bytes * number of rows
	//  + number of bytes preceding the desired pixel in the row
	unsigned int offset = (imgPtr->bmHDR->dwWidth * sizeof(PIXEL) + padding) * row
		+ col * sizeof(PIXEL);

	// offset is in bytes, so we need to do some casting of pointers
	return (PIXEL*)((BYTE*)imgPtr->bmData + offset);
}

void FreeImage(IMAGE* imgPtr)
{
	// Free the image
	free(imgPtr->bmHDR);
	imgPtr->bmHDR = NULL;
	free(imgPtr->bmData);
	imgPtr->bmData = NULL;
}


void HideInImage(IMAGE* imgPtr, FILE* filePtr) {
	unsigned int fileSize = GetFileSize(filePtr);
	imgPtr->bmHDR->dwClrImportant = fileSize; // Store the file size in dwClrImportant

	BYTE byte;
	int bitCount = 0;

	for (int i = 0; i < imgPtr->bmHDR->dwHeight; i++) {
		for (int j = 0; j < imgPtr->bmHDR->dwWidth; j++) {
			if (bitCount < fileSize * 8) {
				fread(&byte, sizeof(BYTE), 1, filePtr);

				// Extract the bits from the byte
				BYTE blueBits = byte >> 4;
				BYTE greenBits = (byte >> 2) & 3;
				BYTE redBits = byte & 3;

				// Get the pixel at i, j
				PIXEL* pixelPtr = GetPixel(i, j, imgPtr);

				// Replace the lower bits of the rgb values with the bits from the file
				pixelPtr->bBlu &= 240;
				pixelPtr->bBlu |= blueBits;
				pixelPtr->bGrn &= 252;
				pixelPtr->bGrn |= greenBits;
				pixelPtr->bRed &= 252;
				pixelPtr->bRed |= redBits;

				bitCount += 8;
			}
			else {
				return;
			}
		}
	}
}

void ExtractFileFromImage(IMAGE* imgPtr, FILE* filePtr) {
	//Reading size of the hidden file from the bitmap header
	unsigned int fileSize = imgPtr->bmHDR->dwClrImportant;
	BYTE* hiddenFile = (BYTE*)malloc(fileSize);

	int byteIndex = 0;
	for (int i = 0; i < imgPtr->bmHDR->dwHeight; i++) {
		for (int j = 0; j < imgPtr->bmHDR->dwWidth; j++) {
			if (byteIndex >= fileSize) {
				break;
			}

			//Collecting rgb values
			BYTE blue = GetPixel(i, j, imgPtr)->bBlu;
			BYTE green = GetPixel(i, j, imgPtr)->bGrn;
			BYTE red = GetPixel(i, j, imgPtr)->bRed;

			//Extracting least significant bits from rgb values
			BYTE hiddenByte = 0;
			hiddenByte |= (blue & 15) << 4;
			hiddenByte |= (green & 3) << 2;
			hiddenByte |= (red & 3);

			//Storing the extracted byte
			hiddenFile[byteIndex] = hiddenByte;
			byteIndex++;
		}
	}

	fwrite(hiddenFile, 1, fileSize, filePtr);
	free(hiddenFile);
}



unsigned int GetFileSize(FILE* filePtr) {
	unsigned int size;

	fseek(filePtr, 0L, SEEK_END);
	size = ftell(filePtr);
	fseek(filePtr, 0L, SEEK_SET);

	return size;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include "crc32.h"
#include "png.h"

const int ERROR_INVALID_PARAMS = 1;
const int ERROR_INVALID_FILE = 2;
const int ERROR_INVALID_CHUNK_DATA = 3;
const int ERROR_NO_UIUC_CHUNK = 4;


/**
 * Opens a PNG file for reading (mode == "r" or mode == "r+") or writing (mode == "w").
 * 
 * (Note: The function follows the same function prototype as `fopen`.)
 * 
 * When the file is opened for reading this function must verify the PNG signature.  When opened for
 * writing, the file should write the PNG signature.
 * 
 * This function must return NULL on any errors; otherwise, return a new PNG struct for use
 * with further fuctions in this library.
 */

PNG * PNG_open(const char *filename, const char *mode) {
  
  char* fileExtension = (char*) malloc(4 * sizeof(char));
  strncpy(fileExtension, filename + strlen(filename) - 3, 3);
  strcpy(fileExtension + 3, "\0");
 
  if (strcmp(fileExtension, "png") != 0) {
    free(fileExtension);
    return NULL;
  } else {
    free(fileExtension);
  }
  
  FILE *fp = fopen(filename, mode);
 
  if (fp == NULL) {
    
    return NULL;
  }
  PNG *toMakeAddr = malloc(sizeof(PNG));

  toMakeAddr->fileName = (char*) malloc((strlen(filename) + 1) * sizeof(char));
  strcpy(toMakeAddr->fileName, filename);
  if (strcmp(mode, "w") == 0) {
    toMakeAddr->byteFile = malloc(9 * sizeof(char));
    toMakeAddr->byteFile[0] = 0x89;
    toMakeAddr->byteFile[1] = 0x50;
    toMakeAddr->byteFile[2] = 0x4e;
    toMakeAddr->byteFile[3] = 0x47;
    toMakeAddr->byteFile[4] = 0x0d;
    toMakeAddr->byteFile[5] = 0x0a;
    toMakeAddr->byteFile[6] = 0x1a;
    toMakeAddr->byteFile[7] = 0x0a;
    toMakeAddr->byteFile[8] = 0x00;
    toMakeAddr->index = 9;
    fwrite(toMakeAddr->byteFile, 1, 8, fp);
    fclose(fp);
    return toMakeAddr;
  }
  fseek(fp, 0, SEEK_END);
  size_t fileLength = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  toMakeAddr->byteFile = (unsigned char*)calloc(fileLength, sizeof(char));
  fread(toMakeAddr->byteFile, 1,fileLength,fp);

  fclose(fp);

  toMakeAddr->index = 8; 

  return toMakeAddr;
}


/**
 * Reads the next PNG chunk from `png`.
 * 
 * If a chunk exists, a the data in the chunk is populated in `chunk` and the
 * number of bytes read (the length of the chunk in the file) is returned.
 * Otherwise, a zero value is returned.
 * 
 * Any memory allocated within `chunk` must be freed in `PNG_free_chunk`.
 * Users of the library must call `PNG_free_chunk` on all returned chunks.
 */
size_t PNG_read(PNG *png, PNG_Chunk *chunk) {

  FILE *fpold = fopen(png->fileName, "r");
  fseek(fpold, 0, SEEK_END);
  size_t fileLength = ftell(fpold);
  fseek(fpold, 0, SEEK_SET);
  unsigned char*currentByteFile = (unsigned char*)calloc(fileLength + 1, sizeof(char));
  currentByteFile[fileLength] = '\0';
  fread(currentByteFile, 1,fileLength,fpold);
  
  fclose(fpold);
  if(png->index == fileLength) {
    png->index = 0;
    free(currentByteFile);
    return 0;
  }
 
  unsigned char *length = (unsigned char*) malloc(4 * sizeof(char));
  for (size_t i = 0; i < 4; i++) {
    length[i] = currentByteFile[png->index + i];
   
  }
  u_int32_t lengthToAdd = (((uint32_t)length[0] << 24) |  ((uint32_t)length[1] << 16) | ((uint32_t)length[2] << 8) | ((uint32_t)length[3]));
  free(length);
 
  chunk->len = lengthToAdd;
  for (size_t i = 0; i < 4; i++) {
    chunk->type[i] = currentByteFile[png->index + 4 + i];
  }
  chunk->type[4] = 0x00;
  if (chunk->len > 0) {
    chunk->data = (char*) malloc((lengthToAdd  + 1) * sizeof(char));
    for (size_t i = 0; i < lengthToAdd ; i++) {
      chunk->data[i] = currentByteFile[png->index + 8 + i];
    }
      chunk->data[lengthToAdd] = 0x00;
  }

  uint32_t crcVal = 0;
  crc32(chunk->type, strlen(chunk->type), &crcVal);
  if(chunk->len > 0) {
    crc32(chunk->data, chunk->len, &crcVal);
  }
  chunk->crc = crcVal;

  png->index = png->index + 4 + 4 + lengthToAdd + 4;

  free(currentByteFile);

  return  4 + 4 + lengthToAdd + 4;
}


/**
 * Writes a PNG chunk to `png`.
 * 
 * Returns the number of bytes written. 
 */
size_t PNG_write(PNG *png, PNG_Chunk *chunk) {
 
  FILE *fpold = fopen(png->fileName, "r");
  fseek(fpold, 0, SEEK_END);
  size_t fileLength = ftell(fpold);
  fseek(fpold, 0, SEEK_SET);
  size_t currentSize = fileLength;
  uint32_t dataSize = chunk->len;
  size_t newSize = currentSize + 4 + 4 + dataSize + 4;

  unsigned char*oldByteFile = (unsigned char*)calloc(newSize + 1, sizeof(char));
  oldByteFile[fileLength] = '\0';
  fread(oldByteFile, 1,fileLength,fpold);
  fclose(fpold);

  uint32_t length = chunk->len;
  oldByteFile[currentSize] = length >> 24;
  oldByteFile[currentSize + 1] = length >> 16;
  oldByteFile[currentSize + 2] = length >> 8;
  oldByteFile[currentSize + 3] = length;
  
  oldByteFile[currentSize + 4] = chunk->type[0];
  oldByteFile[currentSize + 5] = chunk->type[1];
  oldByteFile[currentSize + 6] = chunk->type[2];
  oldByteFile[currentSize + 7] = chunk->type[3];

  if(chunk->len > 0) {
    for (size_t i = 0; i < chunk->len; i++) {
      oldByteFile[currentSize + 8 + i] = chunk->data[i];
    }
  }
  
  uint32_t crcVal = 0;
  crc32(chunk->type, strlen(chunk->type), &crcVal);
  if(chunk->len > 0) {
    crc32(chunk->data, chunk->len, &crcVal);
  }
  
  oldByteFile[currentSize + 8 + dataSize] = crcVal >> 24;
  oldByteFile[currentSize + 8 + dataSize + 1] = crcVal >> 16;
  oldByteFile[currentSize + 8 + dataSize + 2] = crcVal >> 8;
  oldByteFile[currentSize + 8 + dataSize + 3] = crcVal;
   
  FILE *fp = fopen(png->fileName, "w");
  fwrite(oldByteFile, 1, currentSize + 4 + 4 + dataSize + 4, fp);
  fclose(fp);

  free(oldByteFile);

  return 8 + dataSize + 4;
}

/**
 * Frees all memory allocated by this library related to `chunk`.
 */
void PNG_free_chunk(PNG_Chunk *chunk) {
  if(chunk->len > 0) {
    free(chunk->data);
  }
  
  
  return;

}

/**
 * Closes the PNG file and frees all memory related to `png`.
 */
void PNG_close(PNG *png) {
  free(png->fileName);
  free(png->byteFile);
  free(png);

}
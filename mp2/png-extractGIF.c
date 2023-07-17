#include "png-extractGIF.h"

int png_extractGIF(const char *png_filename, const char *gif_filename) {
  // Open the file specified in argv[1] for reading:
  PNG *png = PNG_open(png_filename, "r");
  if (!png) { return ERROR_INVALID_FILE; }
  printf("PNG Header: OK\n");  

  int getGif = 0;
  // Read chunks until reaching "IEND" or an invalid chunk;
  int chunk_Counter = 0;
  while (1) {
    // Read chunk and ensure we get a valid result (exit on error):
    PNG_Chunk chunk;
    if (PNG_read(png, &chunk) == 0) {
      PNG_close(png);
      printf("%s", "in valid data\n");
      //return ERROR_INVALID_CHUNK_DATA;
      printf("%d",422);
      return 422;
    }
    chunk_Counter = chunk_Counter + 1;

    // Report data about the chunk to the command line:
    printf("Chunk: %s (%d bytes of data)\n", chunk.type, chunk.len);

    // Check for the "IEND" chunk to exit:
    if ( strcmp(chunk.type, "IEND") == 0 ) {
      PNG_free_chunk(&chunk);
      break;  
    }

    if (strcmp(chunk.type, "uiuc") == 0) {
      getGif = 1;
      FILE *fp = fopen(gif_filename, "wb");
      fwrite(chunk.data, 1, chunk.len, fp);
      fclose(fp);
    }
    // Free the memory associated with the chunk we just read:
    PNG_free_chunk(&chunk);
  }

  PNG_close(png);

  if (getGif != 1) {
    printf("%d",415);
    return 415;
  }
  printf("%d",0);
  return 0;  // Change the to a zero to indicate success, when your implementaiton is complete.
}

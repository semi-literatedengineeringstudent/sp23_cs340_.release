#include "png-hideGIF.h" 


int png_hideGIF(const char *png_filename_source, const char *gif_filename, const char *png_filename_out) {
    // Open the file specified in argv[1] for reading and argv[2] for writing:
  PNG *png_source = PNG_open(png_filename_source, "r");
  if (!png_source) { return ERROR_INVALID_FILE; }

  PNG *png_out = PNG_open(png_filename_out, "w");
  printf("PNG Header written.\n");
  size_t bytesWritten;
  int written = 0;

  // Read chunks until reaching "IEND" or in invalid chunk:
  while (1) {
    // Read chunk and ensure we get a valid result (exit on error):
    PNG_Chunk chunk;
    if (PNG_read(png_source, &chunk) == 0) {
      PNG_close(png_source);
      PNG_close(png_out);
      return ERROR_INVALID_CHUNK_DATA;
    }

    // Report data about the chunk to the command line:
    bytesWritten = PNG_write(png_out, &chunk);
    printf("PNG chunk %s written (%lu bytes)\n", chunk.type, bytesWritten);

    // Check for the "IEND" chunk to exit:
    if ( strcmp(chunk.type, "IEND") == 0 ) {
      
      PNG_free_chunk(&chunk);
      break;  
    }
    

    if (strcmp(chunk.type, "IHDR") == 0 && written == 0) {
      written = 1;
     
      FILE *gifFile = fopen(gif_filename, "r");
      if (gifFile  == NULL) {
        fclose(gifFile);
      } else {
        PNG_Chunk gif_chunk;
        fseek(gifFile, 0, SEEK_END);
        size_t fileLength = ftell(gifFile);
      
        fseek(gifFile, 0, SEEK_SET);
        unsigned char*wtf = (unsigned char*)calloc((fileLength + 1), sizeof(char));
        
        fread(wtf, sizeof(char), fileLength, gifFile);
        wtf[fileLength] = '\0';
    
        fclose(gifFile);
        gif_chunk.len = (uint32_t)fileLength;
        gif_chunk.data = wtf;
        gif_chunk.type[0] = 'u';
        gif_chunk.type[1] = 'i';
        gif_chunk.type[2] = 'u';
        gif_chunk.type[3] = 'c';
        gif_chunk.type[4] = 0x00;
        bytesWritten = PNG_write(png_out, &gif_chunk);
        printf("PNG chunk %s written (%lu bytes)\n", gif_chunk.type, bytesWritten);
        PNG_free_chunk(&gif_chunk);
      }

    }

    // Free the memory associated with the chunk we just read:
    PNG_free_chunk(&chunk);
  }

  PNG_close(png_source);
  PNG_close(png_out);
  if (written == 1) {
    return 0;
  }
  return 255;  // Change the to a zero to indicate success, when your implementaiton is complete.
}

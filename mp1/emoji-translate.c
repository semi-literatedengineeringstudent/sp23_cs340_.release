#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "emoji.h"
#include "emoji-translate.h"


void emoji_init(emoji_t *emoji) {
 
  emoji->emoji_array = malloc(sizeof(emoji_t) * 20);

  emoji->size = 0;
  return;
}

void emoji_add_translation(emoji_t *emoji, const unsigned char *source, const unsigned char *translation) {

  emoji->emoji_array[emoji->size].source = (char*) calloc((strlen(source) + 1),sizeof(char));
  emoji->emoji_array[emoji->size].translation = (char*) calloc((strlen(translation) + 1),sizeof(char));
  strcpy(emoji->emoji_array[emoji->size].source, source);
  strcpy(emoji->emoji_array[emoji->size].source + strlen(source), "\0");
  strcpy(emoji->emoji_array[emoji->size].translation, translation);
  strcpy(emoji->emoji_array[emoji->size].translation + strlen(translation), "\0");
  emoji->size = emoji->size + 1;
  
  return;
}

char *find_translation(emoji_t * emoji, char *toInspect) {
 
  for (size_t i = 0; i < emoji->size; i++) {
    
    if (strcmp(emoji->emoji_array[i].source, toInspect) == 0) {
      
      return emoji->emoji_array[i].translation;
    }
  }

  return "no translation available";
 }


// Translates the emojis contained in the file `fileName`.
const unsigned char *emoji_translate_file_alloc(emoji_t *emoji, const char *fileName) {
  
  FILE *file;

  file = fopen(fileName, "r");
  if (file == NULL) {
    return NULL;
  }
  fseek(file, 0, SEEK_END);//move file pointer to end of file.
  size_t fileLength = ftell(file);//get length by seeing index of pointer at end of file.
  fseek(file, 0, SEEK_SET);
  char *originalFile = malloc((fileLength + 1) * sizeof(char));
  char currentChar;
  size_t counter = 0;
  while ((currentChar = fgetc(file)) != EOF) {
    originalFile[counter] = currentChar;
    
    counter++;
  }
  originalFile[counter] = '\0';
  fclose(file);

  size_t index = 0;
  size_t length = strlen(originalFile);
  
  char* toReturn = (char *)calloc((length + 1)*10 , sizeof(char));

  size_t toReturnLength = 0;

  while (length - index >= 4) {
    size_t emoji_count = 0;
    size_t valid_emoji_count = 0;
    char* toAdd = (char*) malloc(100*sizeof(char));;

    while(1) {
      if (index + (emoji_count + 1) * 4 > length) {
        break;
      }
      char *toInspect = (char*) malloc(5*sizeof(char));
      
      strncpy(toInspect, originalFile + index + emoji_count * 4, 4);
      strcpy(toInspect + 4, "\0");
     
      char *emoji_source = (char*) malloc((4 * (emoji_count + 1) + 1)* sizeof(char));
      strncpy(emoji_source, originalFile + index, 4 * (emoji_count + 1));
      strcpy(emoji_source + 4 * (emoji_count + 1), "\0");
  
      if (!is_Emoji(toInspect)) {
     
        free(toInspect);
        free(emoji_source);
        
        break;
        
      } else {
      
        char * translation = (char*) malloc(100*sizeof(char));
        strcpy(translation, find_translation(emoji, emoji_source));

        
        if (strcmp(translation, "no translation available") != 0) { 
          strcpy(toAdd, translation);
         
          emoji_count = emoji_count + 1;
          valid_emoji_count = emoji_count;
        } else {
          emoji_count = emoji_count + 1;
        }
        
        free(toInspect);
        free(emoji_source);
        free(translation);
        
      }
    
    }

    if (valid_emoji_count > 0) {
      strncpy(toReturn + toReturnLength, toAdd, strlen(toAdd));
     
      index = index + valid_emoji_count * 4;
      toReturnLength = toReturnLength + strlen(toAdd);
     
    } else {
      strncpy(toReturn + toReturnLength, originalFile + index, 1);
     
      index = index + 1;
      toReturnLength = toReturnLength + 1;
    
    }

    free(toAdd);
  }
  for (size_t i = index; i < length; i++) {
    strncpy(toReturn + toReturnLength, originalFile + i, 1);
    toReturnLength = toReturnLength + 1;
  }
  
  free(originalFile);

  return (const unsigned char*)toReturn;
}

void emoji_destroy(emoji_t *emoji) {
 
  if (emoji->size > 0) {
    for(size_t i = 0; i < emoji->size; i++) {
      free(emoji->emoji_array[i].source);
      free(emoji->emoji_array[i].translation);
    }
  }
  free(emoji->emoji_array);
  return;
}

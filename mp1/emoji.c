#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// Return your favorite emoji.  Do not allocate new memory.
// (This should **really** be your favorite emoji, we plan to use this later in the semester. :))
char *emoji_favorite() {
  char *favoritr_emoji = "\xF0\x9F\x89\x91";
  return favoritr_emoji;
}


int is_Emoji(const char *s) {
  unsigned int val = 0;
 
  for(int i=0; i<4; i++) {
    val = (val << 8) | ((unsigned int)(s[i]) & 0xFF);
  }
 
  return (
    (val >= 14844092 /* U+203C */ && val <= 14912153 /* U+3299 */) ||
    (val >= 4036984960 /* U+1F000 */ && val <= 4036996031 /* U+1FAFF */ )
  );
}

// Count the number of emoji in the UTF-8 string `utf8str`, returning the count.  You should
// consider everything in the ranges starting from (and including) U+1F000 up to (and including) U+1FAFF.
int emoji_count(const unsigned char *utf8str) {
  size_t length = strlen((char*)utf8str);
  int count = 0;
  for (size_t i = 0; i < length - 3; i++){
    char *toInspect = (char*) calloc(4,sizeof(char));
    strncpy(toInspect, (char*)utf8str + i, 4);
    if(is_Emoji(toInspect)){
      count = count + 1;
    }
    free(toInspect);
  }

  return count;
  
}


// Return a random emoji stored in new heap memory you have allocated.  Make sure what
// you return is a valid C-string that contains only one random emoji.
char *emoji_random_alloc() {
  char *toReturn = (char *) calloc(5,sizeof(char));
  toReturn[0] = (char)(0xF0);
  toReturn[1] = (char)(0x9F);
  toReturn[2] = (char)(128 + rand() % 44);
  toReturn[3] = (char)(128 + rand() % 64);
  toReturn[4] = (char)(0x00);
  while(!is_Emoji(toReturn)) {
  
    toReturn[0] = (char)(0xF0);
    toReturn[1] = (char)(0x9F);
    toReturn[2] = (char)(128 + rand() % 44);
    toReturn[3] = (char)(128 + rand() % 64);
    toReturn[4] = (char)(0x00);
    

  }

 
  return toReturn;
}


int is_Smiling(unsigned char *utf8str) {
  return (strcmp((char*)utf8str, "\xF0\x9F\x98\x80") == 1 ||
      strcmp((char*)utf8str, "\xF0\x9F\x98\x81") == 1 ||
      strcmp((char*)utf8str, "\xF0\x9F\x98\x82") == 1 ||
      strcmp((char*)utf8str, "\xF0\x9F\x98\x83") == 1 ||
      strcmp((char*)utf8str, "\xF0\x9F\x98\x84") == 1 ||
      strcmp((char*)utf8str, "\xF0\x9F\x98\x85") == 1 ||
      strcmp((char*)utf8str, "\xF0\x9F\x98\x86") == 1 ||
      strcmp((char*)utf8str, "\xF0\x9F\x98\x87") == 1 ||
      strcmp((char*)utf8str, "\xF0\x9F\x98\x88") == 1 ||
      strcmp((char*)utf8str, "\xF0\x9F\x98\x89") == 1 ||
      strcmp((char*)utf8str, "\xF0\x9F\x98\x8A") == 1 ||
      strcmp((char*)utf8str, "\xF0\x9F\x98\x8B") == 1 ||
      strcmp((char*)utf8str, "\xF0\x9F\x98\x8C") == 1 ||
      strcmp((char*)utf8str, "\xF0\x9F\x98\x8D") == 1 ||
      strcmp((char*)utf8str, "\xF0\x9F\x98\x8E") == 1 ||
      strcmp((char*)utf8str, "\xF0\x9F\x98\x8F") == 1 );
}


// Modify the UTF-8 string `utf8str` to invert the FIRST character (which may be up to 4 bytes)
// in the string if it the first character is an emoji.  At a minimum:
// - Invert "😊" U+1F60A ("\xF0\x9F\x98\x8A") into ANY non-smiling face.
// - Choose at least five more emoji to invert.
void emoji_invertChar(unsigned char *utf8str) {
  size_t length = strlen((char*)utf8str);
  if (length < 4) {
    return;
  }
  if (!is_Emoji((char*)utf8str)) {
    return;
  }
  if (is_Smiling(utf8str) == 1) {
    char* candidate = emoji_random_alloc();
    strcpy((char*)utf8str, candidate);
    free(candidate);
    while(is_Smiling(utf8str)) {
      char* candidate = emoji_random_alloc();
      strcpy((char*)utf8str, candidate);
      free(candidate);
    }

  }
  return;

}


// Modify the UTF-8 string `utf8str` to invert ALL of the character by calling your
// `emoji_invertChar` function on each character.
void emoji_invertAll(unsigned char *utf8str) {

  size_t length = strlen((char*)utf8str);
  
  for (size_t i = 0; i < length - 3; i++){
    char *toInspect = (char*) calloc(5,sizeof(char));
    strncpy((char*)toInspect, (char*)utf8str + i, 4);

    if(is_Emoji(toInspect)){
      
      if (is_Smiling((unsigned char *)toInspect)) {
        emoji_invertChar((unsigned char*)toInspect);
      }
      strncpy((char*)utf8str + i, toInspect, 4);
    }
    free(toInspect);
  }
  return;
}


// Reads the full contents of the file `fileName, inverts all emojis, and
// returns a newly allocated string with the inverted file's content.
unsigned char *emoji_invertFile_alloc(const char *fileName) {
  FILE *file;
  file = fopen(fileName, "r");
  if (file == NULL) {
    return NULL;
  }
  fseek(file, 0, SEEK_END);//move file pointer to end of file.
  int length = ftell(file);//get length by seeing index of pointer at end of file.
  fseek(file, 0, SEEK_SET);
  unsigned char *toReturn = (char*)calloc((length + 1), sizeof(char));
  char currentChar;
  int counter = 0;
  while ((currentChar = fgetc(file)) != EOF) {
    toReturn[counter] = currentChar;
    counter++;
  }
  fclose(file);

  emoji_invertAll(toReturn);
  return toReturn;
}

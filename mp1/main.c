#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "emoji.h"
#include "emoji-translate.h"

int main() {
  // Feel free to delete all of this and use this function in any way you want:
  printf("Currently, `main.c` provides a basic call to your functions.\n");
  printf("- Edit `main.c` to test your functions within the context of `main`.\n");
  printf("- A test suite is provided in `tests` folder.\n");

  printf("Your favorite emoji: %s\n", emoji_favorite()); 

 
  emoji_t emoji;
  emoji_init(&emoji);

  

  emoji_add_translation(&emoji, (const unsigned char *)"🙅", (const unsigned char *)"no, ");
  emoji_add_translation(&emoji, (const unsigned char *)"🖐", (const unsigned char *)"but ");
  emoji_add_translation(&emoji, (const unsigned char *) "🙅👄💬🧑🔮", (const unsigned char *)"We don't talk about Bruno, ");
  emoji_add_translation(&emoji, (const unsigned char *)"💒👰💍🤵", (const unsigned char *)"It was my wedding day ");
  emoji_add_translation(&emoji, (const unsigned char *)"💄💅💇🙅🌤🌄", (const unsigned char *)"We were getting ready, and there wasn't a cloud in the sky ");
  emoji_add_translation(&emoji, (const unsigned char *)"🧑🔮🚶😈", (const unsigned char *)"Bruno walks in with a mischievous grin ");
  emoji_add_translation(&emoji, (const unsigned char *)"😠🗯🗨💢🤫", (const unsigned char *)"You telling this story or am I?");

  unsigned char *translation = (unsigned char *) emoji_translate_file_alloc(&emoji, "tests/txt/long.txt");
  printf("%s", translation);
  free(translation);

  emoji_destroy(&emoji);
  
  return 0;
}

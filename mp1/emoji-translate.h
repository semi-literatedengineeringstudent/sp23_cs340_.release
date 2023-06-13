#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/*typedef struct _emoji_indi {
    char *source;
    char *translation;
} emoji_indi;

typedef struct _emoji_t {
    emoji_indi *emoji_array;
    size_t size;
} emoji_t;*/


typedef struct _emoji_indi {
    char *source;
    char *translation;
    struct _emoji_indi *next;
} emoji_indi;

typedef struct _emoji_t {
    struct _emoji_indi *head;
} emoji_t;







void emoji_init(emoji_t *emoji);
void emoji_add_translation(emoji_t *emoji, const unsigned char *source, const unsigned char *translation);
char *find_translation(emoji_t * emoji, char *toInspect);
//oid find_translation(emoji_t * emoji, char *toInspect, char* toFill);
const unsigned char *emoji_translate_file_alloc(emoji_t *emoji, const char *fileName);
void emoji_destroy(emoji_t *emoji);

#ifdef __cplusplus
}
#endif
